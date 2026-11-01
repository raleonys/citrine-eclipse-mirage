#pragma once
#include "src/configuration/state.hpp"
#include <cstddef>
#include <stdexcept>
#include <string_view>

namespace preloader_l::paths {
using Path = configuration::Path;
using View = std::u16string_view;

// Counts UTF-16 code units, including embedded NUL. No OS normalization occurs.
std::size_t root_name_length_00bb30(View path) noexcept;
bool is_absolute_0195b0(View path) noexcept;
Path filename_00e120(const Path& path);
Path parent_path_019820(const Path& path);
Path join_016b00(const Path& left, const Path& right);

// Saved source pointers in the binary can be invalidated by self-append.
// This exception marks that unrecovered allocation-dependent domain.
class OutOfEvidenceAliasing : public std::invalid_argument {
public:
    OutOfEvidenceAliasing();
};
Path& append_016b50(Path& left, const Path& right);

// Common application-effect binding for the packet's Windows lexical bodies.
// Conversion and filesystem I/O remain caller-owned boundaries.
struct WindowsEffects {
    static Path parent(const Path& value) { return parent_path_019820(value); }
    static Path filename(const Path& value) { return filename_00e120(value); }
    static Path join(const Path& left, const Path& right) { return join_016b00(left, right); }
    static Path resolve(const Path& value, const Path& executable, bool relative) {
        if (relative && !is_absolute_0195b0(value))
            return join_016b00(parent_path_019820(executable), value);
        return value;
    }
};
}
