#pragma once
#include "../anadius-crt-startup-bridge-source/startup_bridge.hpp"
#include "../anadius-crt-argv-source/argv.hpp"
#include "../anadius-crt-heap-source/heap.hpp"
#include "../anadius-crt-invalid-source/invalid.hpp"

namespace anadius::crt_argv_bridge {
class Policy {
public:
    virtual ~Policy()=default;
    virtual void initialize_multibyte_06f5f8()=0;
    virtual std::uint32_t module_filename_06ecdc(std::span<char,260>)=0;
    virtual std::int32_t expand_06ecd4(std::span<char*>,crt_argv_recovered::Expansion&)=0;
};
// Actual recovered heap/error/invalid-parameter cores. Native module path,
// multibyte setup and wildcard search remain mandatory policies.
class ArgvEffects final:public crt_argv_recovered::Effects {
public:
    ArgvEffects(crt_heap_recovered::State&,crt_heap_recovered::Effects&,
        crt_errors_recovered::State&,crt_errors_recovered::Effects&,
        crt_invalid_recovered::Effects&,Policy&);
    void initialize_multibyte_06f5f8() override;
    std::uint32_t module_filename_06ecdc(std::span<char,260>) override;
    crt_errors_recovered::Cell32 errno_05d724() override;
    void invalid_parameter_05c954() override;
    crt_argv_recovered::Allocation calloc_066848(std::uint64_t,std::uint64_t) override;
    void free_0668c0(void*) override;
    std::int32_t expand_06ecd4(std::span<char*>,crt_argv_recovered::Expansion&) override;
private:
    crt_heap_recovered::State& heap_;
    crt_heap_recovered::Effects& heap_effects_;
    crt_errors_recovered::State& errors_;
    crt_errors_recovered::Effects& error_effects_;
    crt_invalid_recovered::Effects& invalid_;
    Policy& policy_;
};
struct CleanupState {
    crt_argv_recovered::State& argv;
    void* narrow_aux_0a58a8{};
    void* wide_aux_0a58b0{};
    void* wide_argv_0a59f8{};
};
// Original 064538 normal body. Each following pointer is loaded before clearing
// the preceding cell, matching the instruction order. argc is not cleared.
bool cleanup_064538(CleanupState&,crt_argv_recovered::Effects&);
struct Dependencies {
    crt_argv_recovered::State& state;
    crt_argv_recovered::Effects& effects;
    crt_argv_parse_recovered::Effects& parser;
};
class ArgvBridge:public crt_startup_bridge::StartupBridge {
public:
    ArgvBridge(crt_initializers_recovered::State&,
        std::span<crt_initializers_recovered::Pair>,
        std::span<crt_initializers_recovered::Address> c,
        std::span<crt_initializers_recovered::Address> cpp,
        crt_initializers_recovered::Effects&,
        crt_ptd_recovered::State&,crt_ptd_recovered::Effects&,
        vcrt_bridge::Dependencies,crt_startup_bridge::Dependencies,Dependencies);
    std::uint32_t configure_narrow_argv_0638e8(std::uint32_t) final;
private:
    const Dependencies deps_;
};
}
