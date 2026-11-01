#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool parse(const char* s, xml::RequestView* out){return xml::parse_request_view(s,std::strlen(s),out)==xml::RequestStatus::ok;}
static bool has(const char* s,const char* n){return s&&std::strstr(s,n)!=nullptr;}
int main(){
  config::clear_runtime_configuration();
  constexpr char cfg[]=R"CFG("Config2" { "Entitlements" {
    "OFFER_A" { "Group" "G1" "Version" "11" "Type" "DEFAULT" "EntitlementTag" "ONLINE_ACCESS" }
    "OFFER_B" { "Group" "G2" "Version" "12" "Type" "DLC" "EntitlementTag" "ITEM_B" }
  } })CFG";
  if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok)return 1;
  char out[8192]{};size_t n=0;xml::RequestView r{};
  if(!parse("<LSX><Request id=\"1\"><QueryEntitlements><FilterOffers>OFFER&#95;B</FilterOffers></QueryEntitlements></Request></LSX>",&r))return 2;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 3;
  if(has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\""))return 4;
  if(!parse("<LSX><Request id=\"2\"><QueryEntitlements><FilterGroups>G1</FilterGroups></QueryEntitlements></Request></LSX>",&r))return 5;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 6;
  if(!has(out,"ItemId=\"OFFER_A\"")||has(out,"ItemId=\"OFFER_B\""))return 7;
  if(!parse("<LSX><Request id=\"3\"><QueryEntitlements><FilterItems>ITEM_B</FilterItems></QueryEntitlements></Request></LSX>",&r))return 8;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 9;
  if(has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\""))return 10;
  if(!parse("<LSX><Request id=\"4\"><QueryEntitlements><FilterOffers><OfferId>OFFER_B</OfferId></FilterOffers></QueryEntitlements></Request></LSX>",&r))return 11;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 12;
  if(!has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\""))return 13;
  if(!parse("<LSX><Request id=\"5\"><QueryEntitlements><UnknownFilter>OFFER_B</UnknownFilter></QueryEntitlements></Request></LSX>",&r))return 14;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 15;
  if(!has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\""))return 16;
  if(!parse("<LSX><Request id=\"6\"><QueryEntitlements ItemId=\"OFFER_B\"/></Request></LSX>",&r))return 17;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 18;
  if(has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\""))return 19;
  if(!parse("<LSX><Request id=\"7\"><QueryEntitlements><FilterOffers>NOPE&amp</FilterOffers></QueryEntitlements></Request></LSX>",&r))return 20;
  // A02-B: exact upstream TinyXML2 accepts incomplete named references as literal text.
  // The filter must NOT match a configured entitlement; this is not XML 1.0 validation.
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok ||
     has(out,"ItemId=\"OFFER_A\"") || has(out,"ItemId=\"OFFER_B\""))return 21;
  config::clear_runtime_configuration();
  if(!parse("<LSX><Request id=\"8\"><QueryEntitlements><FilterGroups>G&amp</FilterGroups></QueryEntitlements></Request></LSX>",&r))return 22;
  if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok || n==0u ||
     has(out,"<Entitlement "))return 23;
  return 0;
}
