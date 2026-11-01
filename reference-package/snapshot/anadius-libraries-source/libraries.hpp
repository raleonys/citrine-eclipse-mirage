#pragma once
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace anadius::recovered::libraries {
using Module = std::uintptr_t;
struct Conversion { int count; std::uint32_t error; };

// Native API boundary. Mocks must preserve GetLastError and used_default semantics.
class SystemCalls {
public:
    virtual ~SystemCalls() = default;
    virtual std::uint32_t get_error_mode() = 0;
    virtual std::uint32_t set_error_mode(std::uint32_t) = 0;
    virtual Module load_library(const wchar_t*) = 0;
    virtual bool free_library(Module) = 0;
    virtual int wide_to_narrow(std::uint32_t codepage, std::uint32_t flags,
        const wchar_t*, int count, char* destination, int capacity, bool* used_default) = 0;
    virtual std::uint32_t last_error() = 0;
};

// Genuine Win32 adapter; compiled, never called in this investigation.
class NativeSystemCalls final : public SystemCalls {
public:
    std::uint32_t get_error_mode() override;
    std::uint32_t set_error_mode(std::uint32_t) override;
    Module load_library(const wchar_t*) override;
    bool free_library(Module) override;
    int wide_to_narrow(std::uint32_t, std::uint32_t, const wchar_t*, int,
                       char*, int, bool*) override;
    std::uint32_t last_error() override;
};

// Unrecovered logging/UI, CRT exception and execution-global bindings. No default
// success implementations; the core below contains the recovered load decisions.
class Effects {
public:
    virtual ~Effects() = default;
    virtual void begin_load_log_0863e0() = 0;
    virtual void append_log_path_and_flush(std::string_view utf8) = 0;
    virtual void message_038bc0(std::string title, std::string text,
                               std::uint32_t flags) = 0;
    [[noreturn]] virtual void throw_runtime_00cd70(int code) = 0;
    [[noreturn]] virtual void throw_filesystem_00cdb0(std::uint32_t code) = 0;
    virtual void wait_steps_036880(int count) = 0;
    virtual void finish_flag_036820() = 0; // byte at original VA 1800a3c3e
};

Conversion convert_wide_048b30(SystemCalls&, std::uint32_t codepage,
    const wchar_t*, int count, char* destination, int capacity);
std::uint32_t suppress_errors_0450a0(SystemCalls&);
Module load_copy_0450d0(SystemCalls&, const std::wstring&);
std::vector<Module> load_list_035260(SystemCalls&, Effects&,
                                    std::span<const std::wstring> paths);

struct State {
    std::vector<std::wstring> from_main; // original global vector 1800a3ea8
    std::vector<std::wstring> from_worker; // 1800a3ec0
    std::vector<Module> loaded; // 1800a3ed8, end 1800a3ee0, capacity 1800a3ee8
};
void load_phase_035850(SystemCalls&, Effects&, State&, bool from_main);
void unload_035ab0(SystemCalls&, const State&);
} // namespace anadius::recovered::libraries
