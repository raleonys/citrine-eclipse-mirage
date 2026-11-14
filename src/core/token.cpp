// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/core/token.hpp"
#include "laureondo/core/token_constants.hpp"

namespace laureondo::token {
std::string extract_game_token(std::string_view value) {
    constexpr std::string_view opening = "<GameToken>";
    constexpr std::string_view closing = "</GameToken>";
    if (value.size() < opening.size() || value.size() - opening.size() < constants::encrypted_offset)
        return {};
    const auto begin = value.find(opening, constants::encrypted_offset);
    if (begin == std::string_view::npos) return {};
    const auto content = begin + opening.size();
    if (value.size() - closing.size() < content) return {};
    const auto end = value.find(closing, content);
    if (end == std::string_view::npos) return {};
    return std::string(value.substr(content, end - content));
}

std::string load_token_02afa0(const Path& path, Dependencies& dependencies) {
    const auto status = dependencies.status_01ea20(path, 3);
    // Original TEST ((kind - 1) & 0xfffffff7): kinds 1 and 9 suppress an error.
    if (status.error != 0 && ((static_cast<std::uint32_t>(status.kind) - 1u) & 0xfffffff7u) != 0)
        dependencies.filesystem_error_018130("status", status.error, path);
    if (status.kind != 2) return {};

    auto bytes = dependencies.read_binary_03b3f0(path);
    crypto::Cipher cipher;
    cipher.set_key_and_iv_042760(constants::key_084a50, constants::iv_084a60);
    if (bytes.size() < constants::encrypted_offset) {
        // Unsigned subtraction in 042480 rejects nonmultiples BEFORE a data
        // access. Congruent short sizes enter CBC with a wrapped huge length.
        // Refuse only that unsupported memory domain, never emulate the OOB.
        if (((bytes.size() - constants::encrypted_offset) & 15u) == 0)
            throw crypto::OutOfEvidenceInput("02afa0 short CBC input enters original out-of-bounds domain");
    } else {
        // 02b068's result is deliberately ignored: unaligned data is scanned
        // unchanged, and padding failures are scanned after in-place mutation.
        static_cast<void>(cipher.decrypt_with_offset(bytes, constants::encrypted_offset));
    }
    return extract_game_token(bytes);
}
} // namespace laureondo::token
