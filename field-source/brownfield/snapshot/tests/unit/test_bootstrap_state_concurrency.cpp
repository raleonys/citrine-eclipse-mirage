#include "preloader_l/runtime/bootstrap_state.hpp"

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

int main()
{
    using namespace preloader_l::runtime;
    reset_bootstrap_for_test();
    if (!mark_bootstrap_scheduled()) return 1;
    if (!mark_bootstrap_running()) return 2;
    if (!mark_bootstrap_runtime_initialized()) return 3;
    if (!mark_bootstrap_listener_active(3226u)) return 4;

    constexpr unsigned kThreads = 8u;
    constexpr unsigned kCallsPerThread = 2000u;
    std::atomic<unsigned> failures{0u};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);
    for (unsigned t = 0; t < kThreads; ++t) {
        threads.emplace_back([&failures]() {
            for (unsigned i = 0; i < kCallsPerThread; ++i) {
                if (!mark_bootstrap_client_served())
                    failures.fetch_add(1u, std::memory_order_relaxed);
            }
        });
    }
    for (auto& thread : threads) thread.join();

    if (failures.load(std::memory_order_relaxed) != 0u) return 5;
    const auto before_complete = snapshot_bootstrap();
    if (before_complete.phase != BootstrapPhase::client_served) return 6;
    if (before_complete.bound_port != 3226u) return 7;
    if (before_complete.client_count != kThreads * kCallsPerThread) return 8;
    if (!mark_bootstrap_completed()) return 9;
    const auto completed = snapshot_bootstrap();
    if (completed.phase != BootstrapPhase::completed) return 10;
    if (completed.client_count != kThreads * kCallsPerThread) return 11;
    return 0;
}
