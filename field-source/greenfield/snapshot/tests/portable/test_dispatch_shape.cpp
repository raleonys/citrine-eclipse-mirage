// GF05 compile-time ABI boundary for ORIGINAL HF19 dispatcher types. This does not
// invoke dispatch or link handler implementations; it cannot claim live LSX.
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/handlers/handler_registry.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include <cstdint>
#include <cstdio>
#include <type_traits>
using namespace preloader_l;
static_assert(std::is_same_v<decltype(core::DispatchResult{}.written), size_t>);
static_assert(std::is_same_v<handlers::HandlerFn,
    handlers::HandlerStatus (*)(const xml::RequestView&,char*,size_t,size_t*) noexcept>);
static_assert(static_cast<uint8_t>(core::DispatchStatus::ok)==0u);
static_assert(static_cast<uint8_t>(core::DispatchStatus::capacity)==9u);
static_assert(static_cast<uint16_t>(router::RouteId::query_entitlements)==7u);
static_assert(static_cast<uint16_t>(router::RouteId::send_invite)==42u);
static_assert(router::route_count==21u);
static_assert(router::compatibility_route_count==6u);
int main(){
    core::DispatchEnvironment env{};
    if(env.defer_event_delivery || env.local_social_session!=nullptr)return 1;
    core::DispatchResult result{};
    if(result.status!=core::DispatchStatus::ok)return 2;
    if(result.written!=0u)return 3;
    std::puts("GF05_REAL_HF19_DISPATCH_TYPE_BOUNDARY_PASS_NOT_LSX");
    return 0;
}
