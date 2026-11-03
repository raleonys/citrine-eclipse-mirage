// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/handlers/handlers.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace preloader_l::misc_handlers {
using handlers::Request;
using handlers::Xml;
using handlers::Attributes;
// Extra operations observed in handlers with request child elements. A missing
// text node stays nullptr; callers must not silently synthesize an empty value.
class Elements {
public:
    virtual ~Elements() = default;
    virtual Request first_child_element_003300(Request) = 0;
    virtual Request next_sibling_element_003fe0(Request) = 0;
    virtual const char* get_text_003860(Request) = 0;
    virtual void push_text_0056b0(const char*,bool cdata) = 0;
};
class IgoEffects {
public:
    virtual ~IgoEffects() = default;
    virtual int lock_02c620_04a6ac() = 0;
    virtual std::int32_t& lock_count_4c() = 0;
    // Selects/initializes the event deque. Its first initialization already
    // contains one DOWN event, separately from the event appended below.
    virtual void acquire_event_queue_02c400() = 0;
    virtual void enqueue_034660(std::string&&) = 0;
    virtual void unlock_04a6b4() = 0;
    [[noreturn]] virtual void thread_error_04a648(int) = 0;
};
class NoResponse {
public:
    virtual ~NoResponse() = default;
    // Typed protocol exception, not an XML error and not a boolean response.
    [[noreturn]] virtual void throw_no_response_02bcd0_04d228() = 0;
};
bool success_02ffb0(Xml&);
bool presence_visibility_02f4e0(Xml&);
bool presence_02f330(Xml&,Attributes& game_0a3d88);
bool query_presence_031ef0(Request,Xml&,Elements&,const std::string& username_0a3bd8);
bool group_info_02f270(Xml&);
bool progressive_available_030cd0(Xml&);
[[noreturn]] void query_chunk_status_030eb0(NoResponse&);
bool chunks_installed_02d720(Request,Xml&,Elements&);
bool query_friends_0314a0(Xml&);
bool block_list_02e5a0(Xml&);
bool voip_02da30(Xml&);
bool wallet_02ff60(Xml&);
bool restart_game_032ab0(Xml&);
bool permission_02d9e0(Xml&);
bool utc_time_02fea0(Xml&,handlers::Effects&);
bool show_igo_032b10(Xml&,IgoEffects&);
bool start_download_032ca0(Xml&);
bool query_offers_031ec0(Xml&);
} // namespace preloader_l::misc_handlers
