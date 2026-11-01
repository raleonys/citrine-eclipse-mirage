#pragma once
// HF12 P2: bounded path join only. OS file I/O, GetLastError, and DLL module
// lookup remain Win32-owned until native acceptance validates replacements.
// This is a native-path operation, never a current-working-directory lookup.
// ghc::filesystem is pristine and never patches the upstream implementation.
#ifndef GHC_WIN_DISABLE_AUTO_PREFIXES
#define GHC_WIN_DISABLE_AUTO_PREFIXES
#endif
#include <ghc/filesystem.hpp>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <new>

namespace preloader_l { namespace config { namespace path_detail {
// Arguments: NUL-free module filename of module_units UTF-16 code units,
// NUL-free relative leaf of leaf_units UTF-16 code units. A successful result
// must EXACTLY match the original raw Win32 module prefix + supplied leaf;
// otherwise fail closed, rather than silently normalizing Unicode, extended
// prefixes or separators that could change module-relative trust authority.
inline bool join_module_relative(const uint16_t* module_path, size_t module_units,
                                 const uint16_t* leaf, size_t leaf_units,
                                 uint16_t* output, size_t output_capacity,
                                 size_t* written, bool* resource_error = nullptr) noexcept {
    if (written) *written = 0;
    if (resource_error) *resource_error = false;
    if (!module_path || !leaf || !output || !written ||
        !module_units || !leaf_units || !output_capacity) return false;
    if (leaf_units >= output_capacity) return false;
    for (size_t i=0;i<module_units;++i) if(!module_path[i]) return false;
    for (size_t i=0;i<leaf_units;++i) {
        if(!leaf[i] || leaf[i]==static_cast<uint16_t>('/') ||
           leaf[i]==static_cast<uint16_t>('\\')) return false;
    }
    size_t prefix=module_units;
    while(prefix) {
        const auto c=module_path[prefix-1u];
        if(c==static_cast<uint16_t>('/') || c==static_cast<uint16_t>('\\')) break;
        --prefix;
    }
    if (!prefix || prefix > output_capacity-1u ||
        leaf_units > output_capacity-1u-prefix) return false;
    try {
        std::u16string module;
        module.reserve(module_units);
        for (size_t i=0;i<module_units;++i) module.push_back(static_cast<char16_t>(module_path[i]));
        std::u16string basename;
        basename.reserve(leaf_units);
        for (size_t i=0;i<leaf_units;++i) basename.push_back(static_cast<char16_t>(leaf[i]));
        const ghc::filesystem::path joined =
            ghc::filesystem::path(module).parent_path() / ghc::filesystem::path(basename);
        const std::u16string result=joined.u16string();
        if (result.size()!=prefix+leaf_units) return false;
        for(size_t i=0;i<prefix;++i)
            if(result[i]!=static_cast<char16_t>(module_path[i])) return false;
        for(size_t i=0;i<leaf_units;++i)
            if(result[prefix+i]!=static_cast<char16_t>(leaf[i])) return false;
        for(size_t i=0;i<result.size();++i) output[i]=static_cast<uint16_t>(result[i]);
        output[result.size()]=0;
        *written=result.size();
        return true;
    } catch (const std::bad_alloc&) {
        if (resource_error) *resource_error = true;
        return false;
    } catch (...) {
        return false;
    }
}
} } }
