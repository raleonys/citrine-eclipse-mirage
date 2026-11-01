#include "preloader_l/config/store_runtime.hpp"
#include "preloader_l/config/store_payload.hpp"
#include <stddef.h>
#include <stdint.h>

using namespace preloader_l::config;

static bool bytes_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    for (size_t i = 0; i < n; ++i) if (a[i] != b[i]) return false;
    return true;
}

static int baseline_case() {
    const char xml[] =
        "<R><Store><Entitlement NumericId=\"7\" Tag=\"a\"/></Store></R>";
    clear_runtime_store();
    if (set_runtime_store_xml(xml, sizeof(xml)-1) != StoreRuntimeStatus::ok) return 1;

    size_t required = 0;
    const StoreByteView empty{nullptr, 0};
    if (runtime_store_payload_size(empty, empty, &required) != StorePayloadStatus::ok) return 2;
    if (required != 43u) return 3;

    uint8_t out[43]{};
    size_t actual = 0;
    if (build_runtime_store_payload(empty, empty, out, sizeof(out), &actual) != StorePayloadStatus::ok) return 4;
    if (actual != sizeof(out)) return 5;
    for (size_t i = 0; i < 38u; ++i) if (out[i] != 0) return 6;
    const uint8_t tail[] = {0x08,0x07,0x2A,0x01,0x61};
    if (!bytes_equal(out + 38u, tail, sizeof(tail))) return 7;
    return 0;
}

static int prefix_trailer_case() {
    const char xml[] =
        "<R><Store><Entitlement NumericId=\"7\" Tag=\"a\"/></Store></R>";
    clear_runtime_store();
    if (set_runtime_store_xml(xml, sizeof(xml)-1) != StoreRuntimeStatus::ok) return 10;
    const uint8_t prefix[] = {0x01,0x02,0x03};
    const uint8_t trailer[] = {0xF0,0x0D};
    const StoreByteView p{prefix,sizeof(prefix)}, t{trailer,sizeof(trailer)};
    uint8_t out[10]{};
    size_t actual = 0;
    if (build_runtime_store_payload(p,t,out,sizeof(out),&actual) != StorePayloadStatus::ok) return 11;
    const uint8_t expected[] = {0x01,0x02,0x03,0x08,0x07,0x2A,0x01,0x61,0xF0,0x0D};
    if (actual != sizeof(expected) || !bytes_equal(out,expected,sizeof(expected))) return 12;
    return 0;
}

static int nested_case() {
    const char xml[] =
        "<R><Store><Entitlement Icon=\"I\" Title=\"T\" ImgThumb=\"M\" ImgCropped=\"C\" ImgHero=\"H\">"
        "<Description>D</Description><FeatureDesc>F</FeatureDesc></Entitlement></Store></R>";
    clear_runtime_store();
    if (set_runtime_store_xml(xml,sizeof(xml)-1) != StoreRuntimeStatus::ok) return 20;
    const StoreByteView empty{nullptr,0};
    uint8_t out[61]{}; size_t actual=0;
    if (build_runtime_store_payload(empty,empty,out,sizeof(out),&actual) != StorePayloadStatus::ok) return 21;
    if (actual != 61u) return 22;
    const uint8_t expected_tail[] = {
        0x32,0x15,0x0A,0x01,0x49,0x12,0x01,0x54,0x1A,0x01,0x44,
        0x22,0x01,0x4D,0x2A,0x01,0x43,0x32,0x01,0x48,0x42,0x01,0x46
    };
    if (!bytes_equal(out+38u,expected_tail,sizeof(expected_tail))) return 23;
    return 0;
}

static int proto_per_record_case() {
    const char xml[] =
        "<R><Store><Entitlement NumericId=\"1\" ProtoBytes=\"QUE=\" Pad=\"0011\"/>"
        "<Entitlement NumericId=\"2\" Tag=\"b\"/></Store></R>";
    clear_runtime_store();
    if (set_runtime_store_xml(xml,sizeof(xml)-1) != StoreRuntimeStatus::ok) return 30;
    const StoreByteView empty{nullptr,0};
    uint8_t out[45]{}; size_t actual=0;
    if (build_runtime_store_payload(empty,empty,out,sizeof(out),&actual) != StorePayloadStatus::ok) return 31;
    if (actual != 45u) return 32;
    const uint8_t expected_tail[] = {0x41,0x41,0x08,0x02,0x2A,0x01,0x62};
    if (!bytes_equal(out+38u,expected_tail,sizeof(expected_tail))) return 33;
    return 0;
}

static int all_field_case() {
    const char xml[] =
        "<R><Store><Entitlement NumericId=\"128\" Category=\"300\" Tag=\"a\" "
        "ReleaseMs=\"3\" OfferId=\"4\" Vendor=\"V\" Key=\"K\" Hash=\"H\" Flags=\"300\" "
        "Field2=\"QQ==\" Pad=\"0011\"><FeatureFlag>G</FeatureFlag></Entitlement></Store></R>";
    clear_runtime_store();
    if (set_runtime_store_xml(xml,sizeof(xml)-1) != StoreRuntimeStatus::ok) return 40;
    const StoreByteView empty{nullptr,0};
    uint8_t out[128]{}; size_t actual=0;
    if (build_runtime_store_payload(empty,empty,out,sizeof(out),&actual) != StorePayloadStatus::ok) return 41;
    const uint8_t expected_tail[] = {
        0x08,0x80,0x01, 0x12,0x01,0x41, 0x20,0xAC,0x02, 0x2A,0x01,0x61,
        0x38,0x03, 0x40,0x04, 0x4A,0x01,0x56, 0x5A,0x01,0x47,
        0x62,0x01,0x4B, 0x6A,0x01,0x48, 0x78,0xAC,0x02, 0x00,0x11
    };
    if (actual != 38u + sizeof(expected_tail)) return 42;
    if (!bytes_equal(out+38u,expected_tail,sizeof(expected_tail))) return 43;

    uint8_t small[2] = {0xAA,0xBB}; size_t required=0;
    if (build_runtime_store_payload(empty,empty,small,sizeof(small),&required) != StorePayloadStatus::output_too_small) return 44;
    if (small[0] != 0xAA || small[1] != 0xBB || required != actual) return 45;
    return 0;
}

int main() {
    int rc=0;
    if ((rc=baseline_case())) return rc;
    if ((rc=prefix_trailer_case())) return rc;
    if ((rc=nested_case())) return rc;
    if ((rc=proto_per_record_case())) return rc;
    if ((rc=all_field_case())) return rc;
    return 0;
}
