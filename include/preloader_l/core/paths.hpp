#pragma once
#include "preloader_l/config/configuration.hpp"
#include <cstddef>
#include <stdexcept>
#include <string_view>

namespace preloader_l::path_recovered {
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
}
