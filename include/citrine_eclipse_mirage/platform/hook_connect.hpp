// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/runtime/execution.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>

namespace citrine_eclipse_mirage::network_hook {
using Socket = std::uintptr_t;
struct OutOfEvidenceInput : std::invalid_argument { using std::invalid_argument::invalid_argument; };
struct StateRefs {
    std::int32_t& ipv4_length_0a10d0;
    std::int32_t& ipv6_length_0a10d4;
    std::uint16_t& server_port_0a3c90;
    std::uint32_t& attempts_0a3888;
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual void wait_steps_036880(std::int32_t count) = 0;
    virtual void log_literal_006880(std::string_view) = 0;
    virtual void log_port_00ad70(std::uint16_t) = 0;
    virtual void log_finish_0088e0() = 0;
    // Pure source callers may use the sequential default. Native owners narrow
    // synchronization to this increment, never the wait or original connect.
    virtual std::uint32_t increment_attempts(std::uint32_t& value) { return ++value; }
    virtual void dialog_038bc0(std::string_view title, std::string_view message,
                               std::uint32_t flags) = 0;
    [[noreturn]] virtual void exit_05cdc0(std::uint32_t) = 0;
    virtual std::int32_t original_connect_0a3838(Socket, std::span<const std::byte> address,
                                               std::int32_t supplied_length) = 0;
};
// Read-only caller storage. Only exact loopback:3216 is redirected using a
// complete stack copy. Other destinations retain the original pointer/length.
std::int32_t connect_00fd00(StateRefs, Effects&, Socket, std::span<const std::byte> address,
                           std::int32_t supplied_length);
// Native entrypoints inspect a protected bounded snapshot. A zero-length
// caller_address retains the exact opaque pointer for original Winsock calls;
// only inspected_address is dereferenced by the redirect decision.
std::int32_t connect_00fd00_inspected(StateRefs, Effects&, Socket,
    std::span<const std::byte> caller_address, std::span<const std::byte> inspected_address,
    std::int32_t supplied_length);

// Composition adapter to the existing synchronization core.
// All other Effects still come from the caller, never original DLL addresses.
class ExecutionEffects final : public Effects {
public:
    ExecutionEffects(Effects& remaining, execution::Execution& execution)
        : remaining_(remaining), execution_(execution) {}
    void wait_steps_036880(std::int32_t) override;
    void log_literal_006880(std::string_view) override;
    void log_port_00ad70(std::uint16_t) override;
    void log_finish_0088e0() override;
    std::uint32_t increment_attempts(std::uint32_t&) override;
    void dialog_038bc0(std::string_view, std::string_view, std::uint32_t) override;
    [[noreturn]] void exit_05cdc0(std::uint32_t) override;
    std::int32_t original_connect_0a3838(Socket, std::span<const std::byte>, std::int32_t) override;
private:
    Effects& remaining_;
    execution::Execution& execution_;
};
}
