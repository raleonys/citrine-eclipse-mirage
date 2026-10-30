#include "preloader_l/platform/hook_connect.hpp"
#include <algorithm>
#include <array>

namespace preloader_l::network_hook_recovered {
namespace {
std::uint16_t little_word(std::span<const std::byte> buffer, std::size_t offset) {
    if (offset + 2 > buffer.size())
        throw OutOfEvidenceInput("00fd00 accesses an unreadable sockaddr prefix");
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(buffer[offset]) |
        (std::to_integer<unsigned>(buffer[offset+1]) << 8));
}
}
std::int32_t connect_00fd00_inspected(StateRefs state, Effects& effects, Socket socket,
    std::span<const std::byte> caller_address, std::span<const std::byte> inspected_address,
    std::int32_t supplied_length) {
    if (!((supplied_length == state.ipv4_length_0a10d0 && little_word(inspected_address,0) == 2) ||
          (supplied_length == state.ipv6_length_0a10d4 && little_word(inspected_address,0) == 23)))
        return effects.original_connect_0a3838(socket,caller_address,supplied_length);
    const auto network_word = little_word(inspected_address,2);
    const auto port = static_cast<std::uint16_t>((network_word >> 8) | (network_word << 8));
    const bool ipv4 = little_word(inspected_address,0) == 2;
    const std::size_t size = ipv4 ? 16 : 28;
    if(inspected_address.size()<size)
        throw OutOfEvidenceInput("Loopback policy requires a complete sockaddr");
    bool loopback;
    if(ipv4) {
        loopback = inspected_address[4]==std::byte{127} && inspected_address[5]==std::byte{0} &&
                   inspected_address[6]==std::byte{0} && inspected_address[7]==std::byte{1};
    } else {
        loopback = std::all_of(inspected_address.begin()+8,inspected_address.begin()+23,
                              [](std::byte b){return b==std::byte{0};}) &&
                   inspected_address[23]==std::byte{1};
    }
    if(!loopback || port!=3216)
        return effects.original_connect_0a3838(socket,caller_address,supplied_length);
    effects.wait_steps_036880(-1);
    // No live listener means no valid replacement destination.
    const bool redirect = state.server_port_0a3c90!=0;
    std::array<std::byte,28> copy{};
    if (redirect) std::copy_n(inspected_address.begin(),size,copy.begin());
    if (redirect) {
        effects.log_literal_006880("game connected on port ");
        effects.log_port_00ad70(port);
        effects.log_literal_006880("; redirecting to ");
        effects.log_port_00ad70(state.server_port_0a3c90);
        effects.log_finish_0088e0();
        const auto attempt = effects.increment_attempts(state.attempts_0a3888);
        if (attempt == 15) {
            // This is a target UI literal, not operational advice to the user.
            effects.dialog_038bc0("error",
                "The game tried to connect to the crack 15 times already. Most probably your firewall or anti-virus blocks it. Disable them and try again.",
                0x10);
            effects.exit_05cdc0(0xffffffffu);
        }
        const auto replacement = state.server_port_0a3c90; // read again after effects
        copy[2] = static_cast<std::byte>(replacement >> 8);
        copy[3] = static_cast<std::byte>(replacement & 255);
    }
    // The original sees a complete live copy only for a redirected call.
    return effects.original_connect_0a3838(socket,redirect ? std::span<const std::byte>{copy}.first(size) : caller_address,supplied_length);
}
std::int32_t connect_00fd00(StateRefs state, Effects& effects, Socket socket,
    std::span<const std::byte> address, std::int32_t supplied_length) {
    return connect_00fd00_inspected(state,effects,socket,address,address,supplied_length);
}
void ExecutionEffects::wait_steps_036880(std::int32_t count) {execution_.wait_steps_036880(count);}
void ExecutionEffects::log_literal_006880(std::string_view s) {remaining_.log_literal_006880(s);}
void ExecutionEffects::log_port_00ad70(std::uint16_t p) {remaining_.log_port_00ad70(p);}
void ExecutionEffects::log_finish_0088e0() {remaining_.log_finish_0088e0();}
std::uint32_t ExecutionEffects::increment_attempts(std::uint32_t& value) {return remaining_.increment_attempts(value);}
void ExecutionEffects::dialog_038bc0(std::string_view t,std::string_view s,std::uint32_t f) {remaining_.dialog_038bc0(t,s,f);}
[[noreturn]] void ExecutionEffects::exit_05cdc0(std::uint32_t c) {remaining_.exit_05cdc0(c);}
std::int32_t ExecutionEffects::original_connect_0a3838(Socket s,std::span<const std::byte> a,std::int32_t n) {return remaining_.original_connect_0a3838(s,a,n);}
}
