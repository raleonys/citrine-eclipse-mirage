#include "queued_write.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
namespace {
QueuedWriteOwner& owner_at(std::uintptr_t identity,QueuedWriteContext& context){
    QueuedWriteOwner* found=nullptr;bool matched=false;
    for(const auto& binding:context.owners){
        if(binding.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Queued write owner identity is ambiguous");
        matched=true;found=binding.owner;
    }
    if(!found)throw UnsupportedInput("Queued write owner has no live projection");
    return *found;
}
std::uintptr_t data_address(const WriteFinishString& string,std::uint64_t size){
    if(size==0)return 0; // Native never reads capacity/heap pointer in this case.
    if(string.capacity_18<=15)return reinterpret_cast<std::uintptr_t>(string.storage_00.data());
    std::uint64_t address=0;
    for(std::size_t i=0;i<8;++i)address|=std::uint64_t{string.storage_00[i]}<<(8*i);
    return static_cast<std::uintptr_t>(address);
}
void release_residual(IoReference* control){
    if(control && control->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->dispose_slot_00();
        // Native uses the SAME cached control even if disposal mutates owner
        // or the local handler. The control stays alive until weak release.
        if(control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
    }
}
}
void start_queued_write_02a500(std::uintptr_t owner_identity,QueuedWriteContext& context,
    QueuedWriteSend call_021fd0){
    auto& owner=owner_at(owner_identity,context);
    if(!owner.queue)throw UnsupportedInput("Queued write deque has no live projection");
    auto& queue=*owner.queue;
    if(queue.count_e8==0)return;
    auto* const acquisition=owner.control_08;
    if(!acquisition)context.terminal.bad_weak_ptr_026860();
    auto expected=acquisition->strong_08.load(std::memory_order_relaxed);
    for(;;){
        if(expected==0)context.terminal.bad_weak_ptr_026860();
        if(acquisition->strong_08.compare_exchange_strong(expected,expected+std::uint32_t{1},
               std::memory_order_seq_cst,std::memory_order_seq_cst))break;
    }
    // Native reloads both fields AFTER successful acquisition; the pointer
    // whose counter was incremented is not substituted for this live control.
    const auto object=owner.object_00;
    auto* const control=owner.control_08;
    const auto index=(queue.map_size_d8-std::uint64_t{1})&queue.offset_e0;
    if(index>=queue.map_d0.size() || !queue.map_d0[static_cast<std::size_t>(index)])
        throw UnsupportedInput("Queued write head leaves bounded deque map");
    const auto& string=*queue.map_d0[static_cast<std::size_t>(index)];
    const auto size=string.size_10;
    const auto data=data_address(string,size);
    SendHandler handler;
    handler.words_60_78={owner_identity+std::uintptr_t{0x10},data,size,0};
    handler.dword_80=1;
    handler.word_88=owner_identity;
    handler.object_90=object;
    handler.control_98=control;
    SendBufferSlice buffer{data,std::min(size,std::uint64_t{0x10000})};
    auto* const stream=owner.stream_10;
    if(!stream || !stream->service || !stream->implementation || !context.write || !call_021fd0)
        throw UnsupportedInput("Queued write send members/context have no live projection");
    call_021fd0(*stream->service,*stream->implementation,buffer,0,handler,
        owner_identity+std::uintptr_t{0x78},*context.write);
    // Publication may already have ended owner/queue/stream lifetime. Only
    // the local handler is inspected. Actual021fd0 normally cleared its pair.
    release_residual(handler.control_98);
}
void RecoveredQueuedWriteApi::next_write_02a500(std::uintptr_t owner_identity){
    start_queued_write_02a500(owner_identity,context_);
}
} // namespace anadius::socket_recovered
