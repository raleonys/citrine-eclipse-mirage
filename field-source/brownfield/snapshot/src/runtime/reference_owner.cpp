#include "preloader_l/runtime/reference_owner.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace {
int32_t atomic_increment(volatile int32_t* value) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedIncrement(reinterpret_cast<volatile long*>(value)));
#else
    return __atomic_add_fetch(value, int32_t{1}, __ATOMIC_ACQ_REL);
#endif
}

int32_t atomic_exchange_add(volatile int32_t* value, int32_t delta) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(value), static_cast<long>(delta)));
#else
    return __atomic_fetch_add(value, delta, __ATOMIC_ACQ_REL);
#endif
}

int32_t atomic_load(const volatile int32_t* value) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedCompareExchange(
        const_cast<volatile long*>(reinterpret_cast<const volatile long*>(value)), 0, 0));
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}
}

namespace preloader_l { namespace runtime {

void reference_owner_add_strong(ReferenceOwnerControl* control) noexcept
{
    if (!control) return;
    (void)atomic_increment(&control->strong_count);
}

void reference_owner_release_strong(ReferenceOwnerControl* control) noexcept
{
    if (!control) return;

    const int32_t previous_strong = atomic_exchange_add(&control->strong_count, -1);
    if (previous_strong != 1) return;

    control->destroy();

    const int32_t previous_weak = atomic_exchange_add(&control->weak_count, -1);
    if (previous_weak == 1)
        control->delete_this();
}

void reference_owner_move_publish(ReferenceAliasOwner* destination,
                                  ReferenceAliasOwner* source) noexcept
{
    if (!destination || !source || destination == source) return;

    ReferenceOwnerControl* old_control = destination->control;
    destination->alias = source->alias;
    destination->control = source->control;
    source->alias = nullptr;
    source->control = nullptr;

    if (old_control) reference_owner_release_strong(old_control);
}

bool reference_owner_copy_acquire(const ReferenceAliasOwner* source,
                                  ReferenceAliasOwner* destination) noexcept
{
    if (!source || !destination || !source->alias || !source->control) return false;
    reference_owner_add_strong(source->control);
    destination->alias = source->alias;
    destination->control = source->control;
    return true;
}

void reference_owner_release_alias(ReferenceAliasOwner* owner) noexcept
{
    if (!owner) return;
    ReferenceOwnerControl* control = owner->control;
    owner->alias = nullptr;
    owner->control = nullptr;
    if (control) reference_owner_release_strong(control);
}

ReferenceOwnerSnapshot reference_owner_snapshot(const ReferenceOwnerControl* control) noexcept
{
    if (!control) return ReferenceOwnerSnapshot{0, 0};
    return ReferenceOwnerSnapshot{
        atomic_load(&control->strong_count),
        atomic_load(&control->weak_count),
    };
}

} }
