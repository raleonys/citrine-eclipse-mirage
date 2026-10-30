#include "preloader_l/net/transport_lifecycle_native.hpp"

namespace preloader_l::socket_recovered {
static_assert(sizeof(void*)==8 && sizeof(LONG)==4);
void NativeSocketGate::enter(){imports_.enter(&section_);}
void NativeSocketGate::leave(){imports_.leave(&section_);}
} // namespace preloader_l::socket_recovered
