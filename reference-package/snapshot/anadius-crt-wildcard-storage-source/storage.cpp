#include "storage.hpp"
#include <bit>
#include <limits>

namespace anadius::crt_wildcard_storage {
std::uint32_t append_06e800(Address source, Address prefix, std::uint64_t prefix_bytes,
                            Vector& vector, Effects& effects) {
    const auto source_bytes = effects.length(source) + std::uint64_t{1};
    if (source_bytes > ~prefix_bytes) return 12;
    const auto allocation_bytes = prefix_bytes + 1 + source_bytes;
    const auto text = effects.calloc_066848(allocation_bytes, 1);
    if (prefix_bytes != 0 &&
        effects.memcpy_s_0767fc(text, allocation_bytes, prefix, prefix_bytes) != 0) {
        effects.watson_05c9a4();
    }
    if (effects.memcpy_s_0767fc(text + prefix_bytes, allocation_bytes - prefix_bytes,
                               source, source_bytes) != 0) {
        effects.watson_05c9a4();
    }
    bool failure = false;
    const auto capacity_end = vector.capacity_end;
    if (vector.end == capacity_end) {
        if (vector.begin == 0) {
            vector.begin = effects.calloc_066848(4, 8);
            effects.free_0668c0(0);
            const auto begin = vector.begin;
            if (begin == 0) failure = true;
            else { vector.end = begin; vector.capacity_end = begin + 32; }
        } else {
            // SUB / SAR 3 / unsigned JA INT64_MAX. Negative wrapped distances
            // fail the original guard; no stronger vector-shape check is added.
            const auto distance = capacity_end - vector.begin;
            const auto count = static_cast<std::uint64_t>(
                std::bit_cast<std::int64_t>(distance) >> 3);
            if (count > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                failure = true;
            } else {
                const auto doubled = count + count;
                const auto replacement = effects.recalloc_070434(vector.begin, doubled, 8);
                if (replacement == 0) {
                    effects.free_0668c0(0);
                    failure = true;
                } else {
                    vector.begin = replacement;
                    vector.end = replacement + count * 8;
                    vector.capacity_end = replacement + doubled * 8;
                    effects.free_0668c0(0);
                }
            }
        }
    }
    if (failure) effects.free_0668c0(text);
    else {
        effects.write_word(vector.end, text);
        vector.end += 8;
    }
    effects.free_0668c0(0);
    return failure ? 12 : 0;
}
}
