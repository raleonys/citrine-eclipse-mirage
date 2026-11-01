// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/platform/crypt_wrapper.hpp"
#include <cassert>
using redfield::platform::CurioCryptObjectDecision;
int main() {
    using redfield::platform::classify_curio_crypt_object;
    using redfield::platform::curio_crypt_forward_plan;
    using redfield::platform::curio_crypt_companion_name;
    assert(classify_curio_crypt_object(1u, nullptr, 0u) == CurioCryptObjectDecision::curio_companion_branch);
    const wchar_t a[] = L"C:\\Game\\preloader_l.dll";
    assert(classify_curio_crypt_object(1u, a, (sizeof(a)/sizeof(a[0]))-1u) == CurioCryptObjectDecision::curio_companion_branch);
    // The oracle uses memcmp on the UTF-8 suffix; it is case-sensitive.
    const wchar_t b[] = L"C:\\Game\\PRELOADER_L.DLL";
    assert(classify_curio_crypt_object(1u, b, (sizeof(b)/sizeof(b[0]))-1u) == CurioCryptObjectDecision::pass_original);
    const wchar_t c[] = L"C:\\Game\\something.dll";
    assert(classify_curio_crypt_object(1u, c, (sizeof(c)/sizeof(c[0]))-1u) == CurioCryptObjectDecision::pass_original);
    const wchar_t d[] = L"C:\\oyun-\u011f\\preloader_l.dll";
    assert(classify_curio_crypt_object(1u, d, (sizeof(d)/sizeof(d[0]))-1u) == CurioCryptObjectDecision::curio_companion_branch);
    const wchar_t e[] = L"C:\\Game\\preloader_l.dll.bak";
    assert(classify_curio_crypt_object(1u, e, (sizeof(e)/sizeof(e[0]))-1u) == CurioCryptObjectDecision::pass_original);
    assert(classify_curio_crypt_object(2u, a, (sizeof(a)/sizeof(a[0]))-1u) == CurioCryptObjectDecision::pass_original);

    const auto null_plan = curio_crypt_forward_plan(1u, nullptr, 0u);
    assert(null_plan.decision == CurioCryptObjectDecision::curio_companion_branch);
    assert(null_plan.effective_object == curio_crypt_companion_name);
    const auto suffix_plan = curio_crypt_forward_plan(1u, a, (sizeof(a)/sizeof(a[0]))-1u);
    assert(suffix_plan.decision == CurioCryptObjectDecision::curio_companion_branch);
    assert(suffix_plan.effective_object == curio_crypt_companion_name);
    const auto pass_plan = curio_crypt_forward_plan(1u, c, (sizeof(c)/sizeof(c[0]))-1u);
    assert(pass_plan.decision == CurioCryptObjectDecision::pass_original);
    assert(pass_plan.effective_object == c);
    const auto other_type_plan = curio_crypt_forward_plan(2u, a, (sizeof(a)/sizeof(a[0]))-1u);
    assert(other_type_plan.decision == CurioCryptObjectDecision::pass_original);
    assert(other_type_plan.effective_object == a);
    return 0;
}
