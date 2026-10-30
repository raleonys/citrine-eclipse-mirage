#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace preloader_l::startup_recovered {
// Bind these references to the same state used by the configuration and server.
// No constructor guesses or resets the original globals' initial values.
struct StateReferences {
    std::uint8_t& language_from_arguments_0a3ad0;
    std::uint8_t& registry_spoof_0a3ad1;
    std::uint8_t& show_lsx_0a3ef0;
    std::uint8_t& show_console_0a3ef1;
    std::uint8_t& no_console_0a3ef2;
    std::map<std::string,std::string,std::less<>>& game_attributes;
};

class InputPlatform {
public:
    virtual ~InputPlatform() = default;
    // Original uses the DLL's statically linked CRT getenv cache, not the Win32
    // GetEnvironmentVariable API. A replacement CRT may expose different state.
    virtual const char* crt_getenv_05f318(const char* name) = 0;
    virtual const wchar_t* get_command_line_w() = 0;
    virtual wchar_t** command_line_to_argv_w(const wchar_t*, int* count) = 0;
    virtual void local_free(void*) = 0;
    virtual int utf8_no_flags(const wchar_t*, int count, char* output, int capacity) = 0;
    virtual bool set_environment_variable_a(const char* name, const char* value) = 0;
};

std::string get_environment_039950(InputPlatform&, const char* name);
std::string narrow_argument_0453d0(InputPlatform&, std::wstring_view);
std::vector<std::string> read_arguments_039720(InputPlatform&);
// Ownership-consuming vector is expressed by value; MSVC vector destruction and
// move ABI are not re-created. Only recognized tokens enter the returned string.
std::string filter_arguments_018480(StateReferences, std::vector<std::string>);
void setup_environment_019910(StateReferences, InputPlatform&);

using ConsoleHandle = std::uintptr_t;
class ConsolePlatform {
public:
    virtual ~ConsolePlatform() = default;
    virtual bool attach_console(std::uint32_t process_id) = 0;
    virtual bool allocate_console() = 0;
    virtual ConsoleHandle console_window() = 0;
    virtual bool move_window(ConsoleHandle, int x, int y, int width, int height, bool repaint) = 0;
    // Opaque ABI boundaries to the original 0x58-byte CRT FILE records. Preserve
    // both calls and both pointer arguments; do not substitute host FILE layout.
    virtual std::uintptr_t stream_052238(unsigned index) = 0;
    virtual int reopen_05f024(std::uintptr_t output_pointer, const char* filename,
                             const char* mode, std::uintptr_t stream) = 0;
};
void setup_console_038760(StateReferences, ConsolePlatform&);

class NativeInputPlatform final : public InputPlatform {
public:
    const char* crt_getenv_05f318(const char*) override;
    const wchar_t* get_command_line_w() override;
    wchar_t** command_line_to_argv_w(const wchar_t*, int*) override;
    void local_free(void*) override;
    int utf8_no_flags(const wchar_t*, int, char*, int) override;
    bool set_environment_variable_a(const char*, const char*) override;
};
// Win32 console adapter remains abstract at the two original CRT boundaries.
class NativeConsolePlatform : public ConsolePlatform {
public:
    bool attach_console(std::uint32_t) override;
    bool allocate_console() override;
    ConsoleHandle console_window() override;
    bool move_window(ConsoleHandle, int, int, int, int, bool) override;
};
} // namespace preloader_l::startup_recovered
