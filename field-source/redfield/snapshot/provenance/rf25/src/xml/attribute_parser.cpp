// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml/attribute_parser.hpp"
#include "tinyxml2_dom.hpp"
namespace redfield { namespace xml {
AttributeStatus find_attribute(TextSpan tag,const char* key,TextSpan* value) noexcept {
    if (value) *value=TextSpan{};
    if (!value || !key || !*key || (!tag.data&&tag.size)) return AttributeStatus::invalid_argument;
    // No synthetic self-closing-tag fragment, fallback document or second
    // parsing pass: caller must supply an owned, parsed element.
    if (!tag.element || !tag.document) return AttributeStatus::malformed;
    const char* semantic=tag.element->Attribute(key);
    if (!semantic) return AttributeStatus::not_found;
    return dom::decoded(semantic,tag.document,value) ? AttributeStatus::found:AttributeStatus::malformed;
}
} }
