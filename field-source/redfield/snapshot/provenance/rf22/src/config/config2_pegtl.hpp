// SPDX-License-Identifier: GPL-3.0-only
#ifndef REDFIELD_HF12_CONFIG2_PEGTL_HPP
#define REDFIELD_HF12_CONFIG2_PEGTL_HPP
#include <stddef.h>
#include <stdint.h>

// A shared Config2 syntax boundary. Raw quoted spans intentionally preserve
// legacy value escaping; each consumer maintains its own transactional state.
namespace redfield { namespace config2_pegtl {
struct Span { size_t off; size_t len; };
enum class Result : uint8_t { ok=0, invalid, resource };
struct Visitor {
    void* user;
    bool (*on_section)(void*, Span);
    bool (*on_leaf)(void*, const Span*, size_t, Span, Span);
};
Result parse(const char* bytes,size_t length,const Visitor& visitor,
             bool service_legacy_trailing_unclosed_comment=false) noexcept;
} }
#endif
