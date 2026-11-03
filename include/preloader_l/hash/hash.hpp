// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/config/configuration.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>

namespace preloader_l::hash {
using Path = configuration::Path;
using DigestWords = std::array<std::uint32_t,5>;

// Logical streaming state, not the original MSVC string/object layout.
class Sha1 final {
public:
    Sha1() = default;
    void update(std::span<const std::uint8_t>);
    void update(std::string_view bytes);
    // 043270 resets the state after producing the five words.
    DigestWords finalize_words_043270();
private:
    void transform_043900(const std::array<std::uint8_t,64>&);
    DigestWords state_{0x67452301u,0xefcdab89u,0x98badcfeu,0x10325476u,0xc3d2e1f0u};
    std::array<std::uint8_t,64> pending_{};
    std::size_t pending_size_ = 0;
    std::uint64_t block_count_ = 0;
};

// Deterministic classic-locale rendering. The original's ambient numeric
// locale is an explicit TextEnvironment dependency in the file checker.
std::string classic_hex_digest(const DigestWords&);

class InputStream {
public:
    virtual ~InputStream() = default;
    virtual std::uint32_t initial_state() const = 0;
    // EOF is -1; successful peek does not consume a byte. Includes the native
    // stream sentry/state/exception behavior, including peek's gcount reset.
    virtual std::int32_t peek_043790() = 0;
    // Returns gcount after read. Includes native stream state/exception policy.
    virtual std::size_t read_03b2a0(std::span<std::uint8_t> destination) = 0;
    // Includes filebuf close + setstate on failure. A close failure with a zero
    // exception mask does not itself make the original matcher return false.
    virtual void close_03ae70() = 0;
};
class FileFactory {
public:
    virtual ~FileFactory() = default;
    // Native 039f70 receives mode 0x20, protection/share value 0x40, construct=1;
    // it adds input bit 1 before calling its CRT file opener. Returned object
    // must exist even when opening fails (nonzero initial_state).
    virtual std::unique_ptr<InputStream> open_input_039f70(
        const Path&,std::uint32_t mode,std::uint32_t protection) = 0;
};
class TextEnvironment {
public:
    virtual ~TextEnvironment() = default;
    // Five unsigned words, hexadecimal/no uppercase, width 8 and fill '0' per
    // word, through the stream's ambient C++ locale. No separators added here.
    virtual std::string format_digest_043270(const DigestWords&) = 0;
    // MOVSX byte means signed char promotion, including -128..-1. Do not turn
    // these into unsigned-char arguments or silently substitute ASCII folding.
    virtual std::int32_t tolower_05fca0(std::int32_t signed_char) = 0;
};

// Expected text is consumed by value, as at its known caller sites. Neither
// the source configuration nor the target file is modified.
bool file_hash_matches_042d20(const Path&,std::string expected,
                            FileFactory&,TextEnvironment&);
} // namespace preloader_l::hash
