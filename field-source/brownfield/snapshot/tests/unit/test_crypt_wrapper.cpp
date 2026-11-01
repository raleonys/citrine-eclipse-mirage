#include "preloader_l/platform/crypt_wrapper.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>

int main() {
    using namespace preloader_l::platform;
    static_assert(curio_crypt_wrapper_rva == 0x00001264u, "oracle wrapper RVA drift");
    static_assert(curio_crypt_suffix_rva == 0x000052C0u, "oracle suffix RVA drift");
    static_assert(curio_crypt_companion_rva == 0x000052D0u, "oracle companion RVA drift");
    static_assert(curio_crypt_iat_rva == 0x00005000u, "oracle IAT RVA drift");
    static_assert(curio_crypt_original_slot_rva == 0x000072D0u, "oracle original slot RVA drift");

    const wchar_t exact[] = L"D:\\x\\preloader_l.dll";
    const wchar_t upper[] = L"D:\\x\\PRELOADER_L.DLL";
    const wchar_t extra[] = L"D:\\x\\preloader_l.dll.tmp";

    assert(classify_curio_crypt_object(1u, nullptr, 0u) ==
           CurioCryptObjectDecision::curio_companion_branch);
    assert(classify_curio_crypt_object(1u, exact, (sizeof(exact)/sizeof(exact[0]))-1u) ==
           CurioCryptObjectDecision::curio_companion_branch);
    assert(classify_curio_crypt_object(1u, upper, (sizeof(upper)/sizeof(upper[0]))-1u) ==
           CurioCryptObjectDecision::pass_original);
    assert(classify_curio_crypt_object(1u, extra, (sizeof(extra)/sizeof(extra[0]))-1u) ==
           CurioCryptObjectDecision::pass_original);
    assert(classify_curio_crypt_object(2u, exact, (sizeof(exact)/sizeof(exact[0]))-1u) ==
           CurioCryptObjectDecision::pass_original);
    return 0;
}
