// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <pugixml.hpp>
#include <string>
#include <string_view>

namespace citrine_eclipse_mirage::xml_reader_bridge::detail {
struct Result {
    int code=0;
    std::string diagnostic;
    std::size_t begin=0,end=0;
    bool construct=false;
};
// First-party project grammar/error policy. No TinyXML2 calls or fallback.
Result inspect(std::string_view);
void construct(pugi::xml_document&,std::string_view);
// Empty result means no replacement is necessary.
std::string normalize_newlines(std::string_view);
std::string decode_references(std::string_view);
}
