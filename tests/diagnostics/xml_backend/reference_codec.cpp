// SPDX-License-Identifier: GPL-3.0-only

// Compile the pinned reference reader once in this test-only translation unit.
// This gives both parsers exactly the same checked value decoder, without
// copying it or exposing a new production API. The source hash is checked by CMake.
#include XML_REFERENCE_READER_SOURCE
#include "backend.hpp"
namespace xml_comparison {
std::string decode_reference_value(std::string_view value) {
    return preloader_l::xml_reader_bridge::decode_references(value);
}
}
