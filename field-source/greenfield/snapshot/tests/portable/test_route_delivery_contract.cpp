// Independent GF04 integration test: exact HF19 route inventory and completion FSM.
// Neither a fabricated LSX handler nor a donor-native test.
#include "preloader_l/router/route_inventory.hpp"
#include "socket_exchange_write_completion.hpp"
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
namespace r=preloader_l::router;
namespace p=preloader_l::protocol::detail;
static int checks=0;
#define CHECK(x) do{ ++checks;if(!(x)){std::fprintf(stderr,"FAILED %s:%d %s\n",__FILE__,__LINE__,#x);std::exit(2);} }while(false)
static const r::RouteDescriptor* select(const char* text){return r::find_known_route({text,std::strlen(text)});}
int main(){
    struct Item{const char* name; r::RouteId id;};
    constexpr Item items[]={
     {"GetConfig",r::RouteId::get_config},
     {"QueryEntitlements",r::RouteId::query_entitlements},
     {"QueryContent",r::RouteId::query_content},
     {"GetProfile",r::RouteId::get_profile},
     {"ShowIGOWindow",r::RouteId::show_igo_window},
     {"BlockUser",r::RouteId::block_user},
     {"SendInvite",r::RouteId::send_invite},
     {"GetUTCtime",r::RouteId::get_utc_time} // intentionally wrong case: reject
    };
    for(size_t i=0;i<sizeof(items)/sizeof(items[0]);++i){
      auto* a=select(items[i].name);
      if(i==7) CHECK(a==nullptr);else CHECK(a && r::descriptor_route_id(*a)==items[i].id);
    }
    CHECK(select("QueryEntitlementsX")==nullptr);
    CHECK(select("<LSX>")==nullptr);
    CHECK(r::find_known_route({nullptr,4})==nullptr);
    CHECK(select("queryentitlements")==nullptr);
    p::DeliveryWriteCursor d{};
    CHECK(!p::begin_delivery_writes(nullptr,7,0));
    CHECK(!p::begin_delivery_writes(&d,0,5));
    CHECK(!p::begin_delivery_writes(&d,SIZE_MAX,5));
    CHECK(p::begin_delivery_writes(&d,8,5));
    CHECK(p::complete_delivery_write(&d,true,8)==p::DeliveryWriteDecision::event_needed);
    CHECK(p::complete_delivery_write(&d,true,5)==p::DeliveryWriteDecision::transaction_done);
    CHECK(p::complete_delivery_write(&d,true,5)==p::DeliveryWriteDecision::reject);
    CHECK(p::begin_delivery_writes(&d,8,0));
    CHECK(p::complete_delivery_write(&d,true,7)==p::DeliveryWriteDecision::reject);
    CHECK(d.phase==p::DeliveryWritePhase::failed);
    CHECK(p::complete_delivery_write(&d,true,8)==p::DeliveryWriteDecision::reject);
    CHECK(p::begin_delivery_writes(&d,8,5));
    CHECK(p::complete_delivery_write(&d,false,8)==p::DeliveryWriteDecision::reject);
    CHECK(p::begin_delivery_writes(&d,8,5));
    CHECK(p::complete_delivery_write(&d,true,8)==p::DeliveryWriteDecision::event_needed);
    CHECK(p::complete_delivery_write(&d,true,4)==p::DeliveryWriteDecision::reject);
    CHECK(p::begin_delivery_writes(&d,8,0));
    CHECK(p::complete_delivery_write(&d,true,8)==p::DeliveryWriteDecision::response_done);
    CHECK(p::complete_delivery_write(&d,true,8)==p::DeliveryWriteDecision::reject);
    std::printf("GF04_ROUTE_DELIVERY_CONTRACT_PASS checks=%d\n",checks);
}
