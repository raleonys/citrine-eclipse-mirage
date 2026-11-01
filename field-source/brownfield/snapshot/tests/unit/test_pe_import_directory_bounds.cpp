#include "preloader_l/platform/pe_import.hpp"
#include <cstdint>
#include <cstring>

namespace {
void put16(uint8_t* p, uint16_t v) {
    p[0]=static_cast<uint8_t>(v); p[1]=static_cast<uint8_t>(v>>8);
}
void put32(uint8_t* p, uint32_t v) {
    for(int i=0;i<4;++i) p[i]=static_cast<uint8_t>(v>>(8*i));
}
void put64(uint8_t* p, uint64_t v) {
    for(int i=0;i<8;++i) p[i]=static_cast<uint8_t>(v>>(8*i));
}
void make_headers(uint8_t* image, size_t image_size, uint32_t import_rva, uint32_t import_size) {
    put16(image,0x5a4d); put32(image+0x3c,0x80); put32(image+0x80,0x4550);
    const size_t opt=0x80+24; put16(image+opt,0x20b); put32(image+opt+56,static_cast<uint32_t>(image_size));
    put32(image+opt+112+8,import_rva); put32(image+opt+112+12,import_size);
}
}

int main() {
    alignas(void*) uint8_t image[0x1000]{};
    make_headers(image,sizeof(image),0x300,20);

    // Descriptor #0 is the sole descriptor inside the declared [0x300,0x314) extent.
    put32(image+0x300+0,0x400); put32(image+0x300+12,0x500); put32(image+0x300+16,0x600);
    std::memcpy(image+0x500,"OTHER.dll",10); put64(image+0x400,0x700); put64(image+0x408,0);
    put16(image+0x700,0); std::memcpy(image+0x702,"OtherFunc",10); put64(image+0x600,0x1111);

    // Descriptor #1 begins exactly at the declared directory end and must never be consumed.
    put32(image+0x314+0,0x420); put32(image+0x314+12,0x520); put32(image+0x314+16,0x620);
    std::memcpy(image+0x520,"CRYPT32.dll",12); put64(image+0x420,0x740); put64(image+0x428,0);
    put16(image+0x740,0); std::memcpy(image+0x742,"CryptQueryObject",17); put64(image+0x620,0x2222);

    preloader_l::platform::PeImportSlot slot{};
    if (preloader_l::platform::find_pe_import_slot(
            image,sizeof(image),"CRYPT32.dll","CryptQueryObject",&slot)) return 1;
    if (slot.slot || slot.current) return 2;

    preloader_l::platform::PeImportSlot matches[2]{};
    if (preloader_l::platform::find_pe_import_slots_by_current(
            image,sizeof(image),reinterpret_cast<void*>(static_cast<uintptr_t>(0x2222)),matches,2) != 0) return 3;

    // Directory extent itself must fit inside SizeOfImage.
    alignas(void*) uint8_t bad_extent[0x1000]{};
    make_headers(bad_extent,sizeof(bad_extent),0xff8,32);
    if (preloader_l::platform::find_pe_import_slot(
            bad_extent,sizeof(bad_extent),"X.dll","X",&slot)) return 4;
    if (preloader_l::platform::find_pe_import_slots_by_current(
            bad_extent,sizeof(bad_extent),reinterpret_cast<void*>(static_cast<uintptr_t>(1)),matches,2) != 0) return 5;

    return 0;
}
