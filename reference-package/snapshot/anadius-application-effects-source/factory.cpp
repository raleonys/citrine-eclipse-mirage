#include "factory.hpp"
#include <mutex>
#include <stdexcept>

namespace anadius::application_effects {
namespace {struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};}
struct Factory::Impl {
    const DWORD controller=::GetCurrentThreadId();void* const controller_fiber=::GetCurrentFiber();
    Options options;std::mutex gate;bool used=false,busy=false,poisoned=false,retiring=false;
    DWORD worker=0;void* fiber=nullptr;
    explicit Impl(Options o):options(std::move(o)){}
    void check(const Context& c){
        Error error;
        {std::lock_guard lock(gate);if(retiring||poisoned||!busy||worker!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Effect composition admission unavailable");}
        c.checkpoint.check(c.checkpoint.context);
        {std::lock_guard lock(gate);if(retiring||poisoned)throw std::logic_error("Effect composition lost admission");}
    }
};
Factory::Factory(Options o):impl_(nullptr){impl_=std::make_unique<Impl>(std::move(o));}
Factory::~Factory()noexcept{
    if(!impl_)return;auto& v=*impl_;
    {std::lock_guard lock(v.gate);if(v.controller!=::GetCurrentThreadId()||v.controller_fiber!=::GetCurrentFiber()||v.busy)std::terminate();v.retiring=true;}
    // Opaque leaf owners may borrow the explicit attempt cells. Destroy the
    // provider settings first while both the cells and sealed facade survive.
    {licensing_effects::Options retiring=std::move(v.options.licensing);}
    {content_effects::Options retiring=std::move(v.options.content);}
    {information_effects::Options retiring=std::move(v.options.information);}
    v.options.attempts.reset();
}
void Factory::compose(Providers& out,const Context& c){
    if(!impl_)throw std::logic_error("Effect factory unattached");auto& v=*impl_;
    {std::lock_guard lock(v.gate);if(v.retiring||v.used){if(v.busy)v.poisoned=true;throw std::logic_error("Effect factory composes once");}v.used=true;v.busy=true;v.worker=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();}
    struct Call {Impl& v;~Call(){std::lock_guard lock(v.gate);v.busy=false;}} call{v};
    try{
        if(!c.checkpoint.context||!c.checkpoint.check)throw std::invalid_argument("Worker composition checkpoint required");
        v.check(c);
        if(out.information||out.content||out.licensing)throw std::invalid_argument("Effect composition requires empty owned slots");
        auto* online=dynamic_cast<online_factory::Online*>(&c.online);
        if(!online||!online->matches(c.state))throw std::invalid_argument("Concrete effects require exact actual Online component");
        const auto& o=v.options;
        if(!o.attempts||o.information.stream.get()!=&c.stream||o.content.stream.get()!=&c.stream||o.licensing.stream.get()!=&c.stream||!o.content.dialog||o.content.dialog!=o.licensing.dialog)
            throw std::invalid_argument("Effects require explicit attempt roots and one shared Graph stream/dialog");
        out.information=std::make_shared<information_effects::Information>(o.information);v.check(c);
        out.content=std::make_shared<content_effects::Content>(c.state,c.paths,*online,o.content);v.check(c);
        out.licensing=std::make_shared<licensing_effects::Handlers>(c.state,*online,o.attempts,o.licensing);v.check(c);
    }catch(...){std::lock_guard lock(v.gate);v.poisoned=true;throw;}
}
}
