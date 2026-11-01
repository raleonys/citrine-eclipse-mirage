#include "environment.hpp"
#include <bit>
#include <stdexcept>

namespace anadius::crt_environment_recovered {
namespace {
template<class T> std::size_t entry_end(std::span<const T> block, std::size_t offset) {
    while (offset < block.size() && block[offset] != T{}) ++offset;
    if (offset == block.size()) throw std::domain_error("unterminated environment block: unsupported input");
    return offset;
}
template<class T> std::size_t block_end(std::span<const T> block) {
    std::size_t offset = 0;
    for (;;) {
        if (offset >= block.size()) throw std::domain_error("missing final environment terminator");
        if (block[offset] == T{}) return offset;
        offset = entry_end(block, offset) + 1;
    }
}
void check_buffer(Buffer b, std::size_t required) {
    if (b.data != nullptr && b.capacity < required)
        throw std::domain_error("provider returned undersized byte storage");
}
}
Buffer convert_block_06f9e4(Effects& effects) {
    const auto wide = effects.get_environment_strings_w();
    if (wide.empty()) return {};
    const auto units = block_end(wide) + 1;
    const auto count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(units));
    const auto bytes = effects.convert_06cd58(wide, count, {}, 0);
    if (bytes == 0) { effects.free_environment_strings_w(wide); return {}; }
    // MOVSXD of EAX before malloc; negative provider results remain sign extended.
    const auto requested = static_cast<std::uint64_t>(static_cast<std::int64_t>(bytes));
    auto result = effects.malloc_06a308(requested);
    if (result.data == nullptr) {
        effects.free_0668c0(nullptr);
        effects.free_environment_strings_w(wide);
        return {};
    }
    if (bytes > 0) check_buffer(result, static_cast<std::size_t>(bytes));
    if (effects.convert_06cd58(wide, count, result, bytes) == 0) {
        effects.free_0668c0(result.data);
        result = {};
    } else {
        effects.free_0668c0(nullptr);
    }
    effects.free_environment_strings_w(wide);
    return result;
}
void free_environment_063d80(Environment environment, Effects& effects) {
    if (environment.data == nullptr) return;
    for (std::size_t i = 0; i < environment.slots; ++i) {
        if (environment.data[i] == nullptr) { effects.free_0668c0(environment.data); return; }
        effects.free_0668c0(environment.data[i]);
    }
    throw std::domain_error("unterminated environment pointer array");
}
Environment build_array_063b54(std::span<const char> block, Effects& effects) {
    const auto final = block_end(block);
    std::uint64_t count = 0;
    for (std::size_t offset = 0; offset < final; offset = entry_end(block, offset) + 1)
        if (block[offset] != '=') ++count;
    auto result = effects.calloc_pointers_066848(count + 1, 8);
    if (result.data == nullptr) { effects.free_0668c0(nullptr); return {}; }
    if (result.slots < count + 1) throw std::domain_error("undersized pointer array");
    std::size_t index = 0;
    for (std::size_t offset = 0; offset < final;) {
        const auto size = entry_end(block, offset) - offset + 1;
        if (block[offset] != '=') {
            auto copy = effects.calloc_chars_066848(size, 1);
            if (copy.data == nullptr) {
                free_environment_063d80(result, effects);
                effects.free_0668c0(nullptr);
                effects.free_0668c0(nullptr);
                return {};
            }
            check_buffer(copy, size);
            if (effects.strcpy_s_05cff0({copy.data, size}, block.subspan(offset, size)) != 0)
                effects.invoke_watson_05c9a4();
            result.data[index++] = copy.data;
            effects.free_0668c0(nullptr);
        }
        offset += size;
    }
    effects.free_0668c0(nullptr);
    return result;
}
std::uint32_t initialize_063a70(State& state, Effects& effects) {
    if (state.current_0a5288.data != nullptr) return 0;
    effects.initialize_multibyte_06f5f8();
    const auto block = convert_block_06f9e4(effects);
    if (block.data == nullptr) { effects.free_0668c0(nullptr); return 0xffffffffu; }
    const auto environment = build_array_063b54({block.data, block.capacity}, effects);
    const auto result = environment.data == nullptr ? 0xffffffffu : 0u;
    if (environment.data != nullptr) {
        state.initial_0a52a0 = environment;
        state.current_0a5288 = environment;
    }
    effects.free_0668c0(nullptr);
    effects.free_0668c0(block.data);
    return result;
}
std::uint32_t thunk_initialize_064054(State& state, Effects& effects) {
    return initialize_063a70(state, effects);
}
}
