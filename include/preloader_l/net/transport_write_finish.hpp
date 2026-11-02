// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_environment_constructor.hpp"

namespace preloader_l::socket {
// Bounded defined-field projection of the native string. The first sixteen
// bytes are shared inline data/heap-address representation; no host ABI cast.
struct WriteFinishString {
    std::array<std::uint8_t,16> storage_00{};
    std::uint64_t size_10=0,capacity_18=15;
};
struct WriteFinishOwner {
    std::span<WriteFinishString*> map_d0;
    std::uint64_t map_size_d8=0,offset_e0=0,count_e8=0;
};
struct WriteFinishOwnerBinding {
    std::uintptr_t identity=0;
    WriteFinishOwner* owner=nullptr;
};
class WriteFinishCategory {
public:
    virtual ~WriteFinishCategory()=default;
    std::uint64_t identity_08=0;
    virtual void message_slot_10(WriteFinishString&,std::int32_t code)=0;
};
// Existing Category carries an equality token, not a full native object. Each
// selected value must have one coherent category-object projection; ambiguous
// objects sharing its key are unsupported, never silently conflated.
struct WriteFinishCategoryBinding {Category key;WriteFinishCategory* category=nullptr;};
struct WriteFinishAllocationHeader {
    std::uintptr_t address=0; // native data-8 header address token
    std::uintptr_t allocation_base=0;
};
struct WriteFinishStream {
    std::uintptr_t identity=0;
    std::int32_t virtual_base_offset_04=0; // live signed field in native vtable
};
class WriteFinishApi {
public:
    virtual ~WriteFinishApi()=default;
    // Call site passes both arguments even though the retained CRT thunk
    // eventually jumps to free. Its implementation is not modeled here.
    virtual void free_sized_04b0f8(std::uintptr_t,std::uint64_t bytes)=0;
    [[noreturn]]virtual void invalid_parameter_05c974()=0;
    virtual void next_write_02a500(std::uintptr_t owner_identity)=0;
    virtual WriteFinishStream& begin_log_006880(std::uintptr_t global,std::uintptr_t prefix)=0;
    virtual WriteFinishStream& append_007990(WriteFinishStream&,std::uintptr_t data,std::uint64_t size)=0;
    virtual std::uint64_t widen_01e510(std::uintptr_t adjusted_stream,std::uint8_t character)=0;
    virtual void put_00fe90(WriteFinishStream&,std::uint8_t character)=0;
    virtual void flush_00e1e0(WriteFinishStream&)=0;
};
struct WriteFinishContext {
    IoCompletionEffects& completion;
    WriteFinishApi& api;
    std::span<WriteFinishOwnerBinding> owners;
    std::span<WriteFinishCategoryBinding> categories;
    std::span<WriteFinishAllocationHeader> allocation_headers;
};
// Normal body only. Providers keep all projected owners/strings/categories
// alive through the native last use. Uninitialized local string bytes become
// defined only when the required message constructor has filled them.
void finish_write_025a70(SendHandler&,Error&,WriteFinishContext&);
class FinishedWriteEffects:public ConstructedWriteEffects {
public:
    FinishedWriteEffects(IoCompletionEnvironment environment,WorkerRunApi& worker,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules,
        AsyncSendApi& send,DispatcherApi& completion,Api& errors,std::span<SendStreamBinding> streams,
        IoEnvironmentConstructorContext& constructor,std::span<IoEnvironmentSourceBinding> sources,
        WriteFinishApi& finish,std::span<WriteFinishOwnerBinding> owners,
        std::span<WriteFinishCategoryBinding> categories,std::span<WriteFinishAllocationHeader> headers)
        :ConstructedWriteEffects(environment,worker,memory,schedules,send,completion,errors,
             streams,constructor,sources),finish_context_{*this,finish,owners,categories,headers}{}
    void finish_write_025a70(SendHandler&,Error&) final;
    WriteFinishContext& finish_context(){return finish_context_;}
private:WriteFinishContext finish_context_;
};
} // namespace preloader_l::socket
