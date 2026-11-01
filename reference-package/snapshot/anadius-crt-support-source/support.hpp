#pragma once
#include <cstdint>

namespace anadius::crt_support_recovered {
using Address=std::uintptr_t;
// Opaque rebuilt provider identities; never original preferred image addresses.
struct Head {Address token;};
struct State {
    std::uint64_t printf_options_0a37d0{};
    std::uint64_t scanf_options_0a37d8{};
    Address tls_callback_0a5a70{};
};
class Effects {
public:
    virtual ~Effects()=default;
    // The same head token maps to live provider-owned native/fixture storage.
    virtual void initialize_slist_head(Head)=0;
    virtual Address interlocked_flush_slist(Head)=0;
    // Nodes expose a live pointer-sized next field at offset zero, before free.
    virtual Address next(Address node)=0;
    virtual void free_05caec(Address node)=0;
};
std::uint64_t* printf_options_0067e0(State&) noexcept;
std::uint64_t* scanf_options_0067f0(State&) noexcept;
void configure_stdio_04c1fc(State&) noexcept;
const Address* tls_callback_slot_04c218(const State&) noexcept;
void initialize_slist_04c1e0(Head,Effects&);
void release_list_04d2f8(Head,Effects&);
void release_slist_04c1f0(Head,Effects&);
}
