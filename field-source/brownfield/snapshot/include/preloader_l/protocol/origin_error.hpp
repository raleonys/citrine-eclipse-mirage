#pragma once

#include <stdint.h>

namespace preloader_l { namespace protocol {

enum class OriginErrorArea : int32_t {
    general = 0,
    sdk = 1 << 16,
    core = 2 << 16,
    igo = 3 << 16,
    friends = 4 << 16,
    presence = 5 << 16,
    commerce = 6 << 16,
    achievements = 7 << 16,
    lsx = 8 << 16,
    proxy = 9 << 16,
};

enum class OriginErrorLevel : int32_t {
    l0 = 0 << 24,
    l1 = 1 << 24,
    l2 = 2 << 24,
    l3 = 3 << 24,
    l4 = 4 << 24,
};

constexpr int32_t make_origin_error(OriginErrorLevel level, OriginErrorArea area, int32_t code) noexcept
{
    return static_cast<int32_t>(
        UINT32_C(0xA0000000) |
        static_cast<uint32_t>(static_cast<int32_t>(level)) |
        static_cast<uint32_t>(static_cast<int32_t>(area)) |
        static_cast<uint32_t>(code));
}

constexpr int32_t make_origin_warning(OriginErrorLevel level, OriginErrorArea area, int32_t code) noexcept
{
    return static_cast<int32_t>(
        UINT32_C(0x40000000) |
        static_cast<uint32_t>(static_cast<int32_t>(level)) |
        static_cast<uint32_t>(static_cast<int32_t>(area)) |
        static_cast<uint32_t>(code));
}

enum class OriginError : int32_t {
    success = 0,
    pending = 1,
    general = -1,

