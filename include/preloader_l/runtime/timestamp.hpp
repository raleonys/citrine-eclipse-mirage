// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/runtime/calendar.hpp"
#include "preloader_l/runtime/achievement_storage.hpp"

namespace preloader_l::timestamp {
inline constexpr std::uint32_t eof_bit=1,fail_bit=2,bad_bit=4;
inline constexpr std::string_view format="%Y-%m-%dT%H:%M:%S";
class Parser {
public:
    virtual ~Parser()=default;
    // Original explicit-length istringstream input; zero-initialized tm comes
    // from041e10. Return NATIVE bit numbering, not host std::ios constants.
    virtual std::uint32_t extract_03d310(std::string_view input,std::string_view format,
                                        calendar::BrokenDown&)=0;
};
// Fixed-format branch under explicit classic ASCII/base-facet
// assumptions. Other formats and any non-ASCII byte throw invalid_argument.
// Does not reconstruct arbitrary native locale/stream/allocator/exception ABI.
class ClassicIso final:public Parser {
public:
    std::uint32_t extract_03d310(std::string_view,std::string_view,
                                calendar::BrokenDown&) override;
};
// Explicit selected-library compatibility implementation. Does not reconstruct
// MSVC time_get facets, locale ownership, allocation/exception ABI or bad input
// behavior. Uses the caller process's current global C++ locale at construction.
class HostGetTime final:public Parser {
public:
    std::uint32_t extract_03d310(std::string_view,std::string_view,
                                calendar::BrokenDown&) override;
};
std::int64_t timestamp_to_epoch_041e10(std::string_view,Parser&);
// Supplies the previous mandatory epoch boundary to achievement Storage.
// All remaining storage filesystem/status/encoding effects remain abstract.
class StorageEffects:public achievement_storage::Effects {
public:
    explicit StorageEffects(Parser& parser):parser_(parser){}
    std::int64_t timestamp_to_epoch_041e10(std::string_view value) final;
private:
    Parser& parser_;
};
} // namespace preloader_l::timestamp
