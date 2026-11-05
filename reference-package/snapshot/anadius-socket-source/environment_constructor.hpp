#pragma once
#include "composed_write.hpp"

namespace anadius::socket_recovered {
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
// Explicit actor-bound projection calls. The caller holds its owning scoped
// constructor reservation across the whole023dd0 algorithm; no shared context
// is temporarily rebound. Scheduling/prepare40/response factories are separate.
class IoEnvironmentCallOperations {
public:
    virtual ~IoEnvironmentCallOperations()=default;
    virtual std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)=0;
    virtual std::uintptr_t query_slot_18(IoHandlerEnvironment&)=0;
    virtual void destroy_slot_00(IoHandlerEnvironment&)=0;
    virtual void construct_slot_58(IoEnvironmentFactory cached,IoHandlerEnvironment&,std::uintptr_t,IoEnvironmentConstructorScratch&)=0;
    virtual IoHandlerEnvironment& move_023cb0(IoHandlerEnvironment&,IoHandlerEnvironment&,IoHandlerSentinels)=0;
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
    IoEnvironmentCallOperations* calls=nullptr;
};
// Temporary payload placeholders are not recovered native initialization.
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
class RecoveredConstructedWriteEffects:public RecoveredComposedWriteEffects {
public:
    RecoveredConstructedWriteEffects(IoCompletionEnvironment environment,WorkerRunApi& worker,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules,
        AsyncSendApi& send,DispatcherApi& completion,Api& errors,std::span<SendStreamBinding> streams,
        IoEnvironmentConstructorContext& constructor,std::span<IoEnvironmentSourceBinding> sources)
        :RecoveredComposedWriteEffects(environment,worker,memory,schedules,send,completion,errors,streams),
         constructor_(constructor),sources_(sources){}
    // Serialized Profile registry publication; no provider or native call.
    void bind_environment_sources(std::span<IoEnvironmentSourceBinding> sources) noexcept {sources_=sources;}
    void construct_environment_023dd0(IoHandlerEnvironment&,std::uintptr_t allocator_identity) final;
private:
    IoEnvironmentConstructorContext& constructor_;
    std::span<IoEnvironmentSourceBinding> sources_;
};
} // namespace anadius::socket_recovered
