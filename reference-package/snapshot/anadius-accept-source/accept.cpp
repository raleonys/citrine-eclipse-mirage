#include "accept.hpp"
#include <algorithm>
#include <bit>
#include <cstring>

namespace anadius::socket_recovered {
namespace {
DispatcherOperationBinding& packet(AsyncAcceptOperation& op){if(!op.packet)throw UnsupportedInput("Accept requires live packet metadata");return *op.packet;}
IocpSchedulerEntry& scheduler(AsyncAcceptService& service){if(!service.scheduler)throw UnsupportedInput("Accept requires live source scheduler");return *service.scheduler;}
IoHandlerVtable& table(IoHandlerEnvironment& env){if(!env.operations_10)throw UnsupportedInput("Accept environment vtable is unbound");return *env.operations_10;}
void reset_packet(CancelOperation& op,DispatcherOperationBinding& metadata){op.overlapped={};op.queued=0;metadata.packet_error={0,{}};metadata.transferred_14=0;}
void copied_error(IocpSchedulerEntry& entry,AsyncAcceptOperation& op,const Error& error,AsyncAcceptContext& c){
    // Selected typed028ab0 boundary: copy defined Error fields; no category lookup.
    op.operation.queued=1;packet(op).packet_error=error;packet(op).transferred_14=0;
    if(c.completion.post(entry.completion.port,0,2,&op.operation.overlapped))return;
    auto& queue=entry.completion;queue.gate.enter();op.operation.next=nullptr;
    if(queue.tail)queue.tail->next=&op.operation;else queue.head=&op.operation;
    queue.tail=&op.operation;queue.dispatch_required.exchange(1);queue.gate.leave();
}
void schedule_accept(IoHandlerEnvironment& environment,AcceptHandler& handler,AsyncAcceptContext& c){
    auto* const executor=environment.executor_28;if(!executor||!executor->prepare_slot_40)throw UnsupportedInput("Accept executor factory is unbound");
    const auto prepare=executor->prepare_slot_40;const auto query=table(environment).query_slot_18(environment);
    IoPrepareScratch scratch;IoHandlerEnvironment temporary;prepare(temporary,query,scratch);
    IoHandlerEnvironment nested;move_io_environment_023cb0(nested,temporary,c.environment.sentinels);
    if(table(temporary).destroy_identity!=c.environment.sentinels.guard_destroy_017a30)table(temporary).destroy_slot_00(temporary);
    AcceptSchedule* schedule=nullptr;bool found=false;
    for(const auto& b:c.schedules)if(b.identity==nested.schedule_20){if(found)throw UnsupportedInput("Ambiguous accept schedule");found=true;schedule=b.table;}
    if(!schedule)throw UnsupportedInput("Accept schedule is unbound");
    if(const auto borrowed=schedule->borrowed_slot_18){AcceptBorrowed descriptor{handler};borrowed(nested,descriptor);}
    else{
        const auto publish=schedule->owned_slot_10;if(!publish)throw UnsupportedInput("Owned accept publisher is unbound");
        const auto local=handler;auto* tls=static_cast<WorkerTlsContext*>(c.worker.tls_get(c.environment.storage.tls_index));
        auto& storage=allocate_handler_cached_021900(tls?tls->state:nullptr,0x28,1,c.environment.storage,c.memory);
        auto& task=c.effects.project_task(storage);task.handler=local;task.callback_identity=0x180022520ULL;
        OwnedHandlerTask* slot=&task;publish(nested,slot);if(slot)slot->call_slot_00(0);
    }
    table(nested).destroy_slot_00(nested);
}
}
AcceptExResult NativeAsyncAcceptApi::accept(Socket listener,Socket accepted,std::span<std::byte> buffer,OVERLAPPED& op){
    if(!imports_.accept||!imports_.error||buffer.size()!=288)throw UnsupportedInput("AcceptEx requires imports and 288-byte endpoint buffer");
    DWORD bytes=0;const auto result=imports_.accept(listener,accepted,buffer.data(),0,0x90,0x90,&bytes,&op);
    const auto error=imports_.error();return {result,static_cast<std::uint32_t>(error)};
}
AcceptAddresses NativeAsyncAcceptApi::addresses(std::span<std::byte> buffer){
    if(!imports_.addresses||buffer.size()!=288)throw UnsupportedInput("AcceptExSockaddrs requires the original bounded buffer");
    sockaddr* local=nullptr;int local_size=0;sockaddr* remote=nullptr;int remote_size=0;
    imports_.addresses(buffer.data(),0,0x90,0x90,&local,&local_size,&remote,&remote_size);
    AcceptAddresses result;result.remote_length=remote_size;
    if(remote_size>=0&&remote_size<=28){
        const auto base=reinterpret_cast<std::uintptr_t>(buffer.data()),address=reinterpret_cast<std::uintptr_t>(remote);
        if(remote_size&&(address<base||address-base>buffer.size()||static_cast<std::size_t>(remote_size)>buffer.size()-(address-base)))throw UnsupportedInput("SDK remote endpoint leaves owned accept buffer");
        if(remote_size)result.remote={reinterpret_cast<const std::byte*>(remote),static_cast<std::size_t>(remote_size)};
    }return result;
}
std::int32_t NativeAsyncAcceptApi::update_context(Socket accepted,Socket& listener){
    static_assert(sizeof(Socket)==8);if(!imports_.option)throw UnsupportedInput("Accept context setsockopt import is unbound");
    return imports_.option(accepted,0xffff,0x700b,reinterpret_cast<const char*>(&listener),8);
}
void start_async_accept_021b00(AsyncAcceptService& service,AcceptorState& listener,AcceptPending& pending,
    AcceptEndpoint* output,const std::uintptr_t& server_this,std::uintptr_t allocator,AsyncAcceptContext& c){
    auto& storage=allocate_io_storage_0269b0(0x1d8,c.environment.storage,c.worker,c.memory);
    auto& op=c.effects.project(storage);const auto state=listener.socket_state;const auto socket=listener.socket;
    op.operation.next=nullptr;op.callback_identity=0x1800271a0ULL;reset_packet(op.operation,packet(op));
    if(storage.bytes.size()<sizeof(OVERLAPPED)||op.address_buffer.size()!=288)throw UnsupportedInput("Accept projection has invalid bounded storage");
    std::fill_n(storage.bytes.begin(),sizeof(OVERLAPPED),std::byte{});
    op.retry_engine_identity=service.retry_engine_identity;op.listener=socket;op.accepted=invalid_socket;op.pending=&pending;
    op.protocol=static_cast<std::uint32_t>(listener.protocol_40);op.endpoint_output=output;
    op.no_retry=(state>>2)&1;op.alternate=nullptr;op.retry_context=0;op.server_this=server_this;
    c.effects.construct_environment(op.environment,allocator);
    const auto protocol=static_cast<std::uint32_t>(listener.protocol_40);const auto old_pending=pending.object.state.socket;
    scheduler(service).work_count_30.fetch_add(1);
    if(listener.socket==invalid_socket)publish_send_status_028b50(scheduler(service),op.operation,packet(op),0x2719,0,c.completion,c.errors);
    else if(old_pending==invalid_socket){
        Error error{0,c.effects.success_category()};const auto fresh=c.effects.create_socket(protocol,1,6,error);
        c.effects.reset_owned_socket(op.accepted);op.accepted=fresh;
        if(fresh==invalid_socket)copied_error(scheduler(service),op,error,c);
        else{
            const auto result=c.api.accept(listener.socket,fresh,op.address_buffer,op.operation.overlapped);
            if(result.result||result.last_error==997)publish_send_started_028c00(scheduler(service),op.operation,c.completion);
            else publish_send_status_028b50(scheduler(service),op.operation,packet(op),result.last_error,0,c.completion,c.errors);
        }
    }else publish_send_status_028b50(scheduler(service),op.operation,packet(op),1,0,c.completion,c.errors);
}
void complete_async_accept_0271a0(std::uintptr_t owner,AsyncAcceptOperation& op,const Error& packet_error,AsyncAcceptContext& c){
    auto error=packet_error;
    if(owner){
        AcceptEndpoint endpoint;endpoint.bytes[0]=std::byte{2};
        const auto accepted=op.accepted;const auto listener=op.listener;
        if(error.value==0x40){error.category=c.errors.system_category_028620();error.value=0x2745;}
        if(error.value==0){
            const auto addresses=c.api.addresses(op.address_buffer);
            if(static_cast<std::uint64_t>(static_cast<std::int64_t>(addresses.remote_length))<29){
                const auto count=static_cast<std::size_t>(addresses.remote_length);
                if(addresses.remote.size()<count)throw UnsupportedInput("Accept remote endpoint copy leaves bounded projection");
                std::copy_n(addresses.remote.begin(),count,endpoint.bytes.begin());
            }else{error.category=c.errors.system_category_028620();error.value=0x2726;}
            auto listener_value=listener;
            if(accepted==invalid_socket){error.category=c.errors.system_category_028620();error.value=0x2719;}
            else if(c.api.update_context(accepted,listener_value)==0)error.value=0;
            else{error.category=c.errors.system_category_028620();error.value=c.errors.get_last_error();}
        }
        const auto system=c.errors.system_category_028620();
        if(error.category.name_identity==system.name_identity&&error.value==0x2745){
            if(op.no_retry==0){
                reset_packet(op.operation,packet(op));
                if(auto* alternate=op.alternate)reset_packet(alternate->operation,alternate->packet);
                auto* selected=op.alternate; // native reload AFTER reset
                c.effects.retry(op,selected?selected->operation:op.operation);return;
            }
        }else if(error.value==0){
            auto* const destination=op.pending;if(!destination)throw UnsupportedInput("Accepted socket requires live destination context");
            const auto file=op.accepted;auto& state=destination->object.state;
            if(state.socket==invalid_socket){
                if(!destination->scheduler)throw UnsupportedInput("Accepted socket attachment requires a live scheduler");
                const auto port=c.sockets.attach_completion_port(file,destination->scheduler->completion.port);
                if(!port){const auto status=c.sockets.get_win32_error();error.category=c.errors.system_category_028620();error.value=std::bit_cast<std::int32_t>(status);}
                if(port||error.value==0){
                    state.socket=file;state.socket_state=0x10;c.effects.reset_pending_owner(*destination);
                    error={0,c.effects.success_category()};state.protocol_40=static_cast<std::int32_t>(op.protocol);state.byte_44=1;
                    std::memcpy(&state.slot_48,endpoint.bytes.data(),8);std::memcpy(&state.slot_50,endpoint.bytes.data()+8,8);
                    std::memcpy(&state.slot_58,endpoint.bytes.data()+16,8);std::memcpy(&state.slot_60,endpoint.bytes.data()+24,4);op.accepted=invalid_socket;
                }
            }else error={1,c.effects.misc_category()};
        }
        if(auto* output=op.endpoint_output)*output=endpoint;
    }
    IoHandlerEnvironment local;move_io_environment_023cb0(local,op.environment,c.environment.sentinels);
    AcceptHandler handler{op.server_this,error};
    table(op.environment).destroy_slot_00(op.environment);
    close_025310(op.accepted,c.errors,c.sockets);
    release_io_storage_026a00(op.storage,0x1d8,c.environment.storage,c.worker,c.memory);
    // From here only moved locals/context are used; raw+projection may be gone.
    if(owner){if(local.state_18==0)c.effects.invoke(handler);else schedule_accept(local,handler,c);}
    table(local).destroy_slot_00(local);
}
} // namespace anadius::socket_recovered
