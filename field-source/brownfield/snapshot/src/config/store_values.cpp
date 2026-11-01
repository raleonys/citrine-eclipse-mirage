#include "store_internal.hpp"

namespace preloader_l { namespace config { namespace store_entitlement_internal {

bool ascii_space(uint8_t c) noexcept {
    return c == ' ' || (c >= '\t' && c <= '\r');
}

int numeric_digit(uint8_t c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

ParseCode parse_u64_span(StoreState& state,Span value, uint64_t* output) noexcept {
    if (!output) return ParseCode::invalid;
    *output = 0;
    if (!value.used) return ParseCode::ok;

    const uint8_t* p = state.arena + value.off;
    size_t pos = 0;
    while (pos < value.len && ascii_space(p[pos])) ++pos;

    const bool hex =
        pos + 1 < value.len &&
        p[pos] == '0' &&
        (p[pos + 1] == 'x' || p[pos + 1] == 'X');

    bool negative = false;
    if (pos < value.len && (p[pos] == '+' || p[pos] == '-')) {
        negative = p[pos] == '-';
        ++pos;
    }

    if (hex) pos += 2;

    const uint64_t base = hex ? 16u : 10u;
    uint64_t result = 0;

    while (pos < value.len) {
        const int d = numeric_digit(p[pos]);
        if (d < 0 || static_cast<uint64_t>(d) >= base) break;

        const uint64_t digit = static_cast<uint64_t>(d);
        if (result > (UINT64_MAX - digit) / base)
            return ParseCode::number_range;

        result = result * base + digit;
        ++pos;
    }

    *output = negative ? uint64_t(0) - result : result;
    return ParseCode::ok;
}

int base64_digit(uint8_t c) noexcept {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

ParseCode decode_quartets(StoreState& state,Span text, Span* output) noexcept {
    if (!output) return ParseCode::invalid;
    *output = Span{state.arena_used, 0, true};
    if (!text.used) return ParseCode::ok;

    // D01: decoder input is itself arena-backed. Reserve the complete
    // worst-case output span before taking a source pointer so no subsequent
    // arena_byte() can invalidate that pointer through arena reallocation.
    const size_t begin = state.arena_used;
    if (text.off > begin || text.len > begin - text.off ||
        text.len > static_cast<size_t>(-1) - begin)
        return ParseCode::invalid;
    if (!reserve_arena(state, begin + text.len)) return ParseCode::resource;
    const uint8_t* input = state.arena + text.off;

    for (size_t i = 0; i + 4 <= text.len; i += 4) {
        const int a = base64_digit(input[i]);
        const int b = base64_digit(input[i + 1]);
        const int c = base64_digit(input[i + 2]);
        const int d = base64_digit(input[i + 3]);

        if (a < 0 || b < 0) break;

        if (!arena_byte(state,static_cast<uint8_t>(
                (static_cast<uint32_t>(a) << 2) |
                (static_cast<uint32_t>(b) >> 4))))
            return ParseCode::resource;

        if (c >= 0) {
            if (!arena_byte(state,static_cast<uint8_t>(
                    (static_cast<uint32_t>(b) << 4) |
                    (static_cast<uint32_t>(c) >> 2))))
                return ParseCode::resource;
        }

        if (d >= 0) {
            if (!arena_byte(state,static_cast<uint8_t>(
                    (static_cast<uint32_t>(c) << 6) |
                    static_cast<uint32_t>(d))))
                return ParseCode::resource;
        }
    }

    output->off = begin;
    output->len = state.arena_used - begin;
    output->used = true;
    return ParseCode::ok;
}

int hex_digit(uint8_t c) noexcept {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

ParseCode decode_hex(StoreState& state,Span text, Span* output) noexcept {
    if (!output) return ParseCode::invalid;
    *output = Span{state.arena_used, 0, true};
    if (!text.used) return ParseCode::ok;
    if (text.len & 1u) return ParseCode::invalid_hex;

    // D01: the decoded output can grow the same arena that owns the input.
    // Reserve once before borrowing the input pointer; hex output is never
    // larger than its source span, so text.len is a safe upper bound.
    const size_t begin = state.arena_used;
    if (text.off > begin || text.len > begin - text.off ||
        text.len > static_cast<size_t>(-1) - begin)
        return ParseCode::invalid;
    if (!reserve_arena(state, begin + text.len)) return ParseCode::resource;
    const uint8_t* input = state.arena + text.off;

    for (size_t i = 0; i < text.len; i += 2) {
        const int high = hex_digit(input[i]);
        const int low = hex_digit(input[i + 1]);
        if (high < 0 || low < 0) return ParseCode::invalid_hex;
        if (!arena_byte(state,static_cast<uint8_t>((high << 4) | low)))
            return ParseCode::resource;
    }

    output->off = begin;
    output->len = state.arena_used - begin;
    output->used = true;
    return ParseCode::ok;
}


} } }
