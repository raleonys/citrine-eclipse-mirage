// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_write_finish.hpp"

namespace preloader_l::socket {
// Additional defined fields of the same owner used by WriteFinishOwner. This
// is a logical sidecar, never a cast onto the original native object layout.
struct QueuedWriteOwner {
    std::uintptr_t object_00=0;
    IoReference* control_08=nullptr;
    WriteFinishOwner* queue=nullptr;
    SendStreamBinding* stream_10=nullptr;
};
struct QueuedWriteOwnerBinding {std::uintptr_t identity=0;QueuedWriteOwner* owner=nullptr;};
class QueuedWriteTerminal {
public:
    virtual ~QueuedWriteTerminal()=default;
    [[noreturn]]virtual void bad_weak_ptr_026860()=0;
};
struct QueuedWriteContext {
    std::span<QueuedWriteOwnerBinding> owners;
    QueuedWriteTerminal& terminal;
    // Bind after constructing the composed-write effects, before use. This
    // solves the API/effects dependency cycle without a new native callback.
    ComposedWriteContext* write=nullptr;
};
using QueuedWriteSend=void(*)(AsyncSendService&,SendSocketImplementation&,SendBufferSlice&,
    std::uint32_t,SendHandler&,std::uintptr_t,ComposedWriteContext&);
// Native02a500 calls021fd0 directly. Default and recovered API bridge use its
// actual source. Explicit boundary injection exists for isolated caller tests
// (including residual cleanup), and is not original-runtime evidence.
void start_queued_write_02a500(std::uintptr_t owner_identity,QueuedWriteContext&,
    QueuedWriteSend call_021fd0=&start_async_send_021fd0);
// Completes the previous WriteFinishApi.next_write hook with actual02a500.
// Logger/free/category providers and the026860 terminal remain required.
class QueuedWriteApi:public WriteFinishApi {
public:
    explicit QueuedWriteApi(QueuedWriteContext& context):context_(context){}
    void next_write_02a500(std::uintptr_t owner_identity) final;
private:QueuedWriteContext& context_;
};
} // namespace preloader_l::socket
