#include "domain_test_fixture.hpp"
#include <csignal>
#include <iostream>
#include <process.h>
#include <thread>
#include <type_traits>

namespace {
using namespace b6_core_test;
void runtime_references_and_error(){
    RuntimeState state;runtime_state=&state;
    {
        td::NativeRuntime runtime(runtime_api());
        ::SetLastError(0x701);const auto a=runtime.load_library_ex_w(u"same",7,0x800);
        check(a==0x100&&state.incoming_load_error==0x701&&state.file==reinterpret_cast<HANDLE>(7)&&state.flags==0x800,"runtime load preserves full SDK operands/incoming LastError");
        check(::GetLastError()==0x651&&runtime.outstanding_modules()==1&&::GetLastError()==0x651,"load bookkeeping and module observation preserve SDK output LastError");
        check(runtime.load_library_ex_w(u"same",0,0)==a&&runtime.outstanding_modules()==2,"duplicate same HANDLE references retain separate tickets");
        const auto before=state.procedures.size();
        rejects<std::invalid_argument>([&]{runtime.get_proc_address(0x999,"RoInitialize");},"foreign module lookup is rejected");
        check(state.procedures.size()==before,"foreign lookup invokes no SDK procedure call");
        check(runtime.get_proc_address(a,"missing")==0&&::GetLastError()==0x652,"missing SDK export and its LastError remain visible");
        check(runtime.get_proc_address(a,"RoInitialize")==reinterpret_cast<std::uintptr_t>(&ro_initialize),"typed rebuilt SDK export address returned unchanged");
        check(runtime.free_library(a)&&runtime.outstanding_modules()==1&&::GetLastError()==0x653,"one duplicate release consumes one reference and preserves output LastError");
        rejects<std::invalid_argument>([&]{runtime.load_library_ex_w(std::u16string_view(u"x\0y",3),0,0);},"embedded-NUL module identity rejected before API");
        rejects<std::invalid_argument>([&]{runtime.get_proc_address(a,std::string_view("x\0y",3));},"embedded-NUL procedure identity rejected");
        check(runtime.close()&&runtime.closed()&&runtime.outstanding_modules()==0&&state.released==std::vector<std::uintptr_t>({0x100,0x100}),"close releases only the remaining reference");
        check(runtime.close(),"runtime close is idempotent");
        rejects<std::logic_error>([&]{runtime.load_library_ex_w(u"later",0,0);},"closed runtime rejects new load admission");
        rejects<std::logic_error>([&]{runtime.process_parameters_flags();},"closed runtime rejects new policy admission");
    }
    {
        td::NativeRuntime runtime(runtime_api());state.fail_load=true;
        check(runtime.load_library_ex_w(u"denied",0,0)==0&&runtime.outstanding_modules()==0&&!runtime.tracking_poisoned(),"normal failed load removes pending reservation without poison");
        check(runtime.close(),"normal failed load leaves safely closable runtime");state.fail_load=false;
    }
    {
        td::NativeRuntime runtime(runtime_api());state.released.clear();
        runtime.load_library_ex_w(u"first",0,0);runtime.load_library_ex_w(u"appmodel",0,0);runtime.load_library_ex_w(u"third",0,0);
        state.release_failure_call=2;
        check(!runtime.close()&&runtime.outstanding_modules()==2&&!runtime.closed(),"failed release retains itself and the unvisited suffix");
        check(state.released==std::vector<std::uintptr_t>({0x100,0x200}),"close preserves consumed release prefix");
        rejects<std::logic_error>([&]{runtime.get_proc_address(0x200,"RoInitialize");},"failed close still excludes new lookup admission");
        state.release_failure_call=0;check(runtime.close()&&state.released==std::vector<std::uintptr_t>({0x100,0x200,0x200,0x100}),"release retry resumes at first unconsumed ticket");
    }
    {
        td::NativeRuntime runtime(runtime_api());bool pending_blocked=false;
        state.load_hook=[&]{pending_blocked=!runtime.close();};runtime.load_library_ex_w(u"pending",0,0);state.load_hook={};
        check(pending_blocked&&runtime.outstanding_modules()==1&&!runtime.closed(),"pending native call prevents close while retaining its eventual module");
        check(runtime.close(),"pending call completion permits explicit close retry");
    }
    auto missing=runtime_api();missing.error=nullptr;rejects<std::invalid_argument>([&]{td::NativeRuntime invalid(missing);},"incomplete runtime SDK table rejected");
    runtime_state=nullptr;
}
void runtime_typed_policy_and_source(){
    RuntimeState state;runtime_state=&state;
    {
        td::NativeRuntime runtime(runtime_api());state.ro_result=static_cast<HRESULT>(-2147024891);
        check(runtime.call_ro_initialize(reinterpret_cast<std::uintptr_t>(&ro_initialize),1)==-2147024891&&state.ro_mode==RO_INIT_MULTITHREADED&&::GetLastError()==0x661,"RoInitialize exact ABI preserves signed HRESULT, mode and LastError");
        runtime.call_ro_uninitialize(reinterpret_cast<std::uintptr_t>(&ro_uninitialize));check(state.unro_calls==1&&::GetLastError()==0x662,"RoUninitialize exact ABI and output LastError retained");
        state.policy=-23;state.policy_result=static_cast<LONG>(-2147024891);std::int32_t out=42;
        check(runtime.call_thread_initialization_policy(reinterpret_cast<std::uintptr_t>(&policy),~std::uintptr_t{5},&out)==0x80070005U&&out==-23&&state.policy_token==reinterpret_cast<HANDLE>(~std::uintptr_t{5})&&::GetLastError()==0x663,"AppPolicy failure still transfers signed output bits without forged enum alias");
        rejects<std::invalid_argument>([&]{runtime.call_ro_initialize(reinterpret_cast<std::uintptr_t>(&ro_initialize),2);},"unsupported RO enum input rejected before native call");
        rejects<std::invalid_argument>([&]{runtime.call_thread_initialization_policy(reinterpret_cast<std::uintptr_t>(&policy),0,nullptr);},"null AppPolicy output cell rejected");
        check(runtime.close(),"typed direct policy calls leave no module ownership behind");
    }
    {
        td::NativeRuntime runtime(runtime_api());anadius::crt_runtime_recovered::State source(0x789abc);state.policy=1;state.policy_result=0;state.ro_result=0;
        const auto old_flags=state.flag_calls,old_policies=state.policy_calls;
        check(anadius::crt_runtime_recovered::runtime_policy_06c0d0(source,runtime)==2&&state.flag_calls==old_flags+1&&state.policy_calls==old_policies+1,"actual recovered runtime source queries injected OS policy and chooses policy2");
        check(source.thread_policy==2&&anadius::crt_runtime_recovered::runtime_policy_06c0d0(source,runtime)==2&&state.policy_calls==old_policies+1,"actual source policy cache suppresses duplicate OS invocation");
        check(anadius::crt_runtime_recovered::initialize_runtime_067320(source,runtime,1)==0,"actual source resolves and calls rebuilt typed RoInitialize");
        anadius::crt_runtime_recovered::uninitialize_runtime_06736c(source,runtime);
        check(runtime.outstanding_modules()==2&&state.names[state.names.size()-2]==L"api-ms-win-appmodel-runtime-l1-1-2"&&state.names.back()==L"api-ms-win-core-winrt-l1-1-0","actual source module candidate identities reach owning runtime backend");
        check(runtime.close(),"actual source's cached module references close after RO pair");
    }
    runtime_state=nullptr;
    {
        td::NativeRuntime runtime;::SetLastError(0x7654);
        const auto kernel=runtime.load_library_ex_w(u"kernel32.dll",0,LOAD_LIBRARY_SEARCH_SYSTEM32);
        check(kernel!=0&&runtime.get_proc_address(kernel,"GetCurrentThreadId")!=0,"actual own system-only module load/lookup works without loading original targets");
        check(runtime.close()&&runtime.outstanding_modules()==0,"actual owned system module reference released");
    }
}
void locale_and_domain_lifecycle(){
    {
        Fixture f;check(f.domain->phase()==td::Phase::Constructed&&f.domain->locale_references()==1&&f.domain->multibyte_references()==0,"owned relocated initial locale has expected reference baselines before FLS init");
        check(f.domain->initialize()&&f.domain->phase()==td::Phase::Ready,"real heap/locks/FLS domain initializes");
        check(f.memory.initialized.size()==3&&f.domain->source_fls_index()==f.domain->native_fls_index(),"domain owns only locks0/4/5 and shares source/native FLS identity");
        check(f.domain->locale_references()==2&&f.domain->multibyte_references()==1&&f.memory.blocks.size()==1,"real main PTD owns exactly one locale/MB reference and real heap block");
        ::SetLastError(0x9abc);auto* main=f.domain->current_ptd();check(main&&::GetLastError()==0x9abc,"actual required PTD lookup preserves current-thread LastError");
        const anadius::crt_ptd_data_recovered::View view(main);const auto ids=f.domain->locale().ids();
        check(view.load<std::uintptr_t>(0)==ids.exception_table&&view.load<std::uintptr_t>(0x88)==ids.multibyte_info&&view.load<std::uintptr_t>(0x90)==ids.initial_locale,"actual PTD constructor uses the owned relocated domain identities");
        f.domain->errno_cell().store(73);check(f.domain->errno_cell().load()==73,"error proxy resolves same live PTD error cell");
        rejects<std::logic_error>([&]{f.domain->initialize();},"ready domain cannot initialize twice");
        bool rejected=false;std::thread other([&]{try{f.domain->close();}catch(const std::logic_error&){rejected=true;}});other.join();check(rejected&&f.domain->phase()==td::Phase::Ready,"foreign control thread cannot start domain teardown");
        auto provider=f.domain->make_threads();check(f.domain->provider_leases()==1&&!f.domain->close()&&f.domain->phase()==td::Phase::Stopping,"thread provider lease blocks domain FLS teardown");
        check(f.memory.blocks.size()==1&&f.memory.slot_releases==0,"blocked close retains main PTD and FLS callbacks");provider.reset();
        check(f.domain->close()&&f.domain->phase()==td::Phase::Closed&&f.memory.blocks.empty(),"provider destruction permits native FLS teardown before lock destruction");
        check(f.domain->locale_references()==1&&!f.domain->multibyte_references()&&f.memory.destroyed.size()==3,"main PTD release restores image counters and deletes all owned locks");
        check(f.memory.destroyed[0]==f.memory.initialized[2]&&f.memory.destroyed[1]==f.memory.initialized[1]&&f.memory.destroyed[2]==f.memory.initialized[0],"lock teardown occurs in exact reverse construction order");
        check(f.domain->close(),"domain close is idempotent");
        rejects<std::logic_error>([&]{f.domain->current_ptd();},"closed domain excludes new PTD admission");
        rejects<std::logic_error>([&]{f.domain->make_threads();},"closed domain excludes new thread provider admission");
    }
    {
        anadius::crt_heap_recovered::State state{0,0,0,0};UnusedHeap heap;anadius::crt_lock_native::Banks banks;
        std::array<std::byte,64> exception_table{};td::InitialLocale locale(state,heap,banks,reinterpret_cast<std::uintptr_t>(exception_table.data()));
        rejects<std::out_of_range>([&]{locale.add_locale_ref_070634(1);},"foreign locale never reaches native reference memory");
        rejects<std::out_of_range>([&]{locale.release_locale_ref_0708c0(0);},"unsupported null locale release rejected at domain boundary");
        rejects<std::out_of_range>([&]{locale.locale_ref_count_10(1);},"foreign locale count read rejected");
        rejects<std::out_of_range>([&]{locale.atomic_fetch_add_32(locale.ids().initial_locale+0x10,1);},"PTD MB effect cannot arbitrarily update locale/foreign counter");
        rejects<std::out_of_range>([&]{locale.free_locale_0706c0(locale.ids().initial_locale);},"image-initial locale cannot be deep-freed");
        rejects<std::out_of_range>([&]{locale.lock_0625f8(0);},"locale data provider admits only lock4/5");
        check(locale.image().counter_value(locale.ids().initial_locale+0x10)==1&&locale.image().counter_value(locale.ids().multibyte_info)==0,"rejected foreign effects leave owned counters unchanged");
    }
    auto invalid=seed();invalid.exception_table_lifetime.reset();rejects<std::invalid_argument>([&]{td::DomainStorage::create(invalid);},"exception identity requires backing lifetime owner");
    invalid=seed();invalid.exception_table_identity=0;rejects<std::invalid_argument>([&]{td::DomainStorage::create(invalid);},"zero exception identity rejected before FLS publication");
}
void domain_failure_prefixes(){
    {
        Fixture f;f.memory.no_heap=true;check(!f.domain->initialize()&&f.domain->phase()==td::Phase::Constructed&&f.memory.initialized.empty(),"missing process heap fails before any lock/FLS publication");
        f.memory.no_heap=false;check(f.domain->initialize()&&f.domain->close(),"clean heap failure supports initialize retry");
    }
    {
        Fixture f;f.memory.lock_failure_call=2;check(!f.domain->initialize()&&f.domain->phase()==td::Phase::Constructed,"lock initialization failure returns clean constructed phase");
        check(f.memory.initialized.size()==2&&f.memory.destroyed==std::vector<LPCRITICAL_SECTION>{f.memory.initialized[0]}&&!f.memory.slot_allocations,"failed lock prefix rolls back only successfully initialized first lock");
        f.memory.lock_failure_call=0;check(f.domain->initialize()&&f.domain->close(),"lock prefix rollback permits full initialize retry");
    }
    {
        Fixture f;f.memory.wide_lock_success=true;
        check(f.domain->initialize()&&f.domain->phase()==td::Phase::Ready,"SDK BOOL success0x100 is true even when low byte is zero");
        check(f.domain->close()&&f.memory.destroyed.size()==3&&f.memory.blocks.empty(),"all three true SDK lock results remain owned and are destroyed");
    }
    for(unsigned failure=0;failure!=3;++failure){
        Fixture f;if(failure==0)f.memory.deny_slot=true;if(failure==1)f.memory.deny_alloc=true;if(failure==2)f.memory.deny_publish=true;
        check(!f.domain->initialize()&&f.domain->phase()==td::Phase::Constructed&&f.memory.blocks.empty(),"normal FLS allocation/publication failures roll back heap ownership");
        check(f.domain->native_fls_index()==ptd::invalid_fls_index&&f.domain->source_fls_index()==ptd::invalid_fls_index&&f.memory.destroyed.size()==3,"normal FLS failure releases native slot and all initialized locks");
        check(f.domain->locale_references()==1&&f.domain->multibyte_references()==0,"failure before PTD construction leaves image reference baselines unchanged");
        f.memory.deny_slot=false;f.memory.deny_alloc=false;f.memory.deny_publish=false;check(f.domain->initialize()&&f.domain->close(),"normal initializer failure permits retry with same owned domain");
    }
    {
        Fixture f;check(f.domain->initialize(),"release-failure case owns initialized real PTD");const auto slot=f.domain->native_fls_index();f.memory.deny_release=true;const auto releases=f.memory.slot_releases;
        check(!f.domain->close()&&f.domain->phase()==td::Phase::Stopping,"failed native FLS release retains domain for close retry");
        check(f.domain->source_fls_index()==ptd::invalid_fls_index&&f.domain->native_fls_index()==slot&&f.memory.slot_releases==releases+1,"source invalidation does not erase retained native slot or silently retry");
        check(f.memory.blocks.size()==1&&f.memory.destroyed.empty()&&f.domain->locale_references()==2,"failed release keeps callback data, heap block and locks live");
        check(!f.domain->close()&&f.memory.slot_releases==releases+2,"explicit close retry attempts native release once");f.memory.deny_release=false;
        check(f.domain->close()&&f.memory.blocks.empty()&&f.domain->locale_references()==1&&!f.domain->multibyte_references(),"successful native retry runs real FLS destructor and restores reference baselines");
    }
    {
        Fixture f;f.memory.deny_alloc=true;f.memory.deny_release=true;
        check(!f.domain->initialize()&&f.domain->phase()==td::Phase::ReleasePending,"initializer failure plus native release failure retains a release-pending owner");
        check(f.domain->source_fls_index()==ptd::invalid_fls_index&&f.domain->native_fls_index()!=ptd::invalid_fls_index&&f.memory.destroyed.empty(),"release-pending constructor failure retains slot and all callbacks' lock dependencies");
        rejects<std::logic_error>([&]{f.domain->initialize();},"release-pending domain cannot overwrite its retained native slot");
        f.memory.deny_alloc=false;check(!f.domain->close(),"release-pending close failure stays retryable");f.memory.deny_release=false;check(f.domain->close()&&f.memory.blocks.empty(),"release-pending cleanup succeeds after actual slot release");
    }
}
int terminal(const std::string& mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::set_terminate([]{::TerminateProcess(::GetCurrentProcess(),87);std::_Exit(87);});
    if(mode=="--runtime-poison"){
        RuntimeState s;runtime_state=&s;s.throw_load=true;td::NativeRuntime r(runtime_api());bool escaped=false;try{r.load_library_ex_w(u"unknown",0,0);}catch(const std::runtime_error&){escaped=true;}
        std::_Exit(escaped&&r.tracking_poisoned()&&!r.close()?86:89);
    }
    if(mode=="--runtime-live"){RuntimeState s;runtime_state=&s;td::NativeRuntime r(runtime_api());r.load_library_ex_w(u"live",0,0);return 90;}
    Fixture f;check(f.domain->initialize(),"own terminal child initializes domain");
    if(mode=="--domain-live"){f.domain.reset();return 91;}
    if(mode=="--invalid-abort"){
        std::signal(SIGABRT,[](int){::TerminateProcess(::GetCurrentProcess(),88);std::_Exit(88);});auto provider=f.domain->make_threads();std::uint32_t id=0;
        anadius::threads_recovered::begin_thread_05e8e0(nullptr,0,0,0,0,&id,*provider);std::_Exit(92);
    }
    std::_Exit(93);
}
}
int main(int argc,char** argv){if(argc==2)return terminal(argv[1]);try{
    runtime_references_and_error();runtime_typed_policy_and_source();locale_and_domain_lifecycle();domain_failure_prefixes();
    for(const auto* mode:{"--runtime-live","--domain-live"})check(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==87,"own live-owner destructor child enforces retained-resource terminal contract");
    check(_spawnl(_P_WAIT,argv[0],argv[0],"--runtime-poison",static_cast<const char*>(nullptr))==86,"unknown native load outcome remains poisoned and unclosable");
    check(_spawnl(_P_WAIT,argv[0],argv[0],"--invalid-abort",static_cast<const char*>(nullptr))==88,"selected compiler invalid-parameter policy aborts in own child");
    std::cout<<"PASS: "<<checks<<" thread-domain core checks; owned initial locale, typed native runtime and real heap/FLS/lock lifecycle with rollback/release retry; originals never executed\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
