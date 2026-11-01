// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/response_dispatch.hpp"
#include <array>
#include <cstdint>
#include <mutex>
namespace redfield {
// One LocalService owns one token across its peers. The render call holds the
// mutex through generation, complete formatting and commit. No DOM/view escapes.
// Successful render commits; socket delivery is not a commit/ack boundary.
class AuthCodeSession final {
public:
    using SeedSource=bool(*)(void*,std::uint64_t&) noexcept;
    explicit AuthCodeSession(SeedSource source=nullptr,void* context=nullptr) noexcept:source_(source),context_(context){}
    ResponseResult respond(const xml::Request&,const ConfigSnapshot&,char*,std::size_t,std::size_t*) noexcept;
    bool ready() const;
    // Terminal, idempotent. LocalService calls after transport drain; a closed
    // owner can never generate or return an Auth8, even with retained snapshots.
    void close();
private:
    mutable std::mutex mutex_;
    SeedSource source_;
    void* context_;
    std::array<char,9> value_{};
    bool ready_=false;
    bool closed_=false;
};
}
