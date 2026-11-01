#include "policy.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <exception>
#include <limits>
#include <mutex>
#include <optional>
#include <system_error>
#include <type_traits>

namespace anadius::session_policy {
namespace {
struct PreserveError {DWORD saved=::GetLastError();~PreserveError(){::SetLastError(saved);}};
bool same(s::Category a,s::Category b){return a.name_identity==b.name_identity&&a.logical==b.logical;}
bool zero(const std::array<std::uintptr_t,2>& p){return p==std::array<std::uintptr_t,2>{};}
class MiscErrorCategory final:public std::error_category {public:const char* name()const noexcept override{return "asio.misc";}std::string message(int n)const override{return "asio.misc error "+std::to_string(n);}};
const std::error_category& compiler_category(unsigned i){static const MiscErrorCategory misc;return i==0?std::generic_category():i==1?std::system_category():misc;}
}
struct Policy::Impl {
    mutable std::mutex gate;Phase phase=Phase::Fresh;bool active=false,binding=false,closing=false;
    Options options;services_completion::ExitWaitRegistry exits{services_completion::InitialRegistry::empty_owned};
    std::array<std::byte,3> identities{};std::optional<rt::Ports> ports;std::shared_ptr<void> lease;std::function<void()> checkpoint;
    DWORD thread=::GetCurrentThreadId();void* fiber=::GetCurrentFiber();
    std::int32_t thread_epoch=std::numeric_limits<std::int32_t>::min();
    bool guard_pending=false,guard_registered=false,guard_initialized=false,attested=false,returned=false;sp::Application* application_identity=nullptr;
    std::array<std::uintptr_t,2>* worker_context=nullptr;std::optional<std::pair<s::ReadLengthError*,std::string>> length;
    std::uint64_t starts=0,finishes=0,cleanups=0,flushes=0;
    enum class LogStage {Empty,Prefix,Message,Widened,Put};LogStage log_stage=LogStage::Empty;
    s::WriteFinishStream stream;std::string loan_text;std::uintptr_t loan_address=0;bool loan_heap=false;
    std::optional<s::WriteFinishString> partial;std::uint8_t widened=0;
    s::Category category(unsigned i)const noexcept{return {reinterpret_cast<std::uintptr_t>(&identities[i]),i==2?server_recovered::ErrorCategory::asio_misc:server_recovered::ErrorCategory::system};}
    struct Catalog:s::CategoryProvider {Impl& v;explicit Catalog(Impl& v):v(v){}s::Category get_028620()override{return v.category(1);}} catalog{*this};
    s::NativeApi native;
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Session policy requires its original worker/fiber");}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed&&phase!=Phase::Retiring)phase=Phase::Poisoned;}
    void valid()const{PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||!active||binding)throw std::logic_error("Session policy admission was poisoned");}
    void source(bool cleanup=false){auto& p=*ports;(void)p.view.memory.buffer_context();const auto backend=p.view.backend.snapshot().phase;
        if((backend!=o::Phase::Ready&&backend!=o::Phase::Running&&!(cleanup&&(backend==o::Phase::Returned||backend==o::Phase::Quiescing)))||p.view.io.snapshot().phase!=s::IoMemoryPhase::Open||p.accept.snapshot().poisoned)throw std::logic_error("Session policy source graph is unavailable");}
    void admit(bool cleanup=false){valid();checkpoint();valid();source(cleanup);valid();}
    void enter(){PreserveError error;std::lock_guard lock(gate);if(active||binding||closing){if(phase!=Phase::Closed)phase=Phase::Poisoned;throw std::logic_error("Session policy calls are nonreentrant");}if(phase!=Phase::Bound)throw std::logic_error("Session policy is not bound/open");affinity();active=true;}
    void leave(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||!active)throw std::logic_error("Session policy completion was poisoned");active=false;}
    void failed(){poison();PreserveError error;std::lock_guard lock(gate);active=false;}
    template<class F>std::invoke_result_t<F> call(F&& f){enter();try{admit();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();admit();leave();}else{std::invoke_result_t<F> result=f();admit();leave();return result;}}catch(...){failed();throw;}}
    void terminal(){enter();try{source();valid();}catch(...){failed();throw;}poison();{PreserveError error;std::lock_guard lock(gate);active=false;}}
    void matching(const o::ServerView& p)const{const auto& own=ports->view;
        if(p.identity.value!=own.identity.value||&p.backend!=&own.backend||&p.memory!=&own.memory||&p.io!=&own.io||&p.scheduler!=&own.scheduler||&p.service!=&own.service||&p.acceptor!=&own.acceptor||&p.pending!=&own.pending)throw std::invalid_argument("Session policy belongs to another source graph");}
    void application(sp::Application& app){const auto p=app.ports();const auto& own=*ports;matching(p.view);const auto& a=p.environment.sentinels;const auto& b=own.environment.sentinels;
        if(p.server_identity!=own.server_identity||&p.accept!=&own.accept||&p.memory!=&own.memory||&p.worker!=&own.worker||&p.completion!=&own.completion||&p.environment.storage.tls_index!=&own.environment.storage.tls_index||p.environment.storage.storage.data()!=own.environment.storage.storage.data()||p.environment.storage.storage.size()!=own.environment.storage.storage.size()||a.empty_operations_846c8!=b.empty_operations_846c8||a.empty_schedule_846e8!=b.empty_schedule_846e8||a.empty_executor_84770!=b.empty_executor_84770||a.guard_destroy_017a30!=b.guard_destroy_017a30||(application_identity&&application_identity!=&app))throw std::invalid_argument("Session policy belongs to another application graph");application_identity=&app;}
    void entry(s::IocpSchedulerEntry& e){if(&e!=&ports->view.scheduler)throw std::invalid_argument("Session policy received a foreign scheduler");}
    unsigned index(s::Category c)const{for(unsigned i=0;i<3;++i)if(same(c,category(i)))return i;throw std::invalid_argument("Unknown session policy category identity");}
    std::string text(unsigned i,std::int32_t code){
        auto result=options.format_override?options.format_override(category(i),code):compiler_category(i).message(code);
        if(result.size()>options.maximum_message_bytes)throw std::length_error("Session category message exceeds its configured bound");return result;
    }
    void message(unsigned i,s::WriteFinishString& out,std::int32_t code){call([&]{
        if(log_stage!=LogStage::Prefix||loan_address||partial||out.size_10||out.capacity_18!=15)throw std::logic_error("Category message requires one fresh active log prefix");
        loan_text=text(i,code);admit();partial.emplace();auto& fresh=*partial;
        if(loan_text.size()<=15){fresh.size_10=loan_text.size();std::copy(loan_text.begin(),loan_text.end(),fresh.storage_00.begin());fresh.storage_00[loan_text.size()]=0;}
        else{auto& context=ports->view.memory.buffer_context();s::resize_receive_buffer_010040(fresh,loan_text.size(),0,context);admit();auto bytes=context.memory.view(s::receive_buffer_address(fresh),loan_text.size());std::copy(loan_text.begin(),loan_text.end(),bytes.begin());}
        admit();out=fresh;partial.reset();loan_address=s::receive_buffer_address(out);loan_heap=out.capacity_18>15;
    });}
    struct Category final:s::WriteFinishCategory {Impl& v;unsigned i;Category(Impl& v,unsigned i):v(v),i(i){identity_08=v.category(i).name_identity;}void message_slot_10(s::WriteFinishString& out,std::int32_t code)override{v.message(i,out,code);}} generic{*this,0},system{*this,1},misc{*this,2};
    struct Guard final:s::DispatcherEffects {
        Impl& v;explicit Guard(Impl& v):v(v){}
        std::uintptr_t capture_tls_slot()override{return v.call([&]{return reinterpret_cast<std::uintptr_t>(&v.thread_epoch);});}
        std::int32_t thread_epoch(std::uintptr_t p)override{return v.call([&]{if(p!=reinterpret_cast<std::uintptr_t>(&v.thread_epoch))throw std::invalid_argument("Foreign owned epoch token");return v.thread_epoch;});}
        void guard_header_04b838(std::int32_t& n)override{v.call([&]{if(&n!=v.options.guard_epoch||v.guard_initialized||v.guard_pending||n==-1)throw std::logic_error("Unsupported owned guard initialization");n=-1;v.guard_pending=true;});}
        void register_atexit_04b770(std::uintptr_t p)override{v.call([&]{if(p!=0x18007dff0ULL||!v.guard_pending||v.guard_registered)throw std::invalid_argument("Unknown or repeated scoped guard cleanup");v.guard_registered=true;});}
        void guard_footer_04b7cc(std::int32_t& n)override{v.call([&]{if(&n!=v.options.guard_epoch||n!=-1||!v.guard_pending||!v.guard_registered)throw std::logic_error("Owned guard footer has no initialized source");n=1;v.thread_epoch=1;v.guard_pending=false;v.guard_initialized=true;});}
        std::uint32_t read_stop_34(s::IocpSchedulerEntry& e)override{return v.call([&]{v.entry(e);return std::atomic_ref(e.high_34).load();});}
        std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& out,const std::array<std::uintptr_t,2>& from)override{return v.call([&]() -> std::array<std::uintptr_t,2>&{if(!v.worker_context||!zero(from))throw s::UnsupportedInput("Foreign pending-exception transfer is unsupported");out={};return out;});}
        [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{v.terminal();throw s::UnsupportedInput("Native pending-exception rethrow is not a compiler exception_ptr");}
        void complete(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{v.call([&]{throw std::logic_error("Runtime owns actual coherent completion routing");});}
    } guard{*this};
    struct Log final:s::WriteFinishApi {
        Impl& v;explicit Log(Impl& v):v(v){}
        void stream(s::WriteFinishStream& s){if(&s!=&v.stream||s.identity!=reinterpret_cast<std::uintptr_t>(&v.stream)||s.virtual_base_offset_04)throw std::invalid_argument("Unknown owned log stream");}
        void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{v.call([&]{throw std::logic_error("Profile owns string sized frees");});}
        [[noreturn]]void invalid_parameter_05c974()override{v.terminal();throw s::UnsupportedInput("Invalid owned source parameter");}
        void next_write_02a500(std::uintptr_t)override{v.call([&]{throw std::logic_error("Profile owns actual queued write continuation");});}
        s::WriteFinishStream& begin_log_006880(std::uintptr_t global,std::uintptr_t prefix)override{return v.call([&]() -> s::WriteFinishStream&{
            if(global!=0x1800a4130ULL||v.log_stage!=LogStage::Empty||v.loan_address)throw std::logic_error("Unknown or already active owned log");
            const auto& text=prefix==0x1800845c8ULL?v.options.read_prefix:prefix==0x1800845d8ULL?v.options.write_prefix:prefix==0x1800845f8ULL?v.options.accept_prefix:throw std::invalid_argument("Unknown source log prefix");
            v.log_stage=LogStage::Prefix;const auto lease=shared_log::acquire(*v.options.stream);*v.options.stream<<text.c_str();return v.stream;
        });}
        s::WriteFinishStream& append_007990(s::WriteFinishStream& s,std::uintptr_t address,std::uint64_t size)override{return v.call([&]() -> s::WriteFinishStream&{
            stream(s);if(v.log_stage!=LogStage::Prefix||!v.loan_address||address!=v.loan_address||size!=v.loan_text.size())throw std::invalid_argument("Log append requires the exact current bounded message");
            if(v.loan_heap)(void)v.ports->view.memory.buffer_context().memory.view(address,size);
            v.log_stage=LogStage::Message;const auto lease=shared_log::acquire(*v.options.stream);v.options.stream->write(v.loan_text.data(),static_cast<std::streamsize>(v.loan_text.size()));return v.stream;
        });}
        std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t c)override{return v.call([&]{if(p!=v.stream.identity||c!=10||v.log_stage!=LogStage::Message)throw std::invalid_argument("Unknown owned log widening token/order");v.log_stage=LogStage::Widened;const auto lease=shared_log::acquire(*v.options.stream);v.widened=static_cast<std::uint8_t>(v.options.stream->widen(static_cast<char>(c)));return std::uint64_t{v.widened};});}
        void put_00fe90(s::WriteFinishStream& s,std::uint8_t c)override{v.call([&]{stream(s);if(v.log_stage!=LogStage::Widened||c!=v.widened)throw std::invalid_argument("Owned log put must use its widened character");v.log_stage=LogStage::Put;const auto lease=shared_log::acquire(*v.options.stream);v.options.stream->put(static_cast<char>(c));});}
        void flush_00e1e0(s::WriteFinishStream& s)override{v.call([&]{stream(s);if(v.log_stage!=LogStage::Put)throw std::logic_error("Owned log flush before complete message");{const auto lease=shared_log::acquire(*v.options.stream);v.options.stream->flush();}v.admit();v.loan_address=0;v.loan_heap=false;v.loan_text.clear();v.log_stage=LogStage::Empty;++v.flushes;});}
    } log{*this};
    explicit Impl(Options o):options(std::move(o)),native(catalog,options.imports){stream.identity=reinterpret_cast<std::uintptr_t>(&stream);}
    Snapshot snapshot()const{affinity();PreserveError error;std::lock_guard lock(gate);return {phase,active,log_stage!=LogStage::Empty,guard_initialized,worker_context!=nullptr,starts,finishes,cleanups,flushes};}
};

