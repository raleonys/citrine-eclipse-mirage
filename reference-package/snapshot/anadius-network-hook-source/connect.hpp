#pragma once
#include "../anadius-execution-source/execution.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>

namespace anadius::network_hook_recovered {
using Socket = std::uintptr_t;
// Native a3c90 is a live word, independent of startup-completion publication.
// Keep its ordinary uint16 storage at a stable address. Every concurrent live
// access must use these helpers; plain reads/writes/copies require quiescence.
// Transient atomic_ref objects leave startup-only parser writes plain.
static_assert(std::atomic_ref<std::uint16_t>::is_always_lock_free);
static_assert(std::atomic_ref<std::uint16_t>::required_alignment<=alignof(std::uint16_t));
inline std::uint16_t load_server_port_0a3c90(std::uint16_t& port) noexcept {
    return std::atomic_ref<std::uint16_t>(port).load(std::memory_order_relaxed);
}
inline void store_server_port_0a3c90(std::uint16_t& port,std::uint16_t value) noexcept {
    std::atomic_ref<std::uint16_t>(port).store(value,std::memory_order_relaxed);
}
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
// Pure prefix classification: never reads port/server/counter or invokes effects.
// Keep the original short-circuit behavior, including equal configured lengths.
bool original_only_00fd00(std::int32_t ipv4_length,std::int32_t ipv6_length,
                         std::span<const std::byte> address,std::int32_t supplied_length);
// Requires four readable bytes; pure network-order port extraction.
std::uint16_t requested_port_00fd00(std::span<const std::byte> address);
// Extracted redirect body: log -> counter -> terminal15 -> port store.
// Four writable bytes required; no wait/original. Full core passes live port
// ref; admitted snapshot callers may pass their immutable local copy.
// Port log and final rewrite perform separate relaxed loads, retaining the
// native reread after effects even when the initial route saw another value.
// Live counter access is through this body's lock-free atomic_ref load/store;
// external plain counter reads/writes require quiescence. Cached increment
// controls terminal15; concurrent lost updates remain possible as in native.
void redirect_body_00fd00(Effects&,std::span<std::byte>,std::uint16_t requested,
                         std::uint16_t& server_port,std::uint32_t& attempts);
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
