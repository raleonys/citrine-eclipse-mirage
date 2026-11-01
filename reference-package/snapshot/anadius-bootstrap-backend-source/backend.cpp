#include "backend.hpp"
#include "../anadius-application-effects-source/composition.hpp"
#include "../anadius-library-effects-source/phase_binding.hpp"
#include "../anadius-config-source/state_binding.hpp"
#include "../anadius-achievement-storage-source/path_binding.hpp"
#include "../anadius-license-source/auth_state.hpp"
#include "../anadius-achievement-runtime-source/mutex_binding.hpp"
#include "../anadius-config-defaults-source/teardown.hpp"
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace anadius::bootstrap_backend {
namespace {
template<class F> F take_function(F& donor){
    F result(std::move(donor));donor=nullptr;return result;
}
}
Resources take_resources(Resources& r){
    // Prvalue aggregate construction guarantees the result object itself is
    // not another moved-from Resources donor. Each callable donor is cleared
    // here while its receiving backing/narrow owner remains alive.
    return Resources{std::move(r.state_lifetime),std::move(r.configuration),std::move(r.files),
        std::move(r.input),std::move(r.console),std::move(r.threads),std::move(r.remaining),
        std::move(r.execution),std::move(r.server),std::move(r.online),std::move(r.library_os),
        std::move(r.library_effects),std::move(r.host),std::move(r.hook_cleanup),take_function(r.stop_sources),
        std::exchange(r.join_api,JoinApi{}),std::exchange(r.join_timeout_ms,0),take_function(r.callback_attachment),
        std::move(r.startup_publication),take_function(r.application_worker),take_function(r.remaining_factory),take_function(r.online_factory)};
}
namespace {
struct Selection {
    std::unique_ptr<RemainingPolicies> narrow;
    Resources resources;
};
Selection checked(Resources& resources,std::unique_ptr<RemainingPolicies> narrow){
    Selection selected{std::move(narrow),take_resources(resources)};
    const auto& r=selected.resources;
    if(!r.configuration||!r.files||!r.input||!r.console||!r.threads||
       (static_cast<bool>(r.remaining)==static_cast<bool>(selected.narrow))||r.remaining_factory||
       !r.execution||(static_cast<bool>(r.server)==static_cast<bool>(r.application_worker))||
       (static_cast<bool>(r.online)==static_cast<bool>(r.online_factory))||!r.library_os||!r.library_effects||
       !r.host||!r.hook_cleanup||!r.stop_sources)
        throw std::invalid_argument("All bootstrap backend providers and stop_sources are required");
    return selected;
}
void sync_paths(configuration::State* config,recovered::libraries::State& state,bool main){
    if(!config)throw std::logic_error("Backend shared state is not bound");
    const auto& source=main?config->extra_dlls_from_main:config->extra_dlls;
    std::vector<std::wstring> paths;paths.reserve(source.size());
    for(const auto& path:source)paths.emplace_back(path.begin(),path.end());
    (main?state.from_main:state.from_worker)=std::move(paths);
}
class LibraryExecution final:public recovered::libraries::Effects {
    recovered::libraries::Effects& lower_;execution_recovered::Execution& execution_;
    std::uint8_t& ready_;configuration::State*& config_;recovered::libraries::State& state_;
public:
    LibraryExecution(recovered::libraries::Effects& lower,execution_recovered::Execution& execution,
        std::uint8_t& ready,configuration::State*& config,recovered::libraries::State& state)
        :lower_(lower),execution_(execution),ready_(ready),config_(config),state_(state){}
    void begin_load_log_0863e0() override{lower_.begin_load_log_0863e0();}
    void append_log_path_and_flush(std::string_view s) override{lower_.append_log_path_and_flush(s);}
    void message_038bc0(std::string a,std::string b,std::uint32_t c) override{lower_.message_038bc0(std::move(a),std::move(b),c);}
    [[noreturn]] void throw_runtime_00cd70(int c) override{lower_.throw_runtime_00cd70(c);std::terminate();}
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t c) override{lower_.throw_filesystem_00cdb0(c);std::terminate();}
    void wait_steps_036880(int c) override{execution_.wait_steps_036880(c);sync_paths(config_,state_,false);}
    void finish_flag_036820() override{execution_.finish_flag_036820(ready_);}
};
class OnlineExecution final:public online_recovered::Effects {
    online_recovered::Effects& lower_;execution_recovered::Execution& execution_;std::uint8_t& ready_;
public:
    OnlineExecution(online_recovered::Effects& l,execution_recovered::Execution& e,std::uint8_t& r):lower_(l),execution_(e),ready_(r){}
    std::uint32_t suppress_errors_0450a0() override{return lower_.suppress_errors_0450a0();}
    void restore_error_mode(std::uint32_t m) override{lower_.restore_error_mode(m);}
    online_recovered::Path filesystem_widen_006d70(std::string_view s) override{return lower_.filesystem_widen_006d70(s);}
    online_recovered::Path parent_path_019820(const online_recovered::Path& p) override{return lower_.parent_path_019820(p);}
    online_recovered::Path join_016b00(const online_recovered::Path& a,const online_recovered::Path& b) override{return lower_.join_016b00(a,b);}
    online_recovered::Module load_copy_0450d0(const online_recovered::Path& p) override{return lower_.load_copy_0450d0(p);}
    online_recovered::Procedure get_proc_address(online_recovered::Module m,std::string_view s) override{return lower_.get_proc_address(m,s);}
    std::int32_t call_version_check(online_recovered::Procedure p) override{return lower_.call_version_check(p);}
    online_recovered::Object call_create_online(online_recovered::Procedure p,const char16_t* s) override{return lower_.call_create_online(p,s);}
    std::int32_t status_virtual_08(online_recovered::Object o) override{return lower_.status_virtual_08(o);}
    const char* username_virtual_10(online_recovered::Object o) override{return lower_.username_virtual_10(o);}
    const char* persona_virtual_18(online_recovered::Object o) override{return lower_.persona_virtual_18(o);}
    const char* user_id_virtual_20(online_recovered::Object o) override{return lower_.user_id_virtual_20(o);}
    std::string copy_username(online_recovered::Object o) override{return lower_.copy_username(o);}
    std::string copy_persona(online_recovered::Object o) override{return lower_.copy_persona(o);}
    std::string copy_user_id(online_recovered::Object o) override{return lower_.copy_user_id(o);}
    void destroy_virtual_00(online_recovered::Object o) override{lower_.destroy_virtual_00(o);}
    bool free_library(online_recovered::Module m) override{return lower_.free_library(m);}
    status_recovered::Status status_01ea20(const online_recovered::Path& p,std::uint32_t f) override{return lower_.status_01ea20(p,f);}
    [[noreturn]] void filesystem_error_018130(std::string_view s,std::uint32_t c,const online_recovered::Path& p) override{lower_.filesystem_error_018130(s,c,p);std::terminate();}
    std::string read_binary_03b3f0(const online_recovered::Path& p) override{return lower_.read_binary_03b3f0(p);}
    bool write_binary_03c1d0(const online_recovered::Path& p,std::string_view s) override{return lower_.write_binary_03c1d0(p,s);}
    void log_literal_006880(std::string_view s) override{lower_.log_literal_006880(s);}
    void log_finish_0088e0() override{lower_.log_finish_0088e0();}
    void dialog_038bc0(std::string_view a,std::string_view b,std::uint32_t c) override{lower_.dialog_038bc0(a,b,c);}
    void wait_steps_036880(std::int32_t c) override{execution_.wait_steps_036880(c);}
    void finish_flag_036820() override{execution_.finish_flag_036820(ready_);}
};
}
struct SourceBackend::Impl {
    RuntimeSeed state;
    // Native +138 ownership: outlives Resources, every worker and Graph.
    std::optional<license_recovered::AuthenticationCode> authentication_code{std::in_place};
    // +428 host backing; native037b20 leaves reserved_04 untouched. Host value
    // initialization is a defined padding policy, not native byte equivalence.
    execution_recovered::MutexState achievement_mutex{};
    // Stable native+408/+4a8 cells. Constructed before Resources and destroyed
    // after every provider and joined worker; graph borrowers do not own them.
    achievement_storage::Paths achievement_paths;
    bool shared_achievement_paths=false;
    std::unique_ptr<RemainingPolicies> narrow;std::unique_ptr<OnlineComponent> online_component;
    Resources resources;configuration::State* config=nullptr;
    std::array<std::uint8_t*,3> console{};
    execution_recovered::Execution execution;TrackingThreads threads;
    LibraryExecution library_effects;std::unique_ptr<OnlineExecution> legacy_online;
    std::unique_ptr<CallbackAttachment> callback_attachment;
    std::unique_ptr<ApplicationWorker> application_worker;
    bool stopped=false,control_busy=false,routes_published=false,ever_published=false,worker_finished=false,retiring=false;
    bool online_binding=false,online_poisoned=false;
    bool providers_retired=false,aggregate_released=false;
    inline static std::mutex gate;inline static Impl* current=nullptr;inline static std::size_t calls=0;
    Impl(RuntimeSeed s,Selection&& selected):state(std::move(s)),narrow(std::move(selected.narrow)),resources(take_resources(selected.resources)),
        execution(state.execution_mutex,{state.server_ready,state.online_ready,state.libraries_ready},state.frequency_cache,*resources.execution,resources.startup_publication.get()),
        threads(*resources.threads,resources.join_api),
        library_effects(*resources.library_effects,execution,state.libraries_ready,config,state.libraries),
        legacy_online(resources.online?std::make_unique<OnlineExecution>(*resources.online,execution,state.online_ready):nullptr){
        execution_recovered::initialize_mutex_04a698(achievement_mutex,2);
    }
    void retire(){
        {std::lock_guard lock(gate);if(providers_retired)return;if(retiring||current==this)std::terminate();retiring=true;}
        if(threads.tracked_jobs()||threads.poisoned()||online_poisoned||online_binding)std::terminate();
        if(application_worker&&!worker_finished){
            if(ever_published||!application_worker->cancel_unpublished())std::terminate();
        }
        // Retire the new opaque owning edges while every referent and seal is
        // alive, outside the per-image gate. Reentry cannot republish routes.
        application_worker.reset();resources.application_worker={};callback_attachment.reset();
        // The facade still points to this Impl throughout all opaque provider
        // destruction. Moving the owner only from ~Impl would clear impl_ first
        // and turn a correctly rejected reentry into a null dereference.
        {Resources retiring_resources=take_resources(resources);}
        online_component.reset(); // bound referents and Closed metadata live here
        narrow.reset(); // closed metadata outlived every opaque Resources owner
        {std::lock_guard lock(gate);providers_retired=true;}
    }
    ~Impl(){std::lock_guard lock(gate);if(!retiring||current==this)std::terminate();}
    void alive(){
        if(retiring)throw std::logic_error("Backend provider retirement has sealed source admission");
        if(online_binding){online_poisoned=true;throw std::logic_error("Online factory reentered Backend source admission");}
        if(online_poisoned)throw std::logic_error("Online factory has an unknown Backend prefix");
    }
    CallbackStateView view(){if(!config)throw std::logic_error("Backend shared state is not bound");return {*config,state,execution};}
    online_recovered::Effects& online_effects(){
        alive();
        if(online_component){if(!online_component->matches(view()))throw std::logic_error("Online component view identity changed");return *online_component;}
        if(!legacy_online)throw std::logic_error("Online component has not been bound");return *legacy_online;
    }
    struct Lease {
        Impl* value;
        Lease(){std::lock_guard lock(gate);if(!current)std::terminate();value=current;++calls;}
        ~Lease(){std::lock_guard lock(gate);--calls;}
    };
    void bind_routes(){
        alive();
        if(!config||stopped||retiring)throw std::logic_error("Backend routes cannot be published");
        std::lock_guard lock(gate);
        if(current&&current!=this)throw std::logic_error("Another backend owns worker routes");
        current=this;routes_published=true;ever_published=true;
    }
    void require_routes(){std::lock_guard lock(gate);if(current!=this||stopped||retiring)throw std::logic_error("Worker routes are not published");}
    online_recovered::StateRefs online_state(){
        if(!config)throw std::logic_error("Backend shared state is not bound");
        return {config->executable_path,config->cookie_path,config->last_profile_path,config->last_avatar_path,
            config->avatar,state.online_module,state.online_object,config->username,config->persona_id,
            config->user_id,config->use_last_online_profile,state.online};
    }
    static void server_worker(){Lease lease;auto& v=*lease.value;
        if(v.application_worker)v.application_worker->run_server(v.config->server_port,v.state.server_ready,v.execution);
        else server_recovered::server_start_029ea0(*v.resources.server,v.config->server_port,v.state.server_ready,v.execution);}
    static void online_worker(){Lease lease;auto& v=*lease.value;
        try{online_recovered::worker_035b60(v.online_state(),v.online_effects());
            if(v.application_worker)v.application_worker->startup_complete(execution_recovered::ReadyStage::Online,&v.execution);
        }catch(...){if(v.application_worker)v.application_worker->startup_failed(&v.execution);throw;}}
    static void library_worker(bool main){Lease lease;auto& v=*lease.value;
        if(main)sync_paths(v.config,v.state.libraries,true);
        recovered::libraries::load_phase_035850(*v.resources.library_os,v.library_effects,v.state.libraries,main);}
};
SourceBackend::SourceBackend(RuntimeSeed seed,Resources resources)
    :SourceBackend(std::move(seed),take_resources(resources),nullptr){}
