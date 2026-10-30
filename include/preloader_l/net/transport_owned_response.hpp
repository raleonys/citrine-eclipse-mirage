#pragma once
#include "preloader_l/net/transport_response_dispatch.hpp"
#include "preloader_l/net/transport_handler_dispatch.hpp"

namespace preloader_l::socket_recovered {
struct OwnedResponseContext;
// Typed host projection, never an ABI cast. Native offsets: callback00,
// function08,response10,object30,control38;48 is the allocator marker.
struct OwnedResponseTask final:PostedFunctor {
    OwnedResponseTask(OperationStorage& storage,OwnedResponseContext& context):storage(storage),context(context){}
    OperationStorage& storage;OwnedResponseContext& context;
    MessageWriteTask payload;std::uintptr_t callback_identity_00=0;
    void call_slot_00(std::uint8_t) override;
};
class OwnedResponseMemory:public PostedMemoryEffects {
public:
    // Must project coherent bounded storage/lifetime; freeing storage may also
    // destroy this projection. A cache entry can be projected anew on reuse.
    virtual OwnedResponseTask& project_owned_response(OperationStorage&,OwnedResponseContext&)=0;
};
struct OwnedResponseContext {
    PostedEnvironment environment;WorkerRunApi& api;OwnedResponseMemory& memory;
    FrameReadContext& strings;ResponseDispatchContext* response=nullptr;
};
void make_owned_response_020cb0(PostedFunctor*&,MessageWriteTask&,OwnedResponseContext&);
void invoke_owned_response_0219d0(MessageWriteTask&,ResponseDispatchContext&,
    ResponseWriteCall call=&invoke_response_write);
using OwnedResponseInvoke=decltype(&invoke_owned_response_0219d0);
void call_owned_response_022280(OwnedResponseTask&,std::uint8_t,OwnedResponseContext&,
    OwnedResponseInvoke invoke=&invoke_owned_response_0219d0,
    ResponseWriteCall call=&invoke_response_write);
// Closes N2's owned factory hook.009eb0 remains a mandatory independent COPY
// provider; this bridge does not invent string allocator or native ABI bodies.
class RecoveredOwnedResponseApi:public ResponseDispatchApi {
public:
    explicit RecoveredOwnedResponseApi(OwnedResponseContext& context):context_(context){}
    void make_owned_response_020cb0(PostedFunctor*&,MessageWriteTask&) final;
private:OwnedResponseContext& context_;
};
} // namespace preloader_l::socket_recovered
