#include "preloader_l/handlers/connection_state_binding.hpp"
#include "preloader_l/handlers/connection_state_route.hpp"
#include "preloader_l/config/runtime_provider.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_runtime_internet_state(const preloader_l::xml::RequestView& r,char*o,size_t c,size_t*w) noexcept{return handle_get_internet_connected_state(r,preloader_l::config::runtime_settings_provider(),o,c,w);}
} }
