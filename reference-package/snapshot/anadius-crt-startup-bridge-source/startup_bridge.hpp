#pragma once
#include "../anadius-vcrt-bridge-source/bridge.hpp"
#include "../anadius-crt-support-source/support.hpp"
#include "../anadius-crt-environment-source/environment.hpp"
#include "../anadius-crt-termination-source/termination.hpp"

namespace anadius::crt_startup_bridge {
// Two fixed, live, mutable ranges of rebuilt bindings. Caller callbacks may
// change later entries; the recovered walker rereads each reached entry.
class TerminationTablesBridge:public crt_termination_recovered::Effects {
public:
    TerminationTablesBridge(crt_initializers_recovered::Effects&,
        std::span<crt_initializers_recovered::Address> preterminators,
        std::span<crt_initializers_recovered::Address> terminators);
    void initterm_range_064614(crt_termination_recovered::Address begin,
        crt_termination_recovered::Address end) final;
private:
    crt_initializers_recovered::Effects& table_effects_;
    std::span<crt_initializers_recovered::Address> preterminators_,terminators_;
};
struct Dependencies {
    crt_support_recovered::State& support;
    crt_support_recovered::Head head;
    crt_support_recovered::Effects& list_effects;
    crt_environment_recovered::State& environment;
    crt_environment_recovered::Effects& environment_effects;
    crt_termination_recovered::State& termination;
    crt_termination_recovered::Effects& termination_effects;
};
// New composition, not another recovered original function. All borrowed state,
// storage and callback bindings outlive Entry calls and late callback release.
// Entry's application table must be termination.onexit.application_0a47a8.
// Termination cache callback and cookie must alias the actual shared bindings.
class StartupBridge:public vcrt_bridge::LifecycleBridge {
public:
    StartupBridge(crt_initializers_recovered::State&,
        std::span<crt_initializers_recovered::Pair>,
        std::span<crt_initializers_recovered::Address> c,
        std::span<crt_initializers_recovered::Address> cpp,
        crt_initializers_recovered::Effects&,
        crt_ptd_recovered::State&,crt_ptd_recovered::Effects&,
        vcrt_bridge::Dependencies,Dependencies);
    void initialize_slist_04c1e0() final;
    void release_slist_04c1f0() final;
    void configure_stdio_04c1fc() final;
    const std::uintptr_t* tls_callback_slot_04c218() final;
    void initialize_narrow_environment_064054() final;
    std::uint32_t c_termination_complete_05cdb8() final;
    void cexit_05cd9c() final;
private:
    const Dependencies deps_;
};
}
