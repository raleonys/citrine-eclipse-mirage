#pragma once
#include "../anadius-online-source/online.hpp"

namespace anadius::online_object {
using Object=online_recovered::Object;
// Observed Windows x64 caller shapes; not a recovered foreign C++ class layout.
using FactoryFunction=Object(*)(const char16_t*);
using DestroyFunction=void(*)(void*);
// Only for a factory-adopted object. Object first8 bytes and its vptr first8
// bytes must be readable; selected destroy code must be callable while leased.
// Byte copies avoid typed class/vtable aliasing. No memory probe, repair,
// deleting-destructor flag, or guessed C++ virtual destructor is introduced.
DestroyFunction destroy_slot(Object);
} // namespace anadius::online_object
