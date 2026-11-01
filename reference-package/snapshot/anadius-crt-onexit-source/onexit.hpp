#pragma once
#include <cstdint>

namespace anadius::crt_onexit_recovered {
using Address = std::uint64_t;
struct Table { Address begin, end, capacity_end; };
static_assert(sizeof(Table) == 24);
struct State {
    std::uint64_t cookie_0a1580;
    Table application_0a47a8;
    Table crt_0a52a8;
};
struct Effects {
    virtual ~Effects() = default;
    // Lock 2 must support registration from a callback on the executing thread.
    virtual void lock_0625f8(std::uint32_t) = 0;
    virtual void unlock_06264c(std::uint32_t) = 0;
    // Opaque rebuilt storage addresses: providers enforce live, aligned bounds.
    // Same failure/preservation contract as recalloc: zero means failure and
    // leaves old storage live; success preserves the old prefix and consumes
    // the old allocation. Provider rejects unsupported sizes before allocation.
    virtual Address recalloc_070434(Address old, std::uint64_t count,
                                    std::uint64_t element_size) = 0;
    virtual void free_0668c0(Address) = 0;
    virtual std::uint64_t read_word(Address) = 0;
    virtual void write_word(Address, std::uint64_t) = 0;
    // Resolve only rebuilt callbacks, never execute original binary addresses.
    virtual void invoke_callback(Address) = 0;
};
Address encode_pointer(Address, std::uint64_t cookie);
Address decode_pointer(Address, std::uint64_t cookie);
int initialize_onexit_table_0643f8(Table*, const State&);
int register_onexit_function_064420(Table*, Address callback, State&, Effects&);
int execute_onexit_table_0643b4(Table*, State&, Effects&);
Address onexit_04b734(Address callback, State&, Effects&);
int atexit_04b770(Address callback, State&, Effects&);
}
