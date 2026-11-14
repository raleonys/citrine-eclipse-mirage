// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/core/paths.hpp"

namespace laureondo::path {
namespace {
constexpr bool separator(char16_t c) noexcept { return c == u'\\' || c == u'/'; }
constexpr bool drive(View path) noexcept {
    return path.size() >= 2 && path[1] == u':' &&
        ((path[0] >= u'A' && path[0] <= u'Z') || (path[0] >= u'a' && path[0] <= u'z'));
}
std::size_t relative_start(View path) noexcept {
    auto position = root_name_length_00bb30(path);
    while (position < path.size() && separator(path[position])) ++position;
    return position;
}
}

std::size_t root_name_length_00bb30(View path) noexcept {
    if (path.size() < 2) return 0;
    if (drive(path)) return 2;
    if (!separator(path[0])) return 0;
    if (path.size() >= 4 && separator(path[3]) &&
        (path.size() == 4 || !separator(path[4])) &&
        ((separator(path[1]) && (path[2] == u'?' || path[2] == u'.')) ||
         (path[1] == u'?' && path[2] == u'?'))) return 3;
    if (path.size() < 3 || !separator(path[1]) || separator(path[2])) return 0;
    std::size_t position = 3;
    while (position < path.size() && !separator(path[position])) ++position;
    return position;
}

bool is_absolute_0195b0(View path) noexcept {
    if (drive(path)) return path.size() > 2 && separator(path[2]);
    return root_name_length_00bb30(path) != 0;
}

Path filename_00e120(const Path& path) {
    const auto minimum = relative_start(path);
    auto position = path.size();
    while (position > minimum && !separator(path[position - 1])) --position;
    return path.substr(position);
}

Path parent_path_019820(const Path& path) {
    const auto minimum = relative_start(path);
    auto position = path.size();
    while (position > minimum && !separator(path[position - 1])) --position;
    while (position > minimum && separator(path[position - 1])) --position;
    return path.substr(0, position);
}

OutOfEvidenceAliasing::OutOfEvidenceAliasing()
    : std::invalid_argument("016b50 self-alias source mutation depends on original MSVC storage") {}

Path& append_016b50(Path& left, const Path& right) {
    if (is_absolute_0195b0(right)) {
        if (&left != &right) left = right;
        return left;
    }
    const auto left_root = root_name_length_00bb30(left);
    const auto right_root = root_name_length_00bb30(right);
    if (right_root != 0 && View(left).substr(0, left_root) != View(right).substr(0, right_root)) {
        left = right;
        return left;
    }
    const bool rooted = right_root < right.size() && separator(right[right_root]);
    const bool add_separator = !rooted &&
        (left_root == left.size() ? left_root >= 3 : !separator(left.back()));
    if (&left == &right && add_separator) throw OutOfEvidenceAliasing();
    Path suffix(right.begin() + static_cast<Path::difference_type>(right_root), right.end());
    // Truncation writes NUL through the saved source pointer before memmove.
    // Original capacity is sufficient because the resulting size is unchanged.
    if (&left == &right && rooted) suffix[0] = 0;
    if (rooted) left.resize(left_root);
    else if (add_separator) left.push_back(u'\\');
    left.append(suffix);
    return left;
}

Path join_016b00(const Path& left, const Path& right) {
    Path result(left);
    append_016b50(result, right);
    return result;
}
}
