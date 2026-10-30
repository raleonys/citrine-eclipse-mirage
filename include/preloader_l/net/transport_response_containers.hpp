#pragma once
#include "preloader_l/net/transport_owned_response.hpp"

namespace preloader_l::socket_recovered {
ReceiveBufferString& copy_response_string_009eb0(ReceiveBufferString& destination,
    const ReceiveBufferString& source,ReceiveBufferContext&);
// A typed pointer-array projection with an explicit native address token.
// No native pointer bytes or host MSVC container layout are reinterpreted.
struct DequeMapBinding {std::uintptr_t address=0;std::span<WriteFinishString*> slots;};
struct DequeMapMemory {
    std::span<DequeMapBinding> bindings;
    std::uintptr_t address_of(std::span<WriteFinishString*>) const;
    std::span<WriteFinishString*> view(std::uintptr_t address,std::uint64_t byte_count) const;
};
class DequeGrowthApi {
public:
    virtual ~DequeGrowthApi()=default;
    // Observable078d30/0793d0 pointer-array contracts, not their native CRT bodies.
    virtual void copy_slots(std::uintptr_t to,std::uintptr_t from,std::uint64_t bytes)=0;
    virtual void zero_slots(std::uintptr_t to,std::uint64_t bytes)=0;
    [[noreturn]]virtual void length_error_018460()=0;
};
class BoundedDequeGrowthApi:public DequeGrowthApi {
public:
    explicit BoundedDequeGrowthApi(DequeMapMemory& memory):memory_(memory){}
    void copy_slots(std::uintptr_t,std::uintptr_t,std::uint64_t) final;
    void zero_slots(std::uintptr_t,std::uint64_t) final;
private:DequeMapMemory& memory_;
};
struct DequeGrowthContext {
    ReceiveBufferContext& allocation;DequeMapMemory& maps;DequeGrowthApi& api;
};
void grow_response_deque_026670(WriteFinishOwner&,std::uint64_t extra,DequeGrowthContext&);
class RecoveredResponseContainersApi:public RecoveredOwnedResponseApi {
public:
    RecoveredResponseContainersApi(OwnedResponseContext& owned,ReceiveBufferContext& strings)
        :RecoveredOwnedResponseApi(owned),strings_(strings){}
    ReceiveBufferString& copy_response_009eb0(ReceiveBufferString&,ReceiveBufferString&) final;
private:ReceiveBufferContext& strings_;
};
class RecoveredResponseQueueApi:public ResponseQueueApi {
public:
    explicit RecoveredResponseQueueApi(DequeGrowthContext& growth):growth_(growth){}
    void grow_deque_026670(WriteFinishOwner&,std::uint64_t extra) final;
    // allocate_string_04b0bc remains a required32-byte object projection/effect.
private:DequeGrowthContext& growth_;
};
} // namespace preloader_l::socket_recovered
