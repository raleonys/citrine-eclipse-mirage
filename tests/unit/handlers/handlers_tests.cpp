// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/handlers/handlers.hpp"
#include "preloader_l/xml/memory_printer.hpp"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace preloader_l::handlers;
static unsigned checks;
static void require(bool v) {++checks;if(!v)throw std::runtime_error("check "+std::to_string(checks));}
struct Fixture {
    Attributes game,settings;
    Profile profile{"Synthetic User","persona-test","user-test","ZZ","TEST","tier-test","sub-test","age-test","steam-test"};
    std::uint8_t online=0,pretend=0;
    StateReferences refs() {return {game,settings,profile,online,pretend};}
};
struct WriterFailure {};
struct FakeXml final : Xml {
    Attributes request;
    std::vector<std::string> opened,trace;
    std::vector<std::pair<std::string,std::string>> attributes;
    int depth=0,closes=0,throw_at_attribute=-1;
    std::function<void()> after_open;
    const char* attribute_002cc0(Request handle,const char* key) override {
        require(handle==77);trace.push_back(std::string("read:")+key);
        auto it=request.find(key);return it==request.end()?nullptr:it->second.c_str();
    }
    void open_004080(const char* element,bool compact) override {
        require(!compact);opened.emplace_back(element);trace.push_back(std::string("open:")+element);++depth;
        if(after_open)after_open();
    }
    void attribute_005560(const char* key,const char* value) override {
        require(key && value);
        if(static_cast<int>(attributes.size())==throw_at_attribute)throw WriterFailure{};
        require(depth>0);attributes.emplace_back(key,value);trace.push_back(std::string("attr:")+key);
    }
    void close_virtual_48(bool compact) override {
        require(!compact && depth>0);--depth;++closes;trace.push_back("close");
    }
};
struct FakeEffects final : Effects {
    std::vector<std::string> profiles;
    unsigned time_calls=0;
    std::string current_time_041b60() override {++time_calls;return "2030-01-02T03:04:05";}
    void log_profile_name_084f48(std::string_view name) override {profiles.emplace_back(name);}
};

static void config_and_connected() {
    FakeXml xml;
    require(get_config_02e5f0(xml));
    require(xml.opened.size()==35 && xml.closes==35 && xml.depth==0);
    require(xml.opened.front()=="GetConfigResponse");
    require(std::count(xml.opened.begin(),xml.opened.end(),"Service")==34);
    require(xml.attributes.size()==68);
    std::vector<std::string> facilities;
    for(std::size_t i=0;i<xml.attributes.size();i+=2) {
        require(xml.attributes[i].first=="Name" && xml.attributes[i+1].first=="Facility");
        facilities.push_back(xml.attributes[i+1].second);
    }
    require(facilities.front()=="SDK" && facilities.back()=="CONTENT");
    require(facilities[31]=="PROGRESSIVE_INSTALLATION" && facilities[32]=="PROGRESSIVE_INSTALLATION_EVENT");
    auto sorted=facilities;std::sort(sorted.begin(),sorted.end());
    require(std::adjacent_find(sorted.begin(),sorted.end())==sorted.end());
    for(auto online:{0u,1u,7u})for(auto pretend:{0u,1u,8u}) {
        Fixture f;f.online=static_cast<std::uint8_t>(online);f.pretend=static_cast<std::uint8_t>(pretend);FakeXml result;
        require(get_connected_02f2d0(result,f.refs()));
        require(result.opened==std::vector<std::string>{"InternetConnectedState"});
        require(result.attributes==std::vector<std::pair<std::string,std::string>>{{"connected",(online||pretend)?"1":"0"}});
        require(f.game.empty() && f.settings.empty());
    }
}

