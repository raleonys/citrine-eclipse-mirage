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
struct Stop {
    // Source capture is destroyed before the callback admission owner.
    std::shared_ptr<foreign_callback::Domain> callbacks;
    std::function<bool()> source;
    const DWORD controller=::GetCurrentThreadId();
    bool busy=false,poisoned=false,source_stopped=false,complete=false;
    Stop(std::shared_ptr<foreign_callback::Domain> c,std::function<bool()>& s):callbacks(std::move(c)),source(std::move(s)){}
    ~Stop()noexcept{if(busy||poisoned)std::abort();}
    bool run(){
        Error error;
        if(::GetCurrentThreadId()!=controller)throw std::logic_error("Source stop controller required");
        if(busy){poisoned=true;throw std::logic_error("Source stop reentry");}
        if(poisoned)throw std::logic_error("Source stop outcome is unknown");
        if(complete)return true;
        busy=true;
        struct Active {bool& busy;~Active(){busy=false;}} active{busy};
        try{
            // False still means new admissions are sealed. The existing
            // stopper must run even with an active callback: it may release it.
            (void)callbacks->pause();
            if(!source_stopped){
                error.restore();bool stopped;
                try{stopped=source();}catch(...){error.capture();throw;}
                error.capture();
                if(poisoned)throw std::logic_error("Source stopper swallowed admission failure");
                if(!stopped)return false;
                source_stopped=true;
            }
            // Recheck after opaque code and on every active-drain retry.
            if(!callbacks->pause())return false;
            complete=true;return true;
        }catch(...){poisoned=true;throw;}
    }
};
}
std::function<bool()> bind_source_stop(std::shared_ptr<foreign_callback::Domain> callbacks,std::function<bool()>& source){
    if(!callbacks||!source)throw std::invalid_argument("Owned callback domain and explicit source stopper required");
    auto owner=std::make_shared<Stop>(std::move(callbacks),source);
    std::function<bool()> result=[owner]{return owner->run();};
    source={}; // std::function move can leave an opaque SBO donor in this CRT
    return result;
}
}
