// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_transaction.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace laureondo::platform::hook_engine;
namespace {
int checks;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
struct Engine final:BatchApi {
    std::vector<int> calls;
    int init=status_code::ok,created=status_code::ok,queued=status_code::ok,applied=status_code::ok,disabled=status_code::ok,closed=status_code::ok;
    int initialize()override{calls.push_back(1);return init;}
    int create(void*,void*,void** original)override{calls.push_back(2);*original=reinterpret_cast<void*>(123);return created;}
    int enable(void*)override{throw std::runtime_error("Individual enable forbidden");}
    int queue_enable(void*)override{calls.push_back(3);return queued;}
    int apply_queued()override{calls.push_back(4);return applied;}
    int disable(void*)override{calls.push_back(5);return disabled;}
    int uninitialize()override{calls.push_back(6);return closed;}
};
struct Calls final:Quiescence {bool quiet=true;int drains=0;bool drain(std::chrono::milliseconds)override{++drains;return quiet;}};
void staged_activation(){
    Engine api;Calls calls;Transaction tx(api,calls);void* original=nullptr;
    require(tx.begin()==status_code::ok,"owned initialization");
    for(int i=0;i<11;++i){require(tx.create(nullptr,nullptr,&original)==status_code::ok,"stage original");require(tx.queue(nullptr)==status_code::ok,"queue target");}
    require(api.calls.size()==23&&original==reinterpret_cast<void*>(123),"no apply during creation/queue");
    require(tx.activate()==status_code::ok&&api.calls.back()==4&&tx.state()==TransactionState::Active,"single batch apply");
    require(tx.finish(std::chrono::milliseconds(1))&&!tx.owns_engine(),"complete shutdown releases engine");
    require(api.calls[24]==5&&api.calls[25]==6&&calls.drains==1,"disable then drain then engine release");
}
void retained_rollback(){
    Engine api;Calls calls;Transaction tx(api,calls);void* original=nullptr;
    tx.begin();tx.create(nullptr,nullptr,&original);tx.queue(nullptr);api.applied=status_code::memory_protect;
    require(tx.activate()!=status_code::ok&&tx.owns_engine(),"failed apply may retain active patches");
    api.disabled=status_code::memory_protect;
    require(!tx.rollback(std::chrono::milliseconds(0))&&tx.state()==TransactionState::RollbackIncomplete&&calls.drains==0,"failed rollback retains engine before drain");
    require(original==reinterpret_cast<void*>(123),"original slot survives incomplete rollback");
    api.disabled=status_code::ok;calls.quiet=false;
    require(!tx.rollback(std::chrono::milliseconds(0))&&tx.owns_engine()&&api.calls.back()==5,"live trampoline blocks engine release");
    calls.quiet=true;api.closed=status_code::memory_protect;
    require(!tx.rollback(std::chrono::milliseconds(0))&&tx.owns_engine(),"uninitialize failure retains ownership");
    const auto count=api.calls.size();api.closed=status_code::ok;
    require(tx.rollback(std::chrono::milliseconds(0))&&!tx.owns_engine()&&api.calls.size()==count+1,"retry only unfinished engine release after successful disable");
}
// A11-07: status_code::disabled reports an already inactive patch, not proof
// that callback drain or final engine cleanup has completed.
void already_disabled_drain_retry(){
    Engine api;Calls calls;Transaction tx(api,calls);void* original=nullptr;
    require(tx.begin()==status_code::ok,"already-disabled case owns the engine");
    require(tx.create(nullptr,nullptr,&original)==status_code::ok&&
            tx.queue(nullptr)==status_code::ok&&tx.activate()==status_code::ok,
            "already-disabled fixture activates one staged hook");
    api.disabled=status_code::disabled;calls.quiet=false;
    require(!tx.finish(std::chrono::milliseconds(0))&&
            tx.state()==TransactionState::CleanupIncomplete&&tx.owns_engine(),
            "already-disabled status cannot bypass an incomplete callback drain");
    require(api.calls==std::vector<int>({1,2,3,4,5})&&calls.drains==1,
            "first attempt performs disable once without uninitializing");
    calls.quiet=true;api.closed=status_code::memory_protect;
    require(!tx.finish(std::chrono::milliseconds(0))&&
            tx.state()==TransactionState::CleanupIncomplete&&tx.owns_engine(),
            "failed engine cleanup retains exact transaction ownership");
    require(api.calls==std::vector<int>({1,2,3,4,5,6})&&calls.drains==2,
            "drain retry must not call disable a second time");
    api.closed=status_code::ok;
    require(tx.finish(std::chrono::milliseconds(0))&&
            tx.state()==TransactionState::Stopped&&!tx.owns_engine(),
            "final retry releases owned engine after the successful drain");
    require(api.calls==std::vector<int>({1,2,3,4,5,6,6})&&
            calls.drains==3&&original==reinterpret_cast<void*>(123),
            "retry only uninitializes unfinished engine and retains original slot");
}
void partial_staging_and_external_owner(){
    {Engine api;Calls calls;Transaction tx(api,calls);void* original=nullptr;tx.begin();api.queued=status_code::not_created;
     tx.create(nullptr,nullptr,&original);require(tx.queue(nullptr)!=status_code::ok,"queue failure");
     require(tx.activate()!=status_code::ok&&api.calls.back()==3,"staging failure prevents apply");require(tx.rollback(std::chrono::milliseconds(0)),"partial staging rollback");}
    {Engine api;Calls calls;Transaction tx(api,calls);api.init=status_code::already_initialized;
     require(tx.begin()!=status_code::ok&&!tx.owns_engine(),"external engine not adopted");
     require(tx.rollback(std::chrono::milliseconds(0))&&api.calls==std::vector<int>{1},"external engine never disabled/uninitialized");}
}
}
int main()try{staged_activation();retained_rollback();already_disabled_drain_retry();partial_staging_and_external_owner();std::cout<<"PASS: "<<checks<<" hook transaction checks\n";}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
