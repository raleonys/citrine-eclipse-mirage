#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Source-level MSVC-compatible owner header: compiler-emitted vptr at +0x00,
// strong count at +0x08, weak count at +0x0C. Four virtual declarations are
// ordered to mirror the curio 0x1080C8 table slots.
// RTTI remains disabled in the normalized build (/GR-), matching the absence of
// a usable RTTI COL on the curio owner-control vftable.
struct ReferenceOwnerControl {
    ReferenceOwnerControl() noexcept : strong_count(1), weak_count(1) {}
    ReferenceOwnerControl(const ReferenceOwnerControl&) = delete;
    ReferenceOwnerControl& operator=(const ReferenceOwnerControl&) = delete;

    virtual void destroy() noexcept = 0;                                  // +0x00
    virtual void delete_this() noexcept = 0;                              // +0x08
    virtual ReferenceOwnerControl* scalar_delete(uint32_t flags) noexcept = 0; // +0x10
    virtual void* null_query(const void* type_token) noexcept = 0;        // +0x18

    volatile int32_t strong_count;                                        // +0x08
    volatile int32_t weak_count;                                          // +0x0C

protected:
    ~ReferenceOwnerControl() = default;
};

struct ReferenceAliasOwner {
    void* alias;
    ReferenceOwnerControl* control;
};

struct ReferenceOwnerSnapshot {
    int32_t strong_count;
    int32_t weak_count;
};

static_assert(sizeof(ReferenceOwnerControl) == 0x10, "owner control ABI drift");
static_assert(sizeof(ReferenceAliasOwner) == 0x10, "alias-owner pair ABI drift");

void reference_owner_add_strong(ReferenceOwnerControl* control) noexcept;
void reference_owner_release_strong(ReferenceOwnerControl* control) noexcept;
void reference_owner_move_publish(ReferenceAliasOwner* destination,
                                  ReferenceAliasOwner* source) noexcept;
bool reference_owner_copy_acquire(const ReferenceAliasOwner* source,
                                  ReferenceAliasOwner* destination) noexcept;
void reference_owner_release_alias(ReferenceAliasOwner* owner) noexcept;
ReferenceOwnerSnapshot reference_owner_snapshot(const ReferenceOwnerControl* control) noexcept;

} }
