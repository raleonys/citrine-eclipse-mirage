// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>
#include "redfield/xml/request_parser.hpp"
namespace redfield { namespace xml {
enum class AttributeStatus : uint8_t { found=0, not_found, malformed, invalid_argument };
AttributeStatus find_attribute(TextSpan opening_tag, const char* name, TextSpan* value) noexcept;
} }
