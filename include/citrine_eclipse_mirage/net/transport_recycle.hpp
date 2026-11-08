// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_cancellation.hpp"

namespace citrine_eclipse_mirage::socket {
inline constexpr std::uint64_t recycle_limit=0x0666666666666666ULL;
struct RecycleMap {
    RecycleMap(CancelMap& map,CancelNode& free_sentinel):map(map),free_sentinel(free_sentinel){}
    CancelMap& map;
    CancelNode& free_sentinel;
    std::uint64_t map_size=0,active_size=0,free_size=0;
};
class RecycleFailureEffects {
public:
    virtual ~RecycleFailureEffects()=default;
    // Native047fc4/CRT exception construction remains required. Opaque image
    // identity is observed metadata, never an executable function pointer.
    [[noreturn]] virtual void length_error_047fc4(std::uintptr_t message_identity)=0;
};
// Caller serializes access and supplies initialized circular lists/buckets.
// The two list subobjects are distinct; no self-splice shortcut is invented.
void recycle_0283c0(RecycleMap&,CancelNode&,RecycleFailureEffects&);
class CancellationRecyclerAdapter final:public CancellationRecycler {
public:
    CancellationRecyclerAdapter(std::span<RecycleMap*> maps,RecycleFailureEffects& failures)
        :maps_(maps),failures_(failures){}
    void recycle_0283c0(CancelMap&,CancelNode&) override;
private:std::span<RecycleMap*> maps_;RecycleFailureEffects& failures_;
};
} // namespace citrine_eclipse_mirage::socket
