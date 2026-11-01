// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/lsx_framing.hpp"
#include <string_view>

namespace preloader_l::xml {
using Request = preloader_l::xml::RequestView;
enum class Result { ok, incomplete, malformed, trailing_data, too_large };
inline constexpr std::size_t max_bytes = 1024 * 1024;
// Complete-frame API. Streaming callers must extract one frame themselves.
Result parse(std::string_view frame, Request& output) noexcept;
}
