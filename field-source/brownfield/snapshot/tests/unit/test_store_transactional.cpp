#include "preloader_l/config/store_runtime.hpp"
#include <stddef.h>
#include <stdint.h>

using namespace preloader_l::config;

static bool text_eq(ValueView v, const char* s) {
    size_t n = 0;
    while (s[n]) ++n;
    if (!v.present || v.size != n) return false;
    for (size_t i = 0; i < n; ++i)
        if (v.data[i] != s[i]) return false;
    return true;
}

int main() {
    clear_runtime_store();
    if (runtime_store_status() != StoreRuntimeStatus::not_loaded) return 1;
    if (runtime_store_loaded()) return 2;

    const char good[] =
        "<Config><Store>"
        "<Entitlement NumericId=\"42\" Category=\"0x100000002\" "
        "OfferId=\"17tail\" ReleaseMs=\"-2\" Flags=\"+7\" "
        "Tag=\"a&amp;b\" ProtoBytes=\"QU!D\" Field2=\"QQ==QUJD\" Pad=\"0aFF\">"
        "<Description><!--lead-->hello&amp;world</Description>"
        "<FeatureDesc>one</FeatureDesc><FeatureDesc><![CDATA[]]></FeatureDesc>"
        "<FeatureFlag>flag</FeatureFlag>"
        "</Entitlement>"
        "<Entitlement NumericId=\"42\"/>"
        "</Store></Config>";

    if (set_runtime_store_xml(good, sizeof(good)-1) != StoreRuntimeStatus::ok) return 3;
    if (!runtime_store_loaded() || !runtime_store_present()) return 4;
    if (runtime_store_entitlement_count() != 2) return 5;

    StoreEntitlementView a{};
    StoreEntitlementView b{};
    if (!runtime_store_entitlement(0, &a) || !runtime_store_entitlement(1, &b)) return 6;
    if (a.numeric_id != 42 || b.numeric_id != 42) return 7;
    if (a.category != 2u || a.offer_id != 17u || a.release_ms != UINT64_MAX-1u || a.flags != 7u) return 8;
    if (!text_eq(a.tag, "a&b") || !text_eq(a.description, "hello&world")) return 9;
    if (a.feature_desc_count != 2 || a.feature_flag_count != 1) return 10;
    if (!text_eq(runtime_store_feature_desc(0,0), "one")) return 11;
    if (runtime_store_feature_desc(0,1).size != 0) return 12;
    if (a.proto_bytes.size != 2 || a.proto_bytes.data[0] != 0x41 || a.proto_bytes.data[1] != 0xC3) return 13;
    if (a.field2.size != 4 || a.field2.data[0] != 0x41 || a.field2.data[1] != 0x41 || a.field2.data[2] != 0x42 || a.field2.data[3] != 0x43) return 14;
    if (a.pad.size != 2 || a.pad.data[0] != 0x0A || a.pad.data[1] != 0xFF) return 15;

    // Failed update must preserve the committed good snapshot but update the
    // latest-attempt status.
    const char bad[] = "<Config><Store><Entitlement NumericId=\"7\" Pad=\"0\"/></Store></Config>";
    if (set_runtime_store_xml(bad, sizeof(bad)-1) != StoreRuntimeStatus::invalid_hex) return 16;
    if (runtime_store_status() != StoreRuntimeStatus::invalid_hex) return 17;
    if (!runtime_store_loaded() || !runtime_store_present()) return 18;
    if (runtime_store_entitlement_count() != 2) return 19;
    StoreEntitlementView retained{};
    if (!runtime_store_entitlement(0, &retained) || retained.numeric_id != 42) return 20;

    clear_runtime_store();
    if (runtime_store_status() != StoreRuntimeStatus::not_loaded) return 21;
    if (runtime_store_loaded() || runtime_store_present() || runtime_store_entitlement_count() != 0) return 22;
    return 0;
}
