#include "bindings.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>
namespace anadius::crt_pair_bindings {
std::array<Pair,pair_count> rebuilt_pairs() noexcept {
    return {{{token(Id::locale_initialize),0},
        {token(Id::encoded_initialize),0},
        {token(Id::thunks_initialize),token(Id::thunks_uninitialize)},
        {token(Id::success),token(Id::success)},
        {token(Id::locks_initialize),token(Id::locks_uninitialize)},
        {token(Id::heap_initialize),token(Id::heap_uninitialize)},
        {0,token(Id::vcrt_uninitialize)},
        {token(Id::ptd_initialize),token(Id::ptd_uninitialize)},
        {token(Id::lowio_initialize),token(Id::lowio_uninitialize)},
        {token(Id::command_lines_initialize),token(Id::success)},
        {token(Id::multibyte_initialize),0},
        {0,token(Id::success)},
        {0,token(Id::argv_cleanup)},
        {0,token(Id::multibyte_uninitialize)},
        {token(Id::success),token(Id::environment_uninitialize)},
        {token(Id::onexit_initialize),token(Id::success)}}};
}
Effects::Effects(Dependencies dependencies):deps_(dependencies),pairs_(rebuilt_pairs()) {
    if(deps_.runtime.cookie!=deps_.heap.cookie_0a1580 ||
       deps_.runtime.cookie!=deps_.termination.onexit.cookie_0a1580)
        throw std::invalid_argument("Pair bindings require one coherent runtime/heap/onexit cookie");
    if(std::addressof(deps_.termination.encoded_callback_a4f10)!=std::addressof(deps_.encoded.a4f10))
        throw std::invalid_argument("Pair bindings require the actual shared A4F10 cell");
    if(std::addressof(deps_.command_lines.narrow)!=std::addressof(deps_.argv_cleanup.argv))
        throw std::invalid_argument("Pair bindings require the actual shared argv State");
}
std::span<Pair,pair_count> Effects::pairs() noexcept {return pairs_;}
std::span<const Pair,pair_count> Effects::pairs() const noexcept {return pairs_;}
std::uint64_t Effects::invoke_boolean_initializer(Address address) {
    switch(static_cast<Id>(address)) {
    case Id::locale_initialize:
        return crt_locale_startup_recovered::initialize_locale_064468(deps_.locale);
    case Id::encoded_initialize: {
        const auto result=crt_cache_recovered::initialize_encoded_globals_0644b0(deps_.encoded,deps_.runtime.cookie);
        // Existing public structs cannot alias these two A5178 representations.
        // One init-only projection after the actual body; later caller mutations
        // must maintain coherence. This is not a canonical global-state graph.
        deps_.heap.encoded_handler_0a5178=deps_.encoded.a5178;
        return result;
    }
    case Id::thunks_initialize:
        return crt_cache_recovered::initialize_thunks_067538(deps_.runtime);
    case Id::success:
        return crt_pair_leaves_recovered::success_006500();
    case Id::locks_initialize:
        return crt_locks_recovered::initialize_acrt_0625b0(deps_.locks,deps_.lock_effects);
    case Id::heap_initialize:
        return crt_heap_recovered::initialize_heap_0704cc(deps_.heap,deps_.heap_effects);
    case Id::ptd_initialize:
        return crt_ptd_recovered::initialize_064e90(deps_.ptd,deps_.ptd_effects);
    case Id::lowio_initialize:
        return crt_lowio_lifecycle_recovered::initialize_lowio_067890(deps_.lowio_effects);
    case Id::command_lines_initialize:
        return crt_pair_leaves_recovered::initialize_command_lines_06f9bc(deps_.command_lines,deps_.command_line_effects);
    case Id::multibyte_initialize:
        return crt_multibyte_control_recovered::initialize_multibyte_06f5f8(deps_.multibyte_ids,deps_.multibyte_effects);
    case Id::onexit_initialize:
        return crt_lifecycle_callbacks_recovered::initialize_onexit_tables_06447c(deps_.termination);
    default:
        throw std::invalid_argument("Unknown or wrong-domain boolean initializer identity");
    }
}
void Effects::invoke_uninitializer(Address address,std::uint32_t argument) {
    switch(static_cast<Id>(address)) {
    case Id::success:
        (void)crt_pair_leaves_recovered::success_006500();return;
    case Id::thunks_uninitialize:
        (void)crt_cache_recovered::uninitialize_thunks_06755c(deps_.runtime,deps_.cache_effects,static_cast<std::uint8_t>(argument));return;
    case Id::locks_uninitialize:
        (void)crt_locks_recovered::uninitialize_acrt_062614(deps_.locks,deps_.lock_effects);return;
    case Id::heap_uninitialize:
        (void)crt_pair_leaves_recovered::uninitialize_heap_0704e8(deps_.heap);return;
    case Id::vcrt_uninitialize:
        // The actual 0644f0 body forces argument zero, ignoring this caller's arg.
        (void)crt_pair_leaves_recovered::uninitialize_vcrt_0644f0(deps_.vcrt,deps_.vcrt_effects);return;
    case Id::ptd_uninitialize:
        (void)crt_ptd_recovered::uninitialize_064ecc(deps_.ptd,deps_.ptd_effects);return;
    case Id::lowio_uninitialize:
        (void)crt_lowio_lifecycle_recovered::uninitialize_lowio_0678cc(deps_.lowio,deps_.lowio_effects);return;
    case Id::argv_cleanup:
        (void)crt_argv_bridge::cleanup_064538(deps_.argv_cleanup,deps_.argv_effects);return;
    case Id::multibyte_uninitialize:
        (void)crt_lifecycle_callbacks_recovered::uninitialize_multibyte_0644f8(deps_.multibyte_shutdown);return;
    case Id::environment_uninitialize:
        (void)crt_environment_cleanup_recovered::uninitialize_callback_0644a0(deps_.environment,deps_.environment_effects);return;
    default:
        throw std::invalid_argument("Unknown or wrong-domain uninitializer identity");
    }
}
static_assert(std::is_abstract_v<Effects>);
}
