#include "preloader_l/handlers/achievement_routes.hpp"
#include "achievement_routes_internal.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus handle_grant_achievement(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    return detail::handle_grant_achievement(r,out,cap,written);
}
HandlerStatus prepare_grant_achievement_delivery(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written,GrantAchievementDelivery* delivery) noexcept {
    return detail::prepare_grant_achievement_delivery(r,out,cap,written,delivery);
}
preloader_l::runtime::PreparedQueuedEvent* take_grant_achievement_delivery_event(GrantAchievementDelivery* delivery) noexcept {
    return detail::take_grant_achievement_delivery_event(delivery);
}
void commit_grant_achievement_delivery_store(GrantAchievementDelivery* delivery) noexcept {
    detail::commit_grant_achievement_delivery_store(delivery);
}
void finish_grant_achievement_delivery(GrantAchievementDelivery* delivery) noexcept {
    detail::finish_grant_achievement_delivery(delivery);
}
void rollback_grant_achievement_delivery(GrantAchievementDelivery* delivery) noexcept {
    detail::rollback_grant_achievement_delivery(delivery);
}
size_t preview_grant_achievement_event_size(const preloader_l::xml::RequestView& r) noexcept {
    return detail::preview_grant_achievement_event_size(r);
}
HandlerStatus handle_query_achievements(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    return detail::handle_query_achievements(r,out,cap,written);
}
} }
