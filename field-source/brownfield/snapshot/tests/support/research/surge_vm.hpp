#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace preloader_l { namespace research {

struct X64State {
    std::uint64_t rax{}, rbx{}, rcx{}, rdx{}, rsi{}, rdi{}, rbp{}, rsp{};
    std::uint64_t r8{}, r9{}, r10{}, r11{}, r12{}, r13{}, r14{}, r15{};
    std::uint64_t rip{}, rflags{2};
};

struct VmEntryResult {
    X64State state;
    std::uint64_t stack_begin{};
    std::array<std::uint64_t, 8> stack_words{};
    std::uint64_t undefined_rflags_mask{};
};

struct VmBootstrapResult {
    X64State state;
    std::uint64_t stack_begin{};
    std::array<std::uint64_t, 21> stack_words{};
    std::uint64_t undefined_rflags_mask{};
};

constexpr std::uint64_t k_default_image_base = 0x180000000ULL;
constexpr std::uint64_t k_context_pool_rva = 0x170549ULL;
constexpr std::uint64_t k_context_stride = 0x180ULL;
constexpr std::uint64_t k_context_lock_offset = 0x4FULL;
constexpr std::size_t k_context_count = 32;

VmEntryResult model_vm_entry_44c5c3(const X64State& input,
                                    std::uint64_t image_base = k_default_image_base);
VmEntryResult model_vm_entry_4911f3(const X64State& input,
                                    std::uint64_t image_base = k_default_image_base);
VmBootstrapResult model_vm_bootstrap_42a24f(const X64State& input,
                                            std::uint64_t stack_top,
                                            std::uint64_t image_base = k_default_image_base);
VmBootstrapResult model_vm_bootstrap_44a24f(const X64State& input,
                                            std::uint64_t stack_top,
                                            std::uint64_t image_base = k_default_image_base);

using VmContextOccupancy = std::array<std::atomic<std::uint32_t>, k_context_count>;
struct VmContextScan {
    bool acquired{};
    std::size_t slot{k_context_count};
    std::size_t attempted_slots{};
    std::uint64_t context_address{};
    std::uint64_t lock_address{};
};
VmContextScan model_try_claim_vm_context(VmContextOccupancy& occupancy,
                                         std::uint64_t image_base = k_default_image_base);

} } // namespace preloader_l::research
