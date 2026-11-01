#pragma once
#include "preloader_l/runtime/service_abi.hpp"
#include "preloader_l/config/store_runtime.hpp"

namespace preloader_l { namespace runtime {

// Materializes the parts of the curio +0x10 service object whose physical
// representation and producer data are independently curio. also
// materializes the byte-pinned 0x1E0 entitlement record vector at +0x4E8. The
// TinyXML2 document/settings map remains a separate 7000 policy gate.
bool materialize_model_service_derived_state_from_store(
    model_abi::ReleaseServiceObject548* service,
    const preloader_l::config::StoreMaterializationSource& source) noexcept;
void swap_model_service_derived_state(
    model_abi::ReleaseServiceObject548* live,
    model_abi::ReleaseServiceObject548* candidate) noexcept;
bool release_model_service_derived_state_retryable(
    model_abi::ReleaseServiceObject548* service) noexcept;
void release_model_service_derived_state(
    model_abi::ReleaseServiceObject548* service) noexcept;

} }
