// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/session_routing.hpp"

namespace citrine_eclipse_mirage::session_bridge {
bool RoutedEffects::registered_handler(std::uint32_t rva,
    const messages::XmlNode& request,messages::XmlPrinter& printer) {
    return router_.registered_handler(rva,request,printer);
}
} // namespace citrine_eclipse_mirage::session_bridge
