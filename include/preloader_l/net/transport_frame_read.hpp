// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_composed_read.hpp"

namespace preloader_l::socket {
// Live sidecars; never cast onto the native owner or MSVC string ABI.
struct FrameReadOwner {std::uintptr_t object_00=0;IoReference* control_08=nullptr;ReceiveBufferString* input_a8=nullptr;};
struct FrameReadOwnerBinding {std::uintptr_t identity=0;FrameReadOwner* owner=nullptr;};
struct FrameMessageClosure {ReceiveBufferString message_00;std::uintptr_t owner_20=0,function_28=0;};
struct FrameThreadHandle {std::uintptr_t handle=0;std::uint32_t id=0;};
class FrameReadApi:public WriteFinishApi {
public:
    // Required existing dependencies; native bodies are not supplied by this stage.
    virtual void construct_message_006b30(ReceiveBufferString&,std::uintptr_t source,std::uint64_t)=0;
    virtual FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t)=0;
    // Entry/function image identities are opaque data, NEVER invoked as VAs.
    virtual std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t security,std::uint32_t stack,
        std::uintptr_t entry,FrameMessageClosure*,std::uint32_t flags,std::uint32_t& id)=0;
    virtual std::int32_t detach_04a458(const FrameThreadHandle&)=0;
    [[noreturn]]virtual void thread_error_04a648(int)=0;
};
class FrameReadTerminal {
public:virtual ~FrameReadTerminal()=default;[[noreturn]]virtual void bad_weak_ptr_026860()=0;
};
struct FrameReadContext {
    ReceiveBufferContext& buffer;IoCompletionEffects& completion;FrameReadApi& api;
    std::span<FrameReadOwnerBinding> owners;std::span<WriteFinishCategoryBinding> categories;
    std::span<WriteFinishAllocationHeader> allocation_headers;FrameReadTerminal& terminal;
    // Logical category projection for literal native object1800a10f8, NOT its
    // object VA substituted for the name-function identity. No callback here.
    Category success_category_a10f8;ComposedReadContext* read=nullptr;
};
using FrameReadComposedCall=decltype(&composed_receive_025c40);
void start_frame_read_0280d0(std::uintptr_t,FrameReadContext&,
    FrameReadComposedCall call_025c40=&composed_receive_025c40);
using FrameReadRestart=void(*)(std::uintptr_t,FrameReadContext&,FrameReadComposedCall);
void finish_frame_read_025480(ReceiveHandler&,Error&,std::uint64_t matched,FrameReadContext&,
    FrameReadRestart call_0280d0=&start_frame_read_0280d0);
// Default production bridge closes the R3 frame hook with actual025480 and
// actual0280d0/025c40; explicit function injection is isolated-test-only.
class FrameReadApi:public ComposedReadApi {
public:
    explicit FrameReadApi(FrameReadContext& context):context_(context){}
    void finish_frame_025480(ReceiveHandler&,Error&,std::uint64_t) final;
private:FrameReadContext& context_;
};
} // namespace preloader_l::socket
