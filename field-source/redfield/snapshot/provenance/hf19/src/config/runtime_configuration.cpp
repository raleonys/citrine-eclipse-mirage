#include "preloader_l/config/runtime_configuration.hpp"
#include "runtime_configuration_internal.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include "preloader_l/runtime/service_state.hpp"
#if defined(_WIN32)
#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/service_projection.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#endif
#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#if !defined(_WIN32)
#include <stdlib.h>
#endif

namespace preloader_l { namespace config {
namespace {
using runtime_configuration_detail::State;
using runtime_configuration_detail::transient_alloc;
using runtime_configuration_detail::transient_free;
#if !defined(_WIN32) || !defined(PRELOADER_L_RELEASE_BOOTSTRAP)
State g_states[2]{};
unsigned g_active=0;
#endif
char* g_cwd=nullptr;
size_t g_cwd_len=0;
size_t g_cwd_capacity=0;
#if defined(_WIN32)
preloader_l::runtime::model_abi::ReleaseServiceObject548* g_model_service=nullptr;
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
// A logically-cleared model policy may still own raw allocations whose physical
// release failed.  Keep that retired policy in a durable retry owner instead of
// leaving it in a stack candidate or partially publishing other config domains.
preloader_l::runtime::model_abi::ReleaseServiceObject548 g_retired_model_policy{};
bool g_retired_model_policy_pending=false;
// The setter used to drop partially released staged policy owners when a
// stack candidate went out of scope. This second, durable retry slot retains
// the exact failed owner until physical release eventually succeeds.
preloader_l::runtime::model_abi::ReleaseServiceObject548 g_deferred_staged_policy{};
bool g_deferred_staged_policy_pending=false;
#else
bool g_model_commit_in_progress=false;
#endif
#endif

volatile uint32_t g_policy_gate_lock=0u;
uint32_t g_policy_readers=0u;
bool g_policy_writer=false;
bool g_policy_frozen_for_hooks=false;
uint64_t g_policy_generation=1u;

bool gate_try_lock() noexcept {
#if defined(_MSC_VER)
    return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(&g_policy_gate_lock),1,0)==0;
#else
    uint32_t expected=0u;
    return __atomic_compare_exchange_n(&g_policy_gate_lock,&expected,1u,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
#endif
}
void gate_unlock() noexcept {
#if defined(_MSC_VER)
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(&g_policy_gate_lock),0);
#else
    __atomic_store_n(&g_policy_gate_lock,0u,__ATOMIC_RELEASE);
#endif
}
void gate_lock() noexcept { while(!gate_try_lock()){} }

bool try_begin_policy_write() noexcept {
    gate_lock();
    if(g_policy_writer||g_policy_readers!=0u||g_policy_frozen_for_hooks){ gate_unlock(); return false; }
    g_policy_writer=true; gate_unlock(); return true;
}
void end_policy_write(bool published) noexcept {
    gate_lock();
    if(published) ++g_policy_generation;
    g_policy_writer=false; gate_unlock();
}
void begin_policy_write_wait() noexcept { while(!try_begin_policy_write()){} }
bool begin_policy_write_wait_unless_frozen() noexcept {
    for (;;) {
        gate_lock();
        if (g_policy_frozen_for_hooks) { gate_unlock(); return false; }
        if (!g_policy_writer && !g_policy_readers) {
            g_policy_writer=true; gate_unlock(); return true;
        }
        gate_unlock();
    }
}


#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
bool retry_retired_model_policy_cleanup() noexcept {
    if(!g_retired_model_policy_pending) return true;
    if(!preloader_l::runtime::release_model_service_policy_retryable(&g_retired_model_policy)) return false;
    g_retired_model_policy=preloader_l::runtime::model_abi::ReleaseServiceObject548{};
    g_retired_model_policy_pending=false;
    return true;
}

bool retry_deferred_staged_policy_cleanup() noexcept {
    if (!g_deferred_staged_policy_pending) return true;
    if (!preloader_l::runtime::release_model_service_policy_retryable(&g_deferred_staged_policy))
        return false;
    g_deferred_staged_policy=preloader_l::runtime::model_abi::ReleaseServiceObject548{};
    g_deferred_staged_policy_pending=false;
    return true;
}

bool release_or_defer_staged_model_policy(
    preloader_l::runtime::model_abi::ReleaseServiceObject548* staged) noexcept {
    if (preloader_l::runtime::release_model_service_policy_retryable(staged)) return true;
    // Serialized by the configuration writer lease. The durable owner starts
    // empty, so the existing swap helper can transfer all model-owned fields
    // and repair the self-referential XML document without allocation.
    if (g_deferred_staged_policy_pending) return false; // invariant violation
    preloader_l::runtime::swap_model_service_configuration_policy(&g_deferred_staged_policy,staged);
    auto path=staged->config.path;
    staged->config.path=g_deferred_staged_policy.config.path;
    g_deferred_staged_policy.config.path=path;
    g_deferred_staged_policy_pending=true;
    return false;
}

bool prepare_empty_retired_model_policy() noexcept {
    if(g_retired_model_policy_pending) return false;
    g_retired_model_policy=preloader_l::runtime::model_abi::ReleaseServiceObject548{};
    return preloader_l::runtime::initialize_model_service_policy(&g_retired_model_policy,nullptr);
}

void detach_live_model_policy_for_clear() noexcept {
    preloader_l::runtime::swap_model_service_configuration_policy(g_model_service,&g_retired_model_policy);
    auto path=g_model_service->config.path;
    g_model_service->config.path=g_retired_model_policy.config.path;
    g_retired_model_policy.config.path=path;
    preloader_l::runtime::set_model_service_configuration_state(g_model_service,false,0u);
    g_retired_model_policy_pending=true;
}
#endif

#if defined(_WIN32)
bool commit_state_to_model_service(preloader_l::runtime::model_abi::ReleaseServiceObject548* target,const State& st) noexcept {
    if(!target) return false;
    preloader_l::runtime::ModelSettingInput* entries=nullptr;
    size_t entry_bytes=0;
    if(st.setting_count){
        if(st.setting_count>static_cast<size_t>(-1)/sizeof(preloader_l::runtime::ModelSettingInput)) return false;
        entry_bytes=st.setting_count*sizeof(preloader_l::runtime::ModelSettingInput);
        entries=static_cast<preloader_l::runtime::ModelSettingInput*>(transient_alloc(entry_bytes));
        if(!entries) return false;
    }
    size_t count=0;
    for(size_t i=0;i<st.setting_count;++i){
        const auto& item=st.settings[i];
        if(!item.used) continue;
        entries[count++]=preloader_l::runtime::ModelSettingInput{
            st.arena+item.name_off,item.name_len,st.arena+item.value_off,item.value_len};
    }
    const bool ok=preloader_l::runtime::commit_model_service_settings(target,entries,count);
    transient_free(entries,entry_bytes);
    return ok;
}
#endif
} // namespace

using namespace runtime_configuration_detail;

bool freeze_runtime_configuration_for_hooks() noexcept {
    gate_lock();
    if (g_policy_frozen_for_hooks || g_policy_writer || g_policy_readers) {
        gate_unlock(); return false;
    }
    g_policy_frozen_for_hooks=true;
    gate_unlock();
    return true;
}
void unfreeze_runtime_configuration_after_hooks() noexcept {
    gate_lock(); g_policy_frozen_for_hooks=false; gate_unlock();
}
bool begin_runtime_configuration_read(RuntimeConfigurationReadLease* lease) noexcept {
    if(!lease) return false;
    *lease=RuntimeConfigurationReadLease{};
    for(;;){
        gate_lock();
        if(g_policy_writer){ gate_unlock(); continue; }
        ++g_policy_readers; lease->generation=g_policy_generation; lease->active=true;
        gate_unlock(); return true;
    }
}
void end_runtime_configuration_read(RuntimeConfigurationReadLease* lease) noexcept {
    if(!lease||!lease->active) return;
    gate_lock(); if(g_policy_readers) --g_policy_readers; gate_unlock();
    lease->active=false;
}
uint64_t runtime_configuration_generation() noexcept {
    gate_lock(); const uint64_t value=g_policy_generation; gate_unlock(); return value;
}

RuntimeConfigurationStatus set_runtime_configuration_cfg(const char* cfg,size_t length) noexcept {
    if(!cfg&&length) return RuntimeConfigurationStatus::invalid_argument;
    if(!cfg||!length) return RuntimeConfigurationStatus::invalid_cfg;
#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    // Reserve the single-owner staging window before any candidate allocations.
    // This excludes a concurrent staged cleanup from overwriting the retry slot.
    if(!try_begin_policy_write()) return RuntimeConfigurationStatus::resource_error;
    if (!g_model_service || g_retired_model_policy_pending ||
        !retry_deferred_staged_policy_cleanup()) {
        end_policy_write(false);
        return RuntimeConfigurationStatus::resource_error;
    }
    const auto state_allocation=preloader_l::runtime::allocate_block(
        sizeof(State),preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
        preloader_l::runtime::windows_allocator_platform_ops());
    if(state_allocation.terminal!=preloader_l::runtime::AllocationTerminal::none||!state_allocation.pointer)
        { end_policy_write(false); return RuntimeConfigurationStatus::resource_error; }
    auto* candidate=static_cast<State*>(state_allocation.pointer); *candidate=State{};
    const ParseCode rc=parse_config2(*candidate,cfg,length);
    if(rc!=ParseCode::ok){
        reset_state(*candidate);
        preloader_l::runtime::release_block(candidate,sizeof(State),preloader_l::runtime::windows_allocator_platform_ops());
        end_policy_write(false);
        return map_code(rc);
    }
    preloader_l::runtime::BehavioralServiceStateCandidate* service_candidate=nullptr;
    if(!preloader_l::runtime::prepare_behavioral_service_state_cfg(cfg,length,&service_candidate)){
        reset_state(*candidate);
        preloader_l::runtime::release_block(candidate,sizeof(State),preloader_l::runtime::windows_allocator_platform_ops());
        end_policy_write(false);
        return RuntimeConfigurationStatus::resource_error;
    }
    preloader_l::runtime::model_abi::ReleaseServiceObject548 staged{};
    auto* adapter=preloader_l::runtime::model_service_adapter(g_model_service);
    if(!adapter||!preloader_l::runtime::initialize_model_service_policy(&staged,nullptr)||
       !commit_state_to_model_service(&staged,*candidate)){
        (void)release_or_defer_staged_model_policy(&staged);
        preloader_l::runtime::release_behavioral_service_state_candidate(service_candidate);
        reset_state(*candidate);
        preloader_l::runtime::release_block(candidate,sizeof(State),preloader_l::runtime::windows_allocator_platform_ops());
        end_policy_write(false);
        return RuntimeConfigurationStatus::resource_error;
    }
    preloader_l::runtime::swap_model_service_configuration_policy(g_model_service,&staged);
    preloader_l::runtime::set_model_service_configuration_state(g_model_service,true,candidate->present);
    // Logical commit is complete. A failed release is kept in a durable retry
    // owner; it must never disappear with the stack candidate.
    (void)release_or_defer_staged_model_policy(&staged);
    preloader_l::runtime::commit_behavioral_service_state_candidate(service_candidate);
    end_policy_write(true);
    reset_state(*candidate);
    preloader_l::runtime::release_block(candidate,sizeof(State),preloader_l::runtime::windows_allocator_platform_ops());
    return RuntimeConfigurationStatus::ok;
#else
    const unsigned next=g_active^1u;
    reset_state(g_states[next]);
    const ParseCode rc=parse_config2(g_states[next],cfg,length);
    if(rc!=ParseCode::ok){ reset_state(g_states[next]); return map_code(rc); }
    preloader_l::runtime::BehavioralServiceStateCandidate* service_candidate=nullptr;
    if(!preloader_l::runtime::prepare_behavioral_service_state_cfg(cfg,length,&service_candidate)){
        reset_state(g_states[next]); return RuntimeConfigurationStatus::resource_error;
    }
    if(!try_begin_policy_write()){
        preloader_l::runtime::release_behavioral_service_state_candidate(service_candidate);
        reset_state(g_states[next]);
        return RuntimeConfigurationStatus::resource_error;
    }
#if defined(_WIN32)
    g_model_commit_in_progress=true;
    const bool committed=commit_state_to_model_service(g_model_service,g_states[next]);
    g_model_commit_in_progress=false;
    if(!committed){
        preloader_l::runtime::release_behavioral_service_state_candidate(service_candidate);
        reset_state(g_states[next]);
        end_policy_write(false);
        return RuntimeConfigurationStatus::resource_error;
    }
    preloader_l::runtime::set_model_service_configuration_state(g_model_service,true,g_states[next].present);
#endif
    g_active=next;
    preloader_l::runtime::commit_behavioral_service_state_candidate(service_candidate);
    end_policy_write(true);
    return RuntimeConfigurationStatus::ok;
#endif
}

bool clear_runtime_configuration_retryable() noexcept {
    if (!begin_policy_write_wait_unless_frozen()) return false;
#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    // If a previous logical clear committed but physical release is still
    // pending, this call is cleanup-only.  Do not publish another generation.
    if (!retry_deferred_staged_policy_cleanup()) {
        end_policy_write(false);
        return false;
    }
    if(g_retired_model_policy_pending){
        const bool cleaned=retry_retired_model_policy_cleanup();
        end_policy_write(false);
        return cleaned;
    }
    if(g_model_service){
        // Stage an empty replacement before touching any live domain.  Failure
        // here preserves model/behavioral/CWD state and generation exactly.
        if(!prepare_empty_retired_model_policy()){
            end_policy_write(false);
            return false;
        }
        // One logical transaction: the live model receives the staged empty
        // policy/path while the exact previous policy moves to a durable owner.
        detach_live_model_policy_for_clear();
    }
#else
    reset_state(g_states[0]); reset_state(g_states[1]); g_active=0u;
#endif
    preloader_l::runtime::clear_behavioral_service_state();
    // The CWD mirror belongs to the same logical configuration generation.
    transient_free(g_cwd,g_cwd_capacity);
    g_cwd=nullptr; g_cwd_len=0u; g_cwd_capacity=0u;
#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    const bool cleanup_ok=retry_retired_model_policy_cleanup();
    end_policy_write(true);
    return cleanup_ok;
#else
    end_policy_write(true);
    return true;
#endif
}

void clear_runtime_configuration() noexcept {
    (void)clear_runtime_configuration_retryable();
}

bool runtime_configuration_loaded() noexcept {
#if defined(_WIN32)
    if(g_model_service) return preloader_l::runtime::model_service_configuration_loaded(g_model_service);
#endif
#if !defined(_WIN32) || !defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    return g_states[g_active].loaded;
#else
    return false;
#endif
}

ValueView runtime_configuration_setting(const char* key) noexcept {
    if(!key) return missing_value();
#if defined(_WIN32)
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    if(g_model_service&&preloader_l::runtime::model_service_configuration_loaded(g_model_service))
        return preloader_l::runtime::model_service_setting(g_model_service,key);
#else
    if(g_model_service&&!g_model_commit_in_progress&&preloader_l::runtime::model_service_configuration_loaded(g_model_service))
        return preloader_l::runtime::model_service_setting(g_model_service,key);
#endif
#endif
#if !defined(_WIN32) || !defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    const size_t key_len=cstr_len(key); const State& st=g_states[g_active];
    for(size_t i=0;i<st.setting_count;++i){
        const auto& item=st.settings[i];
        if(item.used&&item.name_len==key_len&&bytes_equal(st.arena+item.name_off,item.name_len,key,key_len))
            return ValueView{st.arena+item.value_off,item.value_len,true};
    }
#endif
    return missing_value();
}

ProfilePolicyView runtime_profile_policy() noexcept {
    const bool loaded=runtime_configuration_loaded();
    const auto empty=literal_value("");
    const auto resolve=[loaded,empty](const char* key,const char* constructor_default) noexcept -> ValueView {
        if(!loaded) return literal_value(constructor_default);
        const auto v=runtime_configuration_setting(key);
        return v.present?v:empty;
    };
    return ProfilePolicyView{
        resolve("Username","SURGE"),
        resolve("PersonaId","1144668899"),
        resolve("UserId","1000200030000"),
        resolve("Country","US"),
        resolve("Currency","USD"),
        resolve("IsSubscriber","true"),
        resolve("IsSteamSubscriber","false"),
        resolve("SubscriberLevel","2"),
        resolve("IsUnderAge","false"),
    };
}

uint32_t runtime_configuration_present_sections() noexcept {
#if defined(_WIN32)
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    if(g_model_service&&preloader_l::runtime::model_service_configuration_loaded(g_model_service))
#else
    if(g_model_service&&!g_model_commit_in_progress&&preloader_l::runtime::model_service_configuration_loaded(g_model_service))
#endif
        return preloader_l::runtime::model_service_present_sections(g_model_service);
#endif
#if !defined(_WIN32) || !defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    return g_states[g_active].present;
#else
    return 0u;
#endif
}

RuntimeConfigurationStatus set_runtime_current_directory(const char* value,size_t length) noexcept {
    if(!value&&length) return RuntimeConfigurationStatus::invalid_argument;
#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    if(g_retired_model_policy_pending) return RuntimeConfigurationStatus::resource_error;
#endif
    if(length==static_cast<size_t>(-1)) return RuntimeConfigurationStatus::resource_error;
    const size_t required=length+1u;
    auto* fresh=static_cast<char*>(transient_alloc(required));
    if(!fresh) return RuntimeConfigurationStatus::resource_error;
    for(size_t i=0;i<length;++i) fresh[i]=value[i];
    fresh[length]=0;
#if defined(_WIN32)
    if(g_model_service&&!preloader_l::runtime::commit_model_service_current_directory(g_model_service,value,length)){
        transient_free(fresh,required); return RuntimeConfigurationStatus::resource_error;
    }
#endif
    transient_free(g_cwd,g_cwd_capacity); g_cwd=fresh; g_cwd_capacity=required; g_cwd_len=length;
    return RuntimeConfigurationStatus::ok;
}
#if defined(_WIN32)
bool set_runtime_current_directory_wide_policy(const uint16_t* value,size_t length) noexcept {
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    if(g_retired_model_policy_pending) return false;
#endif
    if(!g_model_service||(!value&&length)) return false;
    return preloader_l::runtime::commit_model_service_current_directory_wide(g_model_service,value,length);
}
#endif
ValueView runtime_current_directory() noexcept {
    return g_cwd_len?ValueView{g_cwd,g_cwd_len,true}:missing_value();
}
#if defined(_WIN32)
void bind_runtime_configuration_model_service(preloader_l::runtime::model_abi::ReleaseServiceObject548* service) noexcept { g_model_service=service; }
void unbind_runtime_configuration_model_service() noexcept {
    begin_policy_write_wait();
    g_model_service=nullptr;
#if !defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    g_model_commit_in_progress=false;
#endif
    end_policy_write(true);
}
#endif

} }
