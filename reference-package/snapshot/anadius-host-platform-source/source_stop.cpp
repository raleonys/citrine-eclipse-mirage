#include "source_stop.hpp"
#include <cstdlib>
#include <stdexcept>

namespace anadius::host_platform {
namespace {
struct Error {
    DWORD value=::GetLastError();
    ~Error(){::SetLastError(value);}
    void restore()const{::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
}
SourceStopReceipt::SourceStopReceipt(std::shared_ptr<foreign_callback::Domain> callbacks):callbacks_(std::move(callbacks)){}
bool SourceStopReceipt::completed() const noexcept{return complete_.load(std::memory_order_acquire);}
const foreign_callback::Domain* SourceStopReceipt::callbacks_identity() const noexcept{return callbacks_.get();}
const thread_domain::DomainStorage* SourceStopReceipt::memory_domain() const noexcept{return callbacks_->memory_domain();}
struct SourceStopState {
    // Source capture is destroyed before the callback admission owner.
    std::shared_ptr<SourceStopReceipt> receipt;
    std::function<bool()> source;
    const DWORD controller=::GetCurrentThreadId();
    bool busy=false,poisoned=false,source_stopped=false;
    explicit SourceStopState(std::shared_ptr<foreign_callback::Domain> c):receipt(new SourceStopReceipt(std::move(c))){}
    ~SourceStopState()noexcept{if(busy||poisoned)std::abort();}
    bool run(){
        Error error;
        if(::GetCurrentThreadId()!=controller)throw std::logic_error("Source stop controller required");
        if(busy){poisoned=true;throw std::logic_error("Source stop reentry");}
        if(poisoned)throw std::logic_error("Source stop outcome is unknown");
        if(receipt->completed())return true;
        busy=true;
        struct Active {bool& busy;~Active(){busy=false;}} active{busy};
        try{
            // False still means new admissions are sealed. The existing
            // stopper must run even with an active callback: it may release it.
            (void)receipt->callbacks_->pause();
            if(!source_stopped){
                error.restore();bool stopped;
                try{stopped=source();}catch(...){error.capture();throw;}
                error.capture();
                if(poisoned)throw std::logic_error("Source stopper swallowed admission failure");
                if(!stopped)return false;
                source_stopped=true;
            }
            // Recheck after opaque code and on every active-drain retry.
            if(!receipt->callbacks_->pause())return false;
            receipt->complete_.store(true,std::memory_order_release);return true;
        }catch(...){poisoned=true;throw;}
    }
};
BoundSourceStop bind_source_stop_with_receipt(std::shared_ptr<foreign_callback::Domain> callbacks,std::function<bool()>& source){
    if(!callbacks||!source)throw std::invalid_argument("Owned callback domain and explicit source stopper required");
    auto owner=std::make_shared<SourceStopState>(std::move(callbacks));
    std::function<bool()> result=[owner]{return owner->run();};
    // No allocations or opaque callable copies after consuming the donor.
    owner->source.swap(source);
    return {std::move(result),owner->receipt};
}
std::function<bool()> bind_source_stop(std::shared_ptr<foreign_callback::Domain> callbacks,std::function<bool()>& source){
    return bind_source_stop_with_receipt(std::move(callbacks),source).stop;
}
}