// Frozen compatibility output from the retained reconstruction source at
// e10561b256a5793f857ec4bed4b5a9fb9fcfbadd:src/handlers/handlers.cpp.
// The same 34 ordered pairs occur at archive11 260dd15e9f46b63038ed00e8fec93d27c837765a.
// This literal is independent of the production table at test runtime. It pins
// the advertisement, not the existence of complete external service engines.
static void config_advertisement_wire() {
    struct NoInput final : preloader_l::xml::AttributeReader {
        const char* attribute(Request,const char*) override {
            throw std::logic_error("GetConfig must not read request attributes");
        }
    } reader;
    static constexpr char expected[]=R"(<GetConfigResponse>
    <Service Name="EbisuSDK" Facility="SDK"/>
    <Service Name="EbisuSDK" Facility="PROFILE"/>
    <Service Name="XMPP" Facility="PRESENCE"/>
    <Service Name="XMPP" Facility="FRIENDS"/>
    <Service Name="Commerce" Facility="COMMERCE"/>
    <Service Name="EbisuSDK" Facility="RECENTPLAYER"/>
    <Service Name="EbisuSDK" Facility="IGO"/>
    <Service Name="EbisuSDK" Facility="MISC"/>
    <Service Name="EALS" Facility="LOGIN"/>
    <Service Name="Utility" Facility="UTILITY"/>
    <Service Name="XMPP" Facility="XMPP"/>
    <Service Name="XMPP" Facility="CHAT"/>
    <Service Name="EbisuSDK" Facility="IGO_EVENT"/>
    <Service Name="EALS" Facility="EALS_EVENTS"/>
    <Service Name="EbisuSDK" Facility="LOGIN_EVENT"/>
    <Service Name="XMPP" Facility="INVITE_EVENT"/>
    <Service Name="EbisuSDK" Facility="PROFILE_EVENT"/>
    <Service Name="XMPP" Facility="PRESENCE_EVENT"/>
    <Service Name="XMPP" Facility="FRIENDS_EVENT"/>
    <Service Name="Commerce" Facility="COMMERCE_EVENT"/>
    <Service Name="XMPP" Facility="CHAT_EVENT"/>
    <Service Name="EbisuSDK" Facility="DOWNLOAD_EVENT"/>
    <Service Name="EbisuSDK" Facility="PERMISSION"/>
    <Service Name="EbisuSDK" Facility="RESOURCES"/>
    <Service Name="EbisuSDK" Facility="BLOCKED_USERS"/>
    <Service Name="EbisuSDK" Facility="BLOCKED_USER_EVENT"/>
    <Service Name="EbisuSDK" Facility="GET_USERID"/>
    <Service Name="EbisuSDK" Facility="ONLINE_STATUS_EVENT"/>
    <Service Name="EbisuSDK" Facility="ACHIEVEMENT"/>
    <Service Name="EbisuSDK" Facility="ACHIEVEMENT_EVENT"/>
    <Service Name="EbisuSDK" Facility="BROADCAST_EVENT"/>
    <Service Name="PI" Facility="PROGRESSIVE_INSTALLATION"/>
    <Service Name="PI" Facility="PROGRESSIVE_INSTALLATION_EVENT"/>
    <Service Name="EbisuSDK" Facility="CONTENT"/>
</GetConfigResponse>
)";
    for(int repeat=0;repeat<2;++repeat) {
        preloader_l::xml::MemoryPrinter printer;
        preloader_l::xml::HandlerWriter writer(printer,reader);
        require(get_config_02e5f0(writer));
        require(printer.depth()==0);
        // Include exactly one trailing protocol terminator in the comparison.
        require(printer.buffer_including_terminator()==std::string(expected)+std::string(1,'\0'));
    }
}

