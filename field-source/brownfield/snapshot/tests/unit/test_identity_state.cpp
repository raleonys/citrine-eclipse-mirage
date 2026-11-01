#include "preloader_l/platform/identity_state.hpp"
#include <cstring>

int main() {
    using namespace preloader_l::platform;
    reset_reconstructed_identity_state_for_test();
    auto s = reconstructed_identity_state();
    if (s.schema_version != 1u || s.initialized != 0u || s.branch_hits != 0u) return 1;
    if (std::strcmp(s.target_module_ascii, "preloader_l.dll") != 0) return 2;
    if (std::strcmp(s.companion_module_ascii, "EAAntiCheat.GameServiceLauncher.dll") != 0) return 3;
    if (std::strcmp(s.identity_ascii, "EA Javelin Anticheat") != 0) return 4;
    initialize_reconstructed_identity_state();
    s = reconstructed_identity_state();
    if (s.initialized != 1u || s.branch_hits != 0u) return 5;
    note_reconstructed_identity_branch();
    note_reconstructed_identity_branch();
    s = reconstructed_identity_state();
    if (s.initialized != 1u || s.branch_hits != 2u) return 6;
    return 0;
}
