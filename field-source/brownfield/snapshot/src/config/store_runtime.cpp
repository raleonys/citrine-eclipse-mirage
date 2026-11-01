#include "store_internal.hpp"
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace preloader_l { namespace config {
namespace store_entitlement_internal {

static StoreState g_store_slots[2]{};
static uint8_t g_active_store = 0;
static uint32_t g_store_readers[2]{};
static bool g_store_retired[2]{};
static uint64_t g_store_generation[2]{1u,0u};
static uint64_t g_next_store_generation=1u;
static volatile uint32_t g_store_gate=0u;
static bool g_store_writer=false;
static bool g_store_clear_in_progress=false;
static StoreRuntimeStatus g_last_store_status = StoreRuntimeStatus::not_loaded;

bool store_gate_try_lock() noexcept {
#if defined(_MSC_VER)
    return _InterlockedCompareExchange(reinterpret_cast<volatile long*>(&g_store_gate),1,0)==0;
#else
    uint32_t expected=0u;
    return __atomic_compare_exchange_n(&g_store_gate,&expected,1u,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);
#endif
}
void store_gate_lock() noexcept { while(!store_gate_try_lock()){} }
void store_gate_unlock() noexcept {
#if defined(_MSC_VER)
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(&g_store_gate),0);
#else
    __atomic_store_n(&g_store_gate,0u,__ATOMIC_RELEASE);
#endif
}

StoreRuntimeStatus set_last_runtime_store_status(StoreRuntimeStatus status) noexcept {
    g_last_store_status = status;
    return status;
}

bool active_state_loaded() noexcept {
    return g_store_slots[g_active_store].status==StoreRuntimeStatus::ok;
}

bool begin_store_writer(uint8_t* candidate) noexcept {
    if(!candidate) return false;
    store_gate_lock();
    const uint8_t next=static_cast<uint8_t>(g_active_store^1u);
    if(g_store_writer||g_store_clear_in_progress||g_store_readers[next]!=0u){
        store_gate_unlock(); return false;
    }
    g_store_writer=true;
    *candidate=next;
    store_gate_unlock();
    return true;
}
void abort_store_writer(uint8_t candidate) noexcept {
    reset_store(g_store_slots[candidate]);
    store_gate_lock(); g_store_retired[candidate]=false; g_store_writer=false; store_gate_unlock();
}
void publish_store_writer(uint8_t candidate) noexcept {
    store_gate_lock();
    const uint8_t old=g_active_store;
    g_active_store=candidate;
    g_store_retired[candidate]=false;
    g_store_generation[candidate]=++g_next_store_generation;
    g_store_retired[old]=true;
    if(g_store_readers[old]==0u){ reset_store(g_store_slots[old]); g_store_retired[old]=false; }
    g_store_writer=false;
    store_gate_unlock();
}

} // namespace store_entitlement_internal

using namespace store_entitlement_internal;

bool begin_runtime_store_read(RuntimeStoreReadLease* lease) noexcept {
    if(!lease) return false;
    *lease=RuntimeStoreReadLease{};
    for(;;){
        store_gate_lock();
        if(g_store_clear_in_progress){ store_gate_unlock(); continue; }
        const uint8_t slot=g_active_store;
        if(g_store_slots[slot].status!=StoreRuntimeStatus::ok){ store_gate_unlock(); return false; }
        ++g_store_readers[slot];
        lease->generation=g_store_generation[slot]; lease->slot=slot; lease->active=true;
        store_gate_unlock(); return true;
    }
}

void end_runtime_store_read(RuntimeStoreReadLease* lease) noexcept {
    if(!lease||!lease->active||lease->slot>1u) return;
    store_gate_lock();
    const uint8_t slot=lease->slot;
    if(g_store_readers[slot]) --g_store_readers[slot];
    if(g_store_readers[slot]==0u && g_store_retired[slot] && slot!=g_active_store && !g_store_writer){
        reset_store(g_store_slots[slot]); g_store_retired[slot]=false;
    }
    store_gate_unlock();
    lease->active=false;
}

StoreMaterializationSource runtime_store_source_for_lease(RuntimeStoreReadLease* lease) noexcept {
    if(!lease||!lease->active||lease->slot>1u) return StoreMaterializationSource{};
    store_gate_lock();
    const bool valid=g_store_readers[lease->slot]!=0u && g_store_generation[lease->slot]==lease->generation;
    StoreState* state=valid?&g_store_slots[lease->slot]:nullptr;
    store_gate_unlock();
    return state?source_from_state(state):StoreMaterializationSource{};
}

StoreRuntimeStatus set_runtime_store_xml(const char* xml, size_t length) noexcept {
    if (!xml && length) return set_last_runtime_store_status(StoreRuntimeStatus::invalid_argument);
    if (!xml || !length) return set_last_runtime_store_status(StoreRuntimeStatus::invalid_xml);

    uint8_t candidate=0u;
    if(!begin_store_writer(&candidate)) return set_last_runtime_store_status(StoreRuntimeStatus::resource_error);
    StoreState& candidate_state=g_store_slots[candidate];
    reset_store(candidate_state);
    const ParseCode code=parse_document(candidate_state,xml,length);
    const StoreRuntimeStatus status=map_status(code);
    if(code!=ParseCode::ok){ abort_store_writer(candidate); return set_last_runtime_store_status(status); }
    candidate_state.status=StoreRuntimeStatus::ok;

#if defined(_WIN32) && defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    // Preserve the model-service ABI mirror, but runtime borrowed readers are
    // pinned to the independent immutable StoreState generation above.
    const auto model_status=model_commit_runtime_xml(xml,length);
    if(model_status!=StoreRuntimeStatus::ok){ abort_store_writer(candidate); return model_status; }
#elif defined(_WIN32)
    if(!model_materialize_committed_state(&candidate_state)){
        abort_store_writer(candidate); return set_last_runtime_store_status(StoreRuntimeStatus::resource_error);
    }
#endif
    publish_store_writer(candidate);
    return set_last_runtime_store_status(StoreRuntimeStatus::ok);
}

bool clear_runtime_store_retryable() noexcept {
    store_gate_lock();
    if(g_store_writer||g_store_readers[0]!=0u||g_store_readers[1]!=0u){
        g_last_store_status=StoreRuntimeStatus::resource_error;
        store_gate_unlock(); return false;
    }
    g_store_clear_in_progress=true; store_gate_unlock();
#if defined(_WIN32)
    if(!model_clear_authoritative()){
        store_gate_lock();
        g_last_store_status=StoreRuntimeStatus::resource_error;
        g_store_clear_in_progress=false;
        store_gate_unlock();
        return false;
    }
#endif
    store_gate_lock();
    reset_store(g_store_slots[0]); reset_store(g_store_slots[1]);
    g_active_store=0u; g_store_retired[0]=g_store_retired[1]=false;
    g_store_generation[0]=++g_next_store_generation; g_store_generation[1]=0u;
    g_last_store_status=StoreRuntimeStatus::not_loaded;
    g_store_clear_in_progress=false; store_gate_unlock();
    return true;
}

void clear_runtime_store() noexcept {
    (void)clear_runtime_store_retryable();
}

StoreRuntimeStatus runtime_store_status() noexcept { return g_last_store_status; }

bool runtime_store_loaded() noexcept {
    if(active_state_loaded()) return true;
#if defined(_WIN32)
    return model_runtime_loaded();
#else
    return false;
#endif
}
bool runtime_store_present() noexcept {
    if(active_state_loaded()) return g_store_slots[g_active_store].present;
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_present():false;
#else
    return false;
#endif
}
size_t runtime_store_entitlement_count() noexcept {
    if(active_state_loaded()) return g_store_slots[g_active_store].record_count;
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_entitlement_count():0u;
#else
    return 0u;
#endif
}
bool runtime_store_entitlement(size_t index, StoreEntitlementView* output) noexcept {
    if(active_state_loaded()) return state_entitlement(&g_store_slots[g_active_store],index,output);
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_entitlement(index,output):false;
#else
    return false;
#endif
}
StoreByteView runtime_store_preamble() noexcept {
    if(active_state_loaded()) return state_preamble(&g_store_slots[g_active_store]);
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_preamble():StoreByteView{nullptr,0};
#else
    return StoreByteView{nullptr,0};
#endif
}
StoreByteView runtime_store_trailer() noexcept {
    if(active_state_loaded()) return state_trailer(&g_store_slots[g_active_store]);
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_trailer():StoreByteView{nullptr,0};
#else
    return StoreByteView{nullptr,0};
#endif
}
ValueView runtime_store_feature_desc(size_t entitlement_index,size_t feature_index) noexcept {
    if(active_state_loaded()) return state_feature_desc(&g_store_slots[g_active_store],entitlement_index,feature_index);
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_feature_desc(entitlement_index,feature_index):missing_value();
#else
    return missing_value();
#endif
}
ValueView runtime_store_feature_flag(size_t entitlement_index,size_t feature_index) noexcept {
    if(active_state_loaded()) return state_feature_flag(&g_store_slots[g_active_store],entitlement_index,feature_index);
#if defined(_WIN32)
    return model_runtime_loaded()?model_runtime_feature_flag(entitlement_index,feature_index):missing_value();
#else
    return missing_value();
#endif
}

} }
