#include "routing.hpp"

namespace anadius::session_bridge {
bool RoutedEffects::registered_handler(std::uint32_t rva,
    const messages_recovered::XmlNode& request,messages_recovered::XmlPrinter& printer) {
    return router_.registered_handler(rva,request,printer);
}
} // namespace anadius::session_bridge
