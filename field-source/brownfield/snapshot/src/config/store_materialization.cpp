#include "store_internal.hpp"
#if !defined(_WIN32)
#include <stdlib.h>
#endif
#if defined(_WIN32)
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#endif

namespace preloader_l { namespace config { namespace store_entitlement_internal {

void* transient_alloc(size_t bytes) noexcept {
    if (!bytes) return nullptr;
#if defined(_WIN32)
    return preloader_l::runtime::allocate_transient_bytes(
        bytes, preloader_l::runtime::windows_allocator_platform_ops());
#else
    return malloc(bytes);
#endif
}
void transient_free(void* ptr,size_t bytes) noexcept {
    if(!ptr)return;
#if defined(_WIN32)
    (void)bytes;
    // Audit3 S0277: release failure transfers physical ownership to the
    // allocator's intrusive pending-owner list before this logical view clears.
    (void)preloader_l::runtime::release_transient_bytes(ptr);
#else
    (void)bytes; free(ptr);
#endif
}
bool grow_capacity(size_t current,size_t required,size_t* next) noexcept {
    if(!next) return false;
    size_t value=current?current:8u;
    while(value<required){
        const size_t grown=value+value/2u+1u;
        if(grown<=value){ value=required; break; }
        value=grown;
    }
    *next=value;
    return value>=required;
}
bool reserve_arena(StoreState& st,size_t required) noexcept {
    if(required<=st.arena_capacity) return true;
    size_t next=0;
    if(!grow_capacity(st.arena_capacity,required,&next)) return false;
    auto* fresh=static_cast<uint8_t*>(transient_alloc(next));
    if(!fresh) return false;
    for(size_t i=0;i<st.arena_used;++i) fresh[i]=st.arena[i];
    transient_free(st.arena,st.arena_capacity);
    st.arena=fresh;
    st.arena_capacity=next;
    return true;
}
void reset_store(StoreState& state) noexcept {
    transient_free(state.records,state.record_capacity*sizeof(Record));
    transient_free(state.feature_descs,state.feature_desc_capacity*sizeof(Span));
    transient_free(state.feature_flags,state.feature_flag_capacity*sizeof(Span));
    transient_free(state.arena,state.arena_capacity);
    state=StoreState{};
}

bool arena_raw(StoreState& state,const void* source, size_t length,
               size_t* offset) noexcept {
    if (!source && length) return false;
    if (length > static_cast<size_t>(-1) - state.arena_used) return false;
    const size_t required=state.arena_used+length;
    if(!reserve_arena(state,required))return false;
    if (offset) *offset = state.arena_used;
    const auto* bytes = static_cast<const uint8_t*>(source);
    for (size_t i = 0; i < length; ++i)
        state.arena[state.arena_used + i] = bytes[i];
    state.arena_used = required;
    return true;
}

bool arena_byte(StoreState& state,uint8_t value) noexcept {
    return arena_raw(state,&value, 1, nullptr);
}

bool append_utf8(StoreState& state,uint32_t cp) noexcept {
    if (cp <= 0x7Fu)
        return arena_byte(state,static_cast<uint8_t>(cp));

    if (cp <= 0x7FFu) {
        const uint8_t bytes[2] = {
            static_cast<uint8_t>(0xC0u | (cp >> 6)),
            static_cast<uint8_t>(0x80u | (cp & 0x3Fu))
        };
        return arena_raw(state,bytes, 2, nullptr);
    }

    if (cp >= 0xD800u && cp <= 0xDFFFu)
        return false;

    if (cp <= 0xFFFFu) {
        const uint8_t bytes[3] = {
            static_cast<uint8_t>(0xE0u | (cp >> 12)),
            static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu)),
            static_cast<uint8_t>(0x80u | (cp & 0x3Fu))
        };
        return arena_raw(state,bytes, 3, nullptr);
    }

    if (cp <= 0x10FFFFu) {
        const uint8_t bytes[4] = {
            static_cast<uint8_t>(0xF0u | (cp >> 18)),
            static_cast<uint8_t>(0x80u | ((cp >> 12) & 0x3Fu)),
            static_cast<uint8_t>(0x80u | ((cp >> 6) & 0x3Fu)),
            static_cast<uint8_t>(0x80u | (cp & 0x3Fu))
        };
        return arena_raw(state,bytes, 4, nullptr);
    }

