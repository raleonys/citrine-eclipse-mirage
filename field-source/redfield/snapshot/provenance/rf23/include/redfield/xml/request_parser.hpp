// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <string>

namespace tinyxml2 { class XMLDocument; class XMLElement; }

namespace redfield { namespace xml {

// A source span still needs entity decoding. A decoded span must NEVER be decoded
// again: e.g. `&amp;lt;` has semantic value `&lt;`, not `<`.
enum class TextRepresentation : uint8_t { xml_source=0, decoded_semantic=1 };
// Immutable after publication. The child index is not a second parser: it
// holds pointers into the single owning TinyXML2 XMLDocument.
// Fixed-count allocation avoids an unrelated MSVC STL length-error import.
struct DirectChildIndex {
    std::unique_ptr<const tinyxml2::XMLElement*[]> nodes;
    size_t count=0;
    bool valid_trivia=false; // computed once from immutable owned DOM
};

struct TextSpan {
    const char* data;
    size_t size;
    std::shared_ptr<const std::string> backing;
    std::shared_ptr<const tinyxml2::XMLDocument> document;
    const tinyxml2::XMLElement* element;
    std::shared_ptr<const DirectChildIndex> direct_children;
    TextRepresentation representation;
    TextSpan(const char* p=nullptr,size_t n=0) noexcept
       : data(p),size(n),element(nullptr),representation(TextRepresentation::xml_source) {}
    TextSpan(const char* p,size_t n,std::shared_ptr<const std::string> owner) noexcept
       : data(p),size(n),backing(std::move(owner)),element(nullptr),representation(TextRepresentation::xml_source) {}
    TextSpan(const char* p,size_t n,std::shared_ptr<const tinyxml2::XMLDocument> owner,
             const tinyxml2::XMLElement* node=nullptr) noexcept
       : data(p),size(n),document(std::move(owner)),element(node),representation(TextRepresentation::decoded_semantic) {}
    bool decoded() const noexcept { return representation==TextRepresentation::decoded_semantic; }
};

enum class RequestStatus : uint8_t {
    ok = 0,
    invalid_argument,
    incomplete_frame,
    malformed,
    no_command,
};

struct RequestView {
    TextSpan command;
    TextSpan id;
    TextSpan sender;
    TextSpan recipient;
    TextSpan command_open_tag;
    // DOM node identity and owning document survive RequestView copies.
    // Content markup is exposed only to preserve legacy route-local text policies.
    TextSpan command_content;
    bool has_request_wrapper;
};

// Parse the envelope once with TinyXML2. Document-backed spans retain lifetime
// ownership and semantic attributes carry an explicit decoded type.
RequestStatus parse_request_view(const char* input, size_t length, RequestView* out) noexcept;

bool span_equals(TextSpan span, const char* literal) noexcept;

// Behavioral response-envelope policy: Request recipient becomes Response sender.
// Compatibility requests without recipient retain the normalized EbisuSDK fallback.
TextSpan response_sender_policy(const RequestView& request) noexcept;

// Direct children are traversed on the original owned DOM. Text is already
// semantically decoded by TinyXML2 and must never be decoded a second time.
enum class ChildTextStatus : uint8_t { found=0, not_found, malformed, invalid_argument };
ChildTextStatus find_direct_child_text(TextSpan command_content, const char* name, TextSpan* value) noexcept;

struct DirectChildView {
    TextSpan name;
    TextSpan opening_tag;
    TextSpan content;
    bool self_closing;
};
enum class DirectChildStatus : uint8_t { found=0, end, malformed, invalid_argument };
// Iterate direct children using TinyXML2. `cursor` is a zero-based element ordinal.
DirectChildStatus next_direct_child(TextSpan command_content,size_t* cursor,DirectChildView* child) noexcept;

} } // namespace redfield::xml
