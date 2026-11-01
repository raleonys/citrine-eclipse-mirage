#pragma once
#include <cstdint>

namespace anadius::crt_locale_ref_recovered {
using Address = std::uint64_t;
struct Identities {
    Address category_name_0a1948;
    Address initial_time_08ad20;
};
struct Effects {
    virtual ~Effects() = default;
    virtual Address read_address(Address address) = 0;
    // Must perform one atomic, modulo-2^32 update and return its previous value.
    virtual std::uint32_t atomic_fetch_add_32(Address address, std::uint32_t increment) = 0;
};
void add_locale_ref_070634(Address locale, const Identities&, Effects&);
void release_locale_ref_0708c0(Address locale, const Identities&, Effects&);
std::int32_t add_time_ref_070838(Address time, const Identities&, Effects&);
std::int32_t release_time_ref_070898(Address time, const Identities&, Effects&);
} // namespace anadius::crt_locale_ref_recovered
