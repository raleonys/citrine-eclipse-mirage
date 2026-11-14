// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_posted_operation.hpp"

namespace laureondo::socket {
struct PostedOperationBinding {
    CancelOperation* identity=nullptr;
    PostedOperation* posted=nullptr;
};
// Selected typed callback routing. Identity keys are captured while live; other
// operation kinds remain required and are never silently discarded.
class PostedDispatcherEffects:public DispatcherEffects {
public:
    PostedDispatcherEffects(std::span<PostedOperationBinding> bindings,PostedEnvironment environment,
        WorkerRunApi& api,PostedMemoryEffects& memory)
        :bindings_(bindings),environment_(environment),api_(api),memory_(memory){}
    void complete(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t) final;
    virtual void complete_other(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t)=0;
private:
    std::span<PostedOperationBinding> bindings_;PostedEnvironment environment_;
    WorkerRunApi& api_;PostedMemoryEffects& memory_;
};
} // namespace laureondo::socket
