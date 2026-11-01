#include "preloader_l/config/store_vectors.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include "preloader_l/config/store_payload.hpp"

namespace preloader_l { namespace config {
namespace {

ModelStoreVectorStatus map_vector_status(ModelByteVectorStatus status) noexcept {
    switch (status) {
        case ModelByteVectorStatus::ok: return ModelStoreVectorStatus::ok;
        case ModelByteVectorStatus::size_overflow: return ModelStoreVectorStatus::size_overflow;
        case ModelByteVectorStatus::allocation_failed: return ModelStoreVectorStatus::allocation_failed;
        case ModelByteVectorStatus::release_failed: return ModelStoreVectorStatus::release_failed;
        case ModelByteVectorStatus::destination_not_empty:
        case ModelByteVectorStatus::malformed_layout:
            return ModelStoreVectorStatus::malformed_existing_vector;
        case ModelByteVectorStatus::invalid_argument:
        default: return ModelStoreVectorStatus::invalid_argument;
    }
}

ModelStoreVectorStatus release_one(const ModelByteVectorAllocatorBinding& binding,
                                       MsvcByteVectorLayout* value) noexcept {
    return map_vector_status(model_byte_vector_release(binding, value));
}

bool runtime_loaded_cb(void*) noexcept { return runtime_store_loaded(); }
bool runtime_present_cb(void*) noexcept { return runtime_store_present(); }
size_t runtime_count_cb(void*) noexcept { return runtime_store_entitlement_count(); }
bool runtime_entitlement_cb(void*,size_t i,StoreEntitlementView* out) noexcept { return runtime_store_entitlement(i,out); }
StoreByteView runtime_preamble_cb(void*) noexcept { return runtime_store_preamble(); }
StoreByteView runtime_trailer_cb(void*) noexcept { return runtime_store_trailer(); }
ValueView runtime_desc_cb(void*,size_t i,size_t j) noexcept { return runtime_store_feature_desc(i,j); }
ValueView runtime_flag_cb(void*,size_t i,size_t j) noexcept { return runtime_store_feature_flag(i,j); }
StoreMaterializationSource runtime_source() noexcept {
    return StoreMaterializationSource{nullptr,&runtime_loaded_cb,&runtime_present_cb,&runtime_count_cb,
        &runtime_entitlement_cb,&runtime_preamble_cb,&runtime_trailer_cb,&runtime_desc_cb,&runtime_flag_cb};
}

ModelStoreVectorStatus materialize_impl(const ModelStoreVectorBindings& bindings,
                                            const StoreMaterializationSource& source,
                                            ModelStoreVectorSet* output) {
    if (!output || !source.loaded || !source.entitlement_count || !source.preamble || !source.trailer)
        return ModelStoreVectorStatus::invalid_argument;
    if (!model_byte_vector_empty(output->preamble) ||
        !model_byte_vector_empty(output->trailer) ||
        !model_byte_vector_empty(output->payload))
        return ModelStoreVectorStatus::malformed_existing_vector;
    if (!source.loaded(source.context)) return ModelStoreVectorStatus::store_not_loaded;
    if (source.entitlement_count(source.context) == 0u) return ModelStoreVectorStatus::no_records;

    ModelStoreVectorSet temporary{};
    try {
        const StoreByteView preamble = source.preamble(source.context);
        const StoreByteView trailer = source.trailer(source.context);
        auto status = model_byte_vector_copy_exact(preamble.data,preamble.size,bindings.ae00_aux,&temporary.preamble);
        if(status!=ModelByteVectorStatus::ok){(void)release_model_store_vectors(bindings,&temporary);return map_vector_status(status);}
        status = model_byte_vector_copy_exact(trailer.data,trailer.size,bindings.ae00_aux,&temporary.trailer);
        if(status!=ModelByteVectorStatus::ok){(void)release_model_store_vectors(bindings,&temporary);return map_vector_status(status);}

        size_t payload_size=0u;
        const auto size_status=store_payload_size_from_source(source,&payload_size);
        if(size_status==StorePayloadStatus::not_loaded){(void)release_model_store_vectors(bindings,&temporary);return ModelStoreVectorStatus::store_not_loaded;}
        if(size_status==StorePayloadStatus::no_records){(void)release_model_store_vectors(bindings,&temporary);return ModelStoreVectorStatus::no_records;}
        if(size_status==StorePayloadStatus::size_overflow){(void)release_model_store_vectors(bindings,&temporary);return ModelStoreVectorStatus::size_overflow;}
        if(size_status!=StorePayloadStatus::ok){(void)release_model_store_vectors(bindings,&temporary);return ModelStoreVectorStatus::payload_build_failed;}
        status=model_byte_vector_allocate_exact(payload_size,bindings.d5f0_payload,&temporary.payload);
        if(status!=ModelByteVectorStatus::ok){(void)release_model_store_vectors(bindings,&temporary);return map_vector_status(status);}
        size_t written=0u;
        const auto build_status=build_store_payload_from_source(source,temporary.payload.begin,payload_size,&written);
        if(build_status!=StorePayloadStatus::ok||written!=payload_size){(void)release_model_store_vectors(bindings,&temporary);return ModelStoreVectorStatus::payload_build_failed;}
        temporary.payload.end=temporary.payload.begin+written;
        *output=temporary;
        return ModelStoreVectorStatus::ok;
    } catch (...) {
        (void)release_model_store_vectors(bindings,&temporary);
        throw;
    }
}

}

ModelStoreVectorStatus release_model_store_vectors(const ModelStoreVectorBindings& bindings,
                                                            ModelStoreVectorSet* value) noexcept {
    if(!value)return ModelStoreVectorStatus::invalid_argument;
    ModelStoreVectorStatus first=ModelStoreVectorStatus::ok;
    const auto payload=release_one(bindings.d5f0_payload,&value->payload);
    const auto trailer=release_one(bindings.ae00_aux,&value->trailer);
    const auto preamble=release_one(bindings.ae00_aux,&value->preamble);
    if(payload!=ModelStoreVectorStatus::ok)first=payload;
    else if(trailer!=ModelStoreVectorStatus::ok)first=trailer;
    else if(preamble!=ModelStoreVectorStatus::ok)first=preamble;
    return first;
}

ModelStoreVectorStatus materialize_model_store_vectors(const ModelStoreVectorBindings& bindings,
                                                               ModelStoreVectorSet* output) {
    const auto source=runtime_source();
    return materialize_impl(bindings,source,output);
}

ModelStoreVectorStatus materialize_model_store_vectors_from_source(const ModelStoreVectorBindings& bindings,
                                                                           const StoreMaterializationSource& source,
                                                                           ModelStoreVectorSet* output) {
    return materialize_impl(bindings,source,output);
}

} }
