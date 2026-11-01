#pragma once
#include "preloader_l/config/store_runtime.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace config { namespace store_entitlement_internal {

constexpr size_t kModelMaxDepth = 500u;

struct Span {
    size_t off;
    size_t len;
    bool used;
};

struct FeatureRange {
    uint32_t start;
    uint32_t count;
};

struct Record {
    uint64_t numeric_id;
    uint64_t offer_id;
    uint64_t release_ms;
    uint64_t flags;
    uint32_t category;
    Span text[10];
    FeatureRange feature_desc;
    FeatureRange feature_flag;
    Span bytes[3];
};

struct StoreState {
    StoreRuntimeStatus status;
    bool present;
    size_t record_count;
    size_t record_capacity;
    size_t feature_desc_count;
    size_t feature_desc_capacity;
    size_t feature_flag_count;
    size_t feature_flag_capacity;
    size_t arena_used;
    size_t arena_capacity;
    Span preamble;
    Span trailer;
    Record* records;
    Span* feature_descs;
    Span* feature_flags;
    uint8_t* arena;
};

enum class ParseCode : uint8_t {
    ok = 0,
    invalid,
    resource,
    invalid_hex,
    number_range,
};

StoreRuntimeStatus set_last_runtime_store_status(StoreRuntimeStatus status) noexcept;


void* transient_alloc(size_t bytes) noexcept;
void transient_free(void* ptr, size_t bytes) noexcept;
bool grow_capacity(size_t current, size_t required, size_t* next) noexcept;

template<class T>
bool reserve_array(T** data,size_t* capacity,size_t used,size_t required) noexcept {
    if(!data||!capacity) return false;
    if(required<=*capacity) return true;
    size_t next=0;
    if(!grow_capacity(*capacity,required,&next)||
       next>static_cast<size_t>(-1)/sizeof(T)) return false;
    auto* fresh=static_cast<T*>(transient_alloc(next*sizeof(T)));
    if(!fresh) return false;
    for(size_t i=0;i<used;++i) fresh[i]=(*data)[i];
    transient_free(*data,*capacity*sizeof(T));
    *data=fresh;
    *capacity=next;
    return true;
}

bool reserve_arena(StoreState& st,size_t required) noexcept;
void reset_store(StoreState& state) noexcept;
bool arena_raw(StoreState& state,const void* source,size_t length,size_t* offset) noexcept;
bool arena_byte(StoreState& state,uint8_t value) noexcept;
bool append_utf8(StoreState& state,uint32_t cp) noexcept;

// The store is materialized from TinyXML2's semantic DOM, never scanned as raw XML.
ParseCode parse_u64_span(StoreState& state, Span value, uint64_t* output) noexcept;
ParseCode decode_quartets(StoreState& state,Span text,Span* output) noexcept;
ParseCode decode_hex(StoreState& state,Span text,Span* output) noexcept;
ParseCode parse_document(StoreState& state,const char* s,size_t n) noexcept;
StoreRuntimeStatus map_status(ParseCode code) noexcept;
ValueView view_text(const StoreState& state,Span span) noexcept;
StoreByteView view_bytes(const StoreState& state,Span span) noexcept;
bool state_loaded(void* context) noexcept;
bool state_present(void* context) noexcept;
size_t state_count(void* context) noexcept;
bool state_entitlement(void* context,size_t index,StoreEntitlementView* output) noexcept;
StoreByteView state_preamble(void* context) noexcept;
StoreByteView state_trailer(void* context) noexcept;
ValueView state_feature_desc(void* context,size_t entitlement_index,size_t feature_index) noexcept;
ValueView state_feature_flag(void* context,size_t entitlement_index,size_t feature_index) noexcept;
StoreMaterializationSource source_from_state(StoreState* st) noexcept;

#if defined(_WIN32)
StoreRuntimeStatus model_commit_runtime_xml(const char* xml,size_t length) noexcept;
bool model_materialize_committed_state(StoreState* state) noexcept;
bool model_clear_authoritative() noexcept;
bool model_runtime_loaded() noexcept;
bool model_runtime_present() noexcept;
size_t model_runtime_entitlement_count() noexcept;
bool model_runtime_entitlement(size_t index,StoreEntitlementView* output) noexcept;
StoreByteView model_runtime_preamble() noexcept;
StoreByteView model_runtime_trailer() noexcept;
ValueView model_runtime_feature_desc(size_t entitlement_index,size_t feature_index) noexcept;
ValueView model_runtime_feature_flag(size_t entitlement_index,size_t feature_index) noexcept;
#endif

} } }
