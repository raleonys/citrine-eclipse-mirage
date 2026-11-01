// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/xml/request_parser.hpp"
#include "../../third_party/tinyxml2/tinyxml2.h"
#include <cstring>
#include <memory>

namespace preloader_l { namespace xml { namespace dom {

// Do not let allocation exceptions cross the service's noexcept boundary.
inline bool parse_document(const char* input, size_t n, tinyxml2::XMLDocument* doc) noexcept {
    if (!doc || (!input && n) || n > static_cast<size_t>(-1)-1 ||
        (n && std::memchr(input,'\0',n))) return false;
    try { return doc->Parse(input ? input : "", n)==tinyxml2::XML_SUCCESS; }
    catch (...) { doc->Clear(); return false; }
}

inline bool decoded(const char* text,
                    const std::shared_ptr<const tinyxml2::XMLDocument>& doc,
                    TextSpan* out,const tinyxml2::XMLElement* element=nullptr) noexcept {
    if (!out) return false;
    *out=TextSpan{text,text ? std::strlen(text):0,doc,element};
    return true;
}

inline bool whitespace(const char* data) noexcept {
    if (!data) return true;
    for (;*data;++data) if (*data!=' ' && *data!='\t' && *data!='\n' && *data!='\r') return false;
    return true;
}

// One traversal on the owning immutable DOM; no second XML grammar engine.
// The returned index records the exact child set and its validation result.
inline std::shared_ptr<const DirectChildIndex> child_index(const tinyxml2::XMLElement* parent) {
    auto index=std::make_shared<DirectChildIndex>();
    if (!parent) return index;
    size_t count=0;
    bool valid=true;
    for (auto* child=parent->FirstChild();child;child=child->NextSibling()) {
        if (const auto* text=child->ToText()) {
            if (!whitespace(text->Value())) valid=false;
        } else if (child->ToElement()) {
            if (count>=static_cast<size_t>(-1)/sizeof(const tinyxml2::XMLElement*))
                throw std::bad_alloc();
            ++count;
        } else if (!child->ToComment()) {
            valid=false;
        }
    }
    if (count) index->nodes.reset(new const tinyxml2::XMLElement*[count]);
    index->count=count;
    index->valid_trivia=valid;
    size_t i=0;
    for (const auto* child=parent->FirstChildElement();child;child=child->NextSiblingElement())
        index->nodes[i++]=child;
    return index;
}

} } }
