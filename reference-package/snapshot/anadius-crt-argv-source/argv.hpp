#pragma once
#include "../anadius-crt-argv-parser-source/parse.hpp"
#include "../anadius-crt-error-source/errors.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace anadius::crt_argv_recovered {
// Reconstructed identities: module_filename A5180, program_name A59E0,
// argc A59E8, argv A59F0, command_line A5A00. These are owned C++ state,
// never references into the original image. Zero initialization is a new default.
struct State {
    std::array<char, 260> module_filename{};
    const char* program_name{};
    std::int32_t argc{};
    char** argv{};
    std::span<const char> command_line{}; // empty models NULL; otherwise bounded
};
struct Allocation {
    void* base{};
    // On success, live raw allocation at base, aligned for char*, covering
    // at least the requested bytes. Ownership transfers to State on mode 1.
    std::span<std::byte> storage{};
};
struct Expansion {
    char** base{};
    // On success, contains a bounded, NULL-terminated array of live char*.
    // Strings and this array survive freeing the initial parse allocation.
    // On failure, base may still carry a partially allocated result to free.
    std::span<char*> pointers{};
};
struct Effects {
    virtual ~Effects() = default;
    virtual void initialize_multibyte_06f5f8() = 0;
    // Original HMODULE=NULL, capacity=260. Return is intentionally ignored.
    virtual std::uint32_t module_filename_06ecdc(std::span<char, 260>) = 0;
    virtual crt_errors_recovered::Cell32 errno_05d724() = 0;
    virtual void invalid_parameter_05c954() = 0;
    virtual Allocation calloc_066848(std::uint64_t count, std::uint64_t bytes) = 0;
    virtual void free_0668c0(void*) = 0;
    // Mandatory wildcard expansion; mode 2 must not silently use mode 1.
    virtual std::int32_t expand_06ecd4(std::span<char*> input, Expansion& out) = 0;
};
// Pure register-level guard exposed for boundary checks. element_size==0 is
// an unsupported hardware DIV fault path when the first guard passes.
bool allocation_size_063888(std::uint64_t pointers, std::uint64_t characters,
                            std::uint64_t element_size, std::uint64_t& bytes);
Allocation allocate_buffer_063888(std::uint64_t pointers, std::uint64_t characters,
                                  std::uint64_t element_size, Effects&);
std::uint32_t configure_0638e8(State&, std::int32_t mode, Effects&,
                              crt_argv_parse_recovered::Effects&);
} // namespace anadius::crt_argv_recovered
