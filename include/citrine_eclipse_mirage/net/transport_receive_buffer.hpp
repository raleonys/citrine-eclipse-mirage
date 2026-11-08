// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_write_finish.hpp"

namespace citrine_eclipse_mirage::socket {
// Reuse the existing coherent32-byte defined-field string prefix. This is a
// host projection; native MSVC string ABI and allocator provenance stay open.
using ReceiveBufferString=WriteFinishString;
struct ReceiveBufferRegion {std::uintptr_t address=0;std::span<std::uint8_t> bytes;};
struct ReceiveBufferMemory {
    std::span<ReceiveBufferRegion> regions;
    // Pure bounded memory projection, never an application callback. Register
    // the live inline prefix as well as owned heap/header windows. Overlapping
    // or missing complete mappings fail explicitly without native fault claims.
    std::span<std::uint8_t> view(std::uintptr_t,std::uint64_t) const;
};
class ReceiveBufferApi {
public:
    virtual ~ReceiveBufferApi()=default;
    virtual std::uintptr_t allocate_04b0bc(std::uint64_t)=0;
    virtual void copy_bytes(std::uintptr_t destination,std::uintptr_t source,std::uint64_t)=0;
    virtual void fill_bytes(std::uintptr_t destination,std::uint8_t,std::uint64_t)=0;
    virtual void free_sized_04b0f8(std::uintptr_t,std::uint64_t)=0;
    [[noreturn]]virtual void length_error_00ce80()=0;
    [[noreturn]]virtual void allocation_overflow_00cd30()=0;
    [[noreturn]]virtual void invalid_parameter_05c974()=0;
};
// Selected byte-operation compatibility adapter on owned bounded spans. These
// methods implement observable byte contracts, not original CRT/SIMD bodies.
// Allocation, sized-free and all terminal providers remain mandatory.
class BoundedReceiveBufferApi:public ReceiveBufferApi {
public:
    explicit BoundedReceiveBufferApi(ReceiveBufferMemory& memory):memory_(memory){}
    void copy_bytes(std::uintptr_t,std::uintptr_t,std::uint64_t) final;
    void fill_bytes(std::uintptr_t,std::uint8_t,std::uint64_t) final;
private:ReceiveBufferMemory& memory_;
};
struct ReceiveBufferContext {ReceiveBufferMemory& memory;ReceiveBufferApi& api;};
std::uintptr_t receive_buffer_address(const ReceiveBufferString&);
ReceiveBufferString& grow_receive_buffer_008350(ReceiveBufferString&,std::uint64_t extra,
    std::uint64_t unused_hint,std::uint64_t fill_count,std::uint8_t,ReceiveBufferContext&);
void resize_receive_buffer_010040(ReceiveBufferString&,std::uint64_t new_size,std::uint8_t,ReceiveBufferContext&);
} // namespace citrine_eclipse_mirage::socket
