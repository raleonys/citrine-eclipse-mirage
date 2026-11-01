#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>
#include <map>

namespace anadius::crt_counter_native {
using Address=std::uint64_t;
// Borrowed typed storage only. Bind/unbind and object-lifetime changes require
// external serialization and quiescence. During concurrent operations the map
// stays immutable and each registered LONG remains alive at the same address.
class Counters final {
public:
    Counters()=default;
    Counters(const Counters&)=delete;
    Counters& operator=(const Counters&)=delete;
    Counters(Counters&&)=delete;
    Counters& operator=(Counters&&)=delete;
    Address bind(LONG&);
    void unbind(LONG&);
    bool contains(Address) const noexcept;
    std::uint32_t fetch_add(Address,std::uint32_t);
    std::uint32_t exchange(Address,std::uint32_t);
    std::uint32_t load(Address) const;
private:
    LONG* find(Address) const;
    std::map<Address,LONG*> values_;
};
}
