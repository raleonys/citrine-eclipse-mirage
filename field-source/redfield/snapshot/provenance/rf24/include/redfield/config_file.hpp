// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include <filesystem>
namespace redfield {
enum class FileConfigResult : unsigned { loaded, missing, invalid_path, io_error, invalid_config, resource, too_large };
// module_filename is an absolute trusted module filename, never a CWD anchor.
// The only leaf is redfield.cfg. No caller-provided relative traversal exists.
FileConfigResult load_module_config(const std::filesystem::path& module_filename, ConfigStore& store) noexcept;
}
