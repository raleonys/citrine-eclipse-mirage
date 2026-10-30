#include "preloader_l/net/transport_environment_constructor.hpp"

namespace preloader_l::socket_recovered {
namespace {
IoHandlerVtable& operations(IoHandlerEnvironment& value){
    if(!value.operations_10)throw UnsupportedInput("Environment constructor has no live operations projection");
    return *value.operations_10;
}
IoEnvironmentTypeQuery type_query(std::uintptr_t identity,IoEnvironmentConstructorContext& context){
    IoEnvironmentTypeQuery found=nullptr;
    bool matched=false;
    for(const auto& binding:context.type_queries){
        if(binding.schedule_identity!=identity)continue;
        if(matched)throw UnsupportedInput("Environment type-query identity is ambiguous");
        matched=true;found=binding.query_slot_00;
    }
    if(!found)throw UnsupportedInput("Environment type-query function has no projection");
    return found;
}
IoEnvironmentFactory factory(IoHandlerExecutor* identity,IoEnvironmentConstructorContext& context){
    IoEnvironmentFactory found=nullptr;
    bool matched=false;
    for(const auto& binding:context.factories){
        if(binding.executor_identity!=identity)continue;
        if(matched)throw UnsupportedInput("Environment constructor factory identity is ambiguous");
        matched=true;found=binding.construct_slot_58;
    }
    if(!identity || !found)throw UnsupportedInput("Environment constructor factory has no projection");
    return found;
}
void destroy_if_non_guard(IoHandlerEnvironment& value,IoHandlerSentinels sentinels){
    auto& live=operations(value);
    if(live.destroy_identity!=sentinels.guard_destroy_017a30)live.destroy_slot_00(value);
}
}
IoHandlerEnvironment& construct_io_environment_023dd0(IoHandlerEnvironment& destination,
    IoHandlerEnvironment& source,IoHandlerSentinels sentinels,IoEnvironmentConstructorContext& context){
    const auto type=type_query(source.schedule_20,context)();
    // Native ADD8 uses address-width arithmetic; no host type_info cast.
    if(context.types.compare_type_names_04d2d0(type+std::uintptr_t{8},0x1800a25c8ULL)==0){
        IoHandlerEnvironment temporary;
        temporary.operations_10=sentinels.empty_operations_846c8;
        temporary.state_18=0;
        temporary.schedule_20=sentinels.empty_schedule_846e8;
        temporary.executor_28=sentinels.empty_executor_84770;
        move_io_environment_023cb0(destination,temporary,sentinels);
        destroy_if_non_guard(temporary,sentinels);
    }else{
        // Cache slot58 FUNCTION before query_slot18 may change/delete executor.
        const auto construct=factory(source.executor_28,context);
        const auto query=operations(source).query_slot_18(source);
        IoHandlerEnvironment prepared;
        IoEnvironmentConstructorScratch scratch;
        construct(prepared,query,scratch);
        IoHandlerEnvironment intermediate;
        // The original first move is inlined, byte-for-byte equivalent to the
        // already recovered023cb0 defined-field ordering used here.
        move_io_environment_023cb0(intermediate,prepared,sentinels);
        destroy_if_non_guard(prepared,sentinels);
        move_io_environment_023cb0(destination,intermediate,sentinels);
        // Unlike the first cleanup, native does not guard this destroy.
        operations(intermediate).destroy_slot_00(intermediate);
    }
    return destination;
}
void RecoveredConstructedWriteEffects::construct_environment_023dd0(
    IoHandlerEnvironment& destination,std::uintptr_t allocator_identity){
    IoHandlerEnvironment* found=nullptr;
    bool matched=false;
    for(const auto& binding:sources_){
        if(binding.allocator_identity!=allocator_identity)continue;
        if(matched)throw UnsupportedInput("Environment source identity is ambiguous");
        matched=true;found=binding.source;
    }
    if(!found)throw UnsupportedInput("Environment source identity has no live projection");
    construct_io_environment_023dd0(destination,*found,write_context().environment.sentinels,constructor_);
}
} // namespace preloader_l::socket_recovered
