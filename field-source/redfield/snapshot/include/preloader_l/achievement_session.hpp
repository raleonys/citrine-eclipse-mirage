// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include <memory>
#include <string_view>
namespace preloader_l {
// LocalService-owned Grant state; the TCP reply retains each pending ticket.
// Caller must supply an immutable ConfigStore publication when initializing.
class AchievementSession final {
    struct Control;
    struct Pending;
public:
    static constexpr std::size_t max_records=ConfigStore::max_leaves;
    static constexpr std::size_t max_text_bytes=4*ConfigStore::max_bytes;
    enum class InitializeResult {initialized,already_initialized,closed,invalid,limit};
    enum class PrepareResult {prepared,not_initialized,closed,busy,invalid,limit,resource};
    class PreparedGrant final {
    public:
        PreparedGrant() noexcept=default;
        ~PreparedGrant();
        PreparedGrant(const PreparedGrant&)=delete;
        PreparedGrant& operator=(const PreparedGrant&)=delete;
        PreparedGrant(PreparedGrant&&) noexcept;
        PreparedGrant& operator=(PreparedGrant&&) noexcept;
        // Reports a held handle. Owner close can invalidate its commit ability.
        bool active() const noexcept;
        bool mutated() const noexcept;
        bool completed() const noexcept;
        bool inserted() const noexcept;
        std::size_t index() const noexcept;
        // Owned immutable candidate; a pin survives commit/rollback/owner close.
        std::shared_ptr<const ConfigSnapshot> projected() const noexcept;
        // Consumes reservation. Caller chooses the eventual delivery boundary.
        // No allocation, response formatting, event publication or I/O here.
        bool commit() noexcept;
        void rollback() noexcept;
    private:
        friend class AchievementSession;
        std::shared_ptr<Pending> pending_;
    };
    AchievementSession();
    ~AchievementSession();
    AchievementSession(const AchievementSession&)=delete;
    AchievementSession& operator=(const AchievementSession&)=delete;
    AchievementSession(AchievementSession&&)=delete;
    AchievementSession& operator=(AchievementSession&&)=delete;
    InitializeResult initialize(std::shared_ptr<const ConfigSnapshot>);
    std::shared_ptr<const ConfigSnapshot> snapshot() const;
    PrepareResult prepare(std::string_view id,std::int32_t increment,
                          std::string_view timestamp,PreparedGrant&) noexcept;
    // Terminal and idempotent; cancels outstanding reservations and drops state.
    void close();
private:
    std::shared_ptr<Control> control_;
};
}
