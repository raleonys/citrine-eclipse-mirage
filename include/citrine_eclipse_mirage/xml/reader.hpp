// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/router/router.hpp"

namespace citrine_eclipse_mirage::xml_reader_bridge {
// Pugixml-backed reader with project-owned syntax/error/value compatibility.
// Preserve the established depth, reference-decoding and borrowed-view contract;
// vendor implementation types are private to the source module.
class Backend final:public messages::XmlBackend,
                    public dispatch::NodeAccess {
public:
    messages::ParseResult parse_c_string(std::string_view) override;
    std::unique_ptr<messages::XmlPrinter> printer() override;
    std::vector<std::pair<std::string,std::string>> attributes_in_order(
        const messages::XmlNode&) override;
    const char* text_003860(const messages::XmlNode&) override;
};
} // namespace citrine_eclipse_mirage::xml_reader_bridge
