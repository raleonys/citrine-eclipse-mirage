#include "preloader_l/config/store_payload.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace config {
namespace {

constexpr size_t kDefaultPrefixSize = 38u;

bool checked_add(size_t* value, size_t delta) noexcept {
    if (!value || delta > static_cast<size_t>(-1) - *value) return false;
    *value += delta;
    return true;
}

size_t varint_size(uint64_t value) noexcept {
    size_t n = 1u;
    while (value >= 0x80u) { value >>= 7u; ++n; }
    return n;
}

bool add_varint_field_size(size_t* total, uint64_t value) noexcept {
    if (!value) return true;
    return checked_add(total, 1u) && checked_add(total, varint_size(value));
}

bool add_bytes_field_size(size_t* total, size_t length) noexcept {
    if (!length) return true;
    return checked_add(total, 1u) &&
           checked_add(total, varint_size(static_cast<uint64_t>(length))) &&
           checked_add(total, length);
}

bool add_value_field_size(size_t* total, ValueView value) noexcept {
    return add_bytes_field_size(total, value.size);
}

bool valid_source(const StoreMaterializationSource& source) noexcept {
    return source.loaded && source.entitlement_count && source.entitlement &&
           source.preamble && source.trailer && source.feature_desc && source.feature_flag;
}

bool runtime_loaded_cb(void*) noexcept { return runtime_store_loaded(); }
bool runtime_present_cb(void*) noexcept { return runtime_store_present(); }
size_t runtime_count_cb(void*) noexcept { return runtime_store_entitlement_count(); }
bool runtime_entitlement_cb(void*, size_t i, StoreEntitlementView* out) noexcept {
    return runtime_store_entitlement(i, out);
}
StoreByteView runtime_preamble_cb(void*) noexcept { return runtime_store_preamble(); }
StoreByteView runtime_trailer_cb(void*) noexcept { return runtime_store_trailer(); }
ValueView runtime_desc_cb(void*, size_t i, size_t j) noexcept { return runtime_store_feature_desc(i, j); }
ValueView runtime_flag_cb(void*, size_t i, size_t j) noexcept { return runtime_store_feature_flag(i, j); }

StoreMaterializationSource runtime_source() noexcept {
    return StoreMaterializationSource{
        nullptr, &runtime_loaded_cb, &runtime_present_cb, &runtime_count_cb,
        &runtime_entitlement_cb, &runtime_preamble_cb, &runtime_trailer_cb,
        &runtime_desc_cb, &runtime_flag_cb,
    };
}

bool record_nested_size(const StoreMaterializationSource& source,
                        size_t index,
                        const StoreEntitlementView& record,
                        size_t* nested) noexcept {
    size_t n = 0;
    if (!add_value_field_size(&n, record.icon) ||
        !add_value_field_size(&n, record.title) ||
        !add_value_field_size(&n, record.description) ||
        !add_value_field_size(&n, record.img_thumb) ||
        !add_value_field_size(&n, record.img_cropped) ||
        !add_value_field_size(&n, record.img_hero)) return false;

    for (size_t i = 0; i < record.feature_desc_count; ++i) {
        const ValueView value = source.feature_desc(source.context, index, i);
        if (!add_value_field_size(&n, value)) return false;
    }
    *nested = n;
    return true;
}

bool record_generated_size(const StoreMaterializationSource& source,
                           size_t index,
                           const StoreEntitlementView& record,
                           size_t* total) noexcept {
    size_t n = 0;
    if (!add_varint_field_size(&n, record.numeric_id) ||
        !add_bytes_field_size(&n, record.field2.size) ||
        !add_varint_field_size(&n, record.category) ||
        !add_value_field_size(&n, record.tag)) return false;

    size_t nested = 0;
    if (!record_nested_size(source, index, record, &nested)) return false;
    if (nested && (!checked_add(&n, 1u) ||
                   !checked_add(&n, varint_size(static_cast<uint64_t>(nested))) ||
                   !checked_add(&n, nested))) return false;

    if (!add_varint_field_size(&n, record.release_ms) ||
        !add_varint_field_size(&n, record.offer_id) ||
        !add_value_field_size(&n, record.vendor)) return false;

    for (size_t i = 0; i < record.feature_flag_count; ++i) {
        const ValueView value = source.feature_flag(source.context, index, i);
        if (!add_value_field_size(&n, value)) return false;
    }

    if (!add_value_field_size(&n, record.key) ||
        !add_value_field_size(&n, record.hash) ||
        !add_varint_field_size(&n, record.flags) ||
        !checked_add(&n, record.pad.size)) return false;

    *total = n;
    return true;
}

StorePayloadStatus calculate_size(const StoreMaterializationSource& source,
                                  StoreByteView prefix_override,
                                  StoreByteView trailer,
                                  size_t* output_size) noexcept {
    if (!valid_source(source) || !output_size ||
        (prefix_override.size && !prefix_override.data) ||
        (trailer.size && !trailer.data)) return StorePayloadStatus::invalid_argument;
    if (!source.loaded(source.context)) {
        *output_size = 0;
        return StorePayloadStatus::not_loaded;
    }
    if (source.entitlement_count(source.context) == 0u) {
        *output_size = 0;
        return StorePayloadStatus::no_records;
    }

    size_t total = prefix_override.size ? prefix_override.size : kDefaultPrefixSize;
    const size_t count = source.entitlement_count(source.context);
    for (size_t i = 0; i < count; ++i) {
        StoreEntitlementView record{};
        if (!source.entitlement(source.context, i, &record)) {
            *output_size = 0;
            return StorePayloadStatus::not_loaded;
        }
        if (record.proto_bytes.size) {
            if (!checked_add(&total, record.proto_bytes.size)) {
                *output_size = 0;
                return StorePayloadStatus::size_overflow;
            }
            continue;
        }
        size_t generated = 0;
        if (!record_generated_size(source, i, record, &generated) || !checked_add(&total, generated)) {
            *output_size = 0;
            return StorePayloadStatus::size_overflow;
        }
    }
    if (!checked_add(&total, trailer.size)) {
        *output_size = 0;
        return StorePayloadStatus::size_overflow;
    }
    *output_size = total;
    return StorePayloadStatus::ok;
}

struct Writer {
    uint8_t* data;
    size_t pos;
    void byte(uint8_t value) noexcept { data[pos++] = value; }
    void bytes(const uint8_t* source, size_t size) noexcept { for (size_t i=0;i<size;++i) data[pos++]=source[i]; }
    void text(ValueView value) noexcept { for (size_t i=0;i<value.size;++i) data[pos++]=static_cast<uint8_t>(value.data[i]); }
    void zeroes(size_t size) noexcept { for (size_t i=0;i<size;++i) data[pos++]=0; }
    void varint(uint64_t value) noexcept { while(value>=0x80u){byte(static_cast<uint8_t>((value&0x7Fu)|0x80u));value>>=7u;}byte(static_cast<uint8_t>(value)); }
    void varint_field(uint8_t tag,uint64_t value) noexcept { if(!value)return;byte(tag);varint(value); }
    void bytes_field(uint8_t tag,const uint8_t* source,size_t size) noexcept { if(!size)return;byte(tag);varint(static_cast<uint64_t>(size));bytes(source,size); }
    void text_field(uint8_t tag,ValueView value) noexcept { if(!value.size)return;byte(tag);varint(static_cast<uint64_t>(value.size));text(value); }
};

void write_generated_record(const StoreMaterializationSource& source,
                            Writer* writer,size_t index,
                            const StoreEntitlementView& record) noexcept {
    writer->varint_field(0x08u, record.numeric_id);
    writer->bytes_field(0x12u, record.field2.data, record.field2.size);
    writer->varint_field(0x20u, record.category);
    writer->text_field(0x2Au, record.tag);
    size_t nested=0; (void)record_nested_size(source,index,record,&nested);
    if(nested){
        writer->byte(0x32u); writer->varint(static_cast<uint64_t>(nested));
        writer->text_field(0x0Au,record.icon); writer->text_field(0x12u,record.title);
        writer->text_field(0x1Au,record.description); writer->text_field(0x22u,record.img_thumb);
        writer->text_field(0x2Au,record.img_cropped); writer->text_field(0x32u,record.img_hero);
        for(size_t i=0;i<record.feature_desc_count;++i)
            writer->text_field(0x42u,source.feature_desc(source.context,index,i));
    }
    writer->varint_field(0x38u,record.release_ms); writer->varint_field(0x40u,record.offer_id);
    writer->text_field(0x4Au,record.vendor);
    for(size_t i=0;i<record.feature_flag_count;++i)
        writer->text_field(0x5Au,source.feature_flag(source.context,index,i));
    writer->text_field(0x62u,record.key); writer->text_field(0x6Au,record.hash);
    writer->varint_field(0x78u,record.flags); writer->bytes(record.pad.data,record.pad.size);
}

StorePayloadStatus build_from_source(const StoreMaterializationSource& source,
                                     StoreByteView prefix_override, StoreByteView trailer,
                                     uint8_t* output,size_t capacity,size_t* output_size) noexcept {
    if ((!output && capacity) || !output_size || !valid_source(source)) return StorePayloadStatus::invalid_argument;
    size_t required=0;
    const auto status=calculate_size(source,prefix_override,trailer,&required);
    *output_size=required;
    if(status!=StorePayloadStatus::ok)return status;
    if(capacity<required||(!output&&required))return StorePayloadStatus::output_too_small;
    Writer writer{output,0};
    if(prefix_override.size)writer.bytes(prefix_override.data,prefix_override.size);else writer.zeroes(kDefaultPrefixSize);
    const size_t count=source.entitlement_count(source.context);
    for(size_t i=0;i<count;++i){
        StoreEntitlementView record{}; (void)source.entitlement(source.context,i,&record);
        if(record.proto_bytes.size)writer.bytes(record.proto_bytes.data,record.proto_bytes.size);
        else write_generated_record(source,&writer,i,record);
    }
    writer.bytes(trailer.data,trailer.size); *output_size=writer.pos; return StorePayloadStatus::ok;
}

} // namespace

