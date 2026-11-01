// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/router/router.hpp"

namespace preloader_l::session_bridge {
// Implements exactly one message effect using the actual recovered handler
// router. Logging, atoi, invalid-message and thread-error policy stay abstract.
class RoutedEffects:public messages::Effects {
public:
    explicit RoutedEffects(dispatch::Router& router):router_(router){}
    bool registered_handler(std::uint32_t,const messages::XmlNode&,
                            messages::XmlPrinter&) final;
private:
    dispatch::Router& router_;
};
} // namespace preloader_l::session_bridge
