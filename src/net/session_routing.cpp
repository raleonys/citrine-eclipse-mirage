#include "preloader_l/net/session_routing.hpp"

namespace preloader_l::session_bridge {
bool RoutedEffects::registered_handler(std::uint32_t rva,
    const messages_recovered::XmlNode& request,messages_recovered::XmlPrinter& printer) {
    return router_.registered_handler(rva,request,printer);
}
} // namespace preloader_l::session_bridge
