#include "src/sessions/handler.hpp"

namespace preloader_l::sessions {
void MessageHandler::reset_02bc70() {
    (void)messages::reset_02bc70(state_.cipher,state_.encrypted_0c4,state_.unknown_0c5);
}
std::string MessageHandler::initial_message_02d540() {
    return messages::initial_message_02d540(xml_,random_,state_.version_0a3c98);
}
std::string MessageHandler::process_message_033d70(std::string_view input) {
    // Session's view is borrowed. The native message core mutates its owned
    // per-message string during hex decoding/retries; preserve explicit bytes.
    std::string message(input);
    return messages::process_message_033d70(
        {state_.encrypted_0c4,state_.show_lsx_0a3ef0,state_.cipher,state_.events},
        message,xml_,factory_,random_,effects_);
}
} // namespace preloader_l::session_bridge
