#pragma once
#include <stdint.h>
#include <stddef.h>
#include "preloader_l/config/runtime_configuration_file.hpp"

namespace preloader_l { namespace config {

struct ProcessRuntimeConfigurationResult {
    RuntimeConfigurationFileStatus file_status;
    bool current_directory_available;
    uint32_t current_directory_length;
    uint32_t present_sections;
};

#if defined(_WIN32)

// Narrow process-current-directory mirror retained for curio consumers.
// DLL-adjacent surge.cfg and optional surge_override.cfg lookup is independent and uses a Unicode module path.
// Returns false when the directory cannot be represented in the supplied
// narrow buffer. On success *written excludes the terminal NUL.
bool query_process_current_directory(char* output, size_t capacity,
                                     size_t* written) noexcept;

// Phase x001 behavioral integration adapter. The behavioral argument walker recognizes
// the exact token "-emuGameLang" and consumes the following argument as the
// INSTALLED_LANGUAGE override. This query exposes that process-lifetime
// command-line token without allocating or changing runtime configuration.
// On success, *value may point at a zero-length quoted token and remains valid
// for the process lifetime because GetCommandLineA owns the backing buffer.
bool query_process_emu_game_language(const char** value, size_t* length) noexcept;

// Re-evaluates the process command line and publishes the final installed
// language into the shared runtime settings policy. A missing CLI override
// retracts the runtime override so Config2 Game.Language remains authoritative.
void synchronize_process_language_policy() noexcept;

// Worker-thread integration point. Captures the process CWD for consumers
// whose curio policy is the process directory, but loads surge.cfg plus optional surge_override.cfg via
// UTF-16 exclusively from the directory containing this DLL. File absence is
// reported, not fatal to DLL loading.
ProcessRuntimeConfigurationResult load_process_runtime_configuration() noexcept;
#endif

} }
