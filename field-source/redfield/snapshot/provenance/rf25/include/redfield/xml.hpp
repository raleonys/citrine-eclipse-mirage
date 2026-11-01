// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/xml/attribute_parser.hpp"
#include "redfield/xml/lsx_framing.hpp"
#include <string_view>

namespace redfield::xml {
using Request = redfield::xml::RequestView;
enum class Result { ok, incomplete, malformed, trailing_data, too_large };
inline constexpr std::size_t max_bytes = 1024 * 1024;
// Complete-frame API. Streaming callers must extract one frame themselves.
Result parse(std::string_view frame, Request& output) noexcept;
}
