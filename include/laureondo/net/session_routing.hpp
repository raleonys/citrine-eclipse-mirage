// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/router/router.hpp"

namespace laureondo::session_bridge {
// Implements exactly one message effect through the actual handler
// router. Logging, atoi, invalid-message and thread-error policy stay abstract.
class RoutedEffects:public messages::Effects {
public:
    explicit RoutedEffects(dispatch::Router& router):router_(router){}
    bool registered_handler(std::uint32_t,const messages::XmlNode&,
                            messages::XmlPrinter&) final;
private:
    dispatch::Router& router_;
};
} // namespace laureondo::session_bridge
