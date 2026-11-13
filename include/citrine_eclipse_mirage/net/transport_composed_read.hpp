// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_receive_buffer.hpp"
#include "citrine_eclipse_mirage/net/transport_async_receive.hpp"

namespace citrine_eclipse_mirage::socket {
struct ReceiveStringBinding {std::uintptr_t identity=0;ReceiveBufferString* string=nullptr;};
// Selected live projections of native stream+8 socket and *stream+28 service.
// The allocator token is computed as literal stream identity+68 at the call.
struct ReceiveStreamBinding {
    std::uintptr_t identity=0;AsyncReceiveService* service=nullptr;
    ReceiveSocketImplementation* implementation=nullptr;
};
struct ReadLengthError {}; // opaque constructor/throw identity, not native bytes
class ComposedReadApi {
public:
    virtual ~ComposedReadApi()=default;
    // Native tail begins at handler+38: words_88_98[2] and owned pair. R4 frame
    // completion is still required; callback may end handler/owner lifetime.
    virtual void finish_frame_025480(ReceiveHandler&,Error& private_error,std::uint64_t matched)=0;
    virtual void construct_length_error_024390(ReadLengthError&,std::uintptr_t message_identity)=0;
    [[noreturn]]virtual void throw_length_error_023930(ReadLengthError&)=0;
};
struct ComposedReadContext {
    ReceiveBufferContext& buffer;AsyncReceiveContext& receive;IoCompletionEffects& completion;
    ComposedReadApi& api;std::span<ReceiveStringBinding> strings;std::span<ReceiveStreamBinding> streams;
};
using ComposedReadStart=decltype(&start_async_receive_021d10);
// Defined-field map: words60_78={stream,string,effective_size_or_npos,maximum},
// byte80=delimiter, dword84=flag, words88_98={scan/result,last_request,frame_owner}.
// Optional callee injection is for isolated caller tests only. The
// effects bridge always uses the actual021d10 source through the default.
void composed_receive_025c40(ReceiveHandler&,const Error&,std::uint64_t bytes,std::uint32_t flag,
    ComposedReadContext&,ComposedReadStart call_021d10=&start_async_receive_021d10);
class ComposedReadEffects:public FinishedWriteEffects {
public:
    using FinishedWriteEffects::FinishedWriteEffects;
    // Pure dependency binding before work is submitted. Context, constructor,
    // registries and this effects object must outlive all pending operations.
    void bind_read_context(ComposedReadContext& context){read_=&context;}
    void receive_body_025c40(ReceiveHandler&,Error&,std::uint64_t,std::uint32_t) final;
private:ComposedReadContext* read_=nullptr;
};
} // namespace citrine_eclipse_mirage::socket
