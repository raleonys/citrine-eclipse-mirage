#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/runtime/local_social_host.hpp"
namespace preloader_l { namespace handlers {
// Socket-only local mutation staging. It never consumes a request UserId as
// authority; request actor must exactly match the bound host Config2 profile.
bool prepare_local_social_socket_request(const xml::RequestView&,
    const runtime::LocalSocialSession&,runtime::LocalSocialPending*) noexcept;
HandlerStatus render_local_social_success(const xml::RequestView&,
    char*,size_t,size_t*) noexcept;
HandlerStatus handle_block_user(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_unblock_user(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_request_friend(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_remove_friend(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_send_invite(const xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
