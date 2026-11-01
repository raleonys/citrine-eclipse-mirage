#pragma once
#include "../anadius-dispatch-source/router.hpp"

namespace anadius::session_bridge {
// Implements exactly one message effect using the actual recovered handler
// router. Logging, atoi, invalid-message and thread-error policy stay abstract.
class RoutedEffects:public messages_recovered::Effects {
public:
    explicit RoutedEffects(dispatch_recovered::Router& router):router_(router){}
    bool registered_handler(std::uint32_t,const messages_recovered::XmlNode&,
                            messages_recovered::XmlPrinter&) final;
private:
    dispatch_recovered::Router& router_;
};
} // namespace anadius::session_bridge
