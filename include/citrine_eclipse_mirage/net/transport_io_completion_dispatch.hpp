// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_io_completion.hpp"
#include "citrine_eclipse_mirage/net/transport_posted_operation_dispatch.hpp"

namespace citrine_eclipse_mirage::socket {
struct IoCompletionBinding {
    CancelOperation* identity=nullptr; // captured while live; no lookup dereference
    ReceiveIoOperation* receive=nullptr;
    SendIoOperation* send=nullptr;
};
// Extends the existing posted-job dispatcher router through its required
// foreign-kind boundary. All other operation kinds remain mandatory.
class IoDispatcherEffects:public PostedDispatcherEffects {
public:
    IoDispatcherEffects(std::span<PostedOperationBinding> posted,std::span<IoCompletionBinding> io,
        IoCompletionEnvironment environment,WorkerRunApi& api,PostedMemoryEffects& memory,Api& errors,IoCompletionEffects& effects)
        :PostedDispatcherEffects(posted,environment.storage,api,memory),bindings_(io),environment_(environment),
         api_(api),memory_(memory),errors_(errors),effects_(effects){}
    void complete_other(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t) final;
    virtual void complete_unknown(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t)=0;
private:
    std::span<IoCompletionBinding> bindings_;IoCompletionEnvironment environment_;
    WorkerRunApi& api_;PostedMemoryEffects& memory_;Api& errors_;IoCompletionEffects& effects_;
};
} // namespace citrine_eclipse_mirage::socket
