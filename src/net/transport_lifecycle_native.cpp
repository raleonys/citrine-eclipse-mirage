// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_lifecycle_native.hpp"

namespace citrine_eclipse_mirage::socket {
static_assert(sizeof(void*)==8 && sizeof(LONG)==4);
void NativeSocketGate::enter(){imports_.enter(&section_);}
void NativeSocketGate::leave(){imports_.leave(&section_);}
} // namespace citrine_eclipse_mirage::socket
