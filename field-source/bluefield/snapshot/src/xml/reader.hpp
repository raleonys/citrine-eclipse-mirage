#pragma once
#include "src/protocol/router.hpp"

namespace preloader_l::xml_reader {
// Reader backed by the exact BROWNFIELD-pinned pristine TinyXML2 snapshot.
// The source successful-tree depth=100 policy lives in this adapter, not vendor
// bytes. Parsing/error order and resource footprint on malformed deep documents
// are not original-DLL parity claims.
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
