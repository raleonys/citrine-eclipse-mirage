#include "preloader_l/handlers/misc_handlers.hpp"
#include <cstdlib>
#include <algorithm>
#include <exception>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <source_location>
#include <utility>
#include <vector>
using namespace preloader_l::misc_handlers_recovered;
namespace {
int checks=0;
void check(bool value,std::source_location where=std::source_location::current()) {++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks)+" at line "+std::to_string(where.line()));}
struct Failure {};
struct Trace: Xml,Elements,preloader_l::handlers_recovered::Effects,IgoEffects,NoResponse {
    std::vector<std::string> calls;
    std::map<Request,Request> first,next;
    std::map<Request,std::string> texts;
    int fail_at=-1,lock_result=0;
    std::int32_t count=1;
    bool null_seen=false;
    void record(std::string s) {calls.push_back(std::move(s));if(static_cast<int>(calls.size())==fail_at)throw Failure{};}
    const char* attribute_002cc0(Request,const char*) override {throw Failure{};}
    void open_004080(const char* element,bool compact) override {check(!compact);record(std::string("open:")+element);}
    void attribute_005560(const char* key,const char* value) override {null_seen|=value==nullptr;record(std::string("attr:")+key+"="+(value?value:"<NULL>"));}
    void close_virtual_48(bool compact) override {check(!compact);record("close");}
    Request first_child_element_003300(Request n) override {record("first:"+std::to_string(n));return first[n];}
    Request next_sibling_element_003fe0(Request n) override {record("next:"+std::to_string(n));return next[n];}
    const char* get_text_003860(Request n) override {record("text:"+std::to_string(n));auto it=texts.find(n);return it==texts.end()?nullptr:it->second.c_str();}
    void push_text_0056b0(const char* value,bool cdata) override {check(!cdata);null_seen|=!value;record(std::string("push:")+(value?value:"<NULL>"));}
    void log_profile_name_084f48(std::string_view) override {throw Failure{};}
    std::string current_time_041b60() override {record("time");return "2001-02-03T04:05:06Z";}
    int lock_02c620_04a6ac() override {record("lock");return lock_result;}
    std::int32_t& lock_count_4c() override {record("count");return count;}
    void acquire_event_queue_02c400() override {record("queue");}
    void enqueue_034660(std::string&& value) override {record("enqueue:"+value);}
    void unlock_04a6b4() override {record("unlock");}
    [[noreturn]] void thread_error_04a648(int code) override {record("threaderror:"+std::to_string(code));throw Failure{};}
    [[noreturn]] void throw_no_response_02bcd0_04d228() override {record("noresponse");throw Failure{};}
};
using Fn=bool(*)(Xml&);
struct Fixed {Fn fn;const char* element;std::vector<std::pair<std::string,std::string>> attrs;};
void fixed_tests() {
    const Fixed cases[]={
        {success_02ffb0,"ErrorSuccess",{{"Code","0"},{"Description",""}}},
        {presence_visibility_02f4e0,"GetPresenceVisibilityResponse",{{"Visible","false"}}},
        {group_info_02f270,"ErrorSuccess",{{"Code","-1577058296"},{"Description","User not in a party, create one."}}},
        {progressive_available_030cd0,"IsProgressiveInstallationAvailableResponse",{{"ItemId",""},{"Available","false"}}},
        {query_friends_0314a0,"QueryFriendsResponse",{{"Return","Success"}}},
        {block_list_02e5a0,"GetBlockListResponse",{{"Return","Success"}}},
        {voip_02da30,"ErrorSuccess",{{"Code","-1593835518"},{"Description","The function you are calling is not implemented."}}},
        {wallet_02ff60,"GetWalletBalanceResponse",{{"Balance","0"}}},
        {restart_game_032ab0,"ErrorSuccess",{{"Code","1"},{"Description","Origin is waiting for the game to shutdown."}}},
        {permission_02d9e0,"CheckPermissionResponse",{{"Access","GRANTED"}}},
        {start_download_032ca0,"ErrorSuccess",{{"Code","0"},{"Description","Requested a download of a DLC offer."}}},
        {query_offers_031ec0,"QueryOffersResponse",{}}
    };
    for(const auto& test:cases) {
        Trace t;check(test.fn(t));
        std::vector<std::string> expected={std::string("open:")+test.element};
        for(const auto& [k,v]:test.attrs)expected.push_back("attr:"+k+"="+v);
        expected.push_back("close");check(t.calls==expected);
        // Exceptions leave exactly the prefix already written; no invented retry.
        for(std::size_t i=1;i<=expected.size();++i) {
            Trace f;f.fail_at=static_cast<int>(i);bool caught=false;
            try{(void)test.fn(f);}catch(Failure&){caught=true;}
            check(caught);check(f.calls==std::vector<std::string>(expected.begin(),expected.begin()+i));
        }
    }
}
void presence_tests() {
    Attributes game;Trace t;check(presence_02f330(t,game));check(game.size()==1&&game.at("DISPLAY_NAME").empty());
    check(t.calls==std::vector<std::string>{"open:GetPresenceResponse","attr:Title=","attr:SessionId=","attr:MultiplayerId=","attr:GroupId=","attr:TitleId=","attr:Presence=INGAME","attr:GamePresence=","attr:Group=","attr:RichPresence=","attr:UserId=0","close"});
    game["DISPLAY_NAME"]=std::string("SYNTH\0hidden",12);Trace t2;check(presence_02f330(t2,game));check(t2.calls[9]=="attr:RichPresence=SYNTH");
    for(int i=1;i<=12;++i) {
        Attributes local;Trace f;f.fail_at=i;
        try{(void)presence_02f330(f,local);check(false);}catch(Failure&){}
        check(local.contains("DISPLAY_NAME")== (i>=10));
    }
    Trace empty;check(query_presence_031ef0(10,empty,empty,"synthetic"));
    check(empty.calls==std::vector<std::string>{"open:QueryPresenceResponse","first:10","close"});
    Trace q;q.first[10]=20;q.next[20]=30;q.texts[20]="456";q.texts[30]="789";
    const std::string username("SYNTH\0private",13);check(query_presence_031ef0(10,q,q,username));
    check(q.calls.size()==37);check(q.calls[12]=="attr:Persona=SYNTH");check(q.calls[14]=="text:20");check(q.calls[15]=="attr:UserId=456");check(q.calls[16]=="attr:State=NONE");check(q.calls[18]=="next:20");check(q.calls[32]=="attr:UserId=789");check(q.calls.back()=="close");
    // Every child element is visited without filtering by tag name.
    check(q.calls[35]=="next:30");
    Trace missing;missing.first[10]=20;check(query_presence_031ef0(10,missing,missing,"u"));check(missing.null_seen);check(missing.calls[15]=="attr:UserId=<NULL>");
}
void chunks_tests() {
    Trace empty;check(chunks_installed_02d720(10,empty,empty));check(empty.calls==std::vector<std::string>{"open:AreChunksInstalledResponse","attr:ItemId=","attr:Installed=true","first:10","close"});
    Trace t;t.first[1]=2;t.next[2]=3;t.texts[2]="1&2";t.texts[3]="";check(chunks_installed_02d720(1,t,t));
    check(t.calls==std::vector<std::string>{"open:AreChunksInstalledResponse","attr:ItemId=","attr:Installed=true","first:1","open:ChunkIds","text:2","push:1&2","close","next:2","open:ChunkIds","text:3","push:","close","next:3","close"});
    Trace missing;missing.first[1]=2;check(chunks_installed_02d720(1,missing,missing));check(missing.null_seen);
    Trace no;try{query_chunk_status_030eb0(no);check(false);}catch(Failure&){}check(no.calls==std::vector<std::string>{"noresponse"});
}
void effects_tests() {
    Trace time;check(utc_time_02fea0(time,time));check(time.calls==std::vector<std::string>{"open:GetUTCTimeResponse","time","attr:Time=2001-02-03T04:05:06Z","close"});
    Trace early;early.fail_at=1;try{(void)utc_time_02fea0(early,early);check(false);}catch(Failure&){}check(early.calls.size()==1);
    Trace igo;check(show_igo_032b10(igo,igo));
    check(igo.calls==std::vector<std::string>{"open:ErrorSuccess","attr:Code=0","attr:Description=","close","lock","count","queue","enqueue:<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>","unlock"});
    Trace lockfail;lockfail.lock_result=7;try{(void)show_igo_032b10(lockfail,lockfail);check(false);}catch(Failure&){}check(lockfail.calls.size()==6);check(lockfail.calls.back()=="threaderror:5");
    Trace overflow;overflow.count=std::numeric_limits<std::int32_t>::max();try{(void)show_igo_032b10(overflow,overflow);check(false);}catch(Failure&){}check(overflow.count==std::numeric_limits<std::int32_t>::max()-1);check(overflow.calls.back()=="threaderror:6");check(overflow.calls.size()==7);
    Trace xmlfail;xmlfail.fail_at=4;try{(void)show_igo_032b10(xmlfail,xmlfail);check(false);}catch(Failure&){}check(xmlfail.calls.size()==4);
    for(int fail:{7,8}) {
        Trace failed;failed.fail_at=fail;bool caught=false;
        try{(void)show_igo_032b10(failed,failed);}catch(Failure&){caught=true;}
        check(caught&&failed.calls.back()=="unlock");
        check(std::count(failed.calls.begin(),failed.calls.end(),"unlock")==1);
    }
}
}
int main() {
    try{fixed_tests();presence_tests();chunks_tests();effects_tests();}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return EXIT_FAILURE;}
    catch(...){std::cerr<<"unexpected exception after "<<checks<<" checks\n";return EXIT_FAILURE;}
    std::cout<<"PASS: "<<checks<<" synthetic checks; 18 handler bodies; original execution none\n";
}
