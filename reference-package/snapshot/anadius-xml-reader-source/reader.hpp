#pragma once
#include "../anadius-dispatch-source/router.hpp"

namespace anadius::xml_reader_bridge {
// Actual source-based XML reader using pinned upstream tinyxml2 11.0.0 with
// evidence-backed depth=100. This compatibility choice is NOT proof that the
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
} // namespace anadius::xml_reader_bridge
