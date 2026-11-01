#include "parse.hpp"
#include <bit>
#include <stdexcept>

namespace anadius::crt_argv_parse_recovered {
void parse_0636c0(std::span<const char> input, std::span<char*> argv,
                 std::span<char> text, Counts& counts, Effects& effects) {
    std::size_t in = 0, out = 0, arg = 0;
    const auto read = [&](std::size_t at) {
        if (at >= input.size()) throw std::out_of_range("argv parser input/trail outside bounded storage");
        return input[at];
    };
    const auto put = [&](char value) {
        if (!text.empty()) {
            if (out >= text.size()) throw std::out_of_range("argv parser text storage too small");
            text[out++] = value;
        }
    };
    const auto put_arg = [&](bool sentinel) {
        if (!argv.empty()) {
            if (arg >= argv.size()) throw std::out_of_range("argv parser pointer storage too small");
            argv[arg++] = sentinel || text.empty() ? nullptr : text.data() + out;
        }
    };
    const auto lead = [&](char value) {
        return effects.is_mb_lead_06f9a8(static_cast<std::int32_t>(std::bit_cast<std::int8_t>(value))) != 0;
    };
    const auto space = [](char value) { return value == ' ' || value == '\t'; };

    counts.characters = 0; // 0x1800636ed precedes the pointers store.
    counts.pointers = 1;
    put_arg(false);
    bool quoted = false;
    for (;;) {
        char current = read(in);
        if (current == '"') {
            quoted = !quoted;
            ++in;
        } else {
            ++counts.characters;
            put(current);
            ++in;
            if (lead(current)) {
                ++counts.characters;
                put(read(in));
                ++in;
            }
            if (current == '\0') { --in; break; }
        }
        if (!quoted && space(current)) {
            if (!text.empty()) text[out - 1] = '\0';
            break;
        }
    }

    quoted = false;
    while (read(in) != '\0') {
        while (space(read(in))) ++in;
        if (read(in) == '\0') break;
        put_arg(false);
        ++counts.pointers;
        for (;;) {
            bool copy = true;
            std::uint32_t slashes = 0;
            while (read(in) == '\\') { ++in; ++slashes; }
            if (read(in) == '"') {
                if ((slashes & 1) == 0) {
                    if (quoted && read(in + 1) == '"') ++in;
                    else { copy = false; quoted = !quoted; }
                }
                slashes >>= 1;
            }
            while (slashes != 0) {
                --slashes;
                put('\\');
                ++counts.characters;
            }
            const char current = read(in);
            if (current == '\0' || (!quoted && space(current))) break;
            if (copy) {
                put(current);
                if (lead(current)) {
                    ++counts.characters;
                    ++in;
                    put(read(in));
                }
                ++counts.characters;
            }
            ++in;
        }
        put('\0');
        ++counts.characters;
    }
    put_arg(true);
    ++counts.pointers;
}
}
