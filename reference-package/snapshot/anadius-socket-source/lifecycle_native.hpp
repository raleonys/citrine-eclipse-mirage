#pragma once
#include "lifecycle.hpp"
#include <windows.h>

namespace anadius::socket_recovered {
struct GateImports {
    decltype(&::EnterCriticalSection) enter=&::EnterCriticalSection;
    decltype(&::LeaveCriticalSection) leave=&::LeaveCriticalSection;
};
// Borrows an initialized SDK CRITICAL_SECTION; initialization/deletion/lifetime
// belong to the caller. Injected imports support deterministic offline tests.
class NativeSocketGate final:public SocketGate {
public:
    NativeSocketGate(CRITICAL_SECTION& section,GateImports imports={})
        :section_(section),imports_(imports){}
    void enter() override;
    void leave() override;
private:CRITICAL_SECTION& section_;GateImports imports_;
};
} // namespace anadius::socket_recovered
