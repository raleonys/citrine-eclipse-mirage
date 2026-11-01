#pragma once
#include "../anadius-crt-initializers-source/initializers.hpp"
#include "../anadius-crt-locale-startup-source/locale_startup.hpp"
#include "../anadius-crt-cache-source/cache.hpp"
#include "../anadius-crt-lock-source/locks.hpp"
#include "../anadius-crt-pair-leaves-source/pair_leaves.hpp"
#include "../anadius-crt-lowio-lifecycle-source/lowio_lifecycle.hpp"
#include "../anadius-crt-multibyte-control-source/multibyte_control.hpp"
#include "../anadius-crt-argv-bridge-source/argv_bridge.hpp"
#include "../anadius-crt-environment-cleanup-source/environment_cleanup.hpp"
#include "../anadius-crt-lifecycle-callbacks-source/callbacks.hpp"
#include <array>
#include <span>
namespace anadius::crt_pair_bindings {
using Address=crt_initializers_recovered::Address;
using Pair=crt_initializers_recovered::Pair;
// Opaque source-only identities, never preferred VAs or function pointers.
enum class Id:Address {
    none=0,locale_initialize=1,encoded_initialize,thunks_initialize,success,
    locks_initialize,heap_initialize,ptd_initialize,lowio_initialize,
    command_lines_initialize,multibyte_initialize,onexit_initialize,
    thunks_uninitialize,locks_uninitialize,heap_uninitialize,vcrt_uninitialize,
    ptd_uninitialize,lowio_uninitialize,environment_uninitialize,argv_cleanup,
    multibyte_uninitialize
};
constexpr Address token(Id id) noexcept {return static_cast<Address>(id);}
inline constexpr std::size_t pair_count=16;
std::array<Pair,pair_count> rebuilt_pairs() noexcept;
struct Dependencies {
    crt_locale_startup_recovered::Effects& locale;
    crt_cache_recovered::EncodedGlobals& encoded;
    crt_runtime_recovered::State& runtime;
    crt_cache_recovered::CacheEffects& cache_effects;
    crt_locks_recovered::State& locks;
    crt_locks_recovered::Effects& lock_effects;
    crt_heap_recovered::State& heap;
    crt_heap_recovered::Effects& heap_effects;
    vcrt_ptd_recovered::State& vcrt;
    vcrt_ptd_recovered::Effects& vcrt_effects;
    crt_ptd_recovered::State& ptd;
    crt_ptd_recovered::Effects& ptd_effects;
    crt_lowio_lifecycle_recovered::State& lowio;
    crt_lowio_lifecycle_recovered::Effects& lowio_effects;
    crt_pair_leaves_recovered::CommandLines& command_lines;
    crt_pair_leaves_recovered::CommandLineEffects& command_line_effects;
    const crt_multibyte_control_recovered::Identities& multibyte_ids;
    crt_multibyte_control_recovered::Effects& multibyte_effects;
    crt_argv_bridge::CleanupState& argv_cleanup;
    crt_argv_recovered::Effects& argv_effects;
    crt_environment_cleanup_recovered::State& environment;
    crt_environment_cleanup_recovered::Effects& environment_effects;
    crt_termination_recovered::State& termination;
    crt_lifecycle_callbacks_recovered::MultibyteEffects& multibyte_shutdown;
};
// New composition, zero new original bodies. Borrowed dependencies and this
// nonmoving fixed-size mutable table owner stay live through all walker calls.
// Providers must share their actual heap/lock/PTD/global domains. Constructor
// validates the identities that these existing public types can expose.
class Effects:public crt_initializers_recovered::Effects {
public:
    explicit Effects(Dependencies);
    Effects(const Effects&)=delete;
    Effects& operator=(const Effects&)=delete;
    Effects(Effects&&)=delete;
    Effects& operator=(Effects&&)=delete;
    std::span<Pair,pair_count> pairs() noexcept;
    std::span<const Pair,pair_count> pairs() const noexcept;
    std::uint64_t invoke_boolean_initializer(Address) final;
    void invoke_uninitializer(Address,std::uint32_t argument) final;
    // invoke_void_initializer, invoke_int_initializer and common_flush_all
    // intentionally remain pure mandatory policy boundaries.
private:
    const Dependencies deps_;
    std::array<Pair,pair_count> pairs_;
};
}
