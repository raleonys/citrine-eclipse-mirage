#pragma once
#include "preloader_l/router/router.hpp"

namespace preloader_l::xml_reader_bridge {
// XML reader using the exact owner-provided TinyXML2 archive, with its unchanged
// default depth=500. This vendor choice is NOT proof that the
// original DLL embedded this exact release or that all malformed inputs match.
class Backend final:public messages_recovered::XmlBackend,
                    public dispatch_recovered::NodeAccess {
public:
    messages_recovered::ParseResult parse_c_string(std::string_view) override;
    std::unique_ptr<messages_recovered::XmlPrinter> printer() override;
    std::vector<std::pair<std::string,std::string>> attributes_in_order(
        const messages_recovered::XmlNode&) override;
    const char* text_003860(const messages_recovered::XmlNode&) override;
};
} // namespace preloader_l::xml_reader_bridge
