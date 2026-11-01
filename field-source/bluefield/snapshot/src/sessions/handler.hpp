#pragma once
#include "src/sessions/handler_contract.hpp"
#include "src/protocol/messages.hpp"

namespace preloader_l::sessions {
struct StateReferences {
    std::uint8_t& encrypted_0c4;
    std::uint8_t& unknown_0c5;
    const std::uint8_t& show_lsx_0a3ef0;
    const std::string& version_0a3c98;
    messages::Cipher& cipher;
    messages::EventQueue& events;
};
// New source composition, not a separately recovered native procedure.
// Does not reset at construction: Session constructor and accepted start own
// the two observed reset calls. All referenced dependencies must outlive it.
class MessageHandler final:public server::Handler {
public:
    MessageHandler(StateReferences state,messages::XmlBackend& xml,
        messages::CryptoFactory& factory,messages::Random& random,
        messages::Effects& effects)
        :state_(state),xml_(xml),factory_(factory),random_(random),effects_(effects){}
    void reset_02bc70() override;
    std::string initial_message_02d540() override;
    std::string process_message_033d70(std::string_view) override;
private:
    StateReferences state_;
    messages::XmlBackend& xml_;
    messages::CryptoFactory& factory_;
    messages::Random& random_;
    messages::Effects& effects_;
};
} // namespace preloader_l::session_bridge
