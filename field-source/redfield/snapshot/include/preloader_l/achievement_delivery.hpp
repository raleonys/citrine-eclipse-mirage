// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/achievement_session.hpp"
#include "preloader_l/xml.hpp"
#include <string>
namespace preloader_l {
struct GrantWireBudget {
    std::size_t response_capacity=16384; // Includes terminal NUL capacity.
    std::size_t event_capacity=16384;
    std::size_t total_bytes=32768; // Logical payload bytes, excludes NULs.
};
enum class GrantDeliveryResult {prepared,no_response,rejected,busy,not_ready,limit,resource};
class PreparedAchievementDelivery final {
public:
    static constexpr std::size_t max_frame_capacity=16384;
    PreparedAchievementDelivery() noexcept=default;
    ~PreparedAchievementDelivery()=default;
    PreparedAchievementDelivery(const PreparedAchievementDelivery&)=delete;
    PreparedAchievementDelivery& operator=(const PreparedAchievementDelivery&)=delete;
    PreparedAchievementDelivery(PreparedAchievementDelivery&&) noexcept;
    PreparedAchievementDelivery& operator=(PreparedAchievementDelivery&&) noexcept;
    bool active() const noexcept{return transaction_.active();}
    std::string_view response() const noexcept{return response_;}
    std::string_view completion_event() const noexcept{return event_;}
    std::shared_ptr<const ConfigSnapshot> projected() const noexcept{return transaction_.projected();}
    // Consumes transaction; successful state commit retains owned bytes for
    // future transport/event transfer. Does not enqueue or send either message.
    // Failed commit clears both byte strings. Copies already taken are caller-owned.
    bool commit() noexcept;
    void rollback() noexcept;
private:
    friend GrantDeliveryResult prepare_achievement_delivery(AchievementSession&,const xml::Request&,std::string_view,GrantWireBudget,PreparedAchievementDelivery&) noexcept;
    AchievementSession::PreparedGrant transaction_;
    std::string response_,event_;
};
// Preparation API. LocalService transfers the result into the TCP-owned reply;
// context-free/config-only dispatchers retain no Grant mutation binding.
// Output must be empty; any failure preserves it and rolls back local staging.
GrantDeliveryResult prepare_achievement_delivery(AchievementSession&,const xml::Request&,
    std::string_view timestamp,GrantWireBudget,PreparedAchievementDelivery&) noexcept;
}
