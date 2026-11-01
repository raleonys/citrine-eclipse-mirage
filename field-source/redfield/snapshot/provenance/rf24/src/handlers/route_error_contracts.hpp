// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "redfield/protocol/origin_error.hpp"

namespace redfield { namespace handlers { namespace route_error_contracts {

struct ErrorSuccessContract {
    redfield::protocol::OriginError code;
    const char* description;
};

// Route-local wire descriptions. Numeric meaning/classification comes from the
// Rust protocol oracle; description text is retained only where the route's
// current/Donor1 evidence directly supports it. Do not infer a universal text
// from OriginError alone.
constexpr ErrorSuccessContract show_igo_success{
    redfield::protocol::OriginError::success, ""};
constexpr ErrorSuccessContract set_presence_success{
    redfield::protocol::OriginError::success, ""};
constexpr ErrorSuccessContract start_download_success{
    redfield::protocol::OriginError::success,
    "Requested a download of a DLC offer."};
constexpr ErrorSuccessContract restart_game_pending{
    redfield::protocol::OriginError::pending,
    "Origin is waiting for the game to shutdown."};
constexpr ErrorSuccessContract get_voip_status_not_implemented{
    redfield::protocol::OriginError::not_implemented,
    "The function you are calling is not implemented."};
constexpr ErrorSuccessContract get_auth_code_offline{
    redfield::protocol::OriginError::no_service,
    "You need to be online to get an AuthCode."};
constexpr ErrorSuccessContract check_permission_offline{
    redfield::protocol::OriginError::proxy_bad_request, ""};

} } } // namespace redfield::handlers::route_error_contracts
