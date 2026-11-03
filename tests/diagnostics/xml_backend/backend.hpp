// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/xml/reader.hpp"
namespace xml_comparison {
// Diagnostic implementation only; never linked into a production target.
class Backend final : public preloader_l::messages::XmlBackend,
                      public preloader_l::dispatch::NodeAccess {
public:
    preloader_l::messages::ParseResult parse_c_string(std::string_view) override;
    std::unique_ptr<preloader_l::messages::XmlPrinter> printer() override;
    std::vector<std::pair<std::string,std::string>> attributes_in_order(
        const preloader_l::messages::XmlNode&) override;
    const char* text_003860(const preloader_l::messages::XmlNode&) override;
};
std::string decode_reference_value(std::string_view);
}
