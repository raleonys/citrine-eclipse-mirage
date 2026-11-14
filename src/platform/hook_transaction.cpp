// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_transaction.hpp"
#include <exception>
#include <stdexcept>
namespace laureondo::platform::hook_engine {
Transaction::~Transaction(){if(owns_)std::terminate();}
int Transaction::begin(){
    if(state_!=TransactionState::Idle)throw std::logic_error("Hook transaction is single-use");
    const auto status=api_.initialize();
    owns_=status==status_code::ok;state_=owns_?TransactionState::Staging:TransactionState::Stopped;
    return status;
}
int Transaction::create(void* target,void* detour,void** original){
    if(state_!=TransactionState::Staging)throw std::logic_error("Hook create outside staging");
    const auto status=api_.create(target,detour,original);failed_|=status!=status_code::ok;return status;
}
int Transaction::queue(void* target){
    if(state_!=TransactionState::Staging)throw std::logic_error("Hook queue outside staging");
    const auto status=api_.queue_enable(target);failed_|=status!=status_code::ok;return status;
}
int Transaction::activate(){
    if(state_!=TransactionState::Staging)throw std::logic_error("Hook activation outside staging");
    if(failed_){state_=TransactionState::RollbackIncomplete;return status_code::unknown;}
    // One apply for the complete staged set. A failure may leave live patches;
    // do not discard original slots or context before an explicit rollback.
    const auto status=api_.apply_queued();
    state_=status==status_code::ok?TransactionState::Active:TransactionState::RollbackIncomplete;
    return status;
}
bool Transaction::teardown(std::chrono::milliseconds timeout,bool rollback) noexcept {
    try {
        if(owns_&&!disabled_){
            const auto status=api_.disable(nullptr);
            if(status!=status_code::ok&&status!=status_code::disabled)throw std::runtime_error("Hook disable incomplete");
            disabled_=true;
        }
        if(!calls_.drain(timeout))throw std::runtime_error("Hook calls not quiescent");
        if(owns_&&api_.uninitialize()!=status_code::ok)throw std::runtime_error("Hook engine cleanup incomplete");
        owns_=false;state_=TransactionState::Stopped;return true;
    }catch(...){state_=rollback?TransactionState::RollbackIncomplete:TransactionState::CleanupIncomplete;return false;}
}
bool Transaction::rollback(std::chrono::milliseconds timeout)noexcept{return teardown(timeout,true);}
bool Transaction::finish(std::chrono::milliseconds timeout)noexcept{return teardown(timeout,false);}
}
