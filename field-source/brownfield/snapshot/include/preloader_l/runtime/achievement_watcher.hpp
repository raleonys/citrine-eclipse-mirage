#pragma once

#include <stddef.h>

namespace preloader_l { namespace runtime {

// Phase 1391 recovers an optional process-global watcher policy, but not a
// normalized transport constructor for this project. Keep that boundary exact:
// production remains null unless a host binds an already-owned sink.
//
// Audit3 S0320 reentry policy: the sink is invoked synchronously outside the
// behavioral-state lock. Read-only achievement API reentry is supported while
// the outer GrantAchievement delivery reservation remains authoritative.
// Recursive GrantAchievement mutation is rejected by the route on the callback
// owner thread so the prepared outer transaction cannot be invalidated. The
// callback owner is tracked without static C++ TLS; unrelated threads do not
// borrow the outer reservation.
using AchievementWatcherSink = bool (*)(const char* xml,size_t length) noexcept;

void bind_achievement_watcher_sink(AchievementWatcherSink sink) noexcept;
bool achievement_watcher_active() noexcept;
bool achievement_watcher_callback_active() noexcept;
bool publish_achievement_watcher_snapshot(const char* xml,size_t length) noexcept;
void clear_achievement_watcher_sink_for_test() noexcept;

} }
