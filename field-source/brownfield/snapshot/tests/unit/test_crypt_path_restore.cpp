#include "preloader_l/platform/crypt_path_restore.hpp"

int main()
{
    using namespace preloader_l::platform;
    if (!should_restore_crypt_query_path(1u, nullptr, 0x400u, 2u, 0u, true)) return 1;
    if (should_restore_crypt_query_path(1u, reinterpret_cast<void*>(1), 0x400u, 2u, 0u, true)) return 2;
    if (should_restore_crypt_query_path(2u, nullptr, 0x400u, 2u, 0u, true)) return 3;
    if (should_restore_crypt_query_path(1u, nullptr, 0x401u, 2u, 0u, true)) return 4;
    if (should_restore_crypt_query_path(1u, nullptr, 0x400u, 1u, 0u, true)) return 5;
    if (should_restore_crypt_query_path(1u, nullptr, 0x400u, 2u, 1u, true)) return 6;
    if (should_restore_crypt_query_path(1u, nullptr, 0x400u, 2u, 0u, false)) return 7;
    // Regression guard: the runtime module is preloader_l.dll (lowercase L),
    // not preloader_1.dll (digit one).
    const wchar_t expected[] = L"preloader_l.dll";
    for (unsigned i = 0; expected[i] != L'\0' || crypt_query_restore_module_name[i] != L'\0'; ++i) {
        if (expected[i] != crypt_query_restore_module_name[i]) return 8;
    }
    return 0;
}
