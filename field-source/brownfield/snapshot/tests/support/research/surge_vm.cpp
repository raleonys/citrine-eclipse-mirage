#include "research/surge_vm.hpp"
#include <limits>
#include <stdexcept>

namespace preloader_l { namespace research {
namespace {
constexpr std::uint64_t k_cf=0x001, k_pf=0x004, k_af=0x010;
constexpr std::uint64_t k_zf=0x040, k_sf=0x080, k_of=0x800;
constexpr std::uint64_t k_status_mask=k_cf|k_pf|k_af|k_zf|k_sf|k_of;
constexpr std::uint64_t k_supported_rflags=0xED7;

bool even_low_byte_parity(std::uint64_t value) noexcept {
    auto b=static_cast<std::uint8_t>(value);
    b^=static_cast<std::uint8_t>(b>>4); b^=static_cast<std::uint8_t>(b>>2); b^=static_cast<std::uint8_t>(b>>1);
    return (b&1u)==0;
}
std::uint64_t logical_status(std::uint64_t value) noexcept {
    return (even_low_byte_parity(value)?k_pf:0) | (value==0?k_zf:0) | ((value>>63)?k_sf:0);
}
void validate(const X64State& s, std::uint64_t base, std::uint64_t rva, std::uint64_t min_rsp) {
    if (s.rsp < min_rsp) throw std::invalid_argument("stack underflow");
    if ((s.rflags&2)==0 || (s.rflags&~k_supported_rflags)!=0) throw std::invalid_argument("unsupported rflags");
    if (base > std::numeric_limits<std::uint64_t>::max()-rva) throw std::invalid_argument("address overflow");
}
VmBootstrapResult bootstrap(const X64State& in, std::uint64_t top, std::uint64_t base, bool second) {
    const std::uint64_t entry=second?0x44A24FULL:0x42A24FULL;
    const std::uint64_t compare=second?0x44A671ULL:0x42A666ULL;
    validate(in,base,compare,160);
    VmBootstrapResult out{in,in.rsp-160,{
        second?top:in.rsp-144,
        second?0x8880868DULL:in.r15,
        second?in.rcx:in.r10,
        in.rdi,base,in.r8,in.r9,in.r10,in.r11,in.r12,in.r13,in.r14,in.r15,
        in.rdi,in.rsi,in.rbp,in.rbx,in.rbx,in.rdx,base+entry+5,in.rax
    },0x10};
    out.state.rax=0; out.state.rbx=k_context_lock_offset; out.state.rcx=1;
    out.state.rdx=k_context_stride; out.state.rsi=base+k_context_pool_rva;
    out.state.rdi=k_context_count; out.state.rbp=base+k_context_pool_rva;
    out.state.rsp=in.rsp-136; out.state.r8=in.rcx; out.state.r9=top;
    out.state.rip=base+compare; out.state.rflags=(in.rflags&~0x8D5ULL)|0x44ULL;
    return out;
}
}

VmEntryResult model_vm_entry_44c5c3(const X64State& in, std::uint64_t base) {
    validate(in,base,0x42A24F,64);
    VmEntryResult out{in,in.rsp-64,{0x424B95ULL,in.rbx,in.rbx,in.rcx^0x7FDF4861ULL,0x6B31ULL,in.rflags,0x46EULL,0x424B95ULL},k_af};
    out.state.rsp=in.rsp-32; out.state.rip=base+0x42A24FULL;
    out.state.rflags=(in.rflags&~k_status_mask)|logical_status(in.rcx);
    return out;
}

VmEntryResult model_vm_entry_4911f3(const X64State& in, std::uint64_t base) {
    validate(in,base,0x44A24F,64);
    VmEntryResult out{in,in.rsp-64,{0x480712ULL,in.rsp-40,in.rsp-32,in.r8,0xAE7EULL,in.rflags,0x1AAULL,0x480712ULL},0};
    out.state.rsp=in.rsp-32; out.state.rip=base+0x44A24FULL;
    const std::uint64_t before=in.rsp-32, after=before+8;
    const std::uint64_t add_flags=logical_status(after) | (after<before?k_cf:0) |
        (((before^8ULL^after)&0x10ULL)?k_af:0) |
        (((~(before^8ULL)&(before^after))>>63)?k_of:0);
    out.state.rflags=(in.rflags&~k_status_mask)|add_flags;
    return out;
}

VmBootstrapResult model_vm_bootstrap_42a24f(const X64State& in, std::uint64_t top, std::uint64_t base) {
    return bootstrap(in,top,base,false);
}
VmBootstrapResult model_vm_bootstrap_44a24f(const X64State& in, std::uint64_t top, std::uint64_t base) {
    return bootstrap(in,top,base,true);
}

VmContextScan model_try_claim_vm_context(VmContextOccupancy& occupancy, std::uint64_t base) {
    constexpr std::uint64_t last=k_context_pool_rva+(k_context_count-1)*k_context_stride+k_context_lock_offset+3;
    if (base>std::numeric_limits<std::uint64_t>::max()-last) throw std::invalid_argument("context address overflow");
    for (std::size_t i=0;i<occupancy.size();++i) {
        std::uint32_t expected=0;
        if (occupancy[i].compare_exchange_strong(expected,1,std::memory_order_seq_cst,std::memory_order_seq_cst)) {
            const auto context=base+k_context_pool_rva+i*k_context_stride;
            return {true,i,i+1,context,context+k_context_lock_offset};
        }
    }
    return {false,k_context_count,k_context_count,0,0};
}

} } // namespace preloader_l::research
