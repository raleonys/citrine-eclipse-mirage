#include "stream.hpp"
#include <stdexcept>
#include <type_traits>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace anadius::shared_log {
namespace {
thread_local Stream* active=nullptr;
struct PreserveError {
#ifdef _WIN32
    DWORD value=::GetLastError();
    ~PreserveError(){::SetLastError(value);}
#else
    ~PreserveError(){}
#endif
};
}
detail::BufferOwner::BufferOwner(std::shared_ptr<std::streambuf> value):buffer(std::move(value)){
    if(!buffer)throw std::invalid_argument("Explicit owned streambuf required");
}
Stream::Stream(std::shared_ptr<std::streambuf> value):BufferOwner(std::move(value)),std::ostream(buffer.get()){}
std::shared_ptr<Stream> make_stream(std::shared_ptr<std::streambuf> buffer){return std::make_shared<Stream>(std::move(buffer));}
Lease::Lease(std::ostream& value){
    PreserveError error;
    auto* selected=dynamic_cast<Stream*>(&value);
    if(active)throw std::logic_error("Adapter reentry inside managed ostream operation rejected");
    if(!selected)return;
    stream_=selected->weak_from_this().lock();
    if(!stream_)throw std::invalid_argument("Managed ostream must have its shared factory owner");
    active=selected;bool locked=false;
    try{
        selected->gate_.lock();locked=true;
        if(selected->tie())throw std::logic_error("Managed ostream cannot flush a tied stream outside its gate");
        if(selected->rdbuf()!=selected->buffer.get())throw std::logic_error("Managed ostream cannot replace its owned buffer");
        owner_=std::this_thread::get_id();
    }catch(...){
        if(locked)selected->gate_.unlock();active=nullptr;stream_.reset();throw;
    }
}
Lease::~Lease() noexcept{
    PreserveError error;
    if(!stream_)return;
    if(owner_!=std::this_thread::get_id()||active!=stream_.get())std::terminate();
    active=nullptr;stream_->gate_.unlock();
    // Last owner destruction is also bookkeeping; restore body LastError after it.
    stream_.reset();
}
Lease acquire(std::ostream& stream){return Lease(stream);}
static_assert(!std::is_abstract_v<Stream>&&!std::is_move_constructible_v<Stream>);
static_assert(!std::is_abstract_v<Lease>&&!std::is_move_constructible_v<Lease>);
} // namespace anadius::shared_log