std::shared_ptr<Policy> Policy::create(Options o){
    const auto& i=o.imports;
    if(!o.stream||!o.guard_epoch||*o.guard_epoch||!o.lifecycle.select_owner||!o.lifecycle.after||!o.lifecycle.returned||!o.lifecycle.external_drained||!i.set_last_error||!i.get_last_error||!i.convert||!i.parse||!i.send||!i.select_write||!o.maximum_message_bytes||o.maximum_message_bytes>1048576)
        throw std::invalid_argument("Complete explicit session policy configuration required");
    for(const auto* s:{&o.read_prefix,&o.write_prefix,&o.accept_prefix})if(s->size()>o.maximum_message_bytes||s->find('\0')!=std::string::npos)throw std::invalid_argument("Owned log prefixes require bounded C strings");
    return std::shared_ptr<Policy>(new Policy(std::move(o)));
}
Policy::Policy(Options o):impl_(std::make_unique<Impl>(std::move(o))){}
Policy::~Policy()noexcept{auto& v=*impl_;{PreserveError error;std::lock_guard lock(v.gate);if(v.active||v.binding||v.closing||v.lease||v.worker_context||v.length||v.partial||v.loan_address||v.log_stage!=Impl::LogStage::Empty||(v.phase!=Phase::Fresh&&v.phase!=Phase::Closed))std::terminate();v.phase=Phase::Retiring;}
    try{if(!v.exits.close())std::terminate();}catch(...){std::terminate();}
    // Retire opaque functions and stream backing while metadata/mutex live.
    {Options retiring=std::move(v.options);v.options.lifecycle={};v.options.format_override={};}
}
void Policy::install(sp::Dependencies& d){auto& v=*impl_;PreserveError error;
    v.affinity();
    {std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||v.active||v.binding||d.errors||d.policy||!d.categories.empty()||d.success.name_identity||d.misc.name_identity)throw std::invalid_argument("Policy installation refuses mixed caller providers");}
    auto self=shared_from_this();std::vector<sp::CategoryProvider> categories{{generic_category(),{self,&v.generic}},{system_category(),{self,&v.system}},{misc_category(),{self,&v.misc}}};
    d.errors={self,&v.native};d.policy=self;d.categories=std::move(categories);d.success=generic_category();d.misc=misc_category();
}
std::shared_ptr<s::Api> Policy::errors(){return {shared_from_this(),&impl_->native};}
s::Category Policy::generic_category()const noexcept{return impl_->category(0);}
s::Category Policy::system_category()const noexcept{return impl_->category(1);}
s::Category Policy::misc_category()const noexcept{return impl_->category(2);}
std::int32_t& Policy::guard_epoch()const noexcept{return *impl_->options.guard_epoch;}
services_completion::ExitWaitRegistry& Policy::exit_waits(){return impl_->exits;}
Snapshot Policy::snapshot()const{return impl_->snapshot();}
s::DispatcherEffects& Policy::guards(){return impl_->guard;}
s::WriteFinishApi& Policy::logging(){return impl_->log;}
void Policy::bind(rt::Ports ports,std::function<void()> checkpoint){auto& v=*impl_;
    {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||v.binding||v.active){if(v.binding||v.active)v.phase=Phase::Poisoned;throw std::logic_error("Session policy binds once");}v.affinity();v.binding=true;}
    try{const auto storage=ports.accept.storage_bindings();if(!checkpoint||!ports.server_identity||storage.data()!=ports.environment.storage.storage.data()||storage.size()!=ports.environment.storage.storage.size())throw std::invalid_argument("Session policy requires stable exact Ports");
        (void)ports.view.memory.buffer_context();const auto b=ports.view.backend.snapshot().phase;
        if((b!=o::Phase::Ready&&b!=o::Phase::Running)||ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports.accept.snapshot().poisoned||*v.options.guard_epoch)throw std::logic_error("Session policy requires fresh guard and open source graph");
        v.ports.emplace(ports);v.checkpoint=std::move(checkpoint);checkpoint={};
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh)throw std::logic_error("Session policy bind retirement was poisoned");}
        v.lease=ports.view.memory.acquire_provider_lease_from_operation();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh)throw std::logic_error("Session policy bind was poisoned");v.binding=false;v.phase=Phase::Bound;}
    }catch(...){v.poison();{PreserveError error;std::lock_guard lock(v.gate);v.binding=false;}throw;}
}
std::uint32_t Policy::exchange_stop(s::IocpSchedulerEntry& e,std::uint32_t n){auto& v=*impl_;return v.call([&]{v.entry(e);return std::atomic_ref(e.high_34).exchange(n);});}
std::uint32_t Policy::exchange_wake(s::IocpSchedulerEntry& e,std::uint32_t n){auto& v=*impl_;return v.call([&]{v.entry(e);PreserveError error;std::lock_guard lock(v.gate);const auto old=static_cast<std::uint32_t>(e.word_38);e.word_38=(e.word_38&0xffffffff00000000ULL)|n;return old;});}
void Policy::initialize(std::array<std::uintptr_t,2>& p){auto& v=*impl_;v.call([&]{if(v.worker_context||v.starts)throw std::logic_error("Owned empty worker context initializes once");v.worker_context=&p;p={};++v.starts;});}
void Policy::finish(std::array<std::uintptr_t,2>& p){auto& v=*impl_;v.call([&]{if(v.worker_context!=&p||!zero(p))throw s::UnsupportedInput("Foreign or pending worker exception context");v.worker_context=nullptr;++v.finishes;});}
void Policy::construct_length_error(s::ReadLengthError& e,std::uintptr_t p){auto& v=*impl_;v.call([&]{if(p!=0x180084840ULL||v.length)throw std::invalid_argument("Unknown or occupied read length error context");v.length.emplace(&e,"read_until size exceeded");});}
[[noreturn]]void Policy::throw_length_error(s::ReadLengthError& e){auto& v=*impl_;v.terminal();if(!v.length||v.length->first!=&e)throw s::UnsupportedInput("Unregistered read length error");throw std::length_error(v.length->second);}
[[noreturn]]void Policy::bad_weak_ptr_026860(){impl_->terminal();throw std::bad_weak_ptr();}
[[noreturn]]void Policy::thread_error(int n){impl_->terminal();bootstrap_policies::throw_thread_error_04a648(n);}
void Policy::cleanup_exit_waits(){auto& v=*impl_;v.call([&]{v.exits.cleanup_current_thread();++v.cleanups;});}
[[noreturn]]void Policy::raise_write(s::Error& e,std::uintptr_t p){auto& v=*impl_;v.terminal();if(p!=0x180084720ULL)throw s::UnsupportedInput("Unknown source write exception context");throw std::system_error(e.value,compiler_category(v.index(e.category)),"write");}
[[noreturn]]void Policy::raise(s::Error e,std::uintptr_t p){auto& v=*impl_;v.terminal();if(p!=0x180084530ULL)throw s::UnsupportedInput("Unknown source worker exception context");throw std::system_error(e.value,compiler_category(v.index(e.category)),"post");}
s::IocpSchedulerEntry* Policy::completion_owner(sp::Application& app,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& e,std::uint32_t n){auto& v=*impl_;return v.call([&]{v.application(app);auto* selected=v.options.lifecycle.select_owner(app,owner,op,e,n);if(selected&&selected!=owner)throw std::invalid_argument("External lifecycle selected a foreign completion owner");return selected;});}
void Policy::after_completion(sp::Application& app){auto& v=*impl_;v.call([&]{v.application(app);v.options.lifecycle.after(app);});}
void Policy::worker_returned(sp::Application& app,std::uint64_t n,const s::WorkerRunResult& r){auto& v=*impl_;v.call([&]{v.application(app);
    const bool empty=!v.starts&&!v.finishes&&!n&&r.error&&!r.error->value&&same(r.error->category,v.category(0));
    if(v.returned||v.worker_context||!r.error||(!empty&&(v.starts!=1||v.finishes!=1)))throw std::logic_error("Worker return lacks normal owned context finish");v.options.lifecycle.returned(app,n,r);v.returned=true;});}
