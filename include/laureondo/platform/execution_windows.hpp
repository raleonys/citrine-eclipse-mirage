// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/runtime/execution.hpp"

namespace laureondo::execution {
// Error raiser is a required ABI dependency, not an invented errno mapping.
using ThreadErrorRaiser = void(*)(int observed_error_index);
class WindowsPlatform final : public Platform {
public:
    explicit WindowsPlatform(ThreadErrorRaiser required_error_raiser) : error_raiser_(required_error_raiser) {}
    std::uint32_t current_thread_id() override;
    void acquire_srw_exclusive(std::uintptr_t&) override;
    void release_srw_exclusive(std::uintptr_t&) override;
    std::int64_t query_performance_frequency() override;
    std::int64_t query_performance_counter() override;
    void sleep_milliseconds(std::uint32_t) override;
    [[noreturn]] void raise_thread_error_04a648(int) override;
private:
    ThreadErrorRaiser error_raiser_;
};
}
