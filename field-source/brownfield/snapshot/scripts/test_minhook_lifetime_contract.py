#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, tempfile, sys, textwrap
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('MINHOOK_LIFETIME_CONTRACT: FAIL - no C++ compiler')
    sys.exit(1)

with tempfile.TemporaryDirectory(prefix='preloader_l_minhook_lifetime_') as td_s:
    td=Path(td_s)
    (td/'windows.h').write_text(textwrap.dedent(r'''
        #pragma once
        static inline int SwitchToThread(void) { return 1; }
        static inline unsigned long long GetTickCount64(void) { static unsigned long long t=0; return ++t; }
    '''))
    (td/'MinHook.h').write_text(textwrap.dedent(r'''
        #pragma once
        #ifdef __cplusplus
        extern "C" {
        #endif
        typedef enum MH_STATUS {
            MH_UNKNOWN=-1, MH_OK=0, MH_ERROR_ALREADY_INITIALIZED,
            MH_ERROR_NOT_INITIALIZED, MH_ERROR_ALREADY_CREATED,
            MH_ERROR_NOT_CREATED, MH_ERROR_ENABLED, MH_ERROR_DISABLED,
            MH_ERROR_NOT_EXECUTABLE, MH_ERROR_UNSUPPORTED_FUNCTION,
            MH_ERROR_MEMORY_ALLOC, MH_ERROR_MEMORY_PROTECT,
            MH_ERROR_MODULE_NOT_FOUND, MH_ERROR_FUNCTION_NOT_FOUND
        } MH_STATUS;
        #define MH_ALL_HOOKS ((void*)0)
        MH_STATUS MH_Initialize(void);
        MH_STATUS MH_Uninitialize(void);
        MH_STATUS MH_CreateHook(void*,void*,void**);
        MH_STATUS MH_RemoveHook(void*);
        MH_STATUS MH_QueueEnableHook(void*);
        MH_STATUS MH_QueueDisableHook(void*);
        MH_STATUS MH_ApplyQueued(void);
        #ifdef __cplusplus
        }
        #endif
    '''))
    test=td/'test.cpp'
    test.write_text(textwrap.dedent(r'''
        #include "preloader_l/platform/minhook_runtime.hpp"
        #include <MinHook.h>
        #include <atomic>
        #include <cassert>
        #include <cstdint>
        #include <initializer_list>
        #include <thread>

        static MH_STATUS g_apply[16];
        static int g_apply_n=0, g_apply_i=0;
        static bool g_initialized=false;
        static void* g_fake_original=reinterpret_cast<void*>(0x3000u);
        static MH_STATUS g_queue_enable_status=MH_OK;
        static MH_STATUS g_queue_disable_status=MH_OK;
        static MH_STATUS g_remove_status=MH_OK;
        static void script(std::initializer_list<MH_STATUS> xs) {
            g_apply_n=0; g_apply_i=0;
            for (auto x:xs) g_apply[g_apply_n++]=x;
        }
        extern "C" MH_STATUS MH_Initialize(void){ if(g_initialized)return MH_ERROR_ALREADY_INITIALIZED; g_initialized=true; return MH_OK; }
        extern "C" MH_STATUS MH_Uninitialize(void){ if(!g_initialized)return MH_ERROR_NOT_INITIALIZED; g_initialized=false; return MH_OK; }
        extern "C" MH_STATUS MH_CreateHook(void*,void*,void** out){ *out=g_fake_original; return MH_OK; }
        extern "C" MH_STATUS MH_RemoveHook(void*){ return g_remove_status; }
        extern "C" MH_STATUS MH_QueueEnableHook(void*){ return g_queue_enable_status; }
        extern "C" MH_STATUS MH_QueueDisableHook(void*){ return g_queue_disable_status; }
        extern "C" MH_STATUS MH_ApplyQueued(void){ return g_apply_i<g_apply_n ? g_apply[g_apply_i++] : MH_OK; }


        using namespace preloader_l::platform;
        static void* target(){ return reinterpret_cast<void*>(0x1000u); }
        static void* detour(){ return reinterpret_cast<void*>(0x2000u); }

        static void clean_success() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_OK}); assert(enable_minhook_batch(ts,gs,1));
            assert(t.enabled==1u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(g_fake_original));
            script({MH_OK}); assert(disable_minhook_batch(ts,gs,1));
            assert(t.enabled==0u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(target()));
            assert(remove_minhook(&t)); assert(!minhook_ticket_has_ownership(&t));
            assert(shutdown_minhook_runtime());
        }
        static void remove_failure_retains_for_retry() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_OK}); assert(enable_minhook_batch(ts,gs,1));
            script({MH_OK}); assert(disable_minhook_batch(ts,gs,1));
            g_remove_status=MH_ERROR_MEMORY_PROTECT;
            assert(!remove_minhook(&t)); assert(minhook_ticket_has_ownership(&t));
            assert(!shutdown_minhook_runtime());
            g_remove_status=MH_OK;
            assert(remove_minhook(&t)); assert(!minhook_ticket_has_ownership(&t));
            assert(shutdown_minhook_runtime());
        }
        static void invalid_batch_rejection_preserves_gate_state() {
            MinHookTicket valid{}, invalid{};
            MinHookCallGate first{};
            MinHookCallGate second{};
            assert(create_minhook(reinterpret_cast<void*>(0x1050u),detour(),&valid)==MinHookInstallStatus::installed);
            first.state=0;
            first.direct_target=reinterpret_cast<void*>(0x4455u);
            first.forwarding_target=reinterpret_cast<void*>(0x4455u);
            MinHookTicket* ts[]={&valid,&invalid};
            MinHookCallGate* gs[]={&first,&second};
            assert(!enable_minhook_batch(ts,gs,2));
            // F323-S0191-001: a later invalid entry must not move the valid
            // prefix gate to installing or rewrite its forwarding target.
            assert(first.state==0);
            assert(first.direct_target==reinterpret_cast<void*>(0x4455u));
            assert(minhook_gate_forwarding_target(&first)==static_cast<uintptr_t>(0x4455u));
            assert(remove_minhook(&valid));
            assert(shutdown_minhook_runtime());
        }
        static void failed_enable_clean_rollback() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_ERROR_MEMORY_ALLOC,MH_OK}); assert(!enable_minhook_batch(ts,gs,1));
            assert(t.enabled==0u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(target()));
            assert(remove_minhook(&t)); assert(shutdown_minhook_runtime());
        }
        static void failed_enable_failed_rollback_retains() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_ERROR_MEMORY_ALLOC,MH_ERROR_MEMORY_ALLOC}); assert(!enable_minhook_batch(ts,gs,1));
            assert(t.enabled==1u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(g_fake_original));
            assert(!remove_minhook(&t));
            script({MH_OK}); assert(disable_minhook_batch(ts,gs,1));
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(target()));
            assert(remove_minhook(&t)); assert(shutdown_minhook_runtime());
        }
        static void failed_disable_retains_trampoline() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_OK}); assert(enable_minhook_batch(ts,gs,1));
            script({MH_ERROR_MEMORY_ALLOC}); assert(!disable_minhook_batch(ts,gs,1));
            assert(t.enabled==1u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(g_fake_original));
            assert(!remove_minhook(&t));
            script({MH_OK}); assert(disable_minhook_batch(ts,gs,1));
            assert(remove_minhook(&t)); assert(shutdown_minhook_runtime());
        }
        static void transaction_success_one_apply() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            script({});
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1100u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1200u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1));
            assert(enable_minhook_batch(tb,gsb,1));
            assert(g_apply_i==0); // staging does not mutate executable bytes
            script({MH_OK});
            assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::committed);
            assert(g_apply_i==1 && a.enabled==1u && b.enabled==1u);
            script({MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(g_apply_i==1 && a.enabled==0u && b.enabled==0u);
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }
        static void transaction_commit_failure_clean_rollback() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            script({});
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1300u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1400u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1)); assert(enable_minhook_batch(tb,gsb,1));
            script({MH_ERROR_MEMORY_ALLOC,MH_OK});
            assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(g_apply_i==2 && a.enabled==0u && b.enabled==0u);
            assert(!minhook_batch_transaction_active());
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }
        static void transaction_commit_failure_retains_for_retry() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            script({});
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1500u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1600u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1)); assert(enable_minhook_batch(tb,gsb,1));
            script({MH_ERROR_MEMORY_ALLOC,MH_ERROR_MEMORY_ALLOC});
            assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::rollback_incomplete);
            assert(minhook_batch_transaction_active() && a.enabled==1u && b.enabled==1u);
            assert(!remove_minhook(&a));
            script({MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }
        static void transaction_disable_failure_retains_for_retry() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            script({});
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1700u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1800u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1)); assert(enable_minhook_batch(tb,gsb,1));
            script({MH_OK});
            assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::committed);
            script({MH_ERROR_MEMORY_ALLOC,MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rollback_incomplete);
            assert(minhook_batch_transaction_active() && a.enabled==1u && b.enabled==1u);
            script({MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }

        static void transaction_abort_queue_failure_retains() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1900u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1A00u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1)); assert(enable_minhook_batch(tb,gsb,1));
            g_queue_disable_status=MH_ERROR_MEMORY_ALLOC;
            assert(!abort_minhook_batch_transaction());
            assert(minhook_batch_transaction_active() && a.enabled==1u && b.enabled==1u);
            g_queue_disable_status=MH_OK; script({MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }
        static void transaction_rollback_queue_failure_retains() {
            MinHookTicket a{}, b{}; MinHookCallGate ga{}, gb{};
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1B00u),detour(),&a)==MinHookInstallStatus::installed);
            assert(create_minhook(reinterpret_cast<void*>(0x1C00u),detour(),&b)==MinHookInstallStatus::installed);
            MinHookTicket* ta[]={&a}; MinHookCallGate* gsa[]={&ga};
            MinHookTicket* tb[]={&b}; MinHookCallGate* gsb[]={&gb};
            assert(enable_minhook_batch(ta,gsa,1)); assert(enable_minhook_batch(tb,gsb,1));
            script({MH_OK}); assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::committed);
            g_queue_disable_status=MH_ERROR_MEMORY_ALLOC;
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rollback_incomplete);
            assert(minhook_batch_transaction_active() && a.enabled==1u && b.enabled==1u);
            g_queue_disable_status=MH_OK; script({MH_OK});
            assert(rollback_minhook_batch_transaction()==MinHookBatchTransactionStatus::rolled_back);
            assert(remove_minhook(&a)); assert(remove_minhook(&b)); assert(shutdown_minhook_runtime());
        }


        static void transaction_bounded_drain_timeout_retry() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(begin_minhook_batch_transaction());
            assert(create_minhook(reinterpret_cast<void*>(0x1D00u),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            assert(enable_minhook_batch(ts,gs,1));
            script({MH_OK});
            assert(commit_minhook_batch_transaction()==MinHookBatchTransactionStatus::committed);
            std::atomic<bool> entered=false, release=false;
            std::thread th([&]{
                MinHookCallGuard guard(&g);
                entered.store(true,std::memory_order_release);
                while(!release.load(std::memory_order_acquire)) std::this_thread::yield();
            });
            while(!entered.load(std::memory_order_acquire)) std::this_thread::yield();
            script({MH_OK});
            assert(rollback_minhook_batch_transaction_with_timeout(1u)==
                   MinHookBatchTransactionStatus::quiescence_timeout);
            assert(minhook_batch_transaction_active());
            assert(t.enabled==0u);
            assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(0x1D00u)));
            release.store(true,std::memory_order_release);
            th.join();
            assert(rollback_minhook_batch_transaction_with_timeout(100u)==
                   MinHookBatchTransactionStatus::rolled_back);
            assert(!minhook_batch_transaction_active());
            assert(remove_minhook(&t)); assert(shutdown_minhook_runtime());
        }

        static void disable_waits_for_admitted_call() {
            MinHookTicket t{}; MinHookCallGate g{};
            assert(create_minhook(target(),detour(),&t)==MinHookInstallStatus::installed);
            MinHookTicket* ts[]={&t}; MinHookCallGate* gs[]={&g};
            script({MH_OK}); assert(enable_minhook_batch(ts,gs,1));
            std::atomic<bool> entered=false, release=false, disable_started=false, done=false;
            std::thread th([&]{
                MinHookCallGuard guard(&g);
                assert(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(g_fake_original));
                entered.store(true,std::memory_order_release);
                while(!release.load(std::memory_order_acquire)) std::this_thread::yield();
            });
            while(!entered.load(std::memory_order_acquire)) std::this_thread::yield();
            std::thread disabler([&]{
                disable_started.store(true,std::memory_order_release);
                script({MH_OK}); assert(disable_minhook_batch(ts,gs,1));
                done.store(true,std::memory_order_release);
            });
            while(!disable_started.load(std::memory_order_acquire)) std::this_thread::yield();
            bool forwarding_switched=false;
            for(int i=0;i<1000000 && !done.load(std::memory_order_acquire);++i){
                if(minhook_gate_forwarding_target(&g)==reinterpret_cast<uintptr_t>(target())){forwarding_switched=true;break;}
                std::this_thread::yield();
            }
            assert(forwarding_switched);
            assert(!done.load(std::memory_order_acquire));
            release.store(true,std::memory_order_release);
            th.join(); disabler.join();
            assert(done.load(std::memory_order_acquire));
            assert(remove_minhook(&t)); assert(shutdown_minhook_runtime());
        }
        int main(){
            clean_success();
            remove_failure_retains_for_retry();
            invalid_batch_rejection_preserves_gate_state();
            failed_enable_clean_rollback();
            failed_enable_failed_rollback_retains();
            failed_disable_retains_trampoline();
            transaction_success_one_apply();
            transaction_commit_failure_clean_rollback();
            transaction_commit_failure_retains_for_retry();
            transaction_disable_failure_retains_for_retry();
            transaction_abort_queue_failure_retains();
            transaction_rollback_queue_failure_retains();
            transaction_bounded_drain_timeout_retry();
            disable_waits_for_admitted_call();
            return 0;
        }
    '''))
    exe=td/'minhook_lifetime'
    cmd=[cxx,'-std=c++17','-pthread','-D_WIN32','-D__stdcall=','-D__cdecl=',
         '-D__declspec(x)=','-I',str(td),'-I',str(r/'include'),
         str(test),str(r/'src/platform/minhook_runtime.cpp'),'-o',str(exe)]
    try:
        subprocess.run(cmd,cwd=r,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        subprocess.run([str(exe)],cwd=r,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
    except subprocess.CalledProcessError as e:
        print('MINHOOK_LIFETIME_CONTRACT: FAIL')
        if e.stdout: print(e.stdout)
        if e.stderr: print(e.stderr)
        sys.exit(1)
print('MINHOOK_LIFETIME_CONTRACT: PASS (ordinary lifetime + prevalidation rejection atomicity + single-apply transaction + queue-failure atomicity + rollback-retention + retry + drain ordering + bounded drain timeout/retry)')
