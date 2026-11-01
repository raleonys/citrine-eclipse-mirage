#include "argv_bridge.hpp"
#include <bit>

namespace anadius::crt_argv_bridge {
ArgvEffects::ArgvEffects(crt_heap_recovered::State& heap,crt_heap_recovered::Effects& heap_effects,
    crt_errors_recovered::State& errors,crt_errors_recovered::Effects& error_effects,
    crt_invalid_recovered::Effects& invalid,Policy& policy)
    :heap_(heap),heap_effects_(heap_effects),errors_(errors),error_effects_(error_effects),invalid_(invalid),policy_(policy){}
void ArgvEffects::initialize_multibyte_06f5f8(){policy_.initialize_multibyte_06f5f8();}
std::uint32_t ArgvEffects::module_filename_06ecdc(std::span<char,260> output){return policy_.module_filename_06ecdc(output);}
crt_errors_recovered::Cell32 ArgvEffects::errno_05d724(){return crt_errors_recovered::errno_cell_05d724(errors_,error_effects_);}
void ArgvEffects::invalid_parameter_05c954(){crt_invalid_recovered::invalid_parameter_noinfo_05c954(invalid_);}
crt_argv_recovered::Allocation ArgvEffects::calloc_066848(std::uint64_t count,std::uint64_t size) {
    auto* storage=crt_heap_recovered::calloc_base_066848(count,size,heap_,heap_effects_);
    if(!storage)return {};
    // A successful recovered calloc has already passed the multiplication and
    // maximum-size guards; the provider supplies that many live raw bytes.
    return {storage,{static_cast<std::byte*>(storage),static_cast<std::size_t>(count*size)}};
}
void ArgvEffects::free_0668c0(void* value){crt_heap_recovered::free_0668c0(value,heap_,heap_effects_);}
std::int32_t ArgvEffects::expand_06ecd4(std::span<char*> input,crt_argv_recovered::Expansion& output){return policy_.expand_06ecd4(input,output);}
bool cleanup_064538(CleanupState& state,crt_argv_recovered::Effects& effects) {
    effects.free_0668c0(state.narrow_aux_0a58a8);
    auto* second=state.wide_aux_0a58b0;
    state.narrow_aux_0a58a8=nullptr;
    effects.free_0668c0(second);
    auto* third=state.argv.argv;
    state.wide_aux_0a58b0=nullptr;
    effects.free_0668c0(third);
    auto* fourth=state.wide_argv_0a59f8;
    state.argv.argv=nullptr;
    effects.free_0668c0(fourth);
    state.wide_argv_0a59f8=nullptr;
    return true;
}
ArgvBridge::ArgvBridge(crt_initializers_recovered::State& state,
    std::span<crt_initializers_recovered::Pair> pairs,
    std::span<crt_initializers_recovered::Address> c,std::span<crt_initializers_recovered::Address> cpp,
    crt_initializers_recovered::Effects& effects,crt_ptd_recovered::State& ptd,
    crt_ptd_recovered::Effects& ptd_effects,vcrt_bridge::Dependencies vcrt,
    crt_startup_bridge::Dependencies startup,Dependencies deps)
    :StartupBridge(state,pairs,c,cpp,effects,ptd,ptd_effects,vcrt,startup),deps_(deps){}
std::uint32_t ArgvBridge::configure_narrow_argv_0638e8(std::uint32_t mode) {
    return crt_argv_recovered::configure_0638e8(deps_.state,std::bit_cast<std::int32_t>(mode),deps_.effects,deps_.parser);
}
}
