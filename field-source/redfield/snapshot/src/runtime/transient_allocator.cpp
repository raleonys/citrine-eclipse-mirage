// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/transient_allocator.hpp"
#include "preloader_l/shutdown_budget.hpp"
#include <atomic>
#include <new>
#include <thread>
namespace preloader_l { namespace runtime {
namespace {
struct TransientOwnerHeader {
    TransientOwnerHeader* next;
    size_t allocation_bytes;
    size_t payload_bytes;
    TransientPlatformOps ops;
};

std::atomic_flag g_transient_owner_lock = ATOMIC_FLAG_INIT;
std::atomic_flag g_retry_in_progress = ATOMIC_FLAG_INIT;
TransientOwnerHeader* g_transient_owner_head = nullptr;
struct RetryRelease {
    ~RetryRelease() { g_retry_in_progress.clear(std::memory_order_release); }
};

void transient_owner_lock() noexcept {
    while (g_transient_owner_lock.test_and_set(std::memory_order_acquire)) {}
}
void transient_owner_unlock() noexcept {
    g_transient_owner_lock.clear(std::memory_order_release);
}

TransientOwnerHeader* transient_header(void* payload) noexcept {
    return payload ? static_cast<TransientOwnerHeader*>(payload) - 1 : nullptr;
}

void retain_transient_owner(TransientOwnerHeader* header) noexcept {
    if (!header) return;
    transient_owner_lock();
    header->next = g_transient_owner_head;
    g_transient_owner_head = header;
    transient_owner_unlock();
}

} // namespace
void* allocate_transient_bytes(size_t payload_size,
                               const TransientPlatformOps& ops) noexcept {
    if (!payload_size || !ops.heap_alloc || !ops.heap_free) return nullptr;
    if (payload_size > static_cast<size_t>(-1) - sizeof(TransientOwnerHeader))
        return nullptr;
    const size_t total = sizeof(TransientOwnerHeader) + payload_size;
    void* raw = ops.heap_alloc(ops.context, total);
    if (!raw) return nullptr;
    // The platform supplies raw storage, including on the Win32 heap path.
    // Start the intrusive owner's C++ lifetime explicitly, without allocating
    // again or depending on malloc-specific implicit object creation.
    auto* header = ::new (raw) TransientOwnerHeader{nullptr,total,payload_size,ops};
    return header + 1;
}

bool release_transient_bytes(void* payload) noexcept {
    if (!payload) return true;
    auto* header = transient_header(payload);
    if (!header->ops.heap_free) {
        retain_transient_owner(header);
        return false;
    }
    if (header->ops.heap_free(header->ops.context, header)) return true;
    // The failed allocation itself is the fallback owner record. This path
    // cannot fail due to a second allocation or owner-table capacity.
    retain_transient_owner(header);
    return false;
}

bool retry_pending_transient_releases() noexcept {
    if (g_retry_in_progress.test_and_set(std::memory_order_acquire)) return false;
    RetryRelease release;
    transient_owner_lock();
    TransientOwnerHeader* pending = g_transient_owner_head;
    g_transient_owner_head = nullptr;
    transient_owner_unlock();

    TransientOwnerHeader* failed = nullptr;
    while (pending) {
        TransientOwnerHeader* current = pending;
        pending = current->next;
        current->next = nullptr;
        if (current->ops.heap_free &&
            current->ops.heap_free(current->ops.context, current))
            continue;
        current->next = failed;
        failed = current;
    }

    if (failed) {
        transient_owner_lock();
        TransientOwnerHeader* tail = failed;
        while (tail->next) tail = tail->next;
        tail->next = g_transient_owner_head;
        g_transient_owner_head = failed;
        transient_owner_unlock();
    }
    transient_owner_lock();
    const bool empty=g_transient_owner_head==nullptr;
    transient_owner_unlock();
    return empty;
}

bool transient_release_pending() noexcept {
    transient_owner_lock();
    // Observe the detached-batch flag under the same lock as queue removal;
    // checking it first could race a retry that then removes the final node.
    const bool pending = g_transient_owner_head != nullptr ||
        g_retry_in_progress.test(std::memory_order_acquire);
    transient_owner_unlock();
    return pending;
}

size_t transient_pending_release_count() noexcept {
    size_t count = 0;
    transient_owner_lock();
    for (auto* p = g_transient_owner_head; p; p = p->next) ++count;
    transient_owner_unlock();
    return count;
}

size_t transient_pending_release_bytes() noexcept {
    size_t bytes = 0;
    transient_owner_lock();
    for (auto* p = g_transient_owner_head; p; p = p->next)
        bytes += p->payload_bytes;
    transient_owner_unlock();
    return bytes;
}

bool drain_transient_releases(std::uint32_t timeout_ms) noexcept {
    preloader_l::ShutdownBudget budget(timeout_ms);
    for (;;) {
        if (retry_pending_transient_releases()) return true;
        if (!budget.remaining()) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

} }
