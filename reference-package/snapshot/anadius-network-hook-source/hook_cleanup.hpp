#pragma once
#include <cstdint>
#include <span>
#include <string_view>
namespace anadius::network_hook_recovered {
class CleanupEffects {
public:
    virtual ~CleanupEffects() = default;
    virtual std::int32_t disable_hook_046a00(std::uintptr_t) = 0;
    virtual std::int32_t uninitialize_engine_046ac0() = 0;
    virtual void log_literal_006880(std::string_view) = 0;
    virtual void log_finish_0088e0() = 0;
};
// Stable span snapshots the original begin/end. Backing storage must remain
// valid throughout calls. In-place edits to unvisited elements are observed;
// growth outside this span is not visited. No vector clear, dedup or rollback.
void cleanup_hooks_0102b0(std::span<const std::uintptr_t>, CleanupEffects&);
}
