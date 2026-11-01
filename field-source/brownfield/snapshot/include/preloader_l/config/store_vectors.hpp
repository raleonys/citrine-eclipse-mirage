#pragma once
#include <stdint.h>
#include "preloader_l/config/byte_vector_binding.hpp"
#include "preloader_l/config/store_runtime.hpp"

namespace preloader_l { namespace config {

struct ModelStoreVectorSet {
    MsvcByteVectorLayout preamble;
    MsvcByteVectorLayout trailer;
    MsvcByteVectorLayout payload;
};

struct ModelStoreVectorBindings {
    ModelByteVectorAllocatorBinding ae00_aux;
    ModelByteVectorAllocatorBinding d5f0_payload;
};

enum class ModelStoreVectorStatus : uint8_t {
    ok = 0,
    invalid_argument,
    store_not_loaded,
    no_records,
    size_overflow,
    allocation_failed,
    malformed_existing_vector,
    payload_build_failed,
    release_failed,
};

ModelStoreVectorStatus materialize_model_store_vectors(
    const ModelStoreVectorBindings& bindings,
    ModelStoreVectorSet* output);
ModelStoreVectorStatus materialize_model_store_vectors_from_source(
    const ModelStoreVectorBindings& bindings,
    const StoreMaterializationSource& source,
    ModelStoreVectorSet* output);

ModelStoreVectorStatus release_model_store_vectors(
    const ModelStoreVectorBindings& bindings,
    ModelStoreVectorSet* value) noexcept;

} }
