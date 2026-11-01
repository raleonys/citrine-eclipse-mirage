#pragma once

#include "preloader_l/runtime/service_projection.hpp"
#include "preloader_l/config/store_vectors.hpp"
#include "preloader_l/config/byte_vector_binding.hpp"

namespace preloader_l { namespace runtime { namespace model_derived_internal {

using preloader_l::runtime::model_abi::MsvcString32;
using preloader_l::runtime::model_abi::MsvcVector24;
using preloader_l::runtime::model_abi::ModelStoreEntitlement1E0;
using preloader_l::config::StoreMaterializationSource;

void zero_bytes(uint8_t* p, size_t n) noexcept;
void initialize_empty_string(MsvcString32* value) noexcept;
bool release_string(MsvcString32* value) noexcept;
bool assign_string(MsvcString32* value, preloader_l::config::ValueView source) noexcept;
void clear_vector(MsvcVector24* value) noexcept;
preloader_l::config::ModelByteVectorAllocatorBinding vector_binding() noexcept;
preloader_l::config::MsvcByteVectorLayout byte_layout_from_vector(const MsvcVector24& value) noexcept;
void vector_from_byte_layout(MsvcVector24* destination, preloader_l::config::MsvcByteVectorLayout* source) noexcept;
bool copy_byte_vector(MsvcVector24* destination, preloader_l::config::StoreByteView source) noexcept;
bool release_byte_vector(MsvcVector24* value) noexcept;
bool allocate_raw_vector(MsvcVector24* destination, size_t bytes) noexcept;
bool release_string_vector(MsvcVector24* vector) noexcept;
bool materialize_string_vector(MsvcVector24* destination, const StoreMaterializationSource& source,
                               size_t entitlement_index, size_t count, bool description_vector) noexcept;
void initialize_record(ModelStoreEntitlement1E0* record) noexcept;
bool release_record(ModelStoreEntitlement1E0* record) noexcept;
bool materialize_record(ModelStoreEntitlement1E0* destination,
                        const StoreMaterializationSource& source, size_t index,
                        const preloader_l::config::StoreEntitlementView& view) noexcept;
bool release_entitlement_vector(MsvcVector24* vector) noexcept;
bool source_valid(const StoreMaterializationSource& source) noexcept;
bool materialize_entitlement_vector(model_abi::ReleaseServiceObject548* service,
                                    const StoreMaterializationSource& source) noexcept;
preloader_l::config::ModelStoreVectorBindings store_bindings() noexcept;
bool materialize_store_vectors(model_abi::ReleaseServiceObject548* service,
                               const StoreMaterializationSource& source) noexcept;
bool release_store_vectors(model_abi::ReleaseServiceObject548* service) noexcept;

} } }