    invalid_handle = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::general, 0),
    out_of_memory = make_origin_error(OriginErrorLevel::l0, OriginErrorArea::general, 1),
    not_implemented = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::general, 2),
    invalid_user = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 3),
    invalid_argument = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 4),
    no_callback_specified = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 5),
    buffer_too_small = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 6),
    too_many_values_in_list = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 7),
    not_found = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 8),
    invalid_persona = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 9),
    no_network = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 10),
    no_service = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 11),
    not_logged_in = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 12),
    mandatory_origin_update_pending = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 13),
    account_in_use = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 14),
    too_many_instances = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::general, 15),
    already_exists = make_origin_error(OriginErrorLevel::l3, OriginErrorArea::general, 16),
    invalid_operation = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 17),
    age_restricted = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 18),
    banned = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 19),
    not_ready = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::general, 20),

    sdk_not_initialized = make_origin_error(OriginErrorLevel::l0, OriginErrorArea::sdk, 0),
    sdk_invalid_allocator_deallocator_combination = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::sdk, 1),
    sdk_is_running = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::sdk, 2),
    sdk_not_all_resources_released = make_origin_error(OriginErrorLevel::l3, OriginErrorArea::sdk, 3),
    sdk_invalid_resource = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::sdk, 4),
    sdk_internal_error = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::sdk, 5),
    sdk_internal_buffer_too_small = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::sdk, 6),

    warning_sdk_already_initialized = make_origin_warning(OriginErrorLevel::l2, OriginErrorArea::sdk, 1),
    warning_sdk_still_running = make_origin_warning(OriginErrorLevel::l2, OriginErrorArea::sdk, 2),
    warning_sdk_enumerator_in_use = make_origin_warning(OriginErrorLevel::l2, OriginErrorArea::sdk, 3),
    warning_sdk_enumerator_terminated = make_origin_warning(OriginErrorLevel::l2, OriginErrorArea::sdk, 4),

    core_not_loaded = make_origin_error(OriginErrorLevel::l0, OriginErrorArea::core, 0),
    core_login_failed = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::core, 1),
    core_authentication_failed = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::core, 2),
    core_send_failed = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::core, 4),
    core_receive_failed = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::core, 5),
    core_resource_not_found = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::core, 6),
    core_incompatible_version = make_origin_error(OriginErrorLevel::l0, OriginErrorArea::core, 7),
    core_not_installed = make_origin_error(OriginErrorLevel::l0, OriginErrorArea::core, 8),

    warning_igo_not_loaded = make_origin_warning(OriginErrorLevel::l1, OriginErrorArea::igo, 0),
    warning_igo_support_not_loaded = make_origin_warning(OriginErrorLevel::l1, OriginErrorArea::igo, 1),
    igo_illegal_anchor_point = make_origin_error(OriginErrorLevel::l3, OriginErrorArea::igo, 2),
    igo_illegal_dock_point = make_origin_error(OriginErrorLevel::l3, OriginErrorArea::igo, 3),
    igo_not_available = make_origin_error(OriginErrorLevel::l3, OriginErrorArea::igo, 4),

    no_multiplayer_id = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::presence, 0),

    lsx_invalid_response = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::lsx, 0),
    lsx_no_response = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::lsx, 1),
    lsx_invalid_request = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::lsx, 2),

    commerce_no_such_store = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::commerce, 0),
    commerce_no_such_catalog = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::commerce, 1),
    commerce_invalid_reply = make_origin_error(OriginErrorLevel::l1, OriginErrorArea::commerce, 2),
    commerce_no_categories = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::commerce, 3),
    commerce_no_products = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::commerce, 4),
    commerce_underage_user = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::commerce, 5),
    commerce_deprecated_store = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::commerce, 6),

    proxy = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 0),
    success_proxy_ok = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 200),
    success_proxy_created = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 201),
    success_proxy_accepted = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 202),
    success_proxy_non_auth_info = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 203),
    success_proxy_no_content = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 204),
    success_reset_content = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 205),
    success_partial_content = make_origin_warning(OriginErrorLevel::l4, OriginErrorArea::proxy, 206),
    proxy_bad_request = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 400),
    proxy_unauthorized = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 401),
    proxy_payment_required = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 402),
    proxy_forbidden = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 403),
    proxy_not_found = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 404),
    proxy_method_not_allowed = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 405),
    proxy_not_acceptable = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 406),
    proxy_request_timeout = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 408),
    proxy_conflict = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 409),
    proxy_internal_error = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 500),
    proxy_not_implemented = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 501),
    proxy_bad_gateway = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 502),
    proxy_service_unavailable = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 503),
    proxy_gateway_timeout = make_origin_error(OriginErrorLevel::l2, OriginErrorArea::proxy, 504),
};

constexpr int32_t origin_error_value(OriginError error) noexcept
{
    return static_cast<int32_t>(error);
}

constexpr bool origin_error_is_success(OriginError error) noexcept
{
    return error == OriginError::success || error == OriginError::pending;
}

constexpr bool origin_error_is_warning(OriginError error) noexcept
{
    return origin_error_value(error) > 1;
}

constexpr bool origin_error_is_error(OriginError error) noexcept
{
    return origin_error_value(error) < 0;
}

constexpr int32_t origin_error_area_bits(OriginError error) noexcept
{
    return origin_error_value(error) & 0x00FF0000;
}

constexpr int32_t origin_error_level_bits(OriginError error) noexcept
{
    return origin_error_value(error) & 0x0F000000;
}

constexpr int32_t origin_error_local_code(OriginError error) noexcept
{
    return origin_error_value(error) & 0x0000FFFF;
}

constexpr bool origin_error_is_pending(OriginError error) noexcept
{
    return error == OriginError::pending;
}

