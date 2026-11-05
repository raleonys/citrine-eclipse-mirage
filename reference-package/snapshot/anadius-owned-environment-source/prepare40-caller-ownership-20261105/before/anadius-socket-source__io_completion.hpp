#pragma once
#include "posted_operation.hpp"

namespace anadius::socket_recovered {
struct IoHandlerEnvironment;
struct IoHandlerSentinels;
class IoEnvironmentMoveSlot {
public:
    virtual ~IoEnvironmentMoveSlot()=default;
    virtual void move_slot_10(IoHandlerEnvironment&,IoHandlerEnvironment&)=0;
};
using IoEnvironmentCopyFunction=void(*)(IoHandlerEnvironment&,IoHandlerEnvironment&);
class IoEnvironmentCopySlot {
public:
    virtual ~IoEnvironmentCopySlot()=default;
    virtual void copy_slot_08(IoEnvironmentCopyFunction cached,IoHandlerEnvironment&,IoHandlerEnvironment&)=0;
};
class IoEnvironmentCopyBody {
public:
    virtual ~IoEnvironmentCopyBody()=default;
    virtual IoHandlerEnvironment& run(IoEnvironmentCopySlot* admitted_slot)=0;
};
class IoHandlerVtable {
public:
    explicit IoHandlerVtable(std::uintptr_t destroy_identity):destroy_identity(destroy_identity){}
    virtual ~IoHandlerVtable()=default;
    // Stable identity of slot00 for the native017a30 equality test; never called.
    const std::uintptr_t destroy_identity;
    virtual void destroy_slot_00(IoHandlerEnvironment&)=0;
    virtual void move_slot_10(IoHandlerEnvironment& destination,IoHandlerEnvironment& source)=0;
    virtual std::uintptr_t query_slot_18(IoHandlerEnvironment&)=0;
    // Optional HOST ownership hook, not an additional recovered native slot.
    // An owning provider claims the whole move before native prefix writes,
    // then invokes the same helper with a private, already-admitted slot bridge.
    virtual bool try_move_environment_023cb0(IoHandlerEnvironment&,IoHandlerEnvironment&,const IoHandlerSentinels&){return false;}
    // The actual028d30 caller owns lookup AFTER prefix writes and the final
    // live executor load. An owning HOST provider reserves the whole operation
    // and invokes this stack body with its explicit already-admitted slot.
    virtual bool try_copy_environment_028d30(IoHandlerEnvironment&,IoHandlerEnvironment&,IoEnvironmentCopyBody&){return false;}
};
// Native caches the actual slot40 FUNCTION before the query callback. That
// function receives destination/query/tag; there is no executor-this argument.
struct IoPrepareScratch {}; // native uninitialized scratch contents are not projected
using IoPrepareFunction=void(*)(IoHandlerEnvironment& destination,std::uintptr_t query,IoPrepareScratch&);
struct IoHandlerExecutor {IoPrepareFunction prepare_slot_40=nullptr;};
// Typed defined-field projection, not native polymorphic storage or a C++ ABI.
// move_slot_10 owns the payload00/state18 transfer; move023cb0 does not zero it.
struct IoHandlerEnvironment {
    std::array<std::uintptr_t,2> payload_00{};
    IoHandlerVtable* operations_10=nullptr;
    std::uintptr_t state_18=0,schedule_20=0;
    IoHandlerExecutor* executor_28=nullptr;
};
struct IoHandlerSentinels {
    IoHandlerVtable* empty_operations_846c8=nullptr;
    std::uintptr_t empty_schedule_846e8=0;
    IoHandlerExecutor* empty_executor_84770=nullptr;
    std::uintptr_t guard_destroy_017a30=0;
};
class IoReference {
public:
    virtual ~IoReference()=default;
    std::atomic<std::uint32_t> strong_08{1},weak_0c{1};
    // slot00 may destroy the owned object, but the control block must remain
    // alive through the following weak decrement. slot08 may delete this.
    virtual void dispose_slot_00()=0;
    virtual void delete_slot_08()=0;
};
// Only the defined fields copied by each body are present. Receive byte80 and
// dword84 are distinct; native padding and send's uninitialized dummy prefix
// and remapped-error padding are deliberately not represented.
struct ReceiveHandler {
    std::array<std::uint64_t,4> words_60_78{};
    std::uint8_t byte_80=0;
    std::uint32_t dword_84=0;
    std::array<std::uint64_t,3> words_88_98{};
    std::uintptr_t object_a0=0;
    IoReference* control_a8=nullptr;
    Error error;
    std::uint64_t transferred=0;
};
struct SendHandler {
    std::array<std::uint64_t,4> words_60_78{};
    std::uint32_t dword_80=0;
    std::uint64_t word_88=0;
    std::uintptr_t object_90=0;
    IoReference* control_98=nullptr;
    Error error;
    std::uint64_t transferred=0;
};
struct ReceiveIoOperation {
    OperationStorage& storage;
    CancelOperation operation;
    std::uint8_t state_38=0;
    IoReference* weak_48=nullptr;
    std::uint64_t requested_58=0;
    ReceiveHandler handler;
    IoHandlerEnvironment environment_b0;
};
struct SendIoOperation {
    OperationStorage& storage;
    CancelOperation operation;
    IoReference* weak_40=nullptr;
    SendHandler handler;
    IoHandlerEnvironment environment_a0;
};
template<class T>struct IoCleanupFrame {
    std::uintptr_t handler_identity_00=0; // unused allocation hint only
    OperationStorage* allocation_08=nullptr;
    T* operation_10=nullptr;
};
struct IoCompletionEnvironment {PostedEnvironment storage;IoHandlerSentinels sentinels;};
class IoCompletionEffects {
public:
    virtual ~IoCompletionEffects()=default;
    virtual Category misc_category_0285b0()=0;
    // Remaining native algorithms, not empty callbacks or newly recovered code.
    virtual void receive_body_025c40(ReceiveHandler&,Error& copied_error,std::uint64_t bytes,std::uint32_t flag)=0;
    virtual void send_body_0261d0(SendHandler&,Error& copied_error,std::uint64_t bytes,std::uint32_t flag)=0;
    virtual void schedule_send_0229d0(IoHandlerEnvironment&,SendHandler&)=0;
    virtual void schedule_receive_022c60(IoHandlerEnvironment&,ReceiveHandler&)=0;
};
IoHandlerEnvironment& move_io_environment_023cb0(IoHandlerEnvironment&,IoHandlerEnvironment&,IoHandlerSentinels,
    IoEnvironmentMoveSlot* admitted_slot=nullptr);
void release_io_storage_026a00(OperationStorage&,std::uint64_t size,PostedEnvironment,WorkerRunApi&,PostedMemoryEffects&);
void cleanup_receive_0293f0(IoCleanupFrame<ReceiveIoOperation>&,IoCompletionEnvironment,WorkerRunApi&,PostedMemoryEffects&);
void cleanup_send_0294c0(IoCleanupFrame<SendIoOperation>&,IoCompletionEnvironment,WorkerRunApi&,PostedMemoryEffects&);
void invoke_receive_021a40(ReceiveHandler&,IoCompletionEffects&);
void invoke_send_021a80(SendHandler&,IoCompletionEffects&);
void complete_receive_027710(std::uintptr_t owner,ReceiveIoOperation&,const Error&,std::uint64_t bytes,IoCompletionEnvironment,WorkerRunApi&,PostedMemoryEffects&,Api&,IoCompletionEffects&);
void complete_send_027a20(std::uintptr_t owner,SendIoOperation&,const Error&,std::uint64_t bytes,IoCompletionEnvironment,WorkerRunApi&,PostedMemoryEffects&,Api&,IoCompletionEffects&);
} // namespace anadius::socket_recovered
