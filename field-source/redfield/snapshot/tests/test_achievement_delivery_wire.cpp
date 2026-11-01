// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/achievement_delivery.hpp"
#include <iostream>
using namespace preloader_l;using Delivery=PreparedAchievementDelivery;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto prepared=GrantDeliveryResult::prepared;
bool parse_fixture(const std::string& children,xml::Request& r){std::string source="<LSX><Request id='G&amp;lt;' recipient='Client&amp;X'><GrantAchievement>"+children+"</GrantAchievement></Request></LSX>";const auto result=xml::parse(source,r);source.assign(source.size(),'!');return result==xml::Result::ok;}
std::shared_ptr<const ConfigSnapshot> seed(const std::string& id="A",int32_t total=3){auto s=std::make_shared<ConfigSnapshot>();s->achievements.push_back({id,"Alpha&","","","","0000-00-00T00:00:00","0000-00-00T00:00:00",total,0,0});return s;}
int main(){
    AchievementSession owner;const auto initial=seed("A&");CHECK(owner.initialize(initial)==AchievementSession::InitializeResult::initialized);xml::Request r;Delivery d;const std::string stamp="2026-10-20T00:00:00";
    CHECK(parse_fixture("<AchievementId>A&amp;</AchievementId><Progress>1</Progress>",r));CHECK(prepare_achievement_delivery(owner,r,stamp,{},d)==prepared && d.active() && d.completion_event().empty() && owner.snapshot()==initial);CHECK(d.projected()->achievements[0].progress==1 && d.commit());const auto partial=owner.snapshot();CHECK(partial->achievements[0].progress==1 && !d.response().empty());d.rollback();
    CHECK(parse_fixture("<AchievementId>A&amp;</AchievementId><Progress>2</Progress>",r));CHECK(prepare_achievement_delivery(owner,r,stamp,{},d)==prepared && !d.completion_event().empty() && owner.snapshot()==partial);
    const std::string record="<Achievement Id=\"A&amp;\" Total=\"3\" Count=\"1\" Progress=\"3\" GrantDate=\"2026-10-20T00:00:00\" Expiration=\"0000-00-00T00:00:00\" ImageId=\"\" Name=\"Alpha&amp;\" Description=\"\" HowTo=\"\"/>";
    CHECK(d.response()==" <LSX>\n  "+record+"\n </LSX>\n");CHECK(d.completion_event()=="<LSX><Event sender=\"EbisuSDK\">"+record+"</Event></LSX>");CHECK(d.response().find("Response id=")==std::string_view::npos && d.response().find("G&amp;")==std::string_view::npos);
    const std::string response(d.response()),event(d.completion_event());CHECK(d.commit() && owner.snapshot()->achievements[0].count==1 && d.response()==response && d.completion_event()==event);d.rollback();CHECK(owner.snapshot()->achievements[0].count==1);
    CHECK(prepare_achievement_delivery(owner,r,"ignored",{},d)==prepared && d.completion_event().empty() && d.response()==response);const auto complete=owner.snapshot();CHECK(d.commit() && owner.snapshot()==complete);d.rollback();
    struct Case{const char* body;GrantDeliveryResult result;int32_t progress;};const Case cases[]={
        {"<AchievementId>A</AchievementId>",prepared,1},
        {"<AchievementId> A </AchievementId><Progress> +2 </Progress>",prepared,2},
        {"<AchievementId><![CDATA[ A ]]></AchievementId><Progress><![CDATA[ 2 ]]></Progress>",prepared,2},
        {"<AchievementId>&#65;</AchievementId><Progress>&#50;</Progress>",prepared,2},
        {"<AchievementId>A</AchievementId><Progress>-1</Progress>",prepared,-1},
        {"<AchievementId>A</AchievementId><Progress>0</Progress>",prepared,0},
        {"<AchievementId>A</AchievementId><Progress>2147483647</Progress>",prepared,3},
        {"<AchievementId>A</AchievementId><Progress>-2147483648</Progress>",prepared,INT32_MIN},
        {"<AchievementId>A</AchievementId><Progress>2147483648</Progress>",GrantDeliveryResult::no_response,0},
        {"<AchievementId>A</AchievementId><Progress/>",GrantDeliveryResult::no_response,0},
        {"<AchievementId>A</AchievementId><Progress>1 2</Progress>",GrantDeliveryResult::no_response,0},
        {"<Progress>1</Progress>",GrantDeliveryResult::no_response,0},
        {"<AchievementID>A</AchievementID>",GrantDeliveryResult::no_response,0},
        {"<AchievementId/>",GrantDeliveryResult::rejected,0},
        {"<AchievementId><Nested>A</Nested></AchievementId>",GrantDeliveryResult::rejected,0},
        {"<AchievementId>A<!--c--></AchievementId>",GrantDeliveryResult::rejected,0},
        {"<AchievementId>A</AchievementId><Progress><N>1</N></Progress>",GrantDeliveryResult::rejected,0},
        {"bad<AchievementId>A</AchievementId>",GrantDeliveryResult::rejected,0},
        {"<AchievementId>A</AchievementId><AchievementId><N/></AchievementId><Progress>1</Progress><Progress>bad</Progress>",prepared,1},
        {"<AchievementId>A</AchievementId><Other><Progress>9</Progress></Other>",prepared,1},
        {"<AchievementId>A</AchievementId><progress>2</progress>",prepared,1}
    };
    for(const auto& c:cases){AchievementSession fresh;const auto base=seed();CHECK(fresh.initialize(base)==AchievementSession::InitializeResult::initialized && parse_fixture(c.body,r));Delivery staged;CHECK(prepare_achievement_delivery(fresh,r,stamp,{},staged)==c.result && fresh.snapshot()==base);if(c.result==prepared)CHECK(staged.projected()->achievements[0].progress==c.progress);else CHECK(!staged.active() && staged.response().empty() && staged.completion_event().empty());}
    AchievementSession raw;CHECK(raw.initialize(seed("raw&amp;"))==AchievementSession::InitializeResult::initialized && parse_fixture("<AchievementId>raw&amp;amp;</AchievementId>",r));CHECK(prepare_achievement_delivery(raw,r,stamp,{},d)==prepared && d.projected()->achievements.size()==1 && d.response().find("Id=\"raw&amp;amp;\"")!=std::string_view::npos);d.rollback();
    for(const auto* increment:{"0","-2"}){AchievementSession fresh;CHECK(fresh.initialize(std::make_shared<ConfigSnapshot>())==AchievementSession::InitializeResult::initialized && parse_fixture(std::string("<AchievementId>NEW</AchievementId><Progress>")+increment+"</Progress>",r));CHECK(prepare_achievement_delivery(fresh,r,stamp,{},d)==prepared && d.completion_event().empty() && d.projected()->achievements[0].count==0 && d.commit());d.rollback();}
    std::cout<<"GRANT_RESPONSE_EVENT_LITERAL_SELECTOR_SEMANTIC_POLICY_PASS\n";
}