SourceBackend::SourceBackend(RuntimeSeed seed,Resources resources,std::unique_ptr<RemainingPolicies> narrow)
    :impl_(std::make_unique<Impl>(std::move(seed),checked(resources,std::move(narrow)))){}
SourceBackend::~SourceBackend() noexcept{impl_->retire();}
void SourceBackend::retire_owned_state_037f70() noexcept {
    auto& v=*impl_;
    if(v.aggregate_released||!v.stopped||!v.config)std::terminate();
    v.retire();
    // Host-only cached path copies are not the native +4c8/+4e0 cells.
    decltype(v.state.libraries.from_main){}.swap(v.state.libraries.from_main);
    decltype(v.state.libraries.from_worker){}.swap(v.state.libraries.from_worker);
    configuration_defaults::release_aggregate_storage_037f70(
        {*v.config,v.achievement_paths,v.state.libraries.loaded,v.authentication_code});
    v.aggregate_released=true;
}
bootstrap_owner::Providers SourceBackend::providers(){impl_->alive();auto& r=impl_->resources;
    return {*r.configuration,*r.files,*r.input,*r.console,impl_->threads,*this};}
void SourceBackend::bind_state(configuration::State& state,config_startup::ConsoleFlags flags){
    impl_->alive();
    if(impl_->config||impl_->stopped)throw std::logic_error("Backend shared state can only be bound once");
    impl_->config=&state;impl_->console={&flags.show_lsx,&flags.show_console,&flags.no_console};
    if(auto* binding=dynamic_cast<library_effects::PhaseBinding*>(impl_->resources.library_effects.get()))
        binding->bind_library_phase(impl_->execution,impl_->state.libraries_ready);
    if(auto* binding=dynamic_cast<configuration::StateBinding*>(impl_->resources.configuration.get()))
        binding->bind_configuration_state(state);
    if(auto* binding=dynamic_cast<achievement_storage::PathBinding*>(impl_->resources.configuration.get())){
        binding->bind_achievement_paths(impl_->achievement_paths);
        impl_->shared_achievement_paths=true;
    }
    if(impl_->resources.online_factory){
        auto& v=*impl_;v.online_binding=true;
        try{
            // Adopt before inspecting identity or retiring any SBO donor capture.
            v.online_component=v.resources.online_factory(v.view());
            v.resources.online_factory={};
            if(v.online_poisoned)throw std::logic_error("Online factory lost Backend admission");
            const bool matched=v.online_component&&v.online_component->matches(v.view());
            if(v.online_poisoned)throw std::logic_error("Online identity check lost Backend admission");
            v.online_binding=false;
            if(!matched)throw std::invalid_argument("Online factory returned a null or foreign-view component");
        }catch(...){v.online_binding=false;throw;}
    }
    if(impl_->resources.application_worker){
        auto worker=impl_->resources.application_worker({state,impl_->state,impl_->execution});
        if(!worker)throw std::invalid_argument("Application worker factory returned null");
        impl_->application_worker=std::move(worker);
        if(impl_->online_component)
            if(auto* binding=dynamic_cast<application_effects::OnlineBinding*>(impl_->application_worker.get()))
                binding->bind_effects_online(*impl_->online_component);
        if(auto* binding=dynamic_cast<achievement_runtime::MutexBinding*>(impl_->application_worker.get()))
            binding->bind_achievement_mutex(impl_->achievement_mutex,*impl_->resources.execution);
        if(auto* binding=dynamic_cast<license_recovered::AuthenticationBinding*>(impl_->application_worker.get()))
            binding->bind_authentication_code(*impl_->authentication_code);
        if(impl_->shared_achievement_paths)
            if(auto* binding=dynamic_cast<achievement_storage::PathBinding*>(impl_->application_worker.get()))
                binding->bind_achievement_paths(impl_->achievement_paths);
    }
    if(impl_->resources.callback_attachment){
        auto attachment=impl_->resources.callback_attachment({state,impl_->state,impl_->execution});
        if(!attachment)throw std::invalid_argument("Callback attachment factory returned null");
        impl_->callback_attachment=std::move(attachment);
    }
}
bool SourceBackend::stop_and_join(){
    auto& v=*impl_;
    v.alive();
    if(v.control_busy)throw std::logic_error("Backend control reentry");
    struct Control {bool& b;explicit Control(bool& x):b(x){b=true;}~Control(){b=false;}} control(v.control_busy);
    auto finish_worker=[&]{if(!v.application_worker||v.worker_finished)return true;if(!v.application_worker->finish_after_join())return false;v.worker_finished=true;return true;};
    if(v.stopped){if(v.callback_attachment&&!v.callback_attachment->pause_before_cleanup())return false;return finish_worker();}
    if(!v.resources.stop_sources())return false;
    if(v.application_worker)v.application_worker->request_stop();
    if(v.callback_attachment&&!v.callback_attachment->pause_before_cleanup())return false;
    if(!v.threads.stop_and_join(v.resources.join_timeout_ms))return false;
    {
        std::lock_guard lock(Impl::gate);
        if(v.routes_published){
            if(Impl::current!=&v)throw std::logic_error("Backend route ownership mismatch");
            if(Impl::calls)return false;
            Impl::current=nullptr;v.routes_published=false;
        }
    }
    v.stopped=true;return finish_worker();
}
int SourceBackend::register_exit(bootstrap_native_recovered::ExitCallback callback){impl_->bind_routes();return impl_->narrow?impl_->narrow->register_exit(callback):impl_->resources.remaining->register_exit(callback);}
void SourceBackend::install_terminate(std::terminate_handler h){impl_->alive();if(impl_->narrow)impl_->narrow->install_terminate(h);else impl_->resources.remaining->install_terminate(h);}
void* SourceBackend::log_insert(void* p,std::uintptr_t s){impl_->alive();return impl_->narrow?impl_->narrow->log_insert(p,s):impl_->resources.remaining->log_insert(p,s);}
void SourceBackend::log_finish(void* p){impl_->alive();if(impl_->narrow)impl_->narrow->log_finish(p);else impl_->resources.remaining->log_finish(p);}
void* SourceBackend::allocate(std::size_t n){impl_->alive();return impl_->narrow?impl_->narrow->allocate(n):impl_->resources.remaining->allocate(n);}
void SourceBackend::deallocate(void* p,std::size_t n){impl_->alive();if(impl_->narrow)impl_->narrow->deallocate(p,n);else impl_->resources.remaining->deallocate(p,n);}
recovered::NoArgumentFunction SourceBackend::worker(recovered::Worker w){impl_->require_routes();
    switch(w){case recovered::Worker::first_029ea0:return &Impl::server_worker;case recovered::Worker::second_035b60:return &Impl::online_worker;}
    throw std::invalid_argument("Unknown bootstrap worker");}
