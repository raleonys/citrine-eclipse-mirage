// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_start_download(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_set_presence(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_get_presence_visibility(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_are_chunks_installed(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_restart_game(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_get_voip_status(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_query_chunk_status(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_get_block_list(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;
HandlerStatus handle_go_online(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_enable_voip(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_set_downloader_utilization(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_set_presence_visibility(const xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
