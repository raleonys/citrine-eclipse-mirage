#include "runtime.hpp"

namespace anadius::initial_cells {
bootstrap_runtime::OutsideConstructorState make_outside_defaults(){
    // a45e0: zero-filled image QPF cache, populated lazily by04a824.
    // a3828/a3830:001010 empty map sentinel with initial zero count.
    // a3810..a3828: zero vector begin/end/capacity; a3838..a3888:10zero slots.
    return {0,{},{},{}};
}
std::unique_ptr<bootstrap_runtime::Runtime> make_fresh_runtime(
    bootstrap_backend::Resources& resources,bootstrap_native_recovered::Errors& errors,
    bootstrap_runtime::HostApi host){
    auto outside=make_outside_defaults();
    return bootstrap_runtime::make_observed_runtime_036d20(std::move(outside),resources,errors,host);
}
}
