#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/runtime/service_abi.hpp"
#include "preloader_l/runtime/reference_owner.hpp"

namespace preloader_l { namespace runtime {

// Compiler-vtable-backed source representation of the curio make_shared-like
// 0x558 coallocation: 0x10 owner header followed immediately by 0x548 service.
// This type is intentionally abstract; the Windows live implementation supplies
// the four curio virtual behaviors without adding data members.
struct ModelServiceControl558 : ReferenceOwnerControl {
    model_abi::ReleaseServiceObject548 service;
};

struct ModelServiceStorageView {
    ReferenceOwnerControl* owner;
    model_abi::ReleaseServiceObject548* service;
    size_t storage_size;
};

enum class ModelServiceStorageStatus : uint8_t {
    ok = 0,
    invalid_argument,
};

static_assert(sizeof(ModelServiceControl558) == 0x558,
              "model 0x558 coallocation ABI drift");

ModelServiceStorageStatus model_service_storage_view(
    ModelServiceControl558* control,
    ModelServiceStorageView* output) noexcept;

} }
