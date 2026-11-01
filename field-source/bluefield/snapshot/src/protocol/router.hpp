#pragma once
#include "src/protocol/messages.hpp"
#include "src/handlers/information.hpp"
#include "src/handlers/presence.hpp"
#include "src/handlers/content.hpp"
#include "src/xml/memory_printer.hpp"
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
// Licensing dependency is implemented by license_recovered::Handlers; its
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
    presence::IgoEffects& igo;
    presence::NoResponse& no_response;
    content::Effects& content;
    LicenseHandlers& licensing;
};
// Integration layer chosen for this reconstruction, NOT a separately identified
// original function. It binds verified dispatcher RVAs to recovered source bodies.
class Router {
public:
    Router(handlers::StateReferences information,
           content::StateReferences content,Dependencies deps)
        :information_(information),content_(content),deps_(deps){}
    bool registered_handler(std::uint32_t rva,const messages::XmlNode& request,
                            messages::XmlPrinter& printer);
private:
    handlers::StateReferences information_;
    content::StateReferences content_;
    Dependencies deps_;
};
} // namespace preloader_l::dispatch_recovered
