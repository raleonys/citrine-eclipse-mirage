// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_message_dispatch.hpp"
#include "preloader_l/net/transport_queued_write.hpp"

namespace preloader_l::socket {
struct ResponseExecutorScratch {}; // native uninitialized tag, not recovered bytes
using ResponseFactory20=void(*)(IoHandlerEnvironment&,std::uintptr_t query,std::uintptr_t literal_84425);
using ResponseFactory88=void(*)(IoHandlerEnvironment&,std::uintptr_t query,ResponseExecutorScratch&);
struct ResponseExecutorBinding {IoHandlerExecutor* identity=nullptr;ResponseFactory20 factory_20=nullptr;ResponseFactory88 factory_88=nullptr;};
struct ResponseDispatchContext;
struct BorrowedResponse {
    MessageWriteTask* task=nullptr;std::uintptr_t invoke_identity=0;
    void invoke(ResponseDispatchContext&) const;
};
using ResponseOwnedSchedule=void(*)(IoHandlerEnvironment&,PostedFunctor*&);
using ResponseBorrowedSchedule=void(*)(IoHandlerEnvironment&,BorrowedResponse&);
struct ResponseSchedule {ResponseOwnedSchedule owned_slot_10=nullptr;ResponseBorrowedSchedule borrowed_slot_18=nullptr;};
struct ResponseScheduleBinding {std::uintptr_t identity=0;ResponseSchedule* table=nullptr;};
class ResponseDispatchApi {
public:
    virtual ~ResponseDispatchApi()=default;
    // Mandatory lower bodies.020cb0 consumes the moved task into an owned
    // functor whose native022280 call/discard implementation remains required.
    virtual void make_owned_response_020cb0(PostedFunctor*&,MessageWriteTask&)=0;
    //009eb0 constructs an independent COPY and preserves source. Native heap
    // rounding/alignment/terminal behavior remains a required lower body.
    virtual ReceiveBufferString& copy_response_009eb0(ReceiveBufferString& destination,ReceiveBufferString& source)=0;
};
class ResponseQueueApi {
public:
    virtual ~ResponseQueueApi()=default;
    virtual void grow_deque_026670(WriteFinishOwner&,std::uint64_t extra)=0;
    virtual WriteFinishString* allocate_string_04b0bc(std::uint64_t native_bytes)=0;
};
struct ResponseQueueContext {
    FrameReadContext& strings;ResponseQueueApi& api;std::span<WriteFinishOwnerBinding> owners;
    QueuedWriteContext* queued=nullptr;
};
struct ResponseDispatchContext {
    FrameReadContext& strings;IoHandlerSentinels sentinels;
    std::span<ResponseExecutorBinding> executors;std::span<ResponseScheduleBinding> schedules;
    ResponseDispatchApi& api;ResponseQueueContext* queue=nullptr;
};
using ResponseQueueStart=decltype(&start_queued_write_02a500);
void enqueue_response_028270(std::uintptr_t owner,ReceiveBufferString&,ResponseQueueContext&,
    ResponseQueueStart call_02a500=&start_queued_write_02a500);
using ResponseWriteCall=void(*)(std::uintptr_t function,std::uintptr_t owner,ReceiveBufferString&,ResponseDispatchContext&);
void invoke_response_write(std::uintptr_t,std::uintptr_t,ReceiveBufferString&,ResponseDispatchContext&);
void invoke_borrowed_response_022240(MessageWriteTask&,ResponseDispatchContext&,
    ResponseWriteCall call=&invoke_response_write);
void dispatch_response_021420(IoHandlerEnvironment&,MessageWriteTask&,ResponseDispatchContext&);
// Closes the N1 MessageDispatchApi hook with the actual normal021420 source.
class ResponseDispatchApi:public MessageDispatchApi {
public:
    explicit ResponseDispatchApi(ResponseDispatchContext& context):context_(context){}
    void dispatch_response_021420(IoHandlerEnvironment&,MessageWriteTask&,std::uint64_t,std::uint64_t) final;
private:ResponseDispatchContext& context_;
};
} // namespace preloader_l::socket
