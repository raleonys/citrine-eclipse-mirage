#include "store_internal.hpp"
#if defined(_WIN32)
#include "preloader_l/runtime/service_abi.hpp"
#include "preloader_l/runtime/service_projection.hpp"
#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#endif

namespace preloader_l { namespace config {
namespace store_entitlement_internal {

#if defined(_WIN32)
using preloader_l::runtime::model_abi::MsvcString32;
using preloader_l::runtime::model_abi::MsvcVector24;
using preloader_l::runtime::model_abi::ModelStoreEntitlement1E0;
using preloader_l::runtime::model_abi::ReleaseServiceObject548;

static ReleaseServiceObject548* g_model_service = nullptr;
static bool g_model_store_authoritative = false;

size_t model_vector_count(const MsvcVector24& v, size_t stride) noexcept {
    if (!v.begin || !stride || v.end < v.begin) return 0u;
    const size_t bytes = static_cast<size_t>(v.end - v.begin);
    return bytes % stride == 0u ? bytes / stride : 0u;
}
StoreByteView model_byte_view(const MsvcVector24& v) noexcept {
    if (!v.begin || v.end < v.begin) return StoreByteView{nullptr,0};
    return StoreByteView{reinterpret_cast<const uint8_t*>(v.begin),
                         static_cast<size_t>(v.end - v.begin)};
}
ValueView model_string_view(const MsvcString32& v) noexcept {
    return preloader_l::runtime::model_service_string_view(v);
}
ValueView model_string_vector_at(const MsvcVector24& v, size_t index) noexcept {
    const size_t count = model_vector_count(v, sizeof(MsvcString32));
    if (index >= count) return missing_value();
    const auto* begin = reinterpret_cast<const MsvcString32*>(v.begin);
    return model_string_view(begin[index]);
}

bool model_runtime_loaded() noexcept {
    return g_model_service && g_model_store_authoritative;
}
bool model_runtime_present() noexcept {
    return model_runtime_loaded() &&
        model_vector_count(g_model_service->store_entitlements,sizeof(ModelStoreEntitlement1E0)) != 0u;
}
size_t model_runtime_entitlement_count() noexcept {
    return model_runtime_loaded() ?
        model_vector_count(g_model_service->store_entitlements,sizeof(ModelStoreEntitlement1E0)) : 0u;
}
bool model_runtime_entitlement(size_t index,StoreEntitlementView* output) noexcept {
    if (!model_runtime_loaded()) return false;
    const size_t count=model_vector_count(g_model_service->store_entitlements,sizeof(ModelStoreEntitlement1E0));
    if(!output||index>=count)return false;
    const auto* records=reinterpret_cast<const ModelStoreEntitlement1E0*>(g_model_service->store_entitlements.begin);
    const auto& r=records[index];
    StoreEntitlementView value{};
    value.numeric_id=r.numeric_id; value.offer_id=r.offer_id; value.release_ms=r.release_ms;
    value.flags=r.flags; value.category=static_cast<uint32_t>(r.category);
    value.tag=model_string_view(r.tag); value.icon=model_string_view(r.nested.icon);
    value.title=model_string_view(r.nested.title); value.description=model_string_view(r.nested.description);
    value.img_thumb=model_string_view(r.nested.img_thumb); value.img_cropped=model_string_view(r.nested.img_cropped);
    value.img_hero=model_string_view(r.nested.img_hero); value.vendor=model_string_view(r.vendor);
    value.key=model_string_view(r.key); value.hash=model_string_view(r.hash);
    value.feature_desc_count=model_vector_count(r.nested.feature_desc,sizeof(MsvcString32));
    value.feature_flag_count=model_vector_count(r.feature_flag,sizeof(MsvcString32));
    value.proto_bytes=model_byte_view(r.proto_bytes); value.field2=model_byte_view(r.field2); value.pad=model_byte_view(r.pad);
    *output=value; return true;
}
StoreByteView model_runtime_preamble() noexcept {
    return model_runtime_loaded()?model_byte_view(g_model_service->preamble_bytes):StoreByteView{nullptr,0};
}
StoreByteView model_runtime_trailer() noexcept {
    return model_runtime_loaded()?model_byte_view(g_model_service->trailer_bytes):StoreByteView{nullptr,0};
}
ValueView model_runtime_feature_desc(size_t entitlement_index,size_t feature_index) noexcept {
    if(!model_runtime_loaded())return missing_value();
    const size_t count=model_vector_count(g_model_service->store_entitlements,sizeof(ModelStoreEntitlement1E0));
    if(entitlement_index>=count)return missing_value();
    const auto* r=reinterpret_cast<const ModelStoreEntitlement1E0*>(g_model_service->store_entitlements.begin);
    return model_string_vector_at(r[entitlement_index].nested.feature_desc,feature_index);
}
ValueView model_runtime_feature_flag(size_t entitlement_index,size_t feature_index) noexcept {
    if(!model_runtime_loaded())return missing_value();
    const size_t count=model_vector_count(g_model_service->store_entitlements,sizeof(ModelStoreEntitlement1E0));
    if(entitlement_index>=count)return missing_value();
    const auto* r=reinterpret_cast<const ModelStoreEntitlement1E0*>(g_model_service->store_entitlements.begin);
    return model_string_vector_at(r[entitlement_index].feature_flag,feature_index);
}

bool model_materialize_committed_state(StoreState* state) noexcept {
    if(!g_model_service) return true;
    ReleaseServiceObject548 staged{};
    const auto source=source_from_state(state);
    if(!preloader_l::runtime::materialize_model_service_derived_state_from_store(&staged,source)){
        preloader_l::runtime::release_model_service_derived_state(&staged);
        return false;
    }
    preloader_l::runtime::swap_model_service_derived_state(g_model_service,&staged);
    preloader_l::runtime::release_model_service_derived_state(&staged);
    g_model_store_authoritative=true;
    return true;
}

bool model_clear_authoritative() noexcept {
    if(g_model_service&&g_model_store_authoritative){
        if(!preloader_l::runtime::release_model_service_derived_state_retryable(g_model_service))
            return false;
    }
    g_model_store_authoritative=false;
    return true;
}

StoreRuntimeStatus model_commit_runtime_xml(const char* xml,size_t length) noexcept {
    if(!g_model_service) return set_last_runtime_store_status(StoreRuntimeStatus::resource_error);
    ReleaseServiceObject548 staged{};
    const auto status=stage_runtime_store_xml_to_model_service(xml,length,&staged);
    if(status!=StoreRuntimeStatus::ok){
        preloader_l::runtime::release_model_service_derived_state(&staged);
        return status;
    }
    preloader_l::runtime::swap_model_service_derived_state(g_model_service,&staged);
    preloader_l::runtime::release_model_service_derived_state(&staged);
    g_model_store_authoritative=true;
    return set_last_runtime_store_status(StoreRuntimeStatus::ok);
}
#endif

} // namespace store_entitlement_internal

#if defined(_WIN32)
using namespace store_entitlement_internal;

void bind_runtime_store_model_service(preloader_l::runtime::model_abi::ReleaseServiceObject548* service) noexcept {
    g_model_service=service;
    g_model_store_authoritative=false;
}
void unbind_runtime_store_model_service() noexcept {
    g_model_service=nullptr;
    g_model_store_authoritative=false;
    (void)set_last_runtime_store_status(StoreRuntimeStatus::not_loaded);
}
void note_runtime_store_model_commit(bool authoritative) noexcept {
    g_model_store_authoritative=authoritative;
}
StoreRuntimeStatus stage_runtime_store_xml_to_model_service(
    const char* xml,size_t length,
    preloader_l::runtime::model_abi::ReleaseServiceObject548* candidate_service) noexcept {
    if((!xml&&length)||!candidate_service) return set_last_runtime_store_status(StoreRuntimeStatus::invalid_argument);
    if(!xml||!length) return set_last_runtime_store_status(StoreRuntimeStatus::invalid_xml);
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    const auto allocation=preloader_l::runtime::allocate_block(sizeof(StoreState),
        preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
        preloader_l::runtime::windows_allocator_platform_ops());
    if(allocation.terminal!=preloader_l::runtime::AllocationTerminal::none||!allocation.pointer){
        return set_last_runtime_store_status(StoreRuntimeStatus::resource_error);
    }
    auto* candidate=static_cast<StoreState*>(allocation.pointer);
    *candidate=StoreState{};
#else
    StoreState local{};
    auto* candidate=&local;
#endif
    const ParseCode code=parse_document(*candidate,xml,length);
    const StoreRuntimeStatus parsed=map_status(code);
    if(code!=ParseCode::ok){
        reset_store(*candidate);
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
        preloader_l::runtime::release_block(candidate,sizeof(StoreState),preloader_l::runtime::windows_allocator_platform_ops());
#endif
        return set_last_runtime_store_status(parsed);
    }
    candidate->status=StoreRuntimeStatus::ok;
    const auto source=source_from_state(candidate);
    const bool ok=preloader_l::runtime::materialize_model_service_derived_state_from_store(candidate_service,source);
    reset_store(*candidate);
#if defined(PRELOADER_L_RELEASE_BOOTSTRAP)
    preloader_l::runtime::release_block(candidate,sizeof(StoreState),preloader_l::runtime::windows_allocator_platform_ops());
#endif
    return set_last_runtime_store_status(ok?StoreRuntimeStatus::ok:StoreRuntimeStatus::resource_error);
}
#endif

} }
