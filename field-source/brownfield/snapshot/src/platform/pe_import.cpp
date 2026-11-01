#include "preloader_l/platform/pe_import.hpp"

namespace preloader_l { namespace platform {
namespace {
constexpr uint16_t kDosMagic = 0x5a4du;       // MZ
constexpr uint32_t kPeMagic = 0x00004550u;    // PE\0\0
constexpr uint16_t kPe32PlusMagic = 0x020bu;
constexpr size_t kDosLfanew = 0x3cu;
constexpr size_t kFileHeaderSize = 20u;
constexpr size_t kOptionalHeaderOffsetFromNt = 4u + kFileHeaderSize;
constexpr size_t kOptionalSizeOfImageOffset = 56u;
constexpr size_t kOptionalDataDirectoryOffset = 112u;
constexpr size_t kImportDirectoryIndex = 1u;
constexpr uint64_t kOrdinalFlag64 = UINT64_C(0x8000000000000000);

bool within(size_t off, size_t need, size_t size) noexcept {
    return off <= size && need <= size - off;
}
uint16_t u16(const uint8_t* p) noexcept {
    return static_cast<uint16_t>(p[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(p[1]) << 8u);
}
uint32_t u32(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8u) |
           (static_cast<uint32_t>(p[2]) << 16u) |
           (static_cast<uint32_t>(p[3]) << 24u);
}
uint64_t u64(const uint8_t* p) noexcept {
    return static_cast<uint64_t>(u32(p)) |
           (static_cast<uint64_t>(u32(p + 4)) << 32u);
}
char ascii_lower(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}
bool ascii_ieq(const char* a, const char* b) noexcept {
    if (!a || !b) return false;
    size_t i = 0;
    for (;; ++i) {
        const char ca = ascii_lower(a[i]);
        const char cb = ascii_lower(b[i]);
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}
bool ascii_eq(const char* a, const char* b) noexcept {
    if (!a || !b) return false;
    size_t i = 0;
    for (;; ++i) {
        if (a[i] != b[i]) return false;
        if (a[i] == 0) return true;
    }
}
bool terminated_string(const uint8_t* base, size_t size, uint32_t rva,
                       const char** text) noexcept {
    if (!text || rva >= size) return false;
    for (size_t i = rva; i < size; ++i) {
        if (base[i] == 0) {
            *text = reinterpret_cast<const char*>(base + rva);
            return true;
        }
    }
    return false;
}

bool pe_layout(const uint8_t* base, size_t supplied_size,
               size_t* optional_out, uint32_t* image_size_out) noexcept {
    if (!base || !optional_out || !image_size_out || supplied_size < 0x100u) return false;
    if (!within(0, 2, supplied_size) || u16(base) != kDosMagic) return false;
    if (!within(kDosLfanew, 4, supplied_size)) return false;
    const uint32_t nt = u32(base + kDosLfanew);
    if (!within(nt, kOptionalHeaderOffsetFromNt + kOptionalDataDirectoryOffset + 16u,
                supplied_size)) return false;
    if (u32(base + nt) != kPeMagic) return false;
    const size_t optional = static_cast<size_t>(nt) + kOptionalHeaderOffsetFromNt;
    if (u16(base + optional) != kPe32PlusMagic) return false;
    const uint32_t image_size = u32(base + optional + kOptionalSizeOfImageOffset);
    if (image_size == 0) return false;
    *optional_out = optional;
    *image_size_out = image_size;
    return true;
}
}

bool pe_image_size(void* image_base, size_t* size_out) noexcept {
    if (!image_base || !size_out) return false;
    // The first page contains the DOS + NT headers for a valid loaded image.
    const auto* base = static_cast<const uint8_t*>(image_base);
    size_t optional = 0;
    uint32_t image_size = 0;
    if (!pe_layout(base, 0x1000u, &optional, &image_size)) return false;
    (void)optional;
    *size_out = static_cast<size_t>(image_size);
    return true;
}

bool find_pe_import_slot(void* image_base, size_t image_size,
                         const char* dll_name, const char* function_name,
                         PeImportSlot* result) noexcept {
    if (!image_base || !dll_name || !function_name || !result) return false;
    result->slot = nullptr;
    result->current = nullptr;
    auto* base = static_cast<uint8_t*>(image_base);
    size_t optional = 0;
    uint32_t declared_size = 0;
    if (!pe_layout(base, image_size, &optional, &declared_size)) return false;
    if (declared_size > image_size) return false;

    const size_t import_dir = optional + kOptionalDataDirectoryOffset +
                              kImportDirectoryIndex * 8u;
    if (!within(import_dir, 8, image_size)) return false;
    const uint32_t import_rva = u32(base + import_dir);
    const uint32_t import_size = u32(base + import_dir + 4);
    if (!import_rva || !import_size) return false;
    const size_t import_start = static_cast<size_t>(import_rva);
    const size_t import_extent = static_cast<size_t>(import_size);
    if (import_start >= image_size || import_extent > image_size - import_start) return false;
    const size_t import_end = import_start + import_extent;

    // IMAGE_IMPORT_DESCRIPTOR is five DWORDs. Descriptor traversal is bounded
    // by the PE-declared Import Directory extent, not merely SizeOfImage.
    for (size_t desc = import_start;; desc += 20u) {
        if (!within(desc, 20u, import_end)) return false;
        const uint32_t original_first_thunk = u32(base + desc + 0);
        const uint32_t name_rva = u32(base + desc + 12);
        const uint32_t first_thunk = u32(base + desc + 16);
        if (original_first_thunk == 0 && name_rva == 0 && first_thunk == 0) break;

        const char* imported_dll = nullptr;
        if (!terminated_string(base, image_size, name_rva, &imported_dll)) continue;
        if (!ascii_ieq(imported_dll, dll_name)) continue;

        const uint32_t names_rva = original_first_thunk ? original_first_thunk : first_thunk;
        if (!names_rva || !first_thunk) return false;
        for (size_t ti = 0;; ++ti) {
            const size_t name_thunk = static_cast<size_t>(names_rva) + ti * 8u;
            const size_t iat_thunk = static_cast<size_t>(first_thunk) + ti * 8u;
            if (!within(name_thunk, 8, image_size) || !within(iat_thunk, 8, image_size)) return false;
            const uint64_t name_value = u64(base + name_thunk);
            if (name_value == 0) break;
            if ((name_value & kOrdinalFlag64) != 0) continue;
            const uint32_t ibn_rva = static_cast<uint32_t>(name_value & UINT64_C(0xffffffff));
            if (!within(ibn_rva, 3, image_size)) continue;
            const char* imported_name = nullptr;
            if (!terminated_string(base, image_size, ibn_rva + 2u, &imported_name)) continue;
            if (!ascii_eq(imported_name, function_name)) continue;

            auto** slot = reinterpret_cast<void**>(base + iat_thunk);
            result->slot = slot;
            result->current = *slot;
            return true;
        }
    }
    return false;
}


size_t find_pe_import_slots_by_current(void* image_base, size_t image_size,
                                       void* current, PeImportSlot* output,
                                       size_t capacity) noexcept {
    if (!image_base || !current || (!output && capacity)) return 0;
    auto* base = static_cast<uint8_t*>(image_base);
    size_t optional = 0;
    uint32_t declared_size = 0;
    if (!pe_layout(base, image_size, &optional, &declared_size) || declared_size > image_size) return 0;

    const size_t import_dir = optional + kOptionalDataDirectoryOffset + kImportDirectoryIndex * 8u;
    if (!within(import_dir, 8, image_size)) return 0;
    const uint32_t import_rva = u32(base + import_dir);
    const uint32_t import_size = u32(base + import_dir + 4);
    if (!import_rva || !import_size) return 0;
    const size_t import_start = static_cast<size_t>(import_rva);
    const size_t import_extent = static_cast<size_t>(import_size);
    if (import_start >= image_size || import_extent > image_size - import_start) return 0;
    const size_t import_end = import_start + import_extent;

    size_t matches = 0;
    for (size_t desc = import_start;; desc += 20u) {
        if (!within(desc, 20u, import_end)) return matches;
        const uint32_t original_first_thunk = u32(base + desc + 0);
        const uint32_t name_rva = u32(base + desc + 12);
        const uint32_t first_thunk = u32(base + desc + 16);
        if (original_first_thunk == 0 && name_rva == 0 && first_thunk == 0) break;
        if (!first_thunk) continue;

        for (size_t ti = 0;; ++ti) {
            const size_t iat_thunk = static_cast<size_t>(first_thunk) + ti * sizeof(void*);
            if (!within(iat_thunk, sizeof(void*), image_size)) break;
            auto** slot = reinterpret_cast<void**>(base + iat_thunk);
            void* value = *slot;
            if (!value) break;
            if (value != current) continue;
            if (matches < capacity) output[matches] = PeImportSlot{slot, value};
            ++matches;
        }
    }
    return matches;
}

} }
