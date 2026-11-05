#pragma once
#include "frame_read.hpp"
#include "../anadius-thread-source/threads.hpp"

namespace anadius::socket_recovered {
struct MessageWriteTask {
    std::uintptr_t function_00=0;ReceiveBufferString response_08;
    std::uintptr_t object_28=0;IoReference* control_30=nullptr;
};
struct MessageDispatchOwner {
    // Shares the SAME object/control sidecar as frame initiation.
    FrameReadOwner* lifetime=nullptr;IoHandlerEnvironment* environment_78=nullptr;
};
struct MessageDispatchOwnerBinding {std::uintptr_t identity=0;MessageDispatchOwner* owner=nullptr;};
using MessageEnvironmentCopy=void(*)(IoHandlerEnvironment&,IoHandlerEnvironment&);
struct MessageEnvironmentCopyBinding {IoHandlerVtable* table=nullptr;MessageEnvironmentCopy copy_slot_08=nullptr;};
struct MessageClosureBinding {std::uintptr_t identity=0;FrameMessageClosure* closure=nullptr;};
class MessageDispatchApi {
public:
    virtual ~MessageDispatchApi()=default;
    // Mandatory lower dependencies.033d70 may mutate the owned request string;
    // an adapter that only copies a string_view does not preserve that contract.
    virtual void process_message_033d70(std::uintptr_t handler_f0,ReceiveBufferString& response,ReceiveBufferString& request)=0;
    virtual void dispatch_response_021420(IoHandlerEnvironment&,MessageWriteTask&,std::uint64_t zero_r8,std::uint64_t zero_r9)=0;
    virtual void destroy_empty_message_00aa50(ReceiveBufferString&)=0;
    virtual void cleanup_exit_waits_04a47c()=0;
};
struct MessageDispatchContext {
    FrameReadContext& frame;MessageDispatchApi& api;
    std::span<MessageDispatchOwnerBinding> owners;
    std::span<MessageEnvironmentCopyBinding> copies;
    std::span<MessageClosureBinding> closures;
};
void dispatch_message_028d30(std::uintptr_t owner,ReceiveBufferString& request,MessageDispatchContext&);
// Selected HOST continuation after an already completed033d70. Both prefixes
// remain caller-owned until this normal-return tail consumes them. No initial
// empty-request shortcut and no second message processing; not a new native ABI.
void dispatch_prepared_response(std::uintptr_t owner,ReceiveBufferString& response,
    ReceiveBufferString& request,MessageDispatchContext&);
using FrameMessageInvoker=void(*)(std::uintptr_t function,std::uintptr_t owner,ReceiveBufferString&,MessageDispatchContext&);
// Pure token router; never executes an image VA. Default selects actual028d30.
void invoke_frame_message(std::uintptr_t,std::uintptr_t,ReceiveBufferString&,MessageDispatchContext&);
std::uint32_t message_thread_entry_021770(FrameMessageClosure&,MessageDispatchContext&,
    FrameMessageInvoker invoke=&invoke_frame_message);
std::uint32_t route_message_thread_entry(std::uintptr_t entry,std::uintptr_t argument,MessageDispatchContext&);
// Source bridge to the existing CRT05e8e0, without changing CRT production.
// Effects::invoke_entry must route the opaque entry/argument through the above
// router using a registered live closure. The OS thread backend stays required.
class RecoveredMessageThreadApi:public FrameReadApi {
public:
    explicit RecoveredMessageThreadApi(threads_recovered::Effects& threads):threads_(threads){}
    std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,
        FrameMessageClosure*,std::uint32_t,std::uint32_t&) final;
private:threads_recovered::Effects& threads_;
};
} // namespace anadius::socket_recovered
