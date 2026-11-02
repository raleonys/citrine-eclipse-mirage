// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/router/router.hpp"

namespace preloader_l::xml_reader_bridge {
// XML reader using the exact owner-provided TinyXML2 archive, with its unchanged
// default depth=500. This vendor choice is NOT proof that the
// original DLL embedded this exact release or that all malformed inputs match.
class Backend final:public messages::XmlBackend,
                    public dispatch::NodeAccess {
public:
    messages::ParseResult parse_c_string(std::string_view) override;
    std::unique_ptr<messages::XmlPrinter> printer() override;
    std::vector<std::pair<std::string,std::string>> attributes_in_order(
        const messages::XmlNode&) override;
    const char* text_003860(const messages::XmlNode&) override;
};
} // namespace preloader_l::xml_reader_bridge
