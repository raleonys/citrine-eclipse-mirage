#pragma once
#include "../anadius-online-source/online.hpp"
#include <cstddef>

namespace anadius::online_query {
// Selected Win64 caller shapes. No complete foreign class/header ABI claimed.
using StatusFunction=std::int32_t(*)(void*);
using GetterFunction=const char*(*)(void*);
// 031aad consumes AL as a byte; preserve all nonzero AL values as success.
using AvatarFunction=std::uint8_t(*)(void*,const char16_t*,const char*,std::int32_t);
using AuthFunction=const char*(*)(void*,const char*);
enum class Slot:std::size_t {Status08=0x08,Username10=0x10,Persona18=0x18,UserId20=0x20,Avatar28=0x28,Auth30=0x30};
// Adopted object + readable vptr/table + callable code are explicit contracts.
// Uses byte copies; never aliases the foreign object as a fabricated C++ class.
online_recovered::Procedure query_slot(online_recovered::Object,Slot);
} // namespace anadius::online_query
