#pragma once
#include "response_dispatch.hpp"
#include "../anadius-messages-source/messages.hpp"

namespace anadius::socket_recovered {
struct MessageCoreSession {
    messages_recovered::State state;messages_recovered::XmlBackend& xml;
    messages_recovered::CryptoFactory& factory;messages_recovered::Random& random;
    messages_recovered::Effects& effects;
};
struct MessageCoreBinding {std::uintptr_t handler_f0=0;MessageCoreSession* session=nullptr;};
class MessageCoreStrings {
public:
    virtual ~MessageCoreStrings()=default;
    virtual std::string read_request(const ReceiveBufferString&)=0;
    virtual void replace_request(ReceiveBufferString&,std::string_view)=0;
    // Mandatory native string construction/ownership projection. Must consume
    // the complete view including embedded NUL; the view expires on return.
    virtual void construct_response(ReceiveBufferString&,std::string_view)=0;
};
class BoundedMessageCoreStrings:public MessageCoreStrings {
public:
    explicit BoundedMessageCoreStrings(ReceiveBufferContext& context):context_(context){}
    std::string read_request(const ReceiveBufferString&) final;
    void replace_request(ReceiveBufferString&,std::string_view) final;
private:ReceiveBufferContext& context_;
};
struct MessageCoreContext {std::span<MessageCoreBinding> handlers;MessageCoreStrings& strings;};
// Explicit NORMAL-RETURN compatibility composition of the existing semantic
//033d70 body. This is not a newly recovered native body. Requests are committed
// after the semantic call returns, before response construction. No catch,
// exception-state writeback or native callback-intermediate visibility is
// invented. Sessions and referenced state/providers must outlive the call.
void process_message_core(std::uintptr_t handler_f0,ReceiveBufferString& response,
    ReceiveBufferString& request,MessageCoreContext&);
class CompatibleMessageCoreApi:public RecoveredResponseDispatchApi {
public:
    CompatibleMessageCoreApi(ResponseDispatchContext& responses,MessageCoreContext& core)
        :RecoveredResponseDispatchApi(responses),core_(core){}
    void process_message_033d70(std::uintptr_t,ReceiveBufferString&,ReceiveBufferString&) final;
    // Empty-message destruction and CRT exit waits remain mandatory providers.
private:MessageCoreContext& core_;
};
} // namespace anadius::socket_recovered