bool Policy::external_quiescent(o::ServerView view,sp::Application& app){auto& v=*impl_;v.enter();try{
    v.matching(view);v.application(app);
    const auto phase=view.backend.snapshot().phase;if(!v.returned||(phase!=o::Phase::Returned&&phase!=o::Phase::Quiescing))throw std::logic_error("External proof requires observed worker return");
    {auto outside=view.memory.enter();}const auto result=v.options.lifecycle.external_drained(view,app);v.valid();v.attested=result;v.leave();return result;
}catch(...){v.failed();throw;}}
bool Policy::close(bool attestation){auto& v=*impl_;
    {PreserveError error;std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;}
    v.enter();try{v.admit(true);if(!attestation||!v.attested||!v.returned||v.worker_context||v.length||v.partial||v.loan_address||v.log_stage!=Impl::LogStage::Empty||v.guard_pending||v.exits.active()){v.leave();return false;}
        auto& p=*v.ports;const auto m=p.view.memory.snapshot();const auto io=p.view.io.snapshot();const auto a=p.accept.snapshot();const auto b=p.view.backend.snapshot().phase;
        if((b!=o::Phase::Returned&&b!=o::Phase::Quiescing)||m.allocations||m.controls||m.proxies||m.blocks||m.maps||m.buffers||m.reserved||m.pending_frees||m.partial_constructions||m.buffer_borrows||io.allocations||io.pending||io.active_calls||a.projections||a.pending||a.active_calls||a.poisoned||p.view.scheduler.work_count_30.load()||p.view.scheduler.completion.head||p.view.scheduler.completion.tail){v.leave();return false;}
        {PreserveError error;std::lock_guard lock(v.gate);v.closing=true;}
        if(!v.exits.close())throw std::logic_error("Owned job exit-wait registry remains active");
        {auto retiring=std::move(v.checkpoint);v.checkpoint={};}v.valid();v.lease.reset();v.ports.reset();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound)throw std::logic_error("Policy retirement was poisoned");v.active=false;v.closing=false;v.phase=Phase::Closed;}return true;
    }catch(...){v.failed();{PreserveError error;std::lock_guard lock(v.gate);v.closing=false;}throw;}
}
} // namespace anadius::session_policy
