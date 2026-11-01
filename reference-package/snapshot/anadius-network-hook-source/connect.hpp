#pragma once
#include "../anadius-execution-source/execution.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>

namespace anadius::network_hook_recovered {
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
    virtual void dialog_038bc0(std::string_view title, std::string_view message,
                               std::uint32_t flags) = 0;
    [[noreturn]] virtual void exit_05cdc0(std::uint32_t) = 0;
    virtual std::int32_t original_connect_0a3838(Socket, std::span<std::byte> address,
                                               std::int32_t supplied_length) = 0;
};
// span describes readable/writable backing memory, NOT the supplied Winsock
// length. Untouched paths forward it without inspecting it. Inspected headers
// require 2 or 4 readable bytes; any write requires writable backing memory.
std::int32_t connect_00fd00(StateRefs, Effects&, Socket, std::span<std::byte> address,
                           std::int32_t supplied_length);

// New composition adapter to the already recovered synchronization core.
// All other Effects still come from the caller, never original DLL addresses.
class ExecutionEffects final : public Effects {
public:
    ExecutionEffects(Effects& remaining, execution_recovered::Execution& execution)
        : remaining_(remaining), execution_(execution) {}
    void wait_steps_036880(std::int32_t) override;
    void log_literal_006880(std::string_view) override;
    void log_port_00ad70(std::uint16_t) override;
    void log_finish_0088e0() override;
    void dialog_038bc0(std::string_view, std::string_view, std::uint32_t) override;
    [[noreturn]] void exit_05cdc0(std::uint32_t) override;
    std::int32_t original_connect_0a3838(Socket, std::span<std::byte>, std::int32_t) override;
private:
    Effects& remaining_;
    execution_recovered::Execution& execution_;
};
}
