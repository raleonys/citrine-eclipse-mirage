// SPDX-License-Identifier: GPL-3.0-only
#include "value_codec_internal.hpp"
#include "preloader_l/core/checked_math.hpp"
#include "preloader_l/runtime/transient_allocator.hpp"

#if defined(_WIN32)
extern "C" {
__declspec(dllimport) void* __stdcall GetProcessHeap();
__declspec(dllimport) void* __stdcall HeapAlloc(void*, unsigned long, size_t);
__declspec(dllimport) int __stdcall HeapFree(void*, unsigned long, void*);
}
#else
#include <stdlib.h>
#endif

namespace preloader_l { namespace xml {
using namespace value_codec_detail;
namespace {
#if defined(_WIN32)
void* transient_heap_alloc(void* context,size_t n) noexcept {
    return context ? HeapAlloc(context,0,n) : nullptr;
}
int transient_heap_free(void* context,void* p) noexcept {
    return context && HeapFree(context,0,p) ? 1 : 0;
}
#endif
}

SemanticValue::SemanticValue() noexcept
    : data_(nullptr), size_(0), owned_(nullptr)
#if defined(_WIN32)
    , heap_(nullptr)
#endif
{}

SemanticValue::~SemanticValue() noexcept {
    clear();
}

void SemanticValue::clear() noexcept {
    if (owned_) {
#if defined(_WIN32)
        (void)preloader_l::runtime::release_transient_bytes(owned_);
        heap_ = nullptr;
#else
        free(owned_);
#endif
    }
    data_ = nullptr;
    size_ = 0;
    owned_ = nullptr;
    semantic_backing_.reset();
    semantic_document_.reset();
}

ValueCodecStatus SemanticValue::assign_source(TextSpan source) noexcept {
    // S0291: Never invalidate an owned source span before consuming it.
    // Full self-reassignment is idempotent; partial/overlapping aliases
    // fail closed while retaining the original valid semantic value.
    if (owned_ && source.data) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(owned_);
        const uintptr_t address = reinterpret_cast<uintptr_t>(source.data);
        if ((address >= base && (address - base) <= size_) ||
            (address < base && (base - address) < source.size)) {
            if (source.data == data_ && source.size == size_)
                return ValueCodecStatus::ok;
            return ValueCodecStatus::invalid_argument;
        }
    }
    if (source.decoded()) return ValueCodecStatus::invalid_argument;
    clear();
    if (!valid_span(source)) return ValueCodecStatus::invalid_argument;

    // F319-P111-001: raw aliases cross the same UTF-8/XML-character validation
    // boundary as entity-bearing values. Preserve zero-allocation aliasing only
    // after a successful size/validation pass.
    size_t required = 0;
    ValueCodecStatus st = decode_source_value(source, nullptr, 0, &required);
    if (st != ValueCodecStatus::ok) return st;

    bool encoded = false;
    for (size_t i = 0; i < source.size; ++i) {
        if (source.data[i] == '&') { encoded = true; break; }
    }
    if (!encoded) {
        data_ = source.data;
        size_ = source.size;
        return ValueCodecStatus::ok;
    }
    size_t capacity = 0;
    if (!preloader_l::core::checked_add_size(required, 1u, capacity))
        return ValueCodecStatus::overflow;

#if defined(_WIN32)
    heap_ = GetProcessHeap();
    if (!heap_) return ValueCodecStatus::allocation_failed;
    const preloader_l::runtime::TransientPlatformOps ops{heap_,&transient_heap_alloc,&transient_heap_free};
    owned_ = static_cast<char*>(preloader_l::runtime::allocate_transient_bytes(capacity,ops));
#else
    owned_ = static_cast<char*>(malloc(capacity));
#endif
    if (!owned_) {
#if defined(_WIN32)
        heap_ = nullptr;
#endif
        return ValueCodecStatus::allocation_failed;
    }
    size_t actual = 0;
    st = decode_source_value(source, owned_, capacity, &actual);
    if (st != ValueCodecStatus::ok) {
        clear();
        return st;
    }
    data_ = owned_;
    size_ = actual;
    return ValueCodecStatus::ok;
}

ValueCodecStatus SemanticValue::assign_decoded(TextSpan semantic) noexcept {
    if (!semantic.decoded() || !valid_span(semantic)) return ValueCodecStatus::invalid_argument;
    // An owned semantic buffer must not be destroyed before a self-aliasing
    // parsed span is consumed (same lifetime rule as assign_source).
    if (owned_ && semantic.data) {
        const uintptr_t base=reinterpret_cast<uintptr_t>(owned_);
        const uintptr_t address=reinterpret_cast<uintptr_t>(semantic.data);
        if ((address>=base&&(address-base)<=size_) ||
            (address<base&&(base-address)<semantic.size)) {
            if (semantic.data==data_ && semantic.size==size_) return ValueCodecStatus::ok;
            return ValueCodecStatus::invalid_argument;
        }
    }
    size_t validated=0;
    const auto rc=decode_source_value(semantic,nullptr,0,&validated);
    if (rc!=ValueCodecStatus::ok) return rc;
    clear();
    data_=semantic.data;
    size_=semantic.size;
    semantic_backing_=semantic.backing;
    semantic_document_=semantic.document;
    return ValueCodecStatus::ok;
}
ValueCodecStatus SemanticValue::assign_parsed(TextSpan value) noexcept {
    return value.decoded() ? assign_decoded(value) : assign_source(value);
}
TextSpan SemanticValue::view() const noexcept {
    TextSpan v{data_,size_};
    v.representation=TextRepresentation::decoded_semantic;
    v.backing=semantic_backing_;
    v.document=semantic_document_;
    return v;
}


} }