constexpr OriginErrorArea origin_error_area(OriginError error) noexcept
{
    const int32_t bits = origin_error_area_bits(error);
    switch (bits) {
        case static_cast<int32_t>(OriginErrorArea::sdk): return OriginErrorArea::sdk;
        case static_cast<int32_t>(OriginErrorArea::core): return OriginErrorArea::core;
        case static_cast<int32_t>(OriginErrorArea::igo): return OriginErrorArea::igo;
        case static_cast<int32_t>(OriginErrorArea::friends): return OriginErrorArea::friends;
        case static_cast<int32_t>(OriginErrorArea::presence): return OriginErrorArea::presence;
        case static_cast<int32_t>(OriginErrorArea::commerce): return OriginErrorArea::commerce;
        case static_cast<int32_t>(OriginErrorArea::achievements): return OriginErrorArea::achievements;
        case static_cast<int32_t>(OriginErrorArea::lsx): return OriginErrorArea::lsx;
        case static_cast<int32_t>(OriginErrorArea::proxy): return OriginErrorArea::proxy;
        default: return OriginErrorArea::general;
    }
}

constexpr OriginErrorLevel origin_error_level(OriginError error) noexcept
{
    const int32_t bits = origin_error_level_bits(error);
    switch (bits) {
        case static_cast<int32_t>(OriginErrorLevel::l1): return OriginErrorLevel::l1;
        case static_cast<int32_t>(OriginErrorLevel::l2): return OriginErrorLevel::l2;
        case static_cast<int32_t>(OriginErrorLevel::l3): return OriginErrorLevel::l3;
        case static_cast<int32_t>(OriginErrorLevel::l4): return OriginErrorLevel::l4;
        default: return OriginErrorLevel::l0;
    }
}

