#pragma once

#include "preloader_l/protocol/origin_error.hpp"

namespace preloader_l { namespace handlers { namespace route_error_contracts {

struct ErrorSuccessContract {
    preloader_l::protocol::OriginError code;
    const char* description;
};

// Route-local wire descriptions. Numeric meaning/classification comes from the
// Rust protocol oracle; description text is retained only where the route's
// current/Donor1 evidence directly supports it. Do not infer a universal text
// from OriginError alone.
constexpr ErrorSuccessContract show_igo_success{
    preloader_l::protocol::OriginError::success, ""};
constexpr ErrorSuccessContract set_presence_success{
    preloader_l::protocol::OriginError::success, ""};
constexpr ErrorSuccessContract start_download_success{
    preloader_l::protocol::OriginError::success,
    "Requested a download of a DLC offer."};
constexpr ErrorSuccessContract restart_game_pending{
    preloader_l::protocol::OriginError::pending,
    "Origin is waiting for the game to shutdown."};
constexpr ErrorSuccessContract get_voip_status_not_implemented{
    preloader_l::protocol::OriginError::not_implemented,
    "The function you are calling is not implemented."};
constexpr ErrorSuccessContract get_auth_code_offline{
    preloader_l::protocol::OriginError::no_service,
    "You need to be online to get an AuthCode."};
constexpr ErrorSuccessContract check_permission_offline{
    preloader_l::protocol::OriginError::proxy_bad_request, ""};

} } } // namespace preloader_l::handlers::route_error_contracts
