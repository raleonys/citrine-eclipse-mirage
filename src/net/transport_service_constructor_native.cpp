// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_service_constructor_native.hpp"

namespace citrine_eclipse_mirage::socket {
std::int32_t NativeServiceGateApi::initialize(CRITICAL_SECTION& section,std::uint32_t spin_count){
    return imports_.initialize(&section,spin_count);
}
std::uint32_t NativeServiceGateApi::get_last_error(){return imports_.get_last_error();}
} // namespace citrine_eclipse_mirage::socket
