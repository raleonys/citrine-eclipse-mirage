#include "preloader_l/handlers/handler_registry.hpp"
#include "preloader_l/router/route_inventory.hpp"
using namespace preloader_l;
int main(){
 size_t bound=0;
 const auto* routes=router::route_inventory();
 for(size_t i=0;i<router::route_count;++i){
   const auto id=router::descriptor_route_id(routes[i]);
   if(!handlers::route_has_handler_boundary(id)) return 1;
   if(handlers::find_bound_handler(id)==nullptr) return 2;
   ++bound;
 }
 if(bound!=router::route_count || bound!=21) return 3;
 if(!handlers::route_has_handler_boundary(router::RouteId::show_igo_window)) return 4;
 if(!handlers::route_has_handler_boundary(router::RouteId::block_user)) return 5;
 return 0;
}
