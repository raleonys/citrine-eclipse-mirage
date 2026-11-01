// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "stream_response_support.hpp"
#include "redfield/runtime/record_views.hpp"
namespace redfield { namespace {
// Common validated measure/render contract for immutable record snapshots.
struct RecordStreamWriter : StreamWriter {
    using StreamWriter::StreamWriter;
    using StreamWriter::append_semantic;
    void append_semantic(runtime::ServiceTextView value) noexcept {
        StreamWriter::append_semantic({value.data,value.size});
    }
    bool finish(size_t* written) noexcept {
        if (!out) { if(written)*written=pos;return valid && written; }
        return StreamWriter::finish(written);
    }
};
bool safe_service_text(runtime::ServiceTextView value) noexcept {
    return safe_semantic_attr_value(xml::TextSpan{value.data,value.size});
}
} }
