#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace runtime {
struct GrantAchievementTransaction;
struct PreparedQueuedEvent;
} }
namespace preloader_l { namespace handlers {
HandlerStatus handle_grant_achievement(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
struct GrantAchievementDelivery {
    preloader_l::runtime::GrantAchievementTransaction* transaction;
    preloader_l::runtime::PreparedQueuedEvent* event;
    bool active;
};
HandlerStatus prepare_grant_achievement_delivery(const preloader_l::xml::RequestView&,char*,size_t,size_t*,GrantAchievementDelivery*) noexcept;
preloader_l::runtime::PreparedQueuedEvent* take_grant_achievement_delivery_event(GrantAchievementDelivery*) noexcept;
void commit_grant_achievement_delivery_store(GrantAchievementDelivery*) noexcept;
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept;
void rollback_grant_achievement_delivery(GrantAchievementDelivery*) noexcept;
size_t preview_grant_achievement_event_size(const preloader_l::xml::RequestView&) noexcept;
HandlerStatus handle_query_achievements(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
