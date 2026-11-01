#include "onexit.hpp"
#include <bit>
#include <limits>

namespace anadius::crt_onexit_recovered {
Address encode_pointer(Address p, std::uint64_t cookie) {
    return std::rotl(p, static_cast<int>(cookie & 63)) ^ cookie;
}
Address decode_pointer(Address p, std::uint64_t cookie) {
    return std::rotr(p ^ cookie, static_cast<int>(cookie & 63));
}
int initialize_onexit_table_0643f8(Table* table, const State& state) {
    if (!table) return -1;
    if (table->begin == table->capacity_end)
        *table = {state.cookie_0a1580, state.cookie_0a1580, state.cookie_0a1580};
    return 0;
}
namespace {
std::uint64_t sar_three(std::uint64_t value) {
    return (value >> 3) | ((value >> 63) ? UINT64_C(0xe000000000000000) : 0);
}
int register_unlocked(Table* table, Address callback, State& state, Effects& effects) {
    if (!table) return -1;
    auto cookie = state.cookie_0a1580;
    auto begin = decode_pointer(table->begin, cookie);
    auto end = decode_pointer(table->end, cookie);
    auto limit = decode_pointer(table->capacity_end, cookie);
    if (end == limit) {
        const auto old_count = sar_three(limit - begin);
        auto new_count = old_count + (old_count > 512 ? 512 : old_count);
        if (!new_count) new_count = 32;
        Address allocation = 0;
        if (new_count >= old_count) {
            allocation = effects.recalloc_070434(begin, new_count, 8);
            effects.free_0668c0(0);
        }
        if (!allocation) {
            new_count = old_count + 4;
            allocation = effects.recalloc_070434(begin, new_count, 8);
            effects.free_0668c0(0);
            if (!allocation) return -1;
        }
        begin = allocation;
        end = allocation + old_count * 8;
        limit = allocation + new_count * 8;
        cookie = state.cookie_0a1580;
        const auto fill_count = end > limit ? 0 : (limit - end + 7) >> 3;
        for (std::uint64_t i = 0; i != fill_count; ++i)
            effects.write_word(end + i * 8, cookie);
        cookie = state.cookie_0a1580;
    }
    effects.write_word(end, encode_pointer(callback, cookie));
    table->begin = encode_pointer(begin, state.cookie_0a1580);
    table->end = encode_pointer(end + 8, state.cookie_0a1580);
    table->capacity_end = encode_pointer(limit, state.cookie_0a1580);
    return 0;
}
int execute_unlocked(Table* table, State& state, Effects& effects) {
    if (!table) return -1;
    auto cookie = state.cookie_0a1580;
    auto begin = decode_pointer(table->begin, cookie);
    auto end = decode_pointer(table->end, cookie);
    // 0642d2..0642da: unsigned comparison after subtracting one.
    if (begin == 0 || begin == std::numeric_limits<Address>::max()) return 0;
    const auto cleared_slot = cookie;
    auto observed_begin = begin;
    auto observed_end = end;
    for (;;) {
        end -= 8;
        if (end < begin) break;
        const auto encoded_callback = effects.read_word(end);
        if (encoded_callback == cleared_slot) continue;
        effects.write_word(end, cleared_slot);
        effects.invoke_callback(decode_pointer(encoded_callback, cookie));
        cookie = state.cookie_0a1580;
        const auto current_begin = decode_pointer(table->begin, cookie);
        const auto current_end = decode_pointer(table->end, cookie);
        if (current_begin != observed_begin || current_end != observed_end) {
            begin = observed_begin = current_begin;
            end = observed_end = current_end;
        }
    }
    if (begin != std::numeric_limits<Address>::max()) {
        effects.free_0668c0(begin);
        cookie = state.cookie_0a1580;
    }
    *table = {cookie, cookie, cookie};
    return 0;
}
}
int register_onexit_function_064420(Table* table, Address callback, State& state, Effects& effects) {
    effects.lock_0625f8(2);
    const auto result = register_unlocked(table, callback, state, effects);
    effects.unlock_06264c(2);
    return result;
}
int execute_onexit_table_0643b4(Table* table, State& state, Effects& effects) {
    effects.lock_0625f8(2);
    const auto result = execute_unlocked(table, state, effects);
    effects.unlock_06264c(2);
    return result;
}
Address onexit_04b734(Address callback, State& state, Effects& effects) {
    auto* table = state.application_0a47a8.begin == std::numeric_limits<Address>::max()
        ? &state.crt_0a52a8 : &state.application_0a47a8;
    return register_onexit_function_064420(table, callback, state, effects) == 0 ? callback : 0;
}
int atexit_04b770(Address callback, State& state, Effects& effects) {
    return onexit_04b734(callback, state, effects) ? 0 : -1;
}
}
