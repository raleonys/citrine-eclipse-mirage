// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <filesystem>
namespace preloader_l::paths {
enum class SiblingRoute {gulrak, preserved_native_spelling};
struct SiblingFile {std::filesystem::path value;SiblingRoute route;};
// Pure lexical native-string operation. One filename leaf; no cwd or IO lookup.
// Preserve FIX10 raw spelling if gulrak normalizes it. Allocation errors propagate.
SiblingFile module_sibling(const std::filesystem::path& module_file,const std::filesystem::path& leaf);
}
