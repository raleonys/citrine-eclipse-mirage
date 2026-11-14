// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/session_handler.hpp"

namespace laureondo::session_bridge {
namespace {
struct Receipt final:server::DeliveryReceipt {
    messages::EventDelivery delivery;
    bool finish(bool success) noexcept override{return delivery.finish(success);}
};
}
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
server::PreparedResponse MessageHandler::prepare_response(std::string_view input) {
    auto receipt=std::make_unique<Receipt>();
    std::string message(input);
    auto bytes=messages::process_message_033d70(
        {state_.encrypted_0c4,state_.show_lsx_0a3ef0,state_.cipher,state_.events,&receipt->delivery},
        message,xml_,factory_,random_,effects_);
    return {std::move(bytes),std::move(receipt)};
}
} // namespace laureondo::session_bridge
