#include "argv.hpp"
#include <bit>
#include <limits>
#include <new>
#include <stdexcept>

namespace anadius::crt_argv_recovered {
static_assert(sizeof(char*) == 8, "recovered x64 argv storage layout");
bool allocation_size_063888(std::uint64_t pointers, std::uint64_t characters,
                            std::uint64_t element_size, std::uint64_t& bytes) {
    constexpr auto max = std::numeric_limits<std::uint64_t>::max();
    if (pointers >= 0x1fffffffffffffffULL) return false;
    if (element_size == 0) throw std::domain_error("063888 DIV-by-zero path unsupported");
    if (characters >= max / element_size) return false;
    const auto pointer_bytes = pointers * 8;
    const auto character_bytes = characters * element_size;
    if (character_bytes >= ~pointer_bytes) return false;
    bytes = pointer_bytes + character_bytes;
    return true;
}

Allocation allocate_buffer_063888(std::uint64_t pointers, std::uint64_t characters,
                                  std::uint64_t element_size, Effects& effects) {
    std::uint64_t bytes{};
    if (!allocation_size_063888(pointers, characters, element_size, bytes)) return {};
    auto result = effects.calloc_066848(bytes, 1);
    effects.free_0668c0(nullptr);
    return result;
}

namespace {
struct TypedAllocation { std::span<char*> pointers; std::span<char> text; };
TypedAllocation prepare(Allocation allocation, crt_argv_parse_recovered::Counts count) {
    // Extra source-domain guards reject malformed providers rather than invent
    // behavior for original memory faults or undersized/misaligned storage.
    std::uint64_t required{};
    if (!allocation_size_063888(count.pointers, count.characters, 1, required) ||
        allocation.storage.data() != allocation.base ||
        allocation.storage.size() < required ||
        reinterpret_cast<std::uintptr_t>(allocation.base) % alignof(char*) != 0) {
        throw std::domain_error("argv provider supplied invalid raw storage");
    }
    // Non-allocating placement array new starts actual array lifetimes without
    // invoking a provider or adding allocator metadata in the recovered block.
    auto** pointers = ::new (allocation.base) char*[static_cast<std::size_t>(count.pointers)];
    auto* text_storage = allocation.storage.data() + count.pointers * 8;
    auto* text = ::new (static_cast<void*>(text_storage)) char[static_cast<std::size_t>(count.characters)];
    return {{pointers, static_cast<std::size_t>(count.pointers)},
            {text, static_cast<std::size_t>(count.characters)}};
}
}

std::uint32_t configure_0638e8(State& state, std::int32_t mode, Effects& effects,
                              crt_argv_parse_recovered::Effects& parse_effects) {
    if (mode == 0) return 0;
    if (static_cast<std::uint32_t>(mode) - 1U > 1U) {
        effects.errno_05d724().store(22);
        effects.invalid_parameter_05c954();
        return 22;
    }
    effects.initialize_multibyte_06f5f8();
    (void)effects.module_filename_06ecdc(state.module_filename);
    state.program_name = state.module_filename.data();
    auto input = state.command_line;
    if (input.empty() || input.front() == '\0') input = state.module_filename;
    crt_argv_parse_recovered::Counts count{};
    crt_argv_parse_recovered::parse_0636c0(input, {}, {}, count, parse_effects);
    const auto allocation_count = count;
    auto allocation = allocate_buffer_063888(count.pointers, count.characters, 1, effects);
    if (!allocation.base) {
        effects.errno_05d724().store(12);
        effects.free_0668c0(nullptr);
        return 12;
    }
    auto typed = prepare(allocation, allocation_count);
    crt_argv_parse_recovered::parse_0636c0(input, typed.pointers, typed.text, count, parse_effects);
    if (mode == 1) {
        state.argv = typed.pointers.data(); // assembly stores argv before argc
        const auto argc_bits = static_cast<std::uint32_t>(count.pointers) - 1U;
        state.argc = std::bit_cast<std::int32_t>(argc_bits);
        effects.free_0668c0(nullptr);
        return 0;
    }
    Expansion expanded{};
    const auto expansion_status = effects.expand_06ecd4(typed.pointers, expanded);
    if (expansion_status != 0) {
        effects.free_0668c0(expanded.base);
        expanded = {};
        effects.free_0668c0(allocation.base);
        return static_cast<std::uint32_t>(expansion_status);
    }
    if (!expanded.base || expanded.pointers.data() != expanded.base) {
        throw std::domain_error("successful argv expansion requires a live bounded pointer array");
    }
    std::uint64_t expanded_count{};
    bool terminated{};
    for (auto* value : expanded.pointers) {
        if (!value) { terminated = true; break; }
        ++expanded_count;
    }
    if (!terminated) throw std::domain_error("argv expansion array is not terminated");
    // Only the low DWORD is published; count never narrows through signed overflow.
    state.argc = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(expanded_count));
    state.argv = expanded.base;
    effects.free_0668c0(nullptr);
    effects.free_0668c0(allocation.base);
    return 0;
}
} // namespace anadius::crt_argv_recovered
