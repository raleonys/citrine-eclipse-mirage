// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_composed_write.hpp"

namespace preloader_l::socket {
// Function identities are projected separately from the native vtable layout.
// The slot00 type query has no executor-this argument. Both factories and type
// queries must keep their callable code alive across callbacks deleting tables.
using IoEnvironmentTypeQuery=std::uintptr_t(*)();
struct IoEnvironmentTypeBinding {
    std::uintptr_t schedule_identity=0;
    IoEnvironmentTypeQuery query_slot_00=nullptr;
};
struct IoEnvironmentConstructorScratch {}; // native uninitialized tag, no known bytes
using IoEnvironmentFactory=void(*)(IoHandlerEnvironment&,std::uintptr_t,IoEnvironmentConstructorScratch&);
struct IoEnvironmentFactoryBinding {
    IoHandlerExecutor* executor_identity=nullptr;
    IoEnvironmentFactory construct_slot_58=nullptr;
};
class IoEnvironmentTypeApi {
public:
    virtual ~IoEnvironmentTypeApi()=default;
    // Existing CRT type-info comparator is still a required provider. The
    // arguments are opaque address identities, not host strings to dereference.
    virtual std::int32_t compare_type_names_04d2d0(std::uintptr_t first,std::uintptr_t second)=0;
};
struct IoEnvironmentConstructorContext {
    IoEnvironmentTypeApi& types;
    std::span<IoEnvironmentTypeBinding> type_queries;
    std::span<IoEnvironmentFactoryBinding> factories;
};
// Temporary payload placeholders do not implement native initialization.
// Factory/move providers construct defined fields before any consumer reads
// them; sentinel move/destroy providers must not inspect unspecified payload.
IoHandlerEnvironment& construct_io_environment_023dd0(IoHandlerEnvironment& destination,
    IoHandlerEnvironment& source,IoHandlerSentinels,IoEnvironmentConstructorContext&);
struct IoEnvironmentSourceBinding {
    std::uintptr_t allocator_identity=0;
    IoHandlerEnvironment* source=nullptr;
};
// Closes the previous023dd0 provider in the actual composed-write chain. The
// binding registry is a pure identity projection. All contexts/registries and
// native factory/type-query/move/destroy implementations must remain valid.
class ConstructedWriteEffects:public ComposedWriteEffectsAdapter {
public:
    ConstructedWriteEffects(IoCompletionEnvironment environment,WorkerRunApi& worker,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules,
        AsyncSendApi& send,DispatcherApi& completion,Api& errors,std::span<SendStreamBinding> streams,
        IoEnvironmentConstructorContext& constructor,std::span<IoEnvironmentSourceBinding> sources)
        :ComposedWriteEffectsAdapter(environment,worker,memory,schedules,send,completion,errors,streams),
         constructor_(constructor),sources_(sources){}
    void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t allocator_identity) final;
private:
    IoEnvironmentConstructorContext& constructor_;
    std::span<IoEnvironmentSourceBinding> sources_;
};
} // namespace preloader_l::socket
