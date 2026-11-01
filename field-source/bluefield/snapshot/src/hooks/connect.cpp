#include "src/hooks/connect.hpp"

namespace preloader_l::network_hooks {
namespace {
std::uint16_t little_word(std::span<std::byte> buffer, std::size_t offset) {
    if (offset + 2 > buffer.size())
        throw OutOfEvidenceInput("00fd00 accesses an unreadable sockaddr prefix");
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(buffer[offset]) |
        (std::to_integer<unsigned>(buffer[offset+1]) << 8));
}
}
std::int32_t connect_00fd00(StateRefs state, Effects& effects, Socket socket,
    std::span<std::byte> address, std::int32_t supplied_length) {
    if (!((supplied_length == state.ipv4_length_0a10d0 && little_word(address,0) == 2) ||
          (supplied_length == state.ipv6_length_0a10d4 && little_word(address,0) == 23)))
        return effects.original_connect_0a3838(socket,address,supplied_length);
    const auto network_word = little_word(address,2);
    const auto port = static_cast<std::uint16_t>((network_word >> 8) | (network_word << 8));
    bool redirect;
    if (port == 443 || port == 80) {
        redirect = port == state.server_port_0a3c90;
    } else {
        effects.wait_steps_036880(-1);
        redirect = port == 3216 || port == state.server_port_0a3c90;
    }
    if (redirect) {
        effects.log_literal_006880("game connected on port ");
        effects.log_port_00ad70(port);
        effects.log_literal_006880("; redirecting to ");
        effects.log_port_00ad70(state.server_port_0a3c90);
        effects.log_finish_0088e0();
        const auto attempt=effects.increment_attempts(state.attempts_0a3888);
        if (attempt == 15) {
            // This is a target UI literal, not operational advice to the user.
            effects.dialog_038bc0("error",
                "The game tried to connect to the crack 15 times already. Most probably your firewall or anti-virus blocks it. Disable them and try again.",
                0x10);
            effects.exit_05cdc0(0xffffffffu);
        }
        const auto replacement = state.server_port_0a3c90; // read again after effects
        address[2] = static_cast<std::byte>(replacement >> 8);
        address[3] = static_cast<std::byte>(replacement & 255);
    }
    // Original is an indirect tail JMP: retain its int32 Winsock result.
    return effects.original_connect_0a3838(socket,address,supplied_length);
}
}
