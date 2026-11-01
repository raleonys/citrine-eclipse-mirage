#include "bound.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <mutex>
#include <stdexcept>

namespace anadius::library_effects {
namespace {struct Error{DWORD value=::GetLastError();~Error(){::SetLastError(value);}};}
struct BoundLibrary::Impl {
    const DWORD controller=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    BoundOptions options;mutable std::mutex gate;std::size_t active=0;bool retiring=false,poisoned=false;
    execution_recovered::Execution* execution=nullptr;std::uint8_t* ready=nullptr;
    explicit Impl(BoundOptions o):options(std::move(o)){if(!options.stream||!options.dialog||!options.errors)throw std::invalid_argument("Complete owned library stream/dialog/errors required");}
    void check()const{Error error;std::lock_guard lock(gate);if(retiring||poisoned)throw std::logic_error("Library provider admission unavailable");}
    template<class F>auto leaf(F&& f)->decltype(f()){check();if constexpr(std::is_void_v<decltype(f())>){f();check();}else{auto result=f();check();return result;}}
    struct Call {
        Impl& v;explicit Call(Impl& x):v(x){Error error;std::lock_guard lock(v.gate);if(v.retiring||v.poisoned)throw std::logic_error("Library provider admission unavailable");++v.active;}
        ~Call(){Error error;std::lock_guard lock(v.gate);--v.active;}
    };
};
BoundLibrary::BoundLibrary(BoundOptions o):impl_(nullptr){impl_=std::make_unique<Impl>(std::move(o));}
BoundLibrary::Impl& BoundLibrary::attached()const{if(!impl_)throw std::logic_error("Library provider unattached");return *impl_;}
BoundLibrary::~BoundLibrary()noexcept{
    if(!impl_)return;auto& v=*impl_;
    {Error error;std::lock_guard lock(v.gate);if(v.controller!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.active)std::terminate();v.retiring=true;}
    {BoundOptions retiring=std::move(v.options);}
}
void BoundLibrary::bind_library_phase(execution_recovered::Execution& e,std::uint8_t& flag){
    auto& v=attached();Error error;std::lock_guard lock(v.gate);
    if(v.controller!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.retiring||v.poisoned||v.active||v.execution)
        throw std::logic_error("Library phase binds once on idle original controller");
    v.execution=&e;v.ready=&flag;
}
template<class F>void BoundLibrary::invoke(F&& f){auto& v=attached();Impl::Call call(v);try{f(v);v.check();}catch(...){Error error;std::lock_guard lock(v.gate);v.poisoned=true;throw;}}
void BoundLibrary::begin_load_log_0863e0(){invoke([](Impl& v){const auto lease=shared_log::acquire(*v.options.stream);v.leaf([&]{*v.options.stream<<"Loading ";});});}
void BoundLibrary::append_log_path_and_flush(std::string_view s){invoke([&](Impl& v){const auto lease=shared_log::acquire(*v.options.stream);v.leaf([&]{*v.options.stream<<s;});const auto c=v.leaf([&]{return v.options.stream->widen('\n');});v.leaf([&]{v.options.stream->put(c);});v.leaf([&]{v.options.stream->flush();});});}
void BoundLibrary::message_038bc0(std::string title,std::string text,std::uint32_t flags){invoke([&](Impl& v){v.leaf([&]{v.options.dialog->show(title,text,flags);});});}
[[noreturn]]void BoundLibrary::throw_runtime_00cd70(int code){invoke([&](Impl& v){v.leaf([&]{v.options.errors->runtime_00cd70(code);});});std::terminate();}
[[noreturn]]void BoundLibrary::throw_filesystem_00cdb0(std::uint32_t code){invoke([&](Impl& v){v.leaf([&]{v.options.errors->filesystem_00cdb0(code);});});std::terminate();}
void BoundLibrary::wait_steps_036880(int count){invoke([&](Impl& v){if(!v.execution)throw std::logic_error("Library phase not bound");v.leaf([&]{v.execution->wait_steps_036880(count);});});}
void BoundLibrary::finish_flag_036820(){invoke([](Impl& v){if(!v.execution||!v.ready)throw std::logic_error("Library phase not bound");v.leaf([&]{v.execution->finish_flag_036820(*v.ready);});});}
}
