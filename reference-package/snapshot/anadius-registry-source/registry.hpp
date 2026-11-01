#pragma once
#include "../anadius-host-init-source/host_init.hpp"
#include "abi_cells.hpp"
#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace anadius::registry_recovered {
using Values = host_init_recovered::Values;
using Handle = std::uint64_t;
using Status = std::uint32_t;
using HandleCell = ScalarCell<Handle>;
using DwordCell = ScalarCell<std::uint32_t>;
using Handles = std::set<Handle>;
class OutOfEvidenceInput : public std::domain_error {
public: using std::domain_error::domain_error;
};
class ValueEffects {
public:
    virtual ~ValueEffects() = default;
    // Exact explicit-length filesystem CP conversion, including embedded NUL.
    virtual std::string filesystem_narrow(std::u16string_view) = 0;
    virtual int crt_stricmp(const char*, const char*) = 0;
    virtual int crt_wcsicmp(const char16_t*, const char16_t*) = 0;
    // Required secure CRT copy policy, including invalid-parameter behavior.
    virtual int crt_strcpy_s(char*, std::size_t capacity, const char*) = 0;
    virtual int crt_wcscpy_s(char16_t*, std::size_t capacity, const char16_t*) = 0;
};
std::string resolve_narrow_008980(const Values&, ValueEffects&, std::string_view key);
std::u16string resolve_wide_008b80(const Values&, std::string_view key);
int compare_narrow_0086b0(const Values&, ValueEffects&, const char*, std::string_view key);
int compare_wide_0087f0(const Values&, ValueEffects&, const char16_t*, std::string_view key);
struct StateRefs {
    const Values& values;
    Handles& client_0a37e0;
    Handles& language_0a37f0;
    Handles& fabricated_0a3800;
    const std::uint8_t& language_enabled_0a3ad1;
};
struct OpenA { Handle root; const char* subkey; std::uint32_t options, access; HandleCell result; };
struct OpenW { Handle root; const char16_t* subkey; std::uint32_t options, access; HandleCell result; };
struct QueryA { Handle key; const char* name; void* reserved; DwordCell type; void* data; DwordCell size; };
struct QueryW { Handle key; const char16_t* name; void* reserved; DwordCell type; void* data; DwordCell size; };
class Effects : public ValueEffects {
public:
    virtual Status original_open_a(const OpenA&) = 0;
    virtual Status original_open_w(const OpenW&) = 0;
    virtual Status original_query_a(const QueryA&) = 0;
    virtual Status original_query_w(const QueryW&) = 0;
    virtual Status original_close(Handle) = 0;
    // Original 04b0bc(4) allocation yields opaque key identity. Must throw on
    // allocator failure according to selected runtime; no fake success default.
    virtual Handle allocate_4_04b0bc() = 0;
    virtual void free_04b0f8(Handle) = 0;
    virtual void wait_036880(std::int32_t steps) = 0;
    virtual void log_literal_006880(std::uint64_t va) = 0;
    virtual void log_text_007990(std::string_view) = 0;
    virtual void finish_log_0088e0() = 0;
};
Status write_value_a_0091a0(const Values&, ValueEffects&, void* data,
    DwordCell size, std::string_view key, std::uint32_t original_capacity);
Status write_value_w_0093a0(const Values&, ValueEffects&, void* data,
    DwordCell size, std::string_view key, std::uint32_t original_capacity);
void fabricate_010170(StateRefs, Effects&, HandleCell result, Status& status);
Status open_a_00f800(StateRefs, Effects&, const OpenA&);
Status open_w_00f960(StateRefs, Effects&, const OpenW&);
Status query_a_00fac0(StateRefs, Effects&, const QueryA&);
Status query_w_00fb60(StateRefs, Effects&, const QueryW&);
Status close_00f5b0(StateRefs, Effects&, Handle);
}
