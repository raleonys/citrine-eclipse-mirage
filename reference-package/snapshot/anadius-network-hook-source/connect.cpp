#include "connect.hpp"
#include <atomic>

namespace anadius::network_hook_recovered {
namespace {
std::uint16_t little_word(std::span<const std::byte> buffer, std::size_t offset) {
    if (offset + 2 > buffer.size())
        throw OutOfEvidenceInput("00fd00 accesses an unreadable sockaddr prefix");
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(buffer[offset]) |
        (std::to_integer<unsigned>(buffer[offset+1]) << 8));
}
}
bool original_only_00fd00(std::int32_t v4,std::int32_t v6,std::span<const std::byte> address,std::int32_t length){
    return !((length==v4&&little_word(address,0)==2)||(length==v6&&little_word(address,0)==23));
}
std::uint16_t requested_port_00fd00(std::span<const std::byte> address){
    const auto word=little_word(address,2);
    return static_cast<std::uint16_t>((word>>8)|(word<<8));
}
void redirect_body_00fd00(Effects& effects,std::span<std::byte> address,std::uint16_t port,
    std::uint16_t& server_port,std::uint32_t& attempts){
        if(address.size()<4)throw OutOfEvidenceInput("00fd00 redirect requires writable port prefix");
        effects.log_literal_006880("game connected on port ");
        effects.log_port_00ad70(port);
        effects.log_literal_006880("; redirecting to ");
        effects.log_port_00ad70(load_server_port_0a3c90(server_port));
        effects.log_finish_0088e0();
        // Native00fdc0..00fdce: MOV/INC/MOV, then CMP of cached EAX.
        // Keep distinct relaxed load/store: fetch_add would replace the
        // original unlocked update with a stronger indivisible RMW.
        static_assert(std::atomic_ref<std::uint32_t>::is_always_lock_free);
        static_assert(std::atomic_ref<std::uint32_t>::required_alignment<=alignof(std::uint32_t));
        std::uint32_t updated;
        {
            auto counter=std::atomic_ref<std::uint32_t>(attempts);
            updated=counter.load(std::memory_order_relaxed)+std::uint32_t{1};
            counter.store(updated,std::memory_order_relaxed);
        }
        if (updated == 15) {
            // This is a target UI literal, not operational advice to the user.
            effects.dialog_038bc0("error",
                "The game tried to connect to the crack 15 times already. Most probably your firewall or anti-virus blocks it. Disable them and try again.",
                0x10);
            effects.exit_05cdc0(0xffffffffu);
        }
        const auto replacement = load_server_port_0a3c90(server_port); // read again after effects
        address[2] = static_cast<std::byte>(replacement >> 8);
        address[3] = static_cast<std::byte>(replacement & 255);
}
std::int32_t connect_00fd00(StateRefs state, Effects& effects, Socket socket,
    std::span<std::byte> address, std::int32_t supplied_length) {
    if (original_only_00fd00(state.ipv4_length_0a10d0,state.ipv6_length_0a10d4,address,supplied_length))
        return effects.original_connect_0a3838(socket,address,supplied_length);
    const auto port = requested_port_00fd00(address);
    bool redirect;
    if (port == 443 || port == 80)redirect = port == load_server_port_0a3c90(state.server_port_0a3c90);
    else {effects.wait_steps_036880(-1);redirect = port == 3216 || port == load_server_port_0a3c90(state.server_port_0a3c90);}
    if(redirect)redirect_body_00fd00(effects,address,port,state.server_port_0a3c90,state.attempts_0a3888);
    // Original is an indirect tail JMP: retain its int32 Winsock result.
    return effects.original_connect_0a3838(socket,address,supplied_length);
}
void ExecutionEffects::wait_steps_036880(std::int32_t count) {execution_.wait_steps_036880(count);}
void ExecutionEffects::log_literal_006880(std::string_view s) {remaining_.log_literal_006880(s);}
void ExecutionEffects::log_port_00ad70(std::uint16_t p) {remaining_.log_port_00ad70(p);}
void ExecutionEffects::log_finish_0088e0() {remaining_.log_finish_0088e0();}
void ExecutionEffects::dialog_038bc0(std::string_view t,std::string_view s,std::uint32_t f) {remaining_.dialog_038bc0(t,s,f);}
[[noreturn]] void ExecutionEffects::exit_05cdc0(std::uint32_t c) {remaining_.exit_05cdc0(c);}
std::int32_t ExecutionEffects::original_connect_0a3838(Socket s,std::span<std::byte> a,std::int32_t n) {return remaining_.original_connect_0a3838(s,a,n);}
}
