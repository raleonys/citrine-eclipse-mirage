#include "locks.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace locks = anadius::crt_locks_recovered;
namespace runtime = anadius::crt_runtime_recovered;
namespace {
int checks{};
void check(bool condition, const char* text) {
    ++checks;
    if (!condition) throw std::runtime_error(text);
}
struct Stop {};
struct Event { std::string kind; locks::LockRef lock; };
struct Fake : locks::Effects {
    std::vector<Event> events;
    int failure{-1};
    std::uint64_t success{0x100};
    std::uint64_t failed{0x100000000ULL};
    std::function<void(locks::LockRef)> on_init, on_delete;
    std::uint64_t init(locks::LockRef lock, std::uint32_t spin,
                       std::uint32_t flags, const char* kind) {
        check(spin == 4000 && flags == 0, "fixed spin/flags");
        events.push_back({kind, lock});
        if (on_init) on_init(lock);
        return lock.index == failure ? failed : success;
    }
    std::uint64_t initialize_acrt_06707c(locks::LockRef l, std::uint32_t s,
                                      std::uint32_t f) override { return init(l,s,f,"acrt-init"); }
    std::uint64_t initialize_vcrt_050c38(locks::LockRef l, std::uint32_t s,
                                      std::uint32_t f) override { return init(l,s,f,"vcrt-init"); }
    void delete_critical_section(locks::LockRef l) override {
        events.push_back({"delete",l});
        if (on_delete) on_delete(l);
    }
    void enter_critical_section(locks::LockRef l) override { events.push_back({"enter",l}); }
    void leave_critical_section(locks::LockRef l) override { events.push_back({"leave",l}); }
};
void failed_initialization() {
    for (int failure=0; failure<14; ++failure) {
        locks::State state{}; Fake effects; effects.failure=failure;
        check(!locks::initialize_acrt_0625b0(state,effects), "failure returns AL zero");
        check(state.acrt_count == 0 && state.vcrt_count == 0, "rollback counts");
        check(effects.events.size() == std::size_t(2*failure+1), "failure call count");
        for (int i=0; i<=failure; ++i) {
            const auto& event=effects.events[std::size_t(i)];
            check(event.kind == "acrt-init", "ACRT distinct provider");
            check(event.lock == locks::LockRef{locks::Bank::acrt,i}, "forward init index");
            check(event.lock.byte_offset() == 40*i, "40 byte stride");
        }
        for (int i=0; i<failure; ++i) {
            const auto& event=effects.events[std::size_t(failure+1+i)];
            check(event.kind == "delete", "rollback delete");
            check(event.lock == locks::LockRef{locks::Bank::acrt,failure-1-i}, "reverse rollback");
        }
    }
    locks::State state{}; Fake effects; effects.failure=0;
    check(!locks::initialize_vcrt_05093c(state,effects), "VCRT first-slot failure");
    check(effects.events.size()==1 && effects.events[0].kind=="vcrt-init", "VCRT distinct provider");
    check(state.vcrt_count==0, "VCRT zero-count rollback");
}
void successful_initialization() {
    for (auto value : std::array<std::uint64_t,5>{1,0x100,0x80000000ULL,0xffffffffULL,0x1122334400000100ULL}) {
        locks::State state{}; Fake effects; effects.success=value;
        check(locks::initialize_acrt_0625b0(state,effects), "nonzero low EAX succeeds");
        check(state.acrt_count==14 && state.vcrt_count==0, "ACRT initialized count");
        check(locks::initialize_vcrt_05093c(state,effects), "VCRT success");
        check(state.vcrt_count==1 && state.acrt_count==14, "separate VCRT count");
        check(locks::uninitialize_acrt_062614(state,effects), "ACRT AL true");
        check(locks::uninitialize_vcrt_050984(state,effects), "VCRT AL true");
        check(state.acrt_count==0 && state.vcrt_count==0, "cleanup final count");
    }
    for (std::uint32_t count=0; count<=14; ++count) {
        locks::State state{count,0}; Fake effects;
        check(locks::uninitialize_acrt_062614(state,effects), "partial cleanup true");
        check(effects.events.size()==count, "partial cleanup cardinality");
        for (std::uint32_t i=0; i<count; ++i)
            check(effects.events[i].lock.index==std::int64_t(count-i-1), "partial cleanup index");
        check(state.acrt_count==0, "partial cleanup state");
    }
}
void callback_mutation_and_exceptions() {
    for (auto bank : {locks::Bank::acrt,locks::Bank::vcrt}) {
        locks::State state{}; Fake effects;
        auto& cell=bank==locks::Bank::acrt ? state.acrt_count : state.vcrt_count;
        cell=bank==locks::Bank::acrt ? 3 : 1;
        effects.on_delete=[&](locks::LockRef l) { if(l.index==0)cell=0; else if(l.index==2)cell=100; };
        const auto result=bank==locks::Bank::acrt
            ? locks::uninitialize_acrt_062614(state,effects)
            : locks::uninitialize_vcrt_050984(state,effects);
        check(result, "callback mutation cleanup result");
        check(cell==std::numeric_limits<std::uint32_t>::max(), "global decrement after callback wraps");
        check(effects.events.size()==(bank==locks::Bank::acrt ? 3u:1u), "local snapshot controls cleanup");
    }
    locks::State state{}; Fake effects;
    effects.on_init=[&](locks::LockRef l){ if(l.index==1)state.acrt_count=99; };
    check(locks::initialize_acrt_0625b0(state,effects), "counter mutation initialization success");
    check(state.acrt_count==112, "increments read current global value");
    state={3,0}; effects.events.clear();
    effects.on_delete=[](locks::LockRef){throw Stop{};};
    try { locks::uninitialize_acrt_062614(state,effects); check(false,"delete must throw"); }
    catch(const Stop&) {check(state.acrt_count==3,"throw occurs before decrement");}
    check(effects.events.size()==1 && effects.events[0].lock.index==2,"no invented delete exception cleanup");
    state={}; effects.events.clear();
    effects.on_init=[](locks::LockRef l){if(l.index==2)throw Stop{};};
    try { locks::initialize_acrt_0625b0(state,effects); check(false,"initialize must throw"); }
    catch(const Stop&) {check(state.acrt_count==2,"successful prior increments retained");}
    check(effects.events.size()==3,"no invented initialize exception rollback");
}
void index_and_domains() {
    Fake effects;
    for (auto index : std::array<std::int32_t,7>{0,1,13,-1,14,INT32_MIN,INT32_MAX}) {
        locks::lock_acrt_0625f8(effects,index); locks::unlock_acrt_06264c(effects,index);
        for(std::size_t i=effects.events.size()-2;i<effects.events.size();++i) {
            check(effects.events[i].lock.index==std::int64_t(index), "signed ECX extension");
            check(effects.events[i].lock.byte_offset()==std::int64_t(index)*40,"signed index stride");
            check(effects.events[i].lock.target_base_va()==0x1800a4f40ULL,"base is metadata only");
        }
    }
    check(locks::LockRef{locks::Bank::vcrt,0}.target_base_va()==0x1800a4e78ULL,"VCRT metadata base");
    for (int operation=0;operation<4;++operation) {
        locks::State state{15,2}; effects.events.clear();
        try {
            if(operation==0)locks::initialize_acrt_0625b0(state,effects);
            if(operation==1)locks::uninitialize_acrt_062614(state,effects);
            if(operation==2)locks::initialize_vcrt_05093c(state,effects);
            if(operation==3)locks::uninitialize_vcrt_050984(state,effects);
            check(false,"unsupported domain must be explicit");
        }catch(const std::domain_error&){check(effects.events.empty(),"invalid bank count has no fake success");}
    }
}
struct Resolver : runtime::Effects {
    std::vector<std::string> loaded;
    runtime::Address procedure{0x1234};
    bool first_module_missing{};
    runtime::Module load_library_ex_w(std::u16string_view name,runtime::Module file,std::uint32_t flags) override {
        check(file==0 && flags==0x800,"resolver load metadata");
        loaded.emplace_back(name.begin(),name.end());
        return first_module_missing && loaded.size()==1 ? 0 : 0x4321;
    }
    std::uint32_t last_error() override {return 0x7e;}
    bool free_library(runtime::Module) override {throw std::runtime_error("unexpected free");}
    runtime::Address get_proc_address(runtime::Module module,std::string_view name) override {
        check(module==0x4321 && name=="InitializeCriticalSectionEx","procedure lookup");return procedure;
    }
    std::uint32_t process_parameters_flags() override {throw Stop{};}
    std::int32_t call_ro_initialize(runtime::Address,std::uint32_t) override {throw Stop{};}
    void call_ro_uninitialize(runtime::Address) override {throw Stop{};}
    std::uint32_t call_thread_initialization_policy(runtime::Address,std::uintptr_t,std::int32_t*) override {throw Stop{};}
};
struct Initialize : locks::InitializeEffects {
    int dynamic_calls{},fallback_calls{};
    std::uint64_t result{0xaabbccddeeff0011ULL};
    locks::LockRef expected{locks::Bank::acrt,8};
    std::uint64_t call_initialize_ex(runtime::Address proc,locks::LockRef lock,std::uint32_t spin,std::uint32_t flags) override {
        ++dynamic_calls;check(proc==0x1234 && lock==expected && spin==0x12345678 && flags==0x87654321,"dynamic call preserves args");return result;
    }
    std::uint64_t initialize_and_spin(locks::LockRef lock,std::uint32_t spin) override {
        ++fallback_calls;check(lock==expected && spin==0x12345678,"fallback call drops only flags");return result;
    }
};
void acrt_api() {
    for (bool first_missing : {false,true}) for (bool available : {false,true}) {
        runtime::State state{0x123456789abc}; Resolver resolver; Initialize effects;
        resolver.procedure=available?0x1234:0;
        resolver.first_module_missing=first_missing;
        check(locks::initialize_acrt_api_06707c(state,resolver,effects,effects.expected,0x12345678,0x87654321)==effects.result,"provider result unchanged");
        check(effects.dynamic_calls==(available?1:0) && effects.fallback_calls==(available?0:1),"conditional provider dispatch");
        check(resolver.loaded.size()==(first_missing?2u:1u),"candidate iteration; loaded module without export stops");
        check(resolver.loaded[0]=="api-ms-win-core-synch-l1-2-0","candidate module 7");
        if(first_missing)check(resolver.loaded[1]=="kernel32","candidate module 16");
        const auto before=resolver.loaded.size();
        locks::initialize_acrt_api_06707c(state,resolver,effects,effects.expected,0x12345678,0x87654321);
        check(resolver.loaded.size()==before,"resolved/missing cache reused");
    }
}
}
int main() {
    try {failed_initialization();successful_initialization();callback_mutation_and_exceptions();index_and_domains();acrt_api();
        std::cout << "PASS " << checks << " checks\n";
    } catch(const std::exception& e) {std::cerr << "FAIL " << e.what() << '\n';return 1;}
}
