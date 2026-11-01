#pragma once
#include <cstddef>
#include <cstdint>
#include <span>

namespace anadius::crt_environment_recovered {
struct Buffer { char* data{}; std::size_t capacity{}; };
struct Environment { char** data{}; std::size_t slots{}; };
struct State { Environment current_0a5288{}, initial_0a52a0{}; };
// All storage belongs to the provider. Pointer arrays contain live, zeroed char*
// objects, byte buffers are writable and zeroed for calloc, and input spans cover
// complete terminated blocks. An empty wide span represents acquisition NULL.
// Providers/callbacks may throw; no target SEH/unwind cleanup is reconstructed.
struct Effects {
    virtual ~Effects() = default;
    virtual void initialize_multibyte_06f5f8() = 0;
    virtual std::span<const char16_t> get_environment_strings_w() = 0;
    virtual void free_environment_strings_w(std::span<const char16_t>) = 0;
    // Code page=0, flags=0, default char=NULL, used-default=NULL in both calls.
    virtual std::int32_t convert_06cd58(std::span<const char16_t>, std::int32_t source_count,
                                     Buffer destination, std::int32_t capacity) = 0;
    virtual Buffer malloc_06a308(std::uint64_t bytes) = 0;
    virtual Environment calloc_pointers_066848(std::uint64_t count, std::uint64_t size) = 0;
    virtual Buffer calloc_chars_066848(std::uint64_t count, std::uint64_t size) = 0;
    virtual void free_0668c0(void*) = 0;
    virtual std::int32_t strcpy_s_05cff0(Buffer, std::span<const char> source) = 0;
    [[noreturn]] virtual void invoke_watson_05c9a4() = 0;
};
Buffer convert_block_06f9e4(Effects&);
Environment build_array_063b54(std::span<const char> block, Effects&);
void free_environment_063d80(Environment, Effects&);
std::uint32_t initialize_063a70(State&, Effects&);
std::uint32_t thunk_initialize_064054(State&, Effects&);
}
