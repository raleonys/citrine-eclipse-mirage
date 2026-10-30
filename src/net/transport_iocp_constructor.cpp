#include "preloader_l/net/transport_iocp_constructor.hpp"
#include <bit>

namespace preloader_l::socket_recovered {
IocpSchedulerEntry& construct_iocp_024990(IocpSchedulerEntry& entry,std::uintptr_t owner,
    std::uint32_t hint,std::uint8_t own_thread,IocpConstructorApi& api,ServiceGateApi& gates,Api& errors,IocpConstructionEffects& effects){
    entry.key.type=0;entry.key.id=0;entry.owner_context=owner;entry.next=nullptr;
    entry.table_identity=0x180084510ULL;
    entry.completion.port=0;
    entry.work_count_30.store(0,std::memory_order_seq_cst);entry.high_34=0;entry.word_38=0;
    OSVERSIONINFOEXA version{};
    static_assert(sizeof(version)==0x9c);
    version.dwOSVersionInfoSize=0x9c;version.dwMajorVersion=6;
    const auto mask=api.set_version_mask(0,2,3);
    const auto verified=api.verify_version(version,2,mask);
    entry.timeout_40=verified?0xffffffffU:500U;
    entry.completion.head=nullptr;entry.completion.tail=nullptr;
    entry.completion.dispatch_required.store(0,std::memory_order_seq_cst);
    const auto gate_status=initialize_gate_027be0(entry.section,gates);
    const auto gate_category=errors.system_category_028620();
    if(gate_status)effects.raise_028240({std::bit_cast<std::int32_t>(gate_status),gate_category},0x180084428ULL);
    entry.word_88=0;entry.word_90=0;entry.word_98=0;
    entry.concurrency_a0=hint;entry.worker_a8=nullptr;
    const auto threads=(hint&0x80000000U)?0xffffffffU:hint;
    const auto port=api.create_completion_port(~std::uintptr_t{0},0,0,threads);
    entry.completion.port=port;
    if(!port){
        const auto status=api.get_last_error();
        const auto category=errors.system_category_028620();
        if(status)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},0x180084528ULL);
    }
    if(own_thread){
        entry.work_count_30.fetch_add(1,std::memory_order_seq_cst);
        auto* const fresh=effects.allocate_worker_04b0bc(0x18);
        if(fresh){
            fresh->handle_08=0;fresh->event_10=0;
            auto* const function=effects.allocate_function_04b0bc(0x20);
            if(function){function->table_identity=0x180084710ULL;function->scheduler_18=&entry;}
            effects.start_worker_02a000(*fresh,function,0);
        }
        // Native reloads after the required start effect. Cache the old pointer
        // across CloseHandle/deallocation callbacks, then overwrite the live slot.
        auto* const old=entry.worker_a8;
        if(old){(void)api.close_handle(old->handle_08);effects.delete_worker_04b0f8(*old,0x18);}
        entry.worker_a8=fresh;
    }
    return entry;
}
ServiceEntry* IocpSchedulerRegistryEffects::construct_scheduler_024990(ServiceEntry& allocation,
    std::uintptr_t owner,std::uint32_t hint,std::uint8_t own_thread){
    auto* const entry=dynamic_cast<IocpSchedulerEntry*>(&allocation);
    if(!entry)throw UnsupportedInput("Scheduler allocation has no live IOCP entry projection");
    return &construct_iocp_024990(*entry,owner,hint,own_thread,api_,gates_,errors_,effects_);
}
} // namespace preloader_l::socket_recovered
