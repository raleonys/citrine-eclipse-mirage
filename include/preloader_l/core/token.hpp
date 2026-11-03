// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/config/configuration.hpp"
#include "preloader_l/codec/crypto.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace preloader_l::token {
using Path = configuration::Path;
struct FileStatus { std::int32_t kind, error; };
class Dependencies {
public:
    virtual ~Dependencies() = default;
    virtual FileStatus status_01ea20(const Path&, std::uint32_t flags) = 0;
    [[noreturn]] virtual void filesystem_error_018130(std::string_view operation,
                                                   std::int32_t error, const Path&) = 0;
    virtual std::string read_binary_03b3f0(const Path&) = 0;
};
// Factored search portion of 02afa0, not an additional identified binary function.
// Uses explicit byte lengths; neither XML decoding nor C-string truncation occurs.
std::string extract_game_token(std::string_view decrypted);
std::string load_token_02afa0(const Path&, Dependencies&);
} // namespace preloader_l::token