constexpr const char* origin_error_name(OriginError error) noexcept
{
    switch (error) {
        case OriginError::success: return "Success";
        case OriginError::pending: return "Pending";
        case OriginError::general: return "General";
        case OriginError::invalid_handle: return "InvalidHandle";
        case OriginError::out_of_memory: return "OutOfMemory";
        case OriginError::not_implemented: return "NotImplemented";
        case OriginError::invalid_user: return "InvalidUser";
        case OriginError::invalid_argument: return "InvalidArgument";
        case OriginError::no_callback_specified: return "NoCallbackSpecified";
        case OriginError::buffer_too_small: return "BufferTooSmall";
        case OriginError::too_many_values_in_list: return "TooManyValuesInList";
        case OriginError::not_found: return "NotFound";
        case OriginError::invalid_persona: return "InvalidPersona";
        case OriginError::no_network: return "NoNetwork";
        case OriginError::no_service: return "NoService";
        case OriginError::not_logged_in: return "NotLoggedIn";
        case OriginError::mandatory_origin_update_pending: return "MandatoryOriginUpdatePending";
        case OriginError::account_in_use: return "AccountInUse";
        case OriginError::too_many_instances: return "TooManyInstances";
        case OriginError::already_exists: return "AlreadyExists";
        case OriginError::invalid_operation: return "InvalidOperation";
        case OriginError::age_restricted: return "AgeRestricted";
        case OriginError::banned: return "Banned";
        case OriginError::not_ready: return "NotReady";
        case OriginError::sdk_not_initialized: return "SdkNotInitialized";
        case OriginError::sdk_invalid_allocator_deallocator_combination: return "SdkInvalidAllocatorDeallocatorCombination";
        case OriginError::sdk_is_running: return "SdkIsRunning";
        case OriginError::sdk_not_all_resources_released: return "SdkNotAllResourcesReleased";
        case OriginError::sdk_invalid_resource: return "SdkInvalidResource";
        case OriginError::sdk_internal_error: return "SdkInternalError";
        case OriginError::sdk_internal_buffer_too_small: return "SdkInternalBufferTooSmall";
        case OriginError::warning_sdk_already_initialized: return "WarningSdkAlreadyInitialized";
        case OriginError::warning_sdk_still_running: return "WarningSdkStillRunning";
        case OriginError::warning_sdk_enumerator_in_use: return "WarningSdkEnumeratorInUse";
        case OriginError::warning_sdk_enumerator_terminated: return "WarningSdkEnumeratorTerminated";
        case OriginError::core_not_loaded: return "CoreNotLoaded";
        case OriginError::core_login_failed: return "CoreLoginFailed";
        case OriginError::core_authentication_failed: return "CoreAuthenticationFailed";
        case OriginError::core_send_failed: return "CoreSendFailed";
        case OriginError::core_receive_failed: return "CoreReceiveFailed";
        case OriginError::core_resource_not_found: return "CoreResourceNotFound";
        case OriginError::core_incompatible_version: return "CoreIncompatibleVersion";
        case OriginError::core_not_installed: return "CoreNotInstalled";
        case OriginError::warning_igo_not_loaded: return "WarningIgoNotLoaded";
        case OriginError::warning_igo_support_not_loaded: return "WarningIgoSupportNotLoaded";
        case OriginError::igo_illegal_anchor_point: return "IgoIllegalAnchorPoint";
        case OriginError::igo_illegal_dock_point: return "IgoIllegalDockPoint";
        case OriginError::igo_not_available: return "IgoNotAvailable";
        case OriginError::no_multiplayer_id: return "NoMultiplayerId";
        case OriginError::lsx_invalid_response: return "LsxInvalidResponse";
        case OriginError::lsx_no_response: return "LsxNoResponse";
        case OriginError::lsx_invalid_request: return "LsxInvalidRequest";
        case OriginError::commerce_no_such_store: return "CommerceNoSuchStore";
        case OriginError::commerce_no_such_catalog: return "CommerceNoSuchCatalog";
        case OriginError::commerce_invalid_reply: return "CommerceInvalidReply";
        case OriginError::commerce_no_categories: return "CommerceNoCategories";
        case OriginError::commerce_no_products: return "CommerceNoProducts";
        case OriginError::commerce_underage_user: return "CommerceUnderageUser";
        case OriginError::commerce_deprecated_store: return "CommerceDeprecatedStore";
        case OriginError::proxy: return "Proxy";
        case OriginError::success_proxy_ok: return "SuccessProxyOk";
        case OriginError::success_proxy_created: return "SuccessProxyCreated";
        case OriginError::success_proxy_accepted: return "SuccessProxyAccepted";
        case OriginError::success_proxy_non_auth_info: return "SuccessProxyNonAuthInfo";
        case OriginError::success_proxy_no_content: return "SuccessProxyNoContent";
        case OriginError::success_reset_content: return "SuccessResetContent";
        case OriginError::success_partial_content: return "SuccessPartialContent";
        case OriginError::proxy_bad_request: return "ProxyBadRequest";
        case OriginError::proxy_unauthorized: return "ProxyUnauthorized";
        case OriginError::proxy_payment_required: return "ProxyPaymentRequired";
        case OriginError::proxy_forbidden: return "ProxyForbidden";
        case OriginError::proxy_not_found: return "ProxyNotFound";
        case OriginError::proxy_method_not_allowed: return "ProxyMethodNotAllowed";
        case OriginError::proxy_not_acceptable: return "ProxyNotAcceptable";
        case OriginError::proxy_request_timeout: return "ProxyRequestTimeout";
        case OriginError::proxy_conflict: return "ProxyConflict";
        case OriginError::proxy_internal_error: return "ProxyInternalError";
        case OriginError::proxy_not_implemented: return "ProxyNotImplemented";
        case OriginError::proxy_bad_gateway: return "ProxyBadGateway";
        case OriginError::proxy_service_unavailable: return "ProxyServiceUnavailable";
        case OriginError::proxy_gateway_timeout: return "ProxyGatewayTimeout";
    }
    return "UnknownOriginError";
}

static_assert(origin_error_value(OriginError::not_implemented) == -1593835518, "OriginError::not_implemented drift");
static_assert(origin_error_value(OriginError::proxy_bad_request) == -1576468080, "OriginError::proxy_bad_request drift");
static_assert(origin_error_value(OriginError::no_service) == -1577058293, "OriginError::no_service drift");

} } // namespace preloader_l::protocol
