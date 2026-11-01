#pragma once
#include "preloader_l/handlers/achievement_routes.hpp"

namespace preloader_l { namespace handlers { namespace detail {
HandlerStatus handle_grant_achievement(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus prepare_grant_achievement_delivery(const preloader_l::xml::RequestView&,char*,size_t,size_t*,GrantAchievementDelivery*) noexcept;
preloader_l::runtime::PreparedQueuedEvent* take_grant_achievement_delivery_event(GrantAchievementDelivery*) noexcept;
void commit_grant_achievement_delivery_store(GrantAchievementDelivery*) noexcept;
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept;
void rollback_grant_achievement_delivery(GrantAchievementDelivery*) noexcept;
size_t preview_grant_achievement_event_size(const preloader_l::xml::RequestView&) noexcept;
HandlerStatus handle_query_achievements(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
} } }
