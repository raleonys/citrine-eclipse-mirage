#include "preloader_l/handlers/group_info_route.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "error_success_support.hpp"
#include "route_response_support.hpp"

namespace preloader_l { namespace handlers {
namespace {
HandlerStatus simple_route_result(const xml::RequestView& r,const char* name, protocol::OriginError code,const char* description,char* out,size_t cap,size_t* written) noexcept {
 if(!xml::span_equals(r.command,name)){if(written)*written=0;return HandlerStatus::wrong_command;}
 return detail::render_error_success(r,code,description,out,cap,written);
}
}

HandlerStatus handle_get_group_info(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
 // Donor fixed signed error; use explicit cast rather than inventing OriginError symbol.
 return simple_route_result(r,"GetGroupInfo",static_cast<protocol::OriginError>(-1577058296),"User not in a party, create one.",o,c,w);
}
} }
