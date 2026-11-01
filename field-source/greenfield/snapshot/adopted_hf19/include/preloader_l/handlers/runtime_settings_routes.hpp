#pragma once
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

// Runtime-bound implementations of the curio GetSetting/GetSettings paths.
// These use the evidence-backed SettingId selector and fallback values from the
// pre_surge_v4 recovery rather than inventing a new request ABI.
HandlerStatus handle_runtime_get_setting(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus handle_runtime_get_settings(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
