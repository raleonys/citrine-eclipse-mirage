// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include <filesystem>
namespace preloader_l {
enum class FileConfigResult : unsigned { loaded, missing, invalid_path, io_error, invalid_config, resource, too_large };
// module_filename is an absolute trusted module filename, never a CWD anchor.
// The only leaf is preloader_l.cfg. No caller-provided relative traversal exists.
FileConfigResult load_module_config(const std::filesystem::path& module_filename, ConfigStore& store) noexcept;
}
