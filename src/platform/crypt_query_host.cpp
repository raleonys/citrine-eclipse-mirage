// SPDX-License-Identifier: GPL-3.0-only
#include "citrine_eclipse_mirage/platform/crypt_query.hpp"

namespace citrine_eclipse_mirage::platform {
namespace {
bool read_unit(std::uintptr_t address,char16_t& unit,void* process) noexcept {
    SIZE_T copied=0;
    return ReadProcessMemory(process,reinterpret_cast<const void*>(address),&unit,sizeof unit,&copied)&&copied==sizeof unit;
}
}
hooks::PathProbe WindowsCryptQueryHost::inspect_path(const void* object) noexcept {
    return hooks::probe_path(object,&read_unit,GetCurrentProcess());
}
}
