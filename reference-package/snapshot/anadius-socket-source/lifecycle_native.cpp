#include "lifecycle_native.hpp"

namespace anadius::socket_recovered {
static_assert(sizeof(void*)==8 && sizeof(LONG)==4);
void NativeSocketGate::enter(){imports_.enter(&section_);}
void NativeSocketGate::leave(){imports_.leave(&section_);}
} // namespace anadius::socket_recovered
