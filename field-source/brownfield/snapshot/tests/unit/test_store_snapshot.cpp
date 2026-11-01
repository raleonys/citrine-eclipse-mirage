#include "preloader_l/config/store_runtime.hpp"

static int fail() { return 1; }

int main() {
    using namespace preloader_l::config;

    const char good[] =
        "<Root><Store><Entitlement NumericId=\"7\" Title=\"KeepMe\"/>"
        "</Store></Root>";
    const char bad[] =
        "<Root><Store><Entitlement NumericId=\"9\" Pad=\"0G\"/>"
        "</Store></Root>";

    clear_runtime_store();

    if (set_runtime_store_xml(good, sizeof(good) - 1) !=
        StoreRuntimeStatus::ok) return fail();
    if (!runtime_store_present()) return fail();
    if (runtime_store_entitlement_count() != 1) return fail();

    StoreEntitlementView before{};
    if (!runtime_store_entitlement(0, &before)) return fail();
    if (before.numeric_id != 7) return fail();

    // Audit01/F319-P085-001: an XML numeric entity naming a surrogate is
    // malformed XML, not an allocation/resource failure. The rejected update
    // must preserve the previously committed snapshot.
    const char surrogate[] =
        "<Root><Store><Entitlement NumericId=\"9\" Title=\"&#xD800;\"/>"
        "</Store></Root>";
    if (set_runtime_store_xml(surrogate, sizeof(surrogate) - 1) !=
        StoreRuntimeStatus::invalid_xml) return fail();
    if (runtime_store_status() != StoreRuntimeStatus::invalid_xml) return fail();
    if (!runtime_store_present() || runtime_store_entitlement_count() != 1) return fail();
    StoreEntitlementView retained_after_surrogate{};
    if (!runtime_store_entitlement(0, &retained_after_surrogate)) return fail();
    if (retained_after_surrogate.numeric_id != 7) return fail();

    if (set_runtime_store_xml(bad, sizeof(bad) - 1) ==
        StoreRuntimeStatus::ok) return fail();

    // The failed candidate must not destroy the committed snapshot.
    if (!runtime_store_present()) return fail();
    if (runtime_store_entitlement_count() != 1) return fail();

    StoreEntitlementView after{};
    if (!runtime_store_entitlement(0, &after)) return fail();
    if (after.numeric_id != 7) return fail();


    // HF03/F321-S302-001: a reader pins the old generation while a writer
    // publishes the next one. Old borrowed views stay valid until release.
    RuntimeStoreReadLease lease{};
    if(!begin_runtime_store_read(&lease)) return fail();
    const auto old_source=runtime_store_source_for_lease(&lease);
    StoreEntitlementView old_record{};
    if(!old_source.entitlement || !old_source.entitlement(old_source.context,0,&old_record)) return fail();
    const ValueView old_title=old_record.title;
    constexpr char newer[] =
        "<Doc><Store><Entitlement NumericId=\"9\" Title=\"NEWGEN\"/></Store></Doc>";
    if(set_runtime_store_xml(newer,sizeof(newer)-1)!=StoreRuntimeStatus::ok) return fail();
    if(!old_title.data || old_title.size==0) return fail();
    StoreEntitlementView current{};
    if(!runtime_store_entitlement(0,&current) || !current.title.data || current.title.size!=6) return fail();
    if(current.title.data[0]!='N' || current.title.data[5]!='N') return fail();
    const auto old_source_after=runtime_store_source_for_lease(&lease);
    StoreEntitlementView old_again{};
    if(!old_source_after.entitlement(old_source_after.context,0,&old_again)) return fail();
    if(old_again.title.size!=old_title.size || old_again.title.data!=old_title.data) return fail();
    end_runtime_store_read(&lease);
    return 0;
}
