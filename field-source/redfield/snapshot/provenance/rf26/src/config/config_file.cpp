// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config_file.hpp"
#include <array>
#include <fstream>
#include <system_error>
namespace preloader_l {
FileConfigResult load_module_config(const std::filesystem::path& module_filename, ConfigStore& store) noexcept {
    try {
        const auto& native=module_filename.native();
        if (!module_filename.is_absolute() || !module_filename.has_filename() ||
            native.find(std::filesystem::path::value_type{})!=std::filesystem::path::string_type::npos)
            return FileConfigResult::invalid_path;
        const auto path=module_filename.parent_path()/"preloader_l.cfg";
        // Status is checked first to distinguish an absent file from access or
        // file-kind errors. After a successful status, any open/read failure is
        // an I/O failure, even if a concurrent rename removed the path.
        std::error_code error;
        const auto status=std::filesystem::status(path,error);
        if (error==std::errc::no_such_file_or_directory || status.type()==std::filesystem::file_type::not_found)
            return FileConfigResult::missing;
        if (error || !std::filesystem::is_regular_file(status)) return FileConfigResult::io_error;
        std::ifstream input(path,std::ios::binary);
        if (!input) return FileConfigResult::io_error;
        std::string bytes;
        std::array<char,8192> chunk{};
        while (input) {
            input.read(chunk.data(),static_cast<std::streamsize>(chunk.size()));
            const auto count=static_cast<std::size_t>(input.gcount());
            if (count>ConfigStore::max_bytes-bytes.size()) return FileConfigResult::too_large;
            bytes.append(chunk.data(),count);
        }
        if (input.bad() || !input.eof()) return FileConfigResult::io_error;
        input.clear();
        input.close();
        if (input.fail()) return FileConfigResult::io_error;
        switch (store.replace(bytes)) {
        case ConfigResult::published: return FileConfigResult::loaded;
        case ConfigResult::invalid: return FileConfigResult::invalid_config;
        case ConfigResult::too_large: return FileConfigResult::too_large;
        case ConfigResult::resource: return FileConfigResult::resource;
        }
    } catch (const std::bad_alloc&) { return FileConfigResult::resource; }
      catch (...) { return FileConfigResult::io_error; }
    return FileConfigResult::io_error;
}
}
