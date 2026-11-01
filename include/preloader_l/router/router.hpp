// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/protocol/messages.hpp"
#include "preloader_l/handlers/handlers.hpp"
#include "preloader_l/handlers/misc_handlers.hpp"
#include "preloader_l/handlers/content_handlers.hpp"
#include "preloader_l/xml/memory_printer.hpp"
#include <vector>

namespace preloader_l::dispatch {
// tinyxml2 read/ownership boundary. Attribute iteration order and nullable GetText
// cannot be inferred from the narrower messages::XmlNode interface.
class NodeAccess {
public:
    virtual ~NodeAccess()=default;
    virtual std::vector<std::pair<std::string,std::string>> attributes_in_order(const messages::XmlNode&)=0;
    virtual const char* text_003860(const messages::XmlNode&)=0;
};
// Licensing dependency is implemented by license::Handlers; its
// OS/hash/token/online effects remain explicit required subsystem contracts.
class LicenseHandlers {
public:
    virtual ~LicenseHandlers()=default;
    virtual bool auth_code_02e3b0(const messages::XmlNode&,handlers::Xml&)=0;
    virtual bool request_license_0320c0(const messages::XmlNode&,handlers::Xml&)=0;
    virtual bool invalidate_license_030c50(const messages::XmlNode&,handlers::Xml&)=0;
};
struct Dependencies {
    NodeAccess& nodes;
    handlers::Effects& information;
    misc_handlers::IgoEffects& igo;
    misc_handlers::NoResponse& no_response;
    content_handlers::Effects& content;
    LicenseHandlers& licensing;
};
// Integration layer chosen for this reconstruction, NOT a separately identified
// original function. It binds verified dispatcher RVAs to recovered source bodies.
class Router {
public:
    Router(handlers::StateReferences information,
           content_handlers::StateReferences content,Dependencies deps)
        :information_(information),content_(content),deps_(deps){}
    bool registered_handler(std::uint32_t rva,const messages::XmlNode& request,
                            messages::XmlPrinter& printer);
private:
    handlers::StateReferences information_;
    content_handlers::StateReferences content_;
    Dependencies deps_;
};
} // namespace preloader_l::dispatch
