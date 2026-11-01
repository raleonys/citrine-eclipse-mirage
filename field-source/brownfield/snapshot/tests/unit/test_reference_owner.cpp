#include "preloader_l/runtime/reference_owner.hpp"
#include <stdint.h>

using namespace preloader_l::runtime;

namespace {
uint32_t g_destroy = 0;
uint32_t g_delete_this = 0;
uint32_t g_scalar = 0;
uint32_t g_last_scalar_flags = 0;

struct TestControl final : ReferenceOwnerControl {
    void destroy() noexcept override { ++g_destroy; }
    void delete_this() noexcept override {
        ++g_delete_this;
        (void)scalar_delete(1u);
    }
    ReferenceOwnerControl* scalar_delete(uint32_t flags) noexcept override {
        ++g_scalar;
        g_last_scalar_flags = flags;
        return this;
    }
    void* null_query(const void*) noexcept override { return nullptr; }
};
static_assert(sizeof(TestControl) == 0x10, "test control header must remain 0x10");
}

int main()
{
    TestControl c1{};
    TestControl c2{};
    if (c1.null_query(nullptr) != nullptr) return 12;

    auto s1 = reference_owner_snapshot(&c1);
    if (s1.strong_count != 1 || s1.weak_count != 1) return 1;

    int a = 1, b = 2;
    ReferenceAliasOwner published{&a, &c1};
    ReferenceAliasOwner incoming{&b, &c2};

    reference_owner_move_publish(&published, &incoming);
    if (incoming.alias || incoming.control) return 2;
    if (published.alias != &b || published.control != &c2) return 3;
    if (g_destroy != 1 || g_delete_this != 1 || g_scalar != 1 || g_last_scalar_flags != 1) return 4;
    s1 = reference_owner_snapshot(&c1);
    if (s1.strong_count != 0 || s1.weak_count != 0) return 5;

    ReferenceAliasOwner lease{};
    if (!reference_owner_copy_acquire(&published, &lease)) return 6;
    auto s2 = reference_owner_snapshot(&c2);
    if (s2.strong_count != 2 || s2.weak_count != 1) return 7;
    reference_owner_release_alias(&lease);
    s2 = reference_owner_snapshot(&c2);
    if (s2.strong_count != 1 || s2.weak_count != 1) return 8;
    if (g_destroy != 1 || g_delete_this != 1 || g_scalar != 1) return 9;

    reference_owner_release_alias(&published);
    s2 = reference_owner_snapshot(&c2);
    if (s2.strong_count != 0 || s2.weak_count != 0) return 10;
    if (g_destroy != 2 || g_delete_this != 2 || g_scalar != 2 || g_last_scalar_flags != 1) return 11;
    return 0;
}
