// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace xml {
enum class AttributeStatus : uint8_t { found=0, not_found, malformed, invalid_argument };
AttributeStatus find_attribute(TextSpan opening_tag, const char* name, TextSpan* value) noexcept;
} }
