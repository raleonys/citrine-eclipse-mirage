// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/platform/crypt_wrapper.hpp"

namespace redfield { namespace platform {
namespace {
bool ends_with_target_case_sensitive(const wchar_t* path, size_t chars) noexcept {
    // For a valid UTF-16 Windows path the curio wrapper first converts the
    // full string to UTF-8 and then memcmp()s the final 15 ASCII bytes. Because
    // the suffix is pure ASCII, a case-sensitive wchar_t suffix comparison is
    // branch-equivalent for valid UTF-16 input while avoiding allocation here.
    static constexpr wchar_t suffix[] = L"preloader_l.dll";
    constexpr size_t suffix_chars = (sizeof(suffix) / sizeof(suffix[0])) - 1u;
    if (!path || chars < suffix_chars) return false;
    const size_t start = chars - suffix_chars;
    for (size_t i = 0; i < suffix_chars; ++i) {
        if (path[start + i] != suffix[i]) return false;
    }
    return true;
}
}

CurioCryptObjectDecision classify_curio_crypt_object(
    uint32_t object_type,
    const wchar_t* object_path,
    size_t object_chars) noexcept {
    if (object_type != 1u) return CurioCryptObjectDecision::pass_original;
    if (!object_path) return CurioCryptObjectDecision::curio_companion_branch;
    return ends_with_target_case_sensitive(object_path, object_chars)
        ? CurioCryptObjectDecision::curio_companion_branch
        : CurioCryptObjectDecision::pass_original;
}

CurioCryptForwardPlan curio_crypt_forward_plan(
    uint32_t object_type,
    const wchar_t* object_path,
    size_t object_chars) noexcept {
    const auto decision = classify_curio_crypt_object(
        object_type, object_path, object_chars);
    return CurioCryptForwardPlan{
        decision,
        decision == CurioCryptObjectDecision::curio_companion_branch
            ? curio_crypt_companion_name
            : object_path
    };
}

} }
