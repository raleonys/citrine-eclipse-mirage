#include "preloader_l/platform/pe_import.hpp"
#include <stdint.h>
#include <string.h>

namespace {
void put16(uint8_t* p, uint16_t v) { p[0]=static_cast<uint8_t>(v); p[1]=static_cast<uint8_t>(v>>8); }
void put32(uint8_t* p, uint32_t v) { for (int i=0;i<4;++i) p[i]=static_cast<uint8_t>(v>>(i*8)); }
void put64(uint8_t* p, uint64_t v) { for (int i=0;i<8;++i) p[i]=static_cast<uint8_t>(v>>(i*8)); }
}

int main() {
    alignas(void*) uint8_t image[0x1000]{};
    put16(image+0x00, 0x5a4d);
    put32(image+0x3c, 0x80);
    put32(image+0x80, 0x00004550);
    const size_t opt = 0x80 + 24;
    put16(image+opt, 0x020b);
    put32(image+opt+56, sizeof(image));
    // Import directory.
    put32(image+opt+112+8, 0x300);
    put32(image+opt+112+12, 40);
    // Descriptor.
    put32(image+0x300+0, 0x400); // OriginalFirstThunk
    put32(image+0x300+12, 0x500); // Name
    put32(image+0x300+16, 0x600); // FirstThunk
    memcpy(image+0x500, "CRYPT32.DLL", 12);
    put64(image+0x400, 0x700);
    put64(image+0x408, 0x740);
    put64(image+0x410, 0);
    put16(image+0x700, 0);
    memcpy(image+0x702, "CryptQueryObject", 17);
    put16(image+0x740, 0);
    memcpy(image+0x742, "CryptMsgGetParam", 17);
    const uint64_t sentinel = UINT64_C(0x123456789abcdef0);
    const uint64_t sentinel2 = UINT64_C(0x0fedcba987654321);
    put64(image+0x600, sentinel);
    put64(image+0x608, sentinel2);

    size_t size = 0;
    if (!preloader_l::platform::pe_image_size(image, &size) || size != sizeof(image)) return 1;
    preloader_l::platform::PeImportSlot slot{};
    if (!preloader_l::platform::find_pe_import_slot(image, sizeof(image), "crypt32.dll",
                                                 "CryptQueryObject", &slot)) return 2;
    if (slot.slot != reinterpret_cast<void**>(image+0x600)) return 3;
    if (reinterpret_cast<uintptr_t>(slot.current) != static_cast<uintptr_t>(sentinel)) return 4;
    if (!preloader_l::platform::find_pe_import_slot(image, sizeof(image), "crypt32.dll",
                                                 "CryptMsgGetParam", &slot)) return 5;
    if (slot.slot != reinterpret_cast<void**>(image+0x608)) return 6;
    if (reinterpret_cast<uintptr_t>(slot.current) != static_cast<uintptr_t>(sentinel2)) return 7;
    if (preloader_l::platform::find_pe_import_slot(image, sizeof(image), "CRYPT32.dll",
                                                "DefinitelyNotThere", &slot)) return 8;
    preloader_l::platform::PeImportSlot matches[4]{};
    put64(image+0x610, sentinel);
    // Extend thunk/name arrays by one slot so 0x610 is a valid IAT entry.
    put64(image+0x410, 0x780); put64(image+0x418, 0);
    put16(image+0x780, 0); memcpy(image+0x782, "AnotherImport", 14);
    const size_t count = preloader_l::platform::find_pe_import_slots_by_current(
        image, sizeof(image), reinterpret_cast<void*>(static_cast<uintptr_t>(sentinel)), matches, 4);
    if (count != 2) return 9;
    if (matches[0].slot != reinterpret_cast<void**>(image+0x600) ||
        matches[1].slot != reinterpret_cast<void**>(image+0x610)) return 10;
    return 0;
}
