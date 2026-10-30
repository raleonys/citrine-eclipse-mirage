#include "preloader_l/net/transport_owned_response.hpp"

namespace preloader_l::socket_recovered {
namespace {
void reset(ReceiveBufferString& value){value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;}
void destroy(ReceiveBufferString& value,FrameReadContext& context){
    const auto capacity=value.capacity_18;if(capacity<=15)return;
    auto bytes=capacity+std::uint64_t{1};auto address=receive_buffer_address(value);
    if(bytes>=4096){
        bytes+=39;std::uintptr_t base=0;bool found=false;
        for(const auto& item:context.allocation_headers){
            if(item.address!=address-std::uintptr_t{8})continue;
            if(found)throw UnsupportedInput("Owned response allocation header is ambiguous");
            found=true;base=item.allocation_base;
        }
        if(!found)throw UnsupportedInput("Owned response allocation header is unbound");
        if(address-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        address=base;
    }
    context.api.free_sized_04b0f8(address,bytes);
}
void release(IoReference* control){
    if(control && control->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->dispose_slot_00();
        if(control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
    }
}
void release_storage(OperationStorage& storage,OwnedResponseContext& context){
    auto* const tls=static_cast<WorkerTlsContext*>(context.api.tls_get(context.environment.tls_index));
    auto* const thread=tls?tls->state:nullptr;
    if(thread)for(std::size_t i=4;i<6;++i){
        if(thread->cache_00[i])continue;
        if(storage.bytes.size()<=0x48)throw UnsupportedInput("Owned response cache marker leaves bounded storage");
        storage.bytes[0]=storage.bytes[0x48];thread->cache_00[i]=storage.identity;return;
    }
    context.memory.free_04b788(storage); // may end task and allocation lifetime
}
}
void make_owned_response_020cb0(PostedFunctor*& output,MessageWriteTask& source,OwnedResponseContext& context){
    auto* const tls=static_cast<WorkerTlsContext*>(context.api.tls_get(context.environment.tls_index));
    auto& storage=allocate_handler_cached_021900(tls?tls->state:nullptr,0x48,1,context.environment,context.memory);
    auto& task=context.memory.project_owned_response(storage,context);
    task.payload.function_00=source.function_00;
    task.payload.response_08.storage_00.fill(0);task.payload.response_08.size_10=0;task.payload.response_08.capacity_18=0;
    task.payload.response_08=source.response_08;reset(source.response_08);
    task.payload.object_28=0;task.payload.control_30=nullptr;
    task.payload.object_28=source.object_28;task.payload.control_30=source.control_30;
    source.object_28=0;source.control_30=nullptr;
    task.callback_identity_00=0x180022280ULL;output=&task; //020d65..020d6f
    release(source.control_30);destroy(source.response_08,context.strings);reset(source.response_08);
}
void invoke_owned_response_0219d0(MessageWriteTask& task,ResponseDispatchContext& context,ResponseWriteCall call){
    const auto function=task.function_00,owner=task.object_28; // saved before COPY
    ReceiveBufferString temporary;
    auto& copy=context.api.copy_response_009eb0(temporary,task.response_08);
    if(!call)throw UnsupportedInput("Owned response write callback is unbound");
    call(function,owner,copy,context); // callee consumes independent copy
}
void call_owned_response_022280(OwnedResponseTask& task,std::uint8_t flag,OwnedResponseContext& context,
    OwnedResponseInvoke invoke,ResponseWriteCall call){
    auto& allocation=task.storage;
    MessageWriteTask local;local.function_00=task.payload.function_00;local.response_08=task.payload.response_08;
    reset(task.payload.response_08);local.object_28=task.payload.object_28;local.control_30=task.payload.control_30;
    task.payload.object_28=0;task.payload.control_30=nullptr;
    destroy(task.payload.response_08,context.strings);reset(task.payload.response_08);
    release_storage(allocation,context);
    // From0223ad onward neither task nor allocation is accessed. Both native
    // cleanup pointers are already null on this normal path. No invented
    // RAII/unwind call to024fb0 or second TLS lookup is added.
    if(flag){
        if(!context.response || !invoke)throw UnsupportedInput("Owned response invoke context is unbound");
        invoke(local,*context.response,call);
    }
    release(local.control_30); //0223ca live reload; cached target within release
    destroy(local.response_08,context.strings);reset(local.response_08);
}
void OwnedResponseTask::call_slot_00(std::uint8_t flag){
    auto& borrowed_context=context;call_owned_response_022280(*this,flag,borrowed_context);
}
void RecoveredOwnedResponseApi::make_owned_response_020cb0(PostedFunctor*& out,MessageWriteTask& task){
    socket_recovered::make_owned_response_020cb0(out,task,context_);
}
} // namespace preloader_l::socket_recovered
