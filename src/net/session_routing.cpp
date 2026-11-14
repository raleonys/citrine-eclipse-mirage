// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/session_routing.hpp"

namespace laureondo::session_bridge {
bool RoutedEffects::registered_handler(std::uint32_t rva,
    const messages::XmlNode& request,messages::XmlPrinter& printer) {
    return router_.registered_handler(rva,request,printer);
}
} // namespace laureondo::session_bridge
