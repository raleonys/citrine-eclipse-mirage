#pragma once
#include "closing.hpp"
#include <windows.h>

namespace anadius::socket_recovered {
// Typed logical operation; only overlapped is a real SDK object. The remaining
// fields are not an overlay of original ASIO memory or a native error_code ABI.
struct CancelOperation {
    OVERLAPPED overlapped{};
    CancelOperation* next=nullptr;
    std::uint32_t queued=0;
    Error error;
};
struct CancelNode {
    CancelNode* next=nullptr;
    CancelNode* previous=nullptr;
    Socket socket=0;
    CancelOperation* head=nullptr;
    CancelOperation* tail=nullptr;
};
struct CancelBucket {CancelNode* first=nullptr;CancelNode* last=nullptr;};
struct CancelMap {std::span<CancelBucket> buckets;CancelNode* sentinel=nullptr;};
struct CompletionScheduler {
    explicit CompletionScheduler(SocketGate& gate):gate(gate){}
    std::uintptr_t port=0;
    SocketGate& gate;
    CancelOperation* head=nullptr;
    CancelOperation* tail=nullptr;
    std::atomic<std::int32_t> dispatch_required{0};
};
struct WakeState {Socket socket=invalid_socket;};
struct CancellationState {
    std::array<CancelMap,4> maps;
    CompletionScheduler* scheduler=nullptr;
    WakeState wake;
};
class CancellationRecycler {
public:
    virtual ~CancellationRecycler()=default;
    // Required bucket/list/free-pool change;0283c0 internals are not recovered
    // here. Node storage must remain alive until this call returns.
    virtual void recycle_0283c0(CancelMap&,CancelNode&)=0;
};
class CompletionApi {
public:
    virtual ~CompletionApi()=default;
    virtual std::int32_t post(std::uintptr_t port,CancelOperation&)=0;
};
// Required maps/links are initialized, borrowed, finite valid node/operation
// graphs. No map allocation/rehash or ownership/concurrency is invented here.
void cancel_queue_026aa0(CancellationState&,Socket,const Error&,
    CancellationRecycler&,CompletionApi&,Api& wake_api);
void wake_028850(const WakeState&,Api&);
class QueueCancellationEffects final:public CancellationEffects {
public:
    QueueCancellationEffects(CancellationEngine& engine,CancellationState& state,
        CancellationRecycler& recycler,CompletionApi& completion,Api& wake_api)
        :engine_(engine),state_(state),recycler_(recycler),completion_(completion),wake_api_(wake_api){}
    void cancel_026aa0(CancellationEngine&,Socket,const Error&) override;
private:
    CancellationEngine& engine_;CancellationState& state_;CancellationRecycler& recycler_;
    CompletionApi& completion_;Api& wake_api_;
};
} // namespace anadius::socket_recovered
