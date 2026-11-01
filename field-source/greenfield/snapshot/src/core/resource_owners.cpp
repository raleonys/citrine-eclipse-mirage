#include "greenfield/core/resource_owners.hpp"
#include <mutex>

namespace greenfield::core {
bool ResourceOwners::adopt(std::uintptr_t ticket, Cleanup cleanup, void* context) noexcept {
    if (!ticket || !cleanup) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (in_flight_ || size_ == capacity) return false;
    for (std::size_t i = 0; i < size_; ++i)
        if (entries_[i].ticket == ticket) return false;
    entries_[size_++] = Entry{ticket,cleanup,context};
    return true;
}
ResourceOwners::Drain ResourceOwners::drain() noexcept {
    std::unique_lock<std::mutex> lock(mutex_);
    if (in_flight_) return Drain::busy;
    in_flight_ = true;
    // Remove in reverse install order, but do not drop failed owners.
    for (std::size_t i = size_; i != 0; --i) {
        const Entry next = entries_[i-1];
        if (!next.cleanup) continue;
        lock.unlock();
        const bool closed = next.cleanup(next.context,next.ticket);
        lock.lock();
        if (closed) entries_[i-1].cleanup = nullptr;
    }
    std::size_t live=0;
    for (std::size_t i=0;i<size_;++i)
        if (entries_[i].cleanup) entries_[live++]=entries_[i];
    for (std::size_t i=live;i<size_;++i) entries_[i]={};
    size_=live;
    in_flight_=false;
    return size_ == 0 ? Drain::complete : Drain::retry_required;
}
std::size_t ResourceOwners::remaining() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return size_;
}
}
