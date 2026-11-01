// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/platform/hook_engine_api.hpp"
#include <chrono>

namespace preloader_l::platform::hook_engine {
enum class TransactionState { Idle, Staging, Active, RollbackIncomplete, CleanupIncomplete, Stopped };
class Quiescence {
public:
    virtual ~Quiescence()=default;
    virtual bool drain(std::chrono::milliseconds)=0;
};
// The enclosing owner retains callback context and original slots until finish
// succeeds. Failed disable/drain/uninitialize never relinquishes the engine.
class Transaction final {
public:
    Transaction(BatchApi& api,Quiescence& calls):api_(api),calls_(calls){}
    ~Transaction();
    Transaction(const Transaction&)=delete;
    Transaction& operator=(const Transaction&)=delete;
    int begin();
    int create(void*,void*,void**);
    int queue(void*);
    int activate();
    bool rollback(std::chrono::milliseconds) noexcept;
    bool finish(std::chrono::milliseconds) noexcept;
    bool owns_engine()const noexcept{return owns_;}
    TransactionState state()const noexcept{return state_;}
private:
    bool teardown(std::chrono::milliseconds,bool) noexcept;
    BatchApi& api_;
    Quiescence& calls_;
    TransactionState state_=TransactionState::Idle;
    bool owns_=false,disabled_=false,failed_=false;
};
}
