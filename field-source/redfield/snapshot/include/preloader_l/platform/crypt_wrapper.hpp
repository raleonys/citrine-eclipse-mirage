// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

enum class CurioCryptObjectDecision : uint32_t {
    pass_original = 0,
    curio_companion_branch = 1,
};

struct CurioCryptForwardPlan {
    CurioCryptObjectDecision decision;
    const wchar_t* effective_object;
};

// Curio version-side embedded preloader_l.dll RVA 0x1264 branch predicate for valid UTF-16 file paths.
// Oracle semantics:
//   * dwObjectType != 1 -> pass original object
//   * dwObjectType == 1 && pvObject == nullptr -> special companion branch
//   * otherwise UTF-16 is converted to UTF-8 and the final 15 bytes are
//     compared case-sensitively with "preloader_l.dll" (memcmp).
//
// This function only classifies the branch. It never substitutes a live
// CryptQueryObject target and never changes Windows trust results.
CurioCryptObjectDecision classify_curio_crypt_object(
    uint32_t object_type,
    const wchar_t* object_path,
    size_t object_chars) noexcept;

// Exact observable argument-side effect of curio wrapper RVA 0x1264.
// For the special branch only pvObject/RDX is replaced with the companion
// filename. Other register arguments and caller-stack arguments are forwarded
// unchanged by the original tail jump. The returned pointer is either the
// caller's original object pointer or curio_crypt_companion_name.
CurioCryptForwardPlan curio_crypt_forward_plan(
    uint32_t object_type,
    const wchar_t* object_path,
    size_t object_chars) noexcept;

inline constexpr char curio_crypt_target_suffix_ascii[] = "preloader_l.dll";
// Curio oracle data string retained for static consistency/evidence only.
inline constexpr wchar_t curio_crypt_companion_name[] = L"EAAntiCheat.GameServiceLauncher.dll";
inline constexpr uint32_t curio_crypt_wrapper_rva = 0x00001264u;
inline constexpr uint32_t curio_crypt_suffix_rva = 0x000052C0u;
inline constexpr uint32_t curio_crypt_companion_rva = 0x000052D0u;
inline constexpr uint32_t curio_crypt_iat_rva = 0x00005000u;
inline constexpr uint32_t curio_crypt_original_slot_rva = 0x000072D0u;

} }
