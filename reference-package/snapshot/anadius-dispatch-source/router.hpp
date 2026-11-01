#pragma once
#include "../anadius-messages-source/messages.hpp"
#include "../anadius-handlers-source/handlers.hpp"
#include "../anadius-misc-handlers-source/misc_handlers.hpp"
#include "../anadius-content-handlers-source/content_handlers.hpp"
#include "../anadius-xml-source/memory_printer.hpp"
#include <vector>

namespace anadius::dispatch_recovered {
// tinyxml2 read/ownership boundary. Attribute iteration order and nullable GetText
// cannot be inferred from the narrower messages::XmlNode interface.
class NodeAccess {
public:
    virtual ~NodeAccess()=default;
    virtual std::vector<std::pair<std::string,std::string>> attributes_in_order(const messages_recovered::XmlNode&)=0;
    virtual const char* text_003860(const messages_recovered::XmlNode&)=0;
};
// Optional guarded C++ projections. They retain the selected backend's concrete
// serialization/node contracts without handing raw backing objects to handlers.
// The supplied NodeAccess is the Router's configured backend, not image code.
class GuardedNodeAccess {
public:
    virtual ~GuardedNodeAccess()=default;
    virtual std::vector<std::pair<std::string,std::string>> attributes_with(NodeAccess&)const=0;
    virtual const char* text_with(NodeAccess&)const=0;
};
class GuardedPrinterAccess {
public:
    virtual ~GuardedPrinterAccess()=default;
    virtual void open_element(std::string_view,bool)=0;
    virtual void close_element(bool)=0;
    virtual void push_text(std::string_view,bool)=0;
};
// Licensing dependency is implemented by license_recovered::Handlers; its
// OS/hash/token/online effects remain explicit required subsystem contracts.
class LicenseHandlers {
public:
    virtual ~LicenseHandlers()=default;
    virtual bool auth_code_02e3b0(const messages_recovered::XmlNode&,handlers_recovered::Xml&)=0;
    virtual bool request_license_0320c0(const messages_recovered::XmlNode&,handlers_recovered::Xml&)=0;
    virtual bool invalidate_license_030c50(const messages_recovered::XmlNode&,handlers_recovered::Xml&)=0;
};
struct Dependencies {
    NodeAccess& nodes;
    handlers_recovered::Effects& information;
    misc_handlers_recovered::IgoEffects& igo;
    misc_handlers_recovered::NoResponse& no_response;
    content_handlers_recovered::Effects& content;
    LicenseHandlers& licensing;
};
// Integration layer chosen for this reconstruction, NOT a separately identified
// original function. It binds verified dispatcher RVAs to recovered source bodies.
class Router {
public:
    Router(handlers_recovered::StateReferences information,
           content_handlers_recovered::StateReferences content,Dependencies deps)
        :information_(information),content_(content),deps_(deps){}
    bool registered_handler(std::uint32_t rva,const messages_recovered::XmlNode& request,
                            messages_recovered::XmlPrinter& printer);
private:
    handlers_recovered::StateReferences information_;
    content_handlers_recovered::StateReferences content_;
    Dependencies deps_;
};
} // namespace anadius::dispatch_recovered
