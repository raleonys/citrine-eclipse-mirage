#include "preloader_l/runtime/startup.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <cstdlib>

namespace preloader_l::startup_recovered {
static_assert(sizeof(wchar_t) == 2);
const char* NativeInputPlatform::crt_getenv_05f318(const char* name) { return std::getenv(name); }
const wchar_t* NativeInputPlatform::get_command_line_w() { return GetCommandLineW(); }
wchar_t** NativeInputPlatform::command_line_to_argv_w(const wchar_t* command, int* count) {
    return CommandLineToArgvW(command,count);
}
void NativeInputPlatform::local_free(void* memory) { (void)LocalFree(memory); }
int NativeInputPlatform::utf8_no_flags(const wchar_t* input, int count, char* output, int capacity) {
    return WideCharToMultiByte(65001,0,input,count,output,capacity,nullptr,nullptr);
}
bool NativeInputPlatform::set_environment_variable_a(const char* name, const char* value) {
    return SetEnvironmentVariableA(name,value) != 0;
}
bool NativeConsolePlatform::attach_console(std::uint32_t process) { return AttachConsole(process) != 0; }
bool NativeConsolePlatform::allocate_console() { return AllocConsole() != 0; }
ConsoleHandle NativeConsolePlatform::console_window() { return reinterpret_cast<ConsoleHandle>(GetConsoleWindow()); }
bool NativeConsolePlatform::move_window(ConsoleHandle window, int x, int y, int width, int height, bool repaint) {
    return MoveWindow(reinterpret_cast<HWND>(window),x,y,width,height,repaint) != 0;
}
} // namespace preloader_l::startup_recovered
