#include "src/runtime/startup.hpp"
#include <bit>
#include <utility>

namespace preloader_l::startup {
std::string get_environment_039950(InputPlatform& platform, const char* name) {
    const char* value = platform.crt_getenv_05f318(name);
    return value ? std::string(value) : std::string{};
}

std::string narrow_argument_0453d0(InputPlatform& platform, std::wstring_view argument) {
    if (argument.empty()) return {};
    const int input_count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(argument.size()));
    const int output_count = platform.utf8_no_flags(argument.data(),input_count,nullptr,0);
    std::string result(static_cast<std::size_t>(output_count),'\0');
    // The second call's result is ignored, including zero. No GetLastError call.
    (void)platform.utf8_no_flags(argument.data(),input_count,result.data(),output_count);
    return result;
}

std::vector<std::string> read_arguments_039720(InputPlatform& platform) {
    std::vector<std::string> arguments;
    int count;
    auto wide = platform.command_line_to_argv_w(platform.get_command_line_w(),&count);
    if (wide) {
        // Index zero is the executable name and is deliberately skipped.
        for (int i = 1; i < count; ++i)
            arguments.push_back(narrow_argument_0453d0(platform,wide[i]));
        platform.local_free(wide);
    }
    return arguments;
}

std::string filter_arguments_018480(StateReferences state, std::vector<std::string> arguments) {
    std::string retained;
    const auto append = [&](const std::string& token) {
        if (!retained.empty()) retained.push_back(' ');
        retained.append(token);
    };
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        const auto& token = arguments[i];
        if (token == "-emuGameLang" && i+1 < arguments.size()) {
            state.language_from_arguments_0a3ad0 = 1;
            state.registry_spoof_0a3ad1 = 1;
            auto& language = state.game_attributes["INSTALLED_LANGUAGE"];
            if (!arguments[i+1].empty()) language = arguments[i+1];
            append(token);
            append(arguments[++i]);
        } else if (token == "-showLSX") {
            state.show_lsx_0a3ef0 = 1;
            append(token);
        } else if (token == "-showConsole") {
            state.show_console_0a3ef1 = 1;
            append(token);
        } else if (token == "-noConsole") {
            state.no_console_0a3ef2 = 1;
            append(token);
        }
        // Unrecognized tokens are neither forwarded nor assigned anywhere.
    }
    return retained;
}

void setup_environment_019910(StateReferences state, InputPlatform& platform) {
    const auto environment = get_environment_039950(platform,"OriginEmuCmdArgs");
    std::vector<std::string> arguments;
    if (environment.empty()) {
        arguments = read_arguments_039720(platform);
    } else {
        // 03cf00 limit=0 and delimiter=" ": no quote processing, trimming or
        // tab splitting. Consecutive/trailing spaces generate empty elements.
        std::size_t start = 0;
        for (;;) {
            const auto end = environment.find(' ',start);
            if (end == std::string::npos) { arguments.push_back(environment.substr(start)); break; }
            arguments.push_back(environment.substr(start,end-start));
            start = end+1;
        }
    }
    const auto retained = filter_arguments_018480(state,std::move(arguments));
    (void)platform.set_environment_variable_a("OriginEmuCmdArgs",retained.c_str());
}

void setup_console_038760(StateReferences state, ConsolePlatform& platform) {
    if (state.no_console_0a3ef2 != 0) return;
    if (!platform.attach_console(0xffffffffu)) {
        if (state.show_console_0a3ef1 == 0) return;
        (void)platform.allocate_console(); // return value does not gate later work
        const auto window = platform.console_window();
        (void)platform.move_window(window,100,100,800,500,true);
        const auto input = platform.stream_052238(0);
        const auto input_destination = platform.stream_052238(0);
        (void)platform.reopen_05f024(input_destination,"conin$","r",input);
        const auto output = platform.stream_052238(1);
        const auto output_destination = platform.stream_052238(1);
        (void)platform.reopen_05f024(output_destination,"conout$","w",output);
    }
    const auto error = platform.stream_052238(2);
    const auto error_destination = platform.stream_052238(2);
    (void)platform.reopen_05f024(error_destination,"conout$","w",error);
}
} // namespace preloader_l::startup
