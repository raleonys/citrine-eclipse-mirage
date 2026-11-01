#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/handlers/store_routes.hpp"
#include "preloader_l/handlers/query_presence_route.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <string>
#include <cstdio>
#include <cstring>
using namespace preloader_l;
static int total=0;static bool success=true;
static void check(bool x,const char *name){++total; success&=x;std::printf("CHECK|%s|%s\n",name,x?"PASS":"FAIL");}
static bool cfg(const char *s){return config::set_runtime_configuration_cfg(s,std::strlen(s))==config::RuntimeConfigurationStatus::ok;}
struct Reply{handlers::HandlerStatus status;std::string xml;size_t written;};
static Reply run(const char *cmd,const char* body="",bool is_presence=false){
 std::string req="<LSX><Request id=\"101\" recipient=\"EbisuSDK\"><";req+=cmd;
 if(*body){req+='>';req+=body;req+="</";req+=cmd;req+='>';}
 else req+="/>";
 req+="</Request></LSX>";
 xml::RequestView v{};if(xml::parse_request_view(req.data(),req.size(),&v)!=xml::RequestStatus::ok) return {handlers::HandlerStatus::invalid_request,"",0};
 char buffer[30000]{};size_t w=0;
 auto h= is_presence?handlers::handle_query_presence(v,buffer,sizeof(buffer),&w):(std::strcmp(cmd,"QueryContent")==0?handlers::handle_query_content(v,buffer,sizeof(buffer),&w):handlers::handle_query_entitlements(v,buffer,sizeof(buffer),&w));
 return {h,std::string(buffer,(h==handlers::HandlerStatus::ok&&w<sizeof(buffer))?w:0),w};
}
static bool ok(const Reply& x){return x.status==handlers::HandlerStatus::ok;}
static bool contains(const Reply& x,const char* v){return x.xml.find(v)!=std::string::npos;}
int main(){
 auto gen0=config::runtime_configuration_generation();
 static const char state[]=R"CFG("Config2" { "Entitlements" { "E1" { "Group" "DIRECT" "EntitlementTag" "TAG" "Inner" {"Group" "NESTED" "EntitlementTag" "NESTED_TAG"} } } "Content" { "C1" { "Name" "DIRECT_NAME" "Inner" {"Name" "NESTED_NAME"} } } })CFG";
 check(cfg(state),"config_commit_initial");
 check(config::runtime_configuration_generation()>gen0,"generation_advance");
 auto q1=run("QueryEntitlements");
 check(ok(q1),"entitlement_response_ok");
 check(contains(q1,"Group=\"DIRECT\"")&&!contains(q1,"Group=\"NESTED\""),"entitlement_direct_child_authority");
 auto q2=run("QueryEntitlements","<FilterGroups>DIRECT</FilterGroups>");
 check(ok(q2)&&contains(q2,"ItemId=\"E1\""),"entitlement_filter_direct_match");
 auto q3=run("QueryEntitlements","<FilterGroups>NESTED</FilterGroups>");
 check(ok(q3)&&!contains(q3,"ItemId=\"E1\""),"entitlement_filter_nested_excluded");
 auto qc=run("QueryContent");
 check(ok(qc)&&contains(qc,"DIRECT_NAME")&&!contains(qc,"NESTED_NAME"),"query_content_unchanged");
 const char* mixed[]={"<Id>A<!--c-->B</Id>","<Id><![CDATA[A]]>B</Id>","<Id>A<![CDATA[B]]></Id>","<Id>A<Sub/>B</Id>","<Id><![CDATA[A]]><Sub/>B</Id>","<Id><!--leading--><![CDATA[A]]>B</Id>"};
 for(int i=0;i<6;++i){auto p=run("QueryPresence",mixed[i],true);char name[60];std::snprintf(name,sizeof(name),"presence_mixed_first_node_%d",i+1);check(ok(p)&&contains(p,"UserId=\"A\""),name);}
 auto negative=run("QueryPresence","<Id><Sub/>A</Id>",true);
 check(negative.status==handlers::HandlerStatus::invalid_request,"presence_leading_nested_safe_reject");
 auto textless=run("QueryPresence","<Id></Id>",true);
 check(textless.status==handlers::HandlerStatus::invalid_request,"presence_empty_safe_reject");
 auto post=run("QueryEntitlements");
 check(ok(post)&&contains(post,"Group=\"DIRECT\""),"entitlement_unchanged_after_presence");
 auto postc=run("QueryContent");
 check(ok(postc)&&contains(postc,"DIRECT_NAME"),"content_unchanged_after_presence");
 constexpr char invalid[]=R"CFG("Config2" { "Entitlements" { "BROKEN" { "Group" "X" } )CFG";
 check(!cfg(invalid),"invalid_reload_rejected");
 auto after=run("QueryEntitlements");
 check(ok(after)&&contains(after,"Group=\"DIRECT\""),"transaction_rollback_preserves_entinements");
 config::clear_runtime_configuration();
 auto cleared=run("QueryEntitlements");
 check(ok(cleared)&&contains(cleared,"<QueryEntitlementsResponse/>")&&!contains(cleared,"ItemId="),"config_clear_empty_response");
 auto afterclear=run("QueryPresence","<Id>A<!--z--></Id>",true);
 check(ok(afterclear)&&contains(afterclear,"UserId=\"A\""),"presence_after_config_clear");
 std::printf("TOTAL|%d|%s\n",total,success?"PASS":"FAIL");return success?0:1;
}
