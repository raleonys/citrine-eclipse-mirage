#pragma once
#include "../anadius-server-source/server.hpp"
#include "../anadius-messages-source/messages.hpp"

namespace anadius::session_bridge {
struct StateReferences {
    std::uint8_t& encrypted_0c4;
    std::uint8_t& unknown_0c5;
    const std::uint8_t& show_lsx_0a3ef0;
    const std::string& version_0a3c98;
    messages_recovered::Cipher& cipher;
    messages_recovered::EventQueue& events;
};
// New source composition, not a separately recovered native procedure.
// Does not reset at construction: Session constructor and accepted start own
// the two observed reset calls. All referenced dependencies must outlive it.
class MessageHandler final:public server_recovered::Handler {
public:
    MessageHandler(StateReferences state,messages_recovered::XmlBackend& xml,
        messages_recovered::CryptoFactory& factory,messages_recovered::Random& random,
        messages_recovered::Effects& effects)
        :state_(state),xml_(xml),factory_(factory),random_(random),effects_(effects){}
    void reset_02bc70() override;
    std::string initial_message_02d540() override;
    std::string process_message_033d70(std::string_view) override;
private:
    StateReferences state_;
    messages_recovered::XmlBackend& xml_;
    messages_recovered::CryptoFactory& factory_;
    messages_recovered::Random& random_;
    messages_recovered::Effects& effects_;
};
} // namespace anadius::session_bridge