    return false;
}
StoreRuntimeStatus map_status(ParseCode code) noexcept {
    switch (code) {
        case ParseCode::ok:
            return StoreRuntimeStatus::ok;
        case ParseCode::resource:
            return StoreRuntimeStatus::resource_error;
        case ParseCode::invalid_hex:
            return StoreRuntimeStatus::invalid_hex;
        case ParseCode::number_range:
            return StoreRuntimeStatus::number_range;
        default:
            return StoreRuntimeStatus::invalid_xml;
    }
}

ValueView view_text(const StoreState& state, Span span) noexcept {
    static const char empty[] = "";
    if (!span.used)
        return ValueView{empty, 0, true};
    return ValueView{
        reinterpret_cast<const char*>(state.arena + span.off),
        span.len,
        true
    };
}

StoreByteView view_bytes(const StoreState& state, Span span) noexcept {
    if (!span.used || span.len == 0)
        return StoreByteView{nullptr, 0};
    return StoreByteView{state.arena + span.off, span.len};
}
bool state_loaded(void* context) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    return st && st->status == StoreRuntimeStatus::ok;
}
bool state_present(void* context) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    return st && st->status == StoreRuntimeStatus::ok && st->present;
}
size_t state_count(void* context) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    return st && st->status == StoreRuntimeStatus::ok ? st->record_count : 0u;
}
bool state_entitlement(void* context, size_t index, StoreEntitlementView* output) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    if (!st || !output || st->status != StoreRuntimeStatus::ok || index >= st->record_count) return false;
    const Record& record = st->records[index];
    StoreEntitlementView value{};
    value.numeric_id = record.numeric_id;
    value.offer_id = record.offer_id;
    value.release_ms = record.release_ms;
    value.flags = record.flags;
    value.category = record.category;
    value.tag = view_text(*st, record.text[0]);
    value.icon = view_text(*st, record.text[1]);
    value.title = view_text(*st, record.text[2]);
    value.description = view_text(*st, record.text[3]);
    value.img_thumb = view_text(*st, record.text[4]);
    value.img_cropped = view_text(*st, record.text[5]);
    value.img_hero = view_text(*st, record.text[6]);
    value.vendor = view_text(*st, record.text[7]);
    value.key = view_text(*st, record.text[8]);
    value.hash = view_text(*st, record.text[9]);
    value.feature_desc_count = record.feature_desc.count;
    value.feature_flag_count = record.feature_flag.count;
    value.proto_bytes = view_bytes(*st, record.bytes[0]);
    value.field2 = view_bytes(*st, record.bytes[1]);
    value.pad = view_bytes(*st, record.bytes[2]);
    *output = value;
    return true;
}
StoreByteView state_preamble(void* context) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    return st && st->status == StoreRuntimeStatus::ok ? view_bytes(*st, st->preamble) : StoreByteView{nullptr,0};
}
StoreByteView state_trailer(void* context) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    return st && st->status == StoreRuntimeStatus::ok ? view_bytes(*st, st->trailer) : StoreByteView{nullptr,0};
}
ValueView state_feature_desc(void* context, size_t entitlement_index, size_t feature_index) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    if (!st || st->status != StoreRuntimeStatus::ok || entitlement_index >= st->record_count) return missing_value();
    const FeatureRange range = st->records[entitlement_index].feature_desc;
    if (feature_index >= range.count) return missing_value();
    return view_text(*st, st->feature_descs[static_cast<size_t>(range.start) + feature_index]);
}
ValueView state_feature_flag(void* context, size_t entitlement_index, size_t feature_index) noexcept {
    const auto* st = static_cast<const StoreState*>(context);
    if (!st || st->status != StoreRuntimeStatus::ok || entitlement_index >= st->record_count) return missing_value();
    const FeatureRange range = st->records[entitlement_index].feature_flag;
    if (feature_index >= range.count) return missing_value();
    return view_text(*st, st->feature_flags[static_cast<size_t>(range.start) + feature_index]);
}
StoreMaterializationSource source_from_state(StoreState* st) noexcept {
    return StoreMaterializationSource{st, &state_loaded, &state_present, &state_count,
        &state_entitlement, &state_preamble, &state_trailer, &state_feature_desc, &state_feature_flag};
}

} } }
