#pragma once
#include <stdint.h>

namespace preloader_l { namespace platform {

// Narrow compatibility predicate derived from the live original-vs-clean
// comparison. It only permits path restoration for the exact host call shape
// observed at CryptQueryObject; it never changes a non-null object pointer.
bool should_restore_crypt_query_path(uint32_t object_type,
                                     const void* object,
                                     uint32_t expected_content,
                                     uint32_t expected_format,
                                     uint32_t flags,
                                     bool caller_is_main) noexcept;

constexpr uint32_t crypt_query_restore_object_type = 1u;
constexpr uint32_t crypt_query_restore_expected_content = 0x400u;
constexpr uint32_t crypt_query_restore_expected_format = 2u;
constexpr uint32_t crypt_query_restore_flags = 0u;
inline constexpr char crypt_query_restore_module_ascii[] = "preloader_l.dll";
inline constexpr wchar_t crypt_query_restore_module_name[] = L"preloader_l.dll";

} }
