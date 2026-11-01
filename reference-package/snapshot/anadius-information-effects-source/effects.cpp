#include "effects.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <windows.h>
#include <stdexcept>

namespace anadius::information_effects {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
Options checked(Options o){if(!o.stream||!o.clock||!o.formatter)throw std::invalid_argument("Owned Information stream/clock/formatter required");return o;}
}
struct Information::Impl {
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    Options options;bool busy=false,poisoned=false,retiring=false;content_handlers_recovered::CallCheckpoint checkpoint{};
    explicit Impl(Options o):options(checked(std::move(o))){}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Information requires original Graph worker/fiber");}
    void check()const{Error error;affinity();if(retiring||poisoned)throw std::logic_error("Information admission closed or unknown");if(checkpoint.check)checkpoint.check(checkpoint.context);}
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    struct Call {Impl& v;explicit Call(Impl& v):v(v){v.affinity();if(v.busy){v.poisoned=true;throw std::logic_error("Information operation is nonreentrant");}v.check();v.busy=true;}~Call(){v.busy=false;}};
    struct Clock final:achievement_runtime::Clock {Impl& v;explicit Clock(Impl& v):v(v){}
        std::int64_t time_05eeb8()override{return v.leaf([&]{return v.options.clock->time_05eeb8();});}
        std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{return v.leaf([&]{return v.options.clock->gmtime_05fffc(n);});}
    } clock{*this};
    struct Formatter final:achievement_runtime::Formatter {Impl& v;explicit Formatter(Impl& v):v(v){}
        std::string put_time_03d610(const achievement_runtime::BrokenDown& b,std::string_view p)override{return v.leaf([&]{return v.options.formatter->put_time_03d610(b,p);});}
    } formatter{*this};
    void newline(){const auto c=leaf([&]{return options.stream->widen('\n');});leaf([&]{options.stream->put(c);});leaf([&]{options.stream->flush();});}
};
Information::Information(Options o):impl_(nullptr){impl_=std::make_unique<Impl>(std::move(o));}
Information::Impl& Information::attached()const{if(!impl_)throw std::logic_error("Information is unattached");return *impl_;}
Information::~Information()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy||v.checkpoint.check)std::terminate();v.retiring=true;{Options retired=std::move(v.options);}}
template<class F>auto Information::invoke(F&& f)->std::invoke_result_t<F>{auto& v=attached();Impl::Call call(v);try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}else{std::invoke_result_t<F> result=f();v.check();return result;}}catch(...){v.poisoned=true;throw;}}
void Information::log_profile_name_084f48(std::string_view name){invoke([&]{auto& v=*impl_;const auto lease=shared_log::acquire(*v.options.stream);v.leaf([&]{*v.options.stream<<"profile name: ";});v.leaf([&]{*v.options.stream<<name;});v.newline();});}
std::string Information::current_time_041b60(){return invoke([&]{return achievement_runtime::utc_time_041b60(impl_->clock,impl_->formatter,nullptr,0);});}
void Information::bind_content_checkpoint(content_handlers_recovered::CallCheckpoint c){auto& v=attached();v.affinity();if(v.busy||v.checkpoint.check){v.poisoned=true;throw std::logic_error("Information checkpoint overlaps active call");}v.check();if(!c.context||!c.check)throw std::invalid_argument("Complete Information checkpoint required");{Error error;c.check(c.context);}v.checkpoint=c;}
void Information::clear_content_checkpoint()noexcept{if(!impl_)return;auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.busy)std::terminate();v.checkpoint={};}
static_assert(!std::is_abstract_v<Information>);
}
