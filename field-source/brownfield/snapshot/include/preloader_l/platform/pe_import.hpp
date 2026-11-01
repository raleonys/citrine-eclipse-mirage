#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

struct PeImportSlot {
    void** slot;
    void* current;
};

// Reads SizeOfImage from an already-mapped PE32+ image.
bool pe_image_size(void* image_base, size_t* size_out) noexcept;

// Finds one standard import-table IAT slot. Matching is ASCII
// case-insensitive for the DLL name and exact for the function name.
bool find_pe_import_slot(void* image_base, size_t image_size,
                         const char* dll_name, const char* function_name,
                         PeImportSlot* result) noexcept;

// Enumerates standard-import IAT slots whose current resolved value equals
// `current`. Returns the total number of matches; writes at most `capacity`.
size_t find_pe_import_slots_by_current(void* image_base, size_t image_size,
                                       void* current, PeImportSlot* output,
                                       size_t capacity) noexcept;

} }
