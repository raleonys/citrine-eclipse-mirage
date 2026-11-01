#include "evidence/identity_constants.hpp"
#include <cstring>
#include <cwchar>
#include <iostream>

int main() {
    using namespace preloader_l::platform;
    if (std::strcmp(reconstructed_identity_ascii, "EA Javelin Anticheat") != 0) return 1;
    if (std::strcmp(reconstructed_alt_module_ascii, "EAAntiCheat.GameServiceLauncher.dll") != 0) return 2;
    if (std::wcscmp(reconstructed_alt_module_name, L"EAAntiCheat.GameServiceLauncher.dll") != 0) return 3;
    std::cout << "identity evidence constants PASS\n";
    return 0;
}
