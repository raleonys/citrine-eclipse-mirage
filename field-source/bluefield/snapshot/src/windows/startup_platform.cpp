#include "src/runtime/startup.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <cstdlib>
#include <memory>
#include <system_error>

namespace preloader_l::startup {
static_assert(sizeof(wchar_t) == 2);
const char* NativeInputPlatform::crt_getenv_05f318(const char* name) {
    // Keep the statically linked CRT environment source, with an owned copy
    // valid until this adapter's next environment read or destruction.
    char* value=nullptr;std::size_t size=0;
    const auto error=::_dupenv_s(&value,&size,name);
    std::unique_ptr<char,decltype(&std::free)> allocation(value,&std::free);
    if(error)throw std::system_error(error,std::generic_category(),"CRT environment read failed");
    if(!value){environment_value_.clear();return nullptr;}
    environment_value_.assign(value,size?size-1:0);
    return environment_value_.c_str();
}
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
} // namespace preloader_l::startup
