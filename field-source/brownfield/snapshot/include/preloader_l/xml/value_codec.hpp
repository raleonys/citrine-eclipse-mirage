#pragma once

#include <stddef.h>
#include <stdint.h>

#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace xml {

enum class ValueCodecStatus : uint8_t {
    ok = 0,
    invalid_argument,
    malformed_entity,
    invalid_character,
    capacity,
    overflow,
    allocation_failed,
};

// Decode XML source text/attribute data into its semantic UTF-8 value.
// Only the five predefined XML entities and numeric character references are
// recognized; custom DTD entities are intentionally unsupported. `written`
// excludes the convenience terminal NUL. A null output with zero capacity is
// a pure size query.
ValueCodecStatus decode_source_value(
    TextSpan source,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

// Escape a semantic UTF-8 value for use inside an XML attribute. All five XML
// specials are escaped exactly once. Malformed UTF-8 and non-XML characters
// are rejected. `written` excludes the terminal NUL.
// A null output with zero capacity is a pure size query.
ValueCodecStatus escape_attribute_value(
    TextSpan semantic,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

// Decode XML-source spelling and immediately escape the resulting semantic
// UTF-8 value for attribute output. Malformed raw UTF-8 is rejected. This is
// the no-allocation decode-once / re-escape-once path used for parser-owned
// source attributes.
ValueCodecStatus normalize_source_attribute_value(
    TextSpan source,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

// Compare raw XML-source spelling against an already-semantic value without
// allocating. Malformed entities are reported through `status`.
bool source_value_equals(
    TextSpan source,
    TextSpan semantic,
    ValueCodecStatus* status = nullptr) noexcept;

bool source_value_equals_literal(
    TextSpan source,
    const char* literal,
    ValueCodecStatus* status = nullptr) noexcept;

// Small owning adapter used only when a downstream API requires a contiguous
// semantic span rather than a streaming comparison. Valid plain source values
// alias the caller buffer; encoded values are decoded into owned scratch storage.
class SemanticValue {
public:
    SemanticValue() noexcept;
    ~SemanticValue() noexcept;
    SemanticValue(const SemanticValue&) = delete;
    SemanticValue& operator=(const SemanticValue&) = delete;

    ValueCodecStatus assign_source(TextSpan source) noexcept; // Source-only API, rejects parsed spans.
    ValueCodecStatus assign_decoded(TextSpan semantic) noexcept;
    ValueCodecStatus assign_parsed(TextSpan value) noexcept; // Dispatches by representation.
    TextSpan view() const noexcept;
    void clear() noexcept;

private:
    const char* data_;
    size_t size_;
    char* owned_;
    std::shared_ptr<const std::string> semantic_backing_;
    std::shared_ptr<const tinyxml2::XMLDocument> semantic_document_;
#if defined(_WIN32)
    void* heap_;
#endif
};

} } // namespace preloader_l::xml
