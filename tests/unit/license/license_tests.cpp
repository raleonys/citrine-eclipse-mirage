#include "preloader_l/core/license.hpp"
#include "preloader_l/xml/memory_printer.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
using namespace preloader_l;
using namespace license_recovered;
int checked=0;
void check(bool condition,const char* description) {
    ++checked;
    if(!condition) throw std::runtime_error(description);
}
template<class E,class F> E thrown(F&& function) {
    try { function(); } catch(const E& e) { ++checked; return e; }
    throw std::runtime_error("expected synthetic boundary exception");
}
struct Exit { std::int32_t code; };
struct NoResponse {};
struct FsError { std::string operation; std::int32_t error; Path path; };
struct Injected {};
struct Node final : messages_recovered::XmlNode {
    std::map<std::string,std::string,std::less<>> attributes;
    mutable std::vector<std::string> reads;
    std::string_view name() const override { return "SyntheticRequest"; }
    const messages_recovered::XmlNode* first_element() const override { return nullptr; }
    const messages_recovered::XmlNode* next_element() const override { return nullptr; }
    std::optional<std::string_view> attribute(std::string_view key) const override {
        reads.emplace_back(key);
        auto found=attributes.find(key);
        if(found==attributes.end()) return std::nullopt;
        return found->second;
    }
};
struct Xml final : handlers_recovered::Xml {
    xml_recovered::MemoryPrinter printer;
    std::vector<std::string> calls;
    const char* attribute_002cc0(handlers_recovered::Request,const char*) override { throw Injected{}; }
    void open_004080(const char* element,bool compact) override {
        calls.emplace_back("open:"+std::string(element)); printer.open_element(element,compact);
    }
    void attribute_005560(const char* key,const char* value) override {
        calls.emplace_back("attribute:"+std::string(key)+"="+value); printer.attribute(key,value);
    }
    void close_virtual_48(bool compact) override { calls.emplace_back("close"); printer.close_element(compact); }
    std::string text() const { return std::string(printer.bytes_without_terminator()); }
};
std::string ascii(const Path& path) {
    std::string text;
    for(char16_t ch:path) {
        if(ch>127) throw std::runtime_error("synthetic mock supports ASCII paths only");
        text+=static_cast<char>(ch);
    }
    return text;
}
Path wide(std::string_view text) { return Path(text.begin(),text.end()); }
struct Dialog { std::string title,body; std::uint32_t flags; };
struct Mock final : Effects {
    std::vector<std::string> calls;
    std::vector<Dialog> dialogs;
    std::vector<Path> loaded_paths;
    std::vector<std::string> log;
    std::string written_text;
    Path written_path;
    FileStatus file_status{2,0};
    std::string online_value="synthetic-online";
    bool null_online=false;
    bool dll_matches=true,exe_matches=true,write_result=true;
    std::vector<std::string> tokens{"",""};
    std::size_t token_index=0,wine_index=0;
    std::vector<bool> wine{false,false};
    std::string injected;
    Path expected_dll=u"C:\\fixture\\dbdata.dll",expected_exe=u"C:\\fixture\\game.exe";
    void called(std::string text) {
        calls.push_back(text);
        if(text==injected) throw Injected{};
    }
    const char* online_auth_virtual_30(const char* client) override {
        called("online:"+std::string(client)); return null_online?nullptr:online_value.c_str();
    }
    FileStatus status_01ea20(const Path& path,std::uint32_t flags) override {
        called("status:"+ascii(path)); check(flags==3,"status uses flag 3"); return file_status;
    }
    [[noreturn]] void filesystem_error_018130(std::string_view operation,std::int32_t error,const Path& path) override {
        called("filesystem_error"); throw FsError{std::string(operation),error,path};
    }
    bool file_hash_matches_042d20(const Path& path,std::string expected) override {
        called("hash:"+ascii(path)); check(expected.size()==40,"hash input length passed intact");
        // Simulate the callee's mutation of its expected-hash COPY.
        expected.assign("mutated-copy");
        if(path==expected_dll) return dll_matches;
        check(path==expected_exe,"hash receives only explicitly selected fixture path");
        return exe_matches;
    }
    std::string load_token_02afa0(const Path& path) override {
        called("token:"+ascii(path)); loaded_paths.push_back(path);
        if(token_index>=tokens.size()) throw std::runtime_error("synthetic token results exhausted");
        return tokens[token_index++];
    }
    Path filesystem_widen_006d70(std::string_view text) override { called("widen:"+std::string(text)); return wide(text); }
    Path filename_00e120(const Path& path) override {
        called("filename:"+ascii(path)); const auto slash=path.find_last_of(u"\\/");
        return slash==Path::npos?path:path.substr(slash+1);
    }
    Path parent_path_019820(const Path& path) override {
        called("parent:"+ascii(path)); const auto slash=path.find_last_of(u"\\/");
        return slash==Path::npos?Path{}:path.substr(0,slash);
    }
    Path join_016b00(const Path& parent,const Path& child) override {
        called("join:"+ascii(parent)+"|"+ascii(child)); return parent+u"\\"+child;
    }
    std::string path_utf8_01e3f0(const Path& path) override { called("utf8:"+ascii(path)); return ascii(path); }
    std::string current_time_041b60_null_true() override { called("time:null,true"); return "2000-01-02_03-04-05"; }
    bool wine_detected_045010() override {
        called("wine");
        if(wine_index>=wine.size()) throw std::runtime_error("synthetic Wine results exhausted");
        return wine[wine_index++];
    }
    void log_literal_006880(std::string_view text) override { called("log_literal"); log.emplace_back(text); }
    void log_string_006860(std::string_view text) override { called("log_string"); log.emplace_back(text); }
    bool write_ticket_03c1d0(const Path& path,std::string_view text) override {
        called("write"); written_path=path; written_text=text; return write_result;
    }
    void dialog_038bc0(std::string_view title,std::string_view body,std::uint32_t flags) override {
        called("dialog:"+std::string(title)); dialogs.push_back({std::string(title),std::string(body),flags});
    }
    [[noreturn]] void exit_05cdc0(std::int32_t code) override { called("exit:"+std::to_string(code)); throw Exit{code}; }
    [[noreturn]] void no_response_02bcd0_04d228() override { called("no_response"); throw NoResponse{}; }
};
struct Fixture {
    std::string auth="synthetic-fallback",license,dll_hash=std::string(40,'d'),exe_hash=std::string(40,'e'),content="fixture-content";
    std::uint8_t online=0,fake=0,blocked=0;
    Path dll=u"C:\\fixture\\dbdata.dll",exe=u"C:\\fixture\\game.exe",base=u"C:\\fixture\\license\\fixture-id";
    handlers_recovered::Attributes game{{"DISPLAY_NAME","Fixture Game"},{"INSTALLED_VERSION","1.2"}};
    std::int32_t attempts=0;
    Mock effects;
    Node request;
    Xml xml;
    Handlers handlers;
    Fixture():handlers(StateReferences{auth,online,fake,license,dll_hash,exe_hash,content,dll,exe,base,game,attempts,blocked},effects) {}
    void ticket_attributes() { request.attributes={{"RequestTicket","synthetic-ticket"},{"TicketEngine","synthetic-engine"}}; }
    bool run() { return handlers.request_license_0320c0(request,xml); }
};
void test_auth_and_invalidation() {
    {
        Fixture f;
        check(!f.handlers.auth_code_02e3b0(f.request,f.xml),"missing ClientId returns false");
        check(f.xml.calls.empty()&&f.effects.calls.empty(),"missing ClientId emits and invokes nothing");
    }
    for(int online:{0,1,2}) for(int fake:{0,1,2}) for(bool empty_online:{false,true}) {
        Fixture f; f.online=static_cast<std::uint8_t>(online); f.fake=static_cast<std::uint8_t>(fake);
        f.request.attributes={{"ClientId",""}}; if(empty_online) f.effects.online_value.clear();
        check(f.handlers.auth_code_02e3b0(f.request,f.xml),"auth emits a response for present-empty ClientId");
        if(!online&&!fake) {
            check(f.xml.text()=="<ErrorSuccess Code=\"-1577058293\" Description=\"You need to be online to get an AuthCode.\"/>\n","offline auth error XML");
            check(f.effects.calls.empty(),"offline failure makes no online call");
        } else {
            const auto value=online&&!empty_online?"synthetic-online":"synthetic-fallback";
            check(f.xml.text()=="<AuthCode value=\""+std::string(value)+"\"/>\n","auth replacement/fallback XML");
            check(f.effects.calls.size()==static_cast<std::size_t>(online?1:0),"online callback selection");
        }
        check(f.auth=="synthetic-fallback","auth handler does not mutate configured fallback");
    }
    {
        Fixture f; f.online=1; f.request.attributes={{"ClientId",std::string("client\0ignored",14)}};
        f.effects.online_value="<&\"";
        check(f.handlers.auth_code_02e3b0(f.request,f.xml),"online escaped value succeeds");
        check(f.effects.calls==std::vector<std::string>{"online:client"},"ClientId C-string boundary");
        check(f.xml.text()=="<AuthCode value=\"&lt;&amp;&quot;\"/>\n","actual serializer escapes auth attribute");
    }
    {
        Fixture f; f.online=1; f.request.attributes={{"ClientId","fixture"}}; f.effects.null_online=true;
        thrown<std::invalid_argument>([&]{f.handlers.auth_code_02e3b0(f.request,f.xml);});
        check(f.xml.calls.empty(),"out-of-domain null provider does not fabricate a value");
    }
    {
        Fixture f; f.license=std::string(200,'x'); const auto capacity=f.license.capacity(); f.blocked=1;f.attempts=2;
        check(f.handlers.invalidate_license_030c50(f.request,f.xml),"invalidate emits response");
        check(f.license.empty()&&f.license.capacity()==capacity,"invalidate clears in-memory license without shrinking");
        check(f.blocked==1&&f.attempts==2,"invalidate preserves block and retry state");
        check(f.effects.calls.empty()&&f.request.reads.empty(),"invalidate does no filesystem/provider/request work");
        check(f.xml.text()=="<ErrorSuccess Code=\"0\" Description=\"License Deleted.\"/>\n","invalidate literal XML");
    }
}
void test_guard_order() {
    {
        Fixture f; f.blocked=7; f.dll_hash.clear();
        thrown<NoResponse>([&]{f.run();});
        check(f.effects.calls==std::vector<std::string>{"no_response"},"blocked guard precedes every validation");
    }
    for(std::size_t length=0;length<=41;++length) {
        if(length==40) continue;
        Fixture f;f.dll_hash.assign(length,'?');f.license="preloaded";
        check(thrown<Exit>([&]{f.run();}).code==-4,"DLL hash length exit -4");
        check(f.blocked==1&&f.effects.dialogs.size()==1,"invalid DLL hash blocks before dialog/exit");
        check(f.effects.dialogs[0].title=="Bad dll hash"&&f.effects.dialogs[0].flags==0x10,"DLL hash diagnostic category");
        check(f.effects.dialogs[0].body.find("\"surge.cfg\"")!=std::string::npos,"DLL hash diagnostic names the product config");
        check(f.effects.calls==std::vector<std::string>{"dialog:Bad dll hash","exit:-4"},"preloaded license cannot skip DLL hash guard");
    }
    {
        Fixture f; f.dll.clear();
        check(thrown<Exit>([&]{f.run();}).code==-5,"empty DLL name exit -5");
        check(f.blocked==1&&f.effects.calls==std::vector<std::string>{"dialog:Bad dll name","exit:-5"},"empty name guard order");
        check(f.effects.dialogs[0].body.find("\"surge.cfg\"")!=std::string::npos,"DLL name diagnostic names the product config");
    }
    for(int kind=-2;kind<=10;++kind) for(int error:{0,7}) {
        Fixture f;f.effects.file_status={kind,error};f.license="preloaded";
        const bool throws_fs=error&&((static_cast<std::uint32_t>(kind)-1U)&0xfffffff7U);
        if(throws_fs) {
            const auto caught=thrown<FsError>([&]{f.run();});
            check(caught.operation=="status"&&caught.error==error&&caught.path==f.dll,"status error contract");
            check(f.blocked==0&&f.effects.dialogs.empty(),"status throw does not set blocked or show dialog");
        } else if(kind!=2) {
            check(thrown<Exit>([&]{f.run();}).code==-5,"non-regular status exits -5, including tolerated kind 1/9 errors");
            check(f.blocked==1,"non-regular status marks blocked");
        } else check(f.run(),"kind 2 with zero error accepts preloaded license");
    }
    {
        Fixture f; f.effects.dll_matches=false; f.license="preloaded";
        check(thrown<Exit>([&]{f.run();}).code==-6,"DLL hash mismatch exits -6");
        check(f.blocked==1&&f.attempts==0&&f.effects.loaded_paths.empty(),"hash mismatch blocks before token loading");
        check(f.effects.dialogs[0].body=="\"C:\\fixture\\dbdata.dll\" file has incorrect hash.\n\nThat usually means you didn't update your game or you didn't update \"not a crack\"/\"fixed exe\".","DLL mismatch exact diagnostic");
    }
    {
        Fixture f; f.dll_hash.assign(40,'?'); f.license="cached";
        check(f.run(),"40-byte hash receives validation even without hexadecimal syntax");
        check(f.dll_hash==std::string(40,'?'),"hash callee copy cannot alter global expected hash");
        check(f.effects.calls==std::vector<std::string>{"status:C:\\fixture\\dbdata.dll","hash:C:\\fixture\\dbdata.dll"},"preloaded license still validates file status and hash");
        check(f.xml.text()=="<RequestLicenseResponse License=\"cached\"/>\n","cached token XML");
        check(f.request.reads.empty(),"cached token ignores ticket attributes");
    }
}
void test_attempts_and_blocking() {
    for(int initial:{-1,0,1,2,3}) for(int success_at:{-1,0,1,2}) {
        Fixture f;f.attempts=initial;f.effects.tokens={"","",""};
        if(success_at>=0) f.effects.tokens[static_cast<std::size_t>(success_at)]="synthetic-token";
        const int budget=std::max(0,2-initial);
        const bool success=success_at>=0&&success_at<budget;
        const int consumed=success?success_at+1:budget;
        check(f.run()==success,"attempt result follows signed counter and token content");
        check(f.attempts==initial+consumed,"attempt count increments after each complete token read");
        check(f.effects.loaded_paths.size()==static_cast<std::size_t>(consumed),"bounded token read count");
        for(int index=0;index<consumed;++index) {
            const bool cached=initial+index!=0;
            check(f.effects.loaded_paths[static_cast<std::size_t>(index)]==f.base+(cached?u"_cached.dlf":u".dlf"),"token suffix appends to base and uses counter!=0");
        }
        if(success) {
            check(!f.blocked&&f.request.reads.empty(),"loaded token avoids block and ticket lookup");
            check(f.xml.text()=="<RequestLicenseResponse License=\"synthetic-token\"/>\n","loaded token emits XML");
        } else {
            check(f.blocked==1&&f.xml.calls.empty(),"exhaustion without ticket marks blocked and emits nothing");
            check(f.request.reads==std::vector<std::string>{"RequestTicket","TicketEngine"},"both ticket attributes read in order");
            f.effects.calls.clear();
            thrown<NoResponse>([&]{f.run();});
            check(f.effects.calls==std::vector<std::string>{"no_response"},"subsequent request throws no-response after false return");
        }
    }
    for(const auto& point:std::vector<std::string>{"status:C:\\fixture\\dbdata.dll","hash:C:\\fixture\\dbdata.dll","widen:.dlf","token:C:\\fixture\\license\\fixture-id.dlf"}) {
        Fixture f;f.effects.injected=point;
        thrown<Injected>([&]{f.run();});
        check(f.blocked==0&&f.attempts==0,"throw before first completed read leaves flags/counter unchanged");
    }
    {
        Fixture f; f.effects.tokens={"",""};f.effects.injected="token:C:\\fixture\\license\\fixture-id_cached.dlf";
        thrown<Injected>([&]{f.run();});
        check(f.blocked==0&&f.attempts==1,"throw on second token read preserves completed first attempt");
    }
    for(bool missing_ticket:{false,true}) {
        Fixture f;f.ticket_attributes();f.request.attributes.erase(missing_ticket?"RequestTicket":"TicketEngine");
        check(!f.run()&&f.blocked==1,"either missing attribute returns false after blocking");
        check(f.request.reads==std::vector<std::string>{"RequestTicket","TicketEngine"},"second lookup is not short-circuited");
    }
}
void test_ticket_guards() {
    for(const std::string& value:std::vector<std::string>{"","0"}) {
        Fixture f;f.ticket_attributes();f.content=value;
        check(thrown<Exit>([&]{f.run();}).code==-2,"empty/exact-zero content ID exits -2");
        check(f.blocked==1&&f.effects.dialogs[0].title=="Bad content ID","content diagnostic");
        check(f.effects.dialogs[0].body.find("\"surge.cfg\"")!=std::string::npos,"content diagnostic names the product config");
        check(std::count(f.effects.calls.begin(),f.effects.calls.end(),"hash:C:\\fixture\\game.exe")==0,"content guard precedes exe hash");
    }
    for(std::size_t length:{0U,39U,41U}) {
        Fixture f;f.ticket_attributes();f.exe_hash.assign(length,'e');
        check(thrown<Exit>([&]{f.run();}).code==-3,"exe hash length guard exits -3");
        check(f.blocked==1&&f.effects.dialogs[0].title=="Bad exe hash","exe hash diagnostic title");
        check(f.effects.dialogs[0].body.find("\"surge.cfg\"")!=std::string::npos,"exe hash diagnostic names the product config");
    }
    {
        Fixture f;f.ticket_attributes();f.effects.exe_matches=false;
        check(thrown<Exit>([&]{f.run();}).code==-7,"exe hash mismatch exits -7");
        check(f.effects.dialogs[0].title=="Bad dll hash","original exe mismatch title remains Bad dll hash");
        check(f.effects.dialogs[0].body=="\"C:\\fixture\\game.exe\" file has incorrect hash.\n\nThat usually means you have the new \"not a crack\"/\"fixed exe\" but you opened the wrong exe.","exe mismatch exact diagnostic");
    }
}
void test_ticket_construction_and_effects() {
    for(bool canonical_dll:{false,true}) for(bool write_ok:{false,true}) for(bool wine_first:{false,true}) for(bool wine_second:{false,true}) {
        Fixture f;f.ticket_attributes();f.effects.write_result=write_ok;f.effects.wine={wine_first,wine_second};
        if(!canonical_dll){f.dll=u"C:\\fixture\\Custom.DLL";f.effects.expected_dll=f.dll;}
        const auto caught=thrown<Exit>([&]{f.run();});
        check(caught.code==-1&&f.blocked==1&&f.attempts==2,"ticket branch ends in exit -1 after persistent state changes");
        std::string golden="Fixture Game (v1.2; \"game.exe\") is protected by Denuvo.\n\nexe:"+f.exe_hash+";dll:"+f.dll_hash;
        if(!canonical_dll) golden+=";Custom.DLL";
        golden+="\n\nsynthetic-ticket|synthetic-engine|fixture-content";
        check(f.effects.written_text==golden,"ticket text matches independently constructed golden");
        check(f.effects.log==std::vector<std::string>{"\n\nDENUVO TICKET BELOW\n\n",golden,"\n\nDENUVO TICKET ABOVE\n\n"},"logging order and full string lengths");
        check(f.effects.written_path==u"C:\\fixture\\Denuvo_ticket_2000-01-02_03-04-05.txt","ticket file path joins once then appends converted time suffix");
        check(f.effects.wine_index==static_cast<std::size_t>(write_ok?1:2),"Wine check is repeated only after write failure");
        check(f.effects.dialogs.size()==static_cast<std::size_t>(wine_first?2:1),"pre-write Wine warning dialog count");
        if(wine_first) check(f.effects.dialogs[0].title=="Wine detected"&&f.effects.dialogs[0].flags==0x10,"Wine pre-warning flags");
        const auto& dialog=f.effects.dialogs.back();
        check(dialog.flags==0x40,"ticket completion dialog flags");
        if(write_ok) {
            check(dialog.title=="Ticket saved"&&dialog.body=="Your ticket has been saved to:\nC:\\fixture\\Denuvo_ticket_2000-01-02_03-04-05.txt","written-ticket diagnostic");
        } else if(wine_second) {
            check(dialog.title=="Wine detected"&&dialog.body.find("Copy your ticket from the terminal instead.")!=std::string::npos,"Wine write failure diagnostic");
        } else check(dialog.title=="Token required"&&dialog.body=="Press Ctrl+C to copy this message.\n\n"+golden,"Windows write failure includes ticket");
        check(f.xml.calls.empty(),"ticket-required branch never emits license success XML");
        check(std::count(f.effects.calls.begin(),f.effects.calls.end(),"utf8:"+(canonical_dll?std::string("dbdata.dll"):std::string("Custom.DLL")))==(canonical_dll?1:2),"noncanonical DLL filename is converted twice");
        const auto position=[&](const std::string& value){return std::find(f.effects.calls.begin(),f.effects.calls.end(),value)-f.effects.calls.begin();};
        check(position("log_string")<position("widen:Denuvo_ticket_")&&position("widen:Denuvo_ticket_")<position("parent:C:\\fixture\\game.exe")&&position("time:null,true")<position("write"),"observed log/path/time/write ordering");
    }
    {
        Fixture f;f.ticket_attributes();f.game.clear();f.request.attributes={{"RequestTicket",""},{"TicketEngine",""}};
        f.content=std::string("0\0x",3);
        check(thrown<Exit>([&]{f.run();}).code==-1,"present-empty ticket/engine and non-exact-zero ID continue to ticket generation");
        check(f.game.size()==2&&f.game.contains("DISPLAY_NAME")&&f.game.contains("INSTALLED_VERSION"),"ticket game map lookups insert absent values");
        check(f.effects.written_text.starts_with(" (v; \"game.exe\")"),"missing game values stay empty");
        check(f.effects.written_text.ends_with(std::string("||0\0x",5)),"global content ID uses stored length including NUL");
    }
    {
        Fixture f;f.ticket_attributes();f.request.attributes["RequestTicket"]=std::string("a\0ignored",9);
        f.request.attributes["TicketEngine"]=std::string("b\0ignored",9);
        check(thrown<Exit>([&]{f.run();}).code==-1,"C-string ticket inputs processed");
        check(f.effects.written_text.ends_with("\n\na|b|fixture-content"),"ticket attributes truncate at NUL");
    }
    for(const std::string& point:std::vector<std::string>{"hash:C:\\fixture\\game.exe","filename:C:\\fixture\\game.exe","wine","log_string","widen:Denuvo_ticket_","time:null,true","write","dialog:Ticket saved"}) {
        Fixture f;f.ticket_attributes();f.effects.injected=point;
        thrown<Injected>([&]{f.run();});
        check(f.blocked==1&&f.attempts==2,"late boundary exception preserves block and completed attempts");
        check(f.xml.calls.empty(),"late boundary exceptions fabricate no XML license success");
    }
}
}
int main() {
    try {
        test_auth_and_invalidation();test_guard_order();test_attempts_and_blocking();test_ticket_guards();test_ticket_construction_and_effects();
        std::cout<<"PASS "<<checked<<" deterministic licensing checks (synthetic dependencies; original targets not executed)\n";
        return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