recovered::BooleanFunction SourceBackend::boolean_worker(){impl_->require_routes();return &Impl::library_worker;}
[[noreturn]] void SourceBackend::thread_error(int c){impl_->alive();if(impl_->narrow)impl_->narrow->thread_error(c);else impl_->resources.remaining->thread_error(c);std::terminate();}
void SourceBackend::between_workers(){auto& v=*impl_;v.alive();if(!v.config)throw std::logic_error("Backend shared state is not bound");
    try{
    if(v.callback_attachment)v.callback_attachment->host_setup_begin();
    if(v.callback_attachment)v.callback_attachment->publish();
    host_init_recovered::initialize_host_00e310({*v.config,v.state.registry_values,v.state.enabled_targets,v.state.originals},*v.resources.host);
    if(v.resources.startup_publication)v.resources.startup_publication->published(execution_recovered::ReadyStage::Host,&v.execution);
    if(v.callback_attachment)v.callback_attachment->host_setup_complete();
    if(v.application_worker)v.application_worker->startup_complete(execution_recovered::ReadyStage::Host,&v.execution);
    }catch(...){if(v.application_worker)v.application_worker->startup_failed(&v.execution);throw;}}
void SourceBackend::guard_return(){impl_->alive();if(impl_->narrow)impl_->narrow->guard_return();else impl_->resources.remaining->guard_return();}
void SourceBackend::cleanup_threads(){impl_->alive();if(impl_->narrow)impl_->narrow->cleanup_threads();else impl_->resources.remaining->cleanup_threads();}
void SourceBackend::cleanup_first(){impl_->alive();recovered::libraries::unload_035ab0(*impl_->resources.library_os,impl_->state.libraries);}
void SourceBackend::cleanup_second(){impl_->alive();online_recovered::cleanup_0366f0(impl_->online_state(),impl_->online_effects());}
void SourceBackend::cleanup_network(){impl_->alive();network_hook_recovered::cleanup_hooks_0102b0(impl_->state.enabled_targets,*impl_->resources.hook_cleanup);}
void SourceBackend::terminate_log(){impl_->alive();if(impl_->narrow)impl_->narrow->terminate_log();else impl_->resources.remaining->terminate_log();}
[[noreturn]] void SourceBackend::unexpected_terminate_return(){impl_->alive();if(impl_->narrow)impl_->narrow->unexpected_terminate_return();else impl_->resources.remaining->unexpected_terminate_return();std::terminate();}
const RuntimeSeed& SourceBackend::state() const noexcept{return impl_->state;}
std::size_t SourceBackend::tracked_jobs() const{return impl_->threads.tracked_jobs();}
bool SourceBackend::tracking_poisoned() const{return impl_->threads.poisoned();}
bool SourceBackend::finish_callback_attachment(){
    impl_->alive();
    if(!impl_->stopped)throw std::logic_error("Stop and join before finishing callback attachment");
    return !impl_->callback_attachment||impl_->callback_attachment->close_after_cleanup();
}
bool SourceBackend::finish_remaining_policies(){
    impl_->alive();
    if(!impl_->stopped)throw std::logic_error("Stop/join and registry cleanup before closing remaining policies");
    return !impl_->narrow||impl_->narrow->close_after_cleanup();
}
bool SourceBackend::finish_online_component(){
    impl_->alive();
    if(!impl_->stopped)throw std::logic_error("Stop/join and actual exit cleanup before closing online component");
    if(!impl_->online_component)return true;
    if(!impl_->online_component->matches(impl_->view()))throw std::logic_error("Online close requires its exact Backend view");
    return impl_->online_component->close_after_cleanup();
}
static_assert(!std::is_abstract_v<SourceBackend>);
static_assert(sizeof(wchar_t)==sizeof(char16_t));
static_assert(std::is_nothrow_move_constructible_v<Resources>);
} // namespace anadius::bootstrap_backend
