#include "preloader_l/config/store_runtime.hpp"
#include <stddef.h>
#include <string>

using namespace preloader_l::config;

int main() {
    clear_runtime_store();

    const char valid[] = "<Config><Store/></Config><!--tail-->   ";
    if (set_runtime_store_xml(valid, sizeof(valid)-1) != StoreRuntimeStatus::ok) return 1;
    if (!runtime_store_loaded() || !runtime_store_present()) return 2;

    // Stock TinyXML2 v11.0.0 rejects generic processing instructions; the
    // superseded compatibility bridge may not mask them in production.
    const char unsupported_pi[]="<Config><Store/></Config><?ok x?>";
    if (set_runtime_store_xml(unsupported_pi,sizeof(unsupported_pi)-1)!=StoreRuntimeStatus::invalid_xml) return 91;
    // A second document element must not be silently ignored.
    const char second_root[] = "<Config><Store/></Config><Other/>";
    if (set_runtime_store_xml(second_root, sizeof(second_root)-1) != StoreRuntimeStatus::invalid_xml) return 3;
    // Previous successful snapshot remains committed.
    if (!runtime_store_loaded() || !runtime_store_present()) return 4;

    // Raw trailing text is also outside a well-formed XML document.
    const char trailing_text[] = "<Config><Store/></Config>garbage";
    if (set_runtime_store_xml(trailing_text, sizeof(trailing_text)-1) != StoreRuntimeStatus::invalid_xml) return 5;
    if (!runtime_store_loaded() || !runtime_store_present()) return 6;

    // Self-closing root gets the same document-end validation.
    const char self_closing_extra[] = "<Config/><Other/>";
    if (set_runtime_store_xml(self_closing_extra, sizeof(self_closing_extra)-1) != StoreRuntimeStatus::invalid_xml) return 7;

    auto nested = [](size_t open_count, bool self_closing_leaf) {
        std::string xml;
        xml.reserve(open_count * 7u + 32u);
        for (size_t i = 0; i < open_count; ++i) xml += "<X>";
        if (self_closing_leaf) xml += "<Leaf/>";
        for (size_t i = 0; i < open_count; ++i) xml += "</X>";
        return xml;
    };
    // Document tracker + 498 OPEN elements = retained depth 499: accepted.
    const auto below_limit = nested(498u, false);
    if (set_runtime_store_xml(below_limit.data(), below_limit.size()) != StoreRuntimeStatus::ok) return 8;
    // Document tracker + 499 OPEN elements = retained depth 500: rejected.
    const auto at_limit = nested(499u, false);
    if (set_runtime_store_xml(at_limit.data(), at_limit.size()) != StoreRuntimeStatus::resource_error) return 9;
    // A self-closing child at the same lexical level does not push OPEN depth.
    const auto self_closing_boundary = nested(498u, true);
    if (set_runtime_store_xml(self_closing_boundary.data(), self_closing_boundary.size()) != StoreRuntimeStatus::ok) return 10;
    // Failed deep update must preserve the prior successful committed snapshot.
    if (!runtime_store_loaded()) return 11;

    return 0;
}