static void profile_tests() {
    Fixture f;FakeEffects effects;FakeXml absent;
    require(!get_profile_02f530(77,absent,f.refs(),effects));
    require(absent.opened.empty() && effects.profiles.empty());
    for(auto index:std::vector<std::string>{"","0","not-a-number","99"}) {
        FakeXml xml;xml.request["index"]=index;
        require(get_profile_02f530(77,xml,f.refs(),effects));
        require(xml.opened==std::vector<std::string>{"GetProfileResponse"} && xml.depth==0);
        require(xml.attributes.size()==14);
        Attributes values(xml.attributes.begin(),xml.attributes.end());
        require(values.at("UserIndex")==index && values.at("Persona")=="Synthetic User");
        require(values.at("Country")=="ZZ" && values.at("GeoCountry")=="ZZ" && values.at("CommerceCountry")=="ZZ");
        require(values.at("CommerceCurrency")=="TEST" && values.at("PersonaId")=="persona-test" && values.at("UserId")=="user-test");
        require(values.at("AvatarId").empty() && values.at("IsTrialSubscriber")=="false");
        require(values.at("IsSubscriber")=="sub-test" && values.at("SubscriberLevel")=="tier-test");
        require(values.at("IsUnderAge")=="age-test" && values.at("IsSteamSubscriber")=="steam-test");
    }
    require(effects.profiles.size()==4);
    f.profile.username=std::string("A\0B",3);FakeXml nul;nul.request["index"]="0";
    require(get_profile_02f530(77,nul,f.refs(),effects));
    require(effects.profiles.back().size()==3);
    auto persona=std::find_if(nul.attributes.begin(),nul.attributes.end(),[](const auto& p){return p.first=="Persona";});
    require(persona->second=="A");
}

static void lookup_tests() {
    for(bool game:{false,true}) {
        Fixture f;auto& values=game?f.game:f.settings;
        const auto call=[&](FakeXml& xml){return game?get_game_info_02efd0(77,xml,f.refs()):get_setting_02f7c0(77,xml,f.refs());};
        const char* key_name=game?"GameInfoId":"SettingId";
        FakeXml missing;require(!call(missing) && values.empty() && missing.opened.empty());
        FakeXml unknown;unknown.request[key_name]="missing";
        require(!call(unknown) && values.empty() && unknown.attributes.empty());
        values["Known"]="test";
        FakeXml wrong_case;wrong_case.request[key_name]="known";
        require(!call(wrong_case) && values.size()==1);
        FakeXml found;found.request[key_name]="Known";require(call(found));
        require(found.attributes==std::vector<std::pair<std::string,std::string>>{{game?"GameInfo":"Setting","test"}});
        values[""]="";FakeXml empty;empty.request[key_name]="";
        require(call(empty) && empty.attributes[0].second.empty());
        FakeXml reentry;reentry.request[key_name]="Known";reentry.after_open=[&](){values.erase("Known");};
        require(call(reentry) && values.contains("Known") && values.at("Known").empty());
    }
}

static void aggregate_tests() {
    Fixture f;FakeXml settings;
    f.settings["LANGUAGE"]="synthetic-lang";f.settings["UNKNOWN"]="ignored";
    require(get_settings_02fa60(settings,f.refs()));
    require(settings.attributes.size()==6 && f.settings.size()==7);
    require(std::find(settings.attributes.begin(),settings.attributes.end(),std::pair<std::string,std::string>{"Language","synthetic-lang"})!=settings.attributes.end());
    require(std::none_of(settings.attributes.begin(),settings.attributes.end(),[](const auto& item){return item.second=="ignored";}));
    FakeEffects effects;FakeXml game;
    f.game["INSTALLED_VERSION"]="version-test";
    require(get_all_game_info_02da90(game,f.refs(),effects));
    require(f.game.size()==13 && game.attributes.size()==15 && game.depth==0 && effects.time_calls==1);
    require(game.attributes[13]==std::pair<std::string,std::string>{"HasExpiration","false"});
    require(game.attributes[14]==std::pair<std::string,std::string>{"SystemTime","2030-01-02T03:04:05"});
    Fixture partial;FakeXml fail;fail.throw_at_attribute=1;
    bool threw=false;try{(void)get_all_game_info_02da90(fail,partial.refs(),effects);}catch(WriterFailure&){threw=true;}
    require(threw && partial.game.size()==2 && effects.time_calls==1);
    require(fail.closes==0 && fail.attributes.size()==1);
}
int main(){config_and_connected();config_advertisement_wire();profile_tests();lookup_tests();aggregate_tests();std::printf("PASS: %u checks; synthetic XML/profile state; no target execution\n",checks);}