StorePayloadStatus runtime_store_payload_size(StoreByteView prefix_override, StoreByteView trailer, size_t* output_size) noexcept {
    RuntimeStoreReadLease lease{};
    if(begin_runtime_store_read(&lease)){
        const auto source=runtime_store_source_for_lease(&lease);
        const auto status=calculate_size(source,prefix_override,trailer,output_size);
        end_runtime_store_read(&lease); return status;
    }
    const auto source=runtime_source();
    return calculate_size(source,prefix_override,trailer,output_size);
}
StorePayloadStatus runtime_store_payload_size(size_t* output_size) noexcept {
    RuntimeStoreReadLease lease{};
    if(begin_runtime_store_read(&lease)){
        const auto source=runtime_store_source_for_lease(&lease);
        const auto status=calculate_size(source,source.preamble(source.context),source.trailer(source.context),output_size);
        end_runtime_store_read(&lease); return status;
    }
    const auto source=runtime_source();
    return calculate_size(source,source.preamble(source.context),source.trailer(source.context),output_size);
}
StorePayloadStatus store_payload_size_from_source(const StoreMaterializationSource& source,size_t* output_size) noexcept {
    if(!valid_source(source))return StorePayloadStatus::invalid_argument;
    return calculate_size(source,source.preamble(source.context),source.trailer(source.context),output_size);
}
StorePayloadStatus build_runtime_store_payload(StoreByteView prefix_override,StoreByteView trailer,uint8_t* output,size_t capacity,size_t* output_size) noexcept {
    RuntimeStoreReadLease lease{};
    if(begin_runtime_store_read(&lease)){
        const auto source=runtime_store_source_for_lease(&lease);
        const auto status=build_from_source(source,prefix_override,trailer,output,capacity,output_size);
        end_runtime_store_read(&lease); return status;
    }
    const auto source=runtime_source();
    return build_from_source(source,prefix_override,trailer,output,capacity,output_size);
}
StorePayloadStatus build_runtime_store_payload(uint8_t* output,size_t capacity,size_t* output_size) noexcept {
    RuntimeStoreReadLease lease{};
    if(begin_runtime_store_read(&lease)){
        const auto source=runtime_store_source_for_lease(&lease);
        const auto status=build_from_source(source,source.preamble(source.context),source.trailer(source.context),output,capacity,output_size);
        end_runtime_store_read(&lease); return status;
    }
    const auto source=runtime_source();
    return build_from_source(source,source.preamble(source.context),source.trailer(source.context),output,capacity,output_size);
}
StorePayloadStatus build_store_payload_from_source(const StoreMaterializationSource& source,uint8_t* output,size_t capacity,size_t* output_size) noexcept {
    if(!valid_source(source))return StorePayloadStatus::invalid_argument;
    return build_from_source(source,source.preamble(source.context),source.trailer(source.context),output,capacity,output_size);
}

} }
