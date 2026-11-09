// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/router/router.hpp"
#include "citrine_eclipse_mirage/codec/crypto.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include "citrine_eclipse_mirage/core/license.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace msg=citrine_eclipse_mirage::messages;
namespace xml=citrine_eclipse_mirage::xml;
namespace info=citrine_eclipse_mirage::handlers;
namespace misc=citrine_eclipse_mirage::misc_handlers;
namespace content=citrine_eclipse_mirage::content_handlers;
namespace dispatch=citrine_eclipse_mirage::dispatch;
namespace crypto=citrine_eclipse_mirage::crypto;
namespace config=citrine_eclipse_mirage::configuration;
namespace reader=citrine_eclipse_mirage::xml_reader_bridge;
namespace license=citrine_eclipse_mirage::license;
namespace {
int checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]] void unused(){throw std::logic_error("Unexpected synthetic dependency use");}
struct InvalidMessage{};
struct MissingLicense{};
using NoResponseSignal=msg::NoResponseRequiredError;
struct Node final:msg::XmlNode {
    std::string tag,text;
    bool has_text=false;
    std::vector<std::pair<std::string,std::string>> attrs;
    const Node* child=nullptr;
    const Node* next=nullptr;
    explicit Node(std::string name):tag(std::move(name)){}
    std::string_view name()const override{return tag;}
    const msg::XmlNode* first_element()const override{return child;}
    const msg::XmlNode* next_element()const override{return next;}
    std::optional<std::string_view> attribute(std::string_view key)const override {
        for(const auto& [name,value]:attrs)if(name==key)return value;
        return std::nullopt;
    }
};
struct Document final:msg::XmlDocument {
    Node root{"LSX"},request{"Request"},command;
    Document(std::string command_name,std::vector<std::pair<std::string,std::string>> attributes)
        :command(std::move(command_name)) {
        root.child=&request;request.child=&command;
        request.attrs={{"id","42"},{"recipient","EbisuSDK"}};command.attrs=std::move(attributes);
    }
    const msg::XmlNode* first_element()const override{return &root;}
    std::string_view error_name()const override{return "synthetic fixture";}
};
// Explicit fixture reader, NOT an XML parser. It accepts precisely the supplied
// plaintext fixture and constructs its already-decoded tree. Serialization and
// encryption below use actual reconstructed code.
struct Backend final:msg::XmlBackend {
    std::string expected,command="GetInternetConnectedState";
    std::vector<std::pair<std::string,std::string>> attrs;
    unsigned reads=0;
    msg::ParseResult parse_c_string(std::string_view input)override {
        check(input==expected,"decrypted request differs from supplied fixture");++reads;
        return {std::make_unique<Document>(command,attrs),0,false};
    }
    std::unique_ptr<msg::XmlPrinter> printer()override{return std::make_unique<xml::MemoryPrinter>();}
};
struct Nodes final:dispatch::NodeAccess {
    std::vector<std::pair<std::string,std::string>> attributes_in_order(const msg::XmlNode& n)override {
        return dynamic_cast<const Node&>(n).attrs;
    }
    const char* text_003860(const msg::XmlNode& n)override {
        const auto& typed=dynamic_cast<const Node&>(n);return typed.has_text?typed.text.c_str():nullptr;
    }
};
struct Information final:info::Effects {
    void log_profile_name_084f48(std::string_view)override{unused();}
    std::string current_time_041b60()override{return "2000-01-01T00:00:00";}
};
struct Mutex final:content::Mutex {
    std::int32_t count=0;unsigned locks=0,unlocks=0;
    int lock_04a6ac()override{++locks;return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{++unlocks;}
    [[noreturn]] void thread_error_04a648(int)override{unused();}
};
struct ContentEvents final:content::EventQueue {
    content::Mutex& mutex_02c620()override{unused();}
    bool initialized_locked()const override{unused();}
    void initialize_locked(std::string)override{unused();}
    void push_persisted_locked(std::string,const std::function<void()>&)override{unused();}
    void push_locked_034660(std::string)override{unused();}
};
struct ContentEffects final:content::Effects {
    std::int32_t atoi_05ee08(std::string_view)override{unused();}
    void unknown_entitlement_field(std::string_view)override{unused();}
    std::string utc_time_041b60(std::uint64_t&)override{unused();}
    std::unique_ptr<content::DocumentWriter> printer_0019b0()override{unused();}
    void write_xml_03c1d0(const content::Path&,std::string_view)override{unused();}
    void save_steam_01f9a0(const content::AchievementMap&)override{unused();}
    content::Path filesystem_widen_006d70(std::string_view)override{unused();}
    content::Path path_append_016b50(const content::Path&,const content::Path&)override{unused();}
    bool online_avatar_virtual_28(const content::Path&)override{unused();}
    bool copy_overwrite_048d20(const content::Path&,const content::Path&)override{unused();}
    std::string path_to_utf8_031ca0(const content::Path&)override{unused();}
    void log_saved_size_085530(std::int32_t)override{unused();}
};
struct Igo final:misc::IgoEffects {
    int lock_02c620_04a6ac()override{unused();}
    std::int32_t& lock_count_4c()override{unused();}
    void acquire_event_queue_02c400()override{unused();}
    void enqueue_034660(std::string&&)override{unused();}
    void unlock_04a6b4()override{unused();}
    [[noreturn]] void thread_error_04a648(int)override{unused();}
};
struct NoResponseBoundary final:misc::NoResponse {
    [[noreturn]] void throw_no_response_02bcd0_04d228()override{throw NoResponseSignal{};}
};
struct LicenseBoundary final:dispatch::LicenseHandlers {
    unsigned calls=0;
    bool auth_code_02e3b0(const msg::XmlNode&,info::Xml&)override{++calls;throw MissingLicense{};}
    bool request_license_0320c0(const msg::XmlNode&,info::Xml&)override{++calls;throw MissingLicense{};}
    bool invalidate_license_030c50(const msg::XmlNode&,info::Xml&)override{++calls;throw MissingLicense{};}
};
// Synthetic provider/filesystem outcomes for integration tests only. Production
// Handlers has no fallback implementation of these required effects.
struct LicenseEffects final:license::Effects {
    unsigned statuses=0,hashes=0,token_reads=0;
    const char* online_auth_virtual_30(const char*)override{return "synthetic-online-auth";}
    license::FileStatus status_01ea20(const license::Path&,std::uint32_t flags)override{
        check(flags==3,"license status flags");++statuses;return {2,0};
    }
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const license::Path&)override{unused();}
    bool file_hash_matches_042d20(const license::Path&,std::string)override{++hashes;return true;}
    std::string load_token_02afa0(const license::Path&)override{++token_reads;return {};}
    license::Path filesystem_widen_006d70(std::string_view text)override{return license::Path(text.begin(),text.end());}
    license::Path filename_00e120(const license::Path&)override{unused();}
    license::Path parent_path_019820(const license::Path&)override{unused();}
    license::Path join_016b00(const license::Path&,const license::Path&)override{unused();}
    std::string path_utf8_01e3f0(const license::Path&)override{unused();}
    std::string current_time_041b60_null_true()override{unused();}
    bool wine_detected_045010()override{unused();}
    void log_literal_006880(std::string_view)override{unused();}
    void log_string_006860(std::string_view)override{unused();}
    bool write_ticket_03c1d0(const license::Path&,std::string_view)override{unused();}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{unused();}
    [[noreturn]] void exit_05cdc0(std::int32_t)override{unused();}
    [[noreturn]] void no_response_02bcd0_04d228()override{throw NoResponseSignal{};}
};
struct EmptyEvents final:msg::EventQueue {
    bool nonempty_relaxed()const override{return false;}
    int lock_04a6ac()override{unused();}
    std::int32_t& lock_count_4c()override{unused();}
    bool nonempty_locked()const override{unused();}
    std::string copy_front_locked()const override{unused();}
    void pop_front_locked()override{unused();}
    void unlock_04a6b4()override{unused();}
};
struct Clock final:crypto::TimeSource {std::int64_t unix_seconds_05eeb8()override{return 946684800;}};
struct Effects final:msg::Effects {
    dispatch::Router& router;
    std::vector<std::string> bad_xml;
    unsigned decrypt_failures=0;
    explicit Effects(dispatch::Router& value):router(value){}
    int atoi_05ee08(std::string_view value)override{return std::atoi(std::string(value).c_str());}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode& node,msg::XmlPrinter& printer)override {
        return router.registered_handler(rva,node,printer);
    }
    [[noreturn]] void invalid_message_02bd30_04d228()override{throw InvalidMessage{};}
    [[noreturn]] void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{unused();}
    void log_response(std::string_view)override{unused();}
    void log_event(std::string_view)override{unused();}
    void log_bad_xml(std::string_view diagnostic,std::string_view)override{bad_xml.emplace_back(diagnostic);}
    void log_decryption_failure(std::string_view)override{++decrypt_failures;}
};
struct Fixture {
    info::Attributes game{{"DISPLAY_NAME","Synthetic & Game"}},settings;
    info::Profile profile{};
    std::uint8_t online=1,pretend=0,encrypted=0,show=0;
    std::vector<config::Content> contents{{"fixture-id","A & B","1.2","READY"}};
    std::vector<config::Entitlement> entitlements{{"skip","group","v","type","a"},{"chosen","group","v","type","tag"}};
    content::AchievementMap achievements;
    std::string set="set",content_id="fixture-id",user_id="fixture-user";
    content::Path path;
    std::int32_t saved_size=0;
    Mutex mutex;
    ContentEvents content_events;
    Nodes nodes;Information information;Igo igo;NoResponseBoundary no_response;
    ContentEffects content_effects;LicenseBoundary licensing;
    reader::Backend actual_backend;
    std::string auth="synthetic<&",license_value,dll_hash=std::string(40,'a'),exe_hash=std::string(40,'b');
    license::Path dll_path=u"fixture.dll",executable_path=u"fixture.exe",license_base=u"fixture";
    std::uint8_t fake_auth=0,license_blocked=0;
    std::int32_t attempted_files=0;
    LicenseEffects license_effects;
    license::Handlers license_core{{auth,online,fake_auth,license_value,dll_hash,exe_hash,content_id,
        dll_path,executable_path,license_base,game,attempted_files,license_blocked},license_effects};
    dispatch::Router router;
    Effects effects;
    Backend backend;
    crypto::Cipher cipher;crypto::CryptoFactory factory;
    Clock clock;crypto::Random random{clock};
    EmptyEvents events;
    explicit Fixture(bool actual_reader=false,bool actual_licensing=false):router({game,settings,profile,online,pretend},
        {contents,entitlements,game,achievements,set,content_id,user_id,path,path,path,path,path,
         online,saved_size,mutex,content_events},
        {actual_reader?static_cast<dispatch::NodeAccess&>(actual_backend):static_cast<dispatch::NodeAccess&>(nodes),information,igo,no_response,content_effects,
         actual_licensing?static_cast<dispatch::LicenseHandlers&>(license_core):static_cast<dispatch::LicenseHandlers&>(licensing)}),effects(router){}
    std::string process(std::string command,bool encrypt=false) {
        backend.command=command;
        backend.expected="<LSX><Request id=\"42\" recipient=\"EbisuSDK\"><"+command+"/></Request></LSX>";
        std::string input=backend.expected;
        if(encrypt){check(cipher.encrypt_042610(input),"request encrypt");msg::hex_encode_03d190(input);}
        encrypted=encrypt;
        return msg::process_message_033d70({encrypted,show,cipher,events},input,backend,factory,random,effects);
    }
    std::string process_actual(std::string input,bool encrypt=false,bool auto_detect=false) {
        encrypted=encrypt&&!auto_detect;
        if(encrypt){check(cipher.encrypt_042610(input),"actual request encrypt");msg::hex_encode_03d190(input);}
        return msg::process_message_033d70({encrypted,show,cipher,events},input,actual_backend,factory,random,effects);
    }
};
std::string wrapped(std::string_view body) {
    return "<LSX>\n    <Response id=\"42\" sender=\"EbisuSDK\">\n"+std::string(body)+
           "    </Response>\n</LSX>\n";
}
void plaintext_and_aes() {
    Fixture f;
    const auto expected=wrapped("        <InternetConnectedState connected=\"1\"/>\n");
    auto actual=f.process("GetInternetConnectedState");
    check(actual==expected+std::string(1,'\0'),"plaintext exact XML/NUL");
    check(f.backend.reads==1,"fixture reader invoked once");
    f.online=0;
    check(f.process("GetInternetConnectedState")==wrapped("        <InternetConnectedState connected=\"0\"/>\n")+std::string(1,'\0'),"live state reference");
    f.pretend=1;
    crypto::Key key{};for(unsigned i=0;i<key.size();++i)key[i]=static_cast<std::uint8_t>(i);
    f.cipher.set_key_042780(key);
    actual=f.process("GetInternetConnectedState",true);
    check(actual.size()>1&&actual.back()=='\0',"encrypted NUL frame");actual.pop_back();
    check(actual.find_first_not_of("0123456789abcdef")==std::string::npos,"encrypted response hex");
    msg::hex_decode_03ce30(actual);
    crypto::Cipher independent_receiver;independent_receiver.set_key_042780(key);
    check(independent_receiver.decrypt_042480(actual),"response decrypt/padding");
    check(actual==expected,"AES dispatcher handler serializer response matches plaintext");
}
void alias_content_and_errors() {
    Fixture f;
    const auto success=wrapped("        <ErrorSuccess Code=\"0\" Description=\"\"/>\n")+std::string(1,'\0');
    for(const auto* name:{"GoOnline","SetPresenceVisibility","SetPresence","SetDownloaderUtilization","SelectStore"})
        check(f.process(name)==success,"shared success handler aliases");
    const auto content_response=wrapped("        <QueryContentResponse>\n            <Game progressValue=\"0\" contentID=\"fixture-id\" availableVersion=\"1.2\" displayName=\"A &amp; B\" state=\"READY\" installedVersion=\"1.2\"/>\n        </QueryContentResponse>\n");
    check(f.process("QueryContent")==content_response+std::string(1,'\0'),"content state to actual escaped XML");
    // QueryImage is incomplete without its required attributes. The dispatcher
    // must propagate invalid-message instead of manufacturing success.
    bool invalid=false;try{(void)f.process("QueryImage");}catch(const InvalidMessage&){invalid=true;}
    check(invalid,"missing image attributes rejected");
    check(f.process("QueryChunkStatus").empty(),"recognized no-response command returns no wire frame");
    for(const auto* name:{"GetAuthCode","RequestLicense","InvalidateLicense"}) {
        bool missing=false;try{(void)f.process(name);}catch(const MissingLicense&){missing=true;}
        check(missing,"license body remains required external dependency");
    }
    check(f.licensing.calls==3,"all licensing routes forwarded");
    Node n{"Unregistered"};xml::MemoryPrinter printer;
    bool unknown=false;try{(void)f.router.registered_handler(0,n,printer);}catch(const std::invalid_argument&){unknown=true;}
    check(unknown,"unknown RVA is integration error");
}
void actual_reader_pipeline() {
    Fixture f(true);
    for(const auto* command:{"QueryPresence","AreChunksInstalled"}) {
        bool rejected=false;
        try{(void)f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><"+std::string(command)+"><Id/></"+command+"></Request></LSX>");}
        catch(const std::invalid_argument&){rejected=true;}
        check(rejected,"missing child text rejected before null C-string serialization");
        check(!f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><GoOnline/></Request></LSX>").empty(),"a rejected request does not poison later valid dispatch");
    }
    f.settings["fixture&key"]="value < & \"quote\"";
    const auto response=f.process_actual("<?xml version='1.0'?><!--comment--><LSX><Request id='a&amp;b' recipient='EbisuSDK'><GetSetting SettingId='fixture&amp;key'/></Request></LSX>");
    check(response=="<LSX>\n    <Response id=\"a&amp;b\" sender=\"EbisuSDK\">\n        <GetSettingResponse Setting=\"value &lt; &amp; &quot;quote&quot;\"/>\n    </Response>\n</LSX>\n"+std::string(1,'\0'),"real parser/entity lookup/router/writer pipeline");
    // Real child text and ordered content attributes traverse NodeAccess rather
    // than the original synthetic Node fixture.
    auto chunks=f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><AreChunksInstalled><Id><!--skip-->7&amp;9</Id><Id><![CDATA[<raw>]]></Id></AreChunksInstalled></Request></LSX>");
    check(chunks==wrapped("        <AreChunksInstalledResponse ItemId=\"\" Installed=\"true\">\n            <ChunkIds>7&amp;9</ChunkIds>\n            <ChunkIds>&lt;raw&gt;</ChunkIds>\n        </AreChunksInstalledResponse>\n")+std::string(1,'\0'),"real GetText to escaped text serialization");
    const auto filtered=f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><QueryEntitlements><FilterItems>tag</FilterItems></QueryEntitlements></Request></LSX>");
    check(filtered.find("ItemId=\"chosen\"")!=std::string::npos&&filtered.find("ItemId=\"skip\"")==std::string::npos,"real request-tree child filter");
    check(filtered.find("EntitlementId=\"1000000000002\"")!=std::string::npos,"filtered-out entry still increments id");
    crypto::Key key{};for(unsigned i=0;i<key.size();++i)key[i]=static_cast<std::uint8_t>(i);
    f.cipher.set_key_042780(key);
    auto encrypted=f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><GetInternetConnectedState/></Request></LSX>",true,true);
    check(f.encrypted==1,"real parser error8 switches to encrypted loop");
    check(!encrypted.empty()&&encrypted.back()=='\0',"actual parser encrypted frame NUL");
    encrypted.pop_back();msg::hex_decode_03ce30(encrypted);
    crypto::Cipher receiver;receiver.set_key_042780(key);
    check(receiver.decrypt_042480(encrypted),"actual parser response decrypt");
    check(encrypted==wrapped("        <InternetConnectedState connected=\"1\"/>\n"),"actual parser encrypted golden output");
    const auto bad=f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><GetSetting a=></Request></LSX>");
    check(bad.empty()&&f.effects.bad_xml.size()==1,"real error7 retry ends in logged failure");
    check(f.effects.bad_xml.back().find("Error=XML_ERROR_PARSING_ATTRIBUTE ErrorID=7")!=std::string::npos,"full ErrorStr propagated");
    bool rejected=false;try{(void)f.process_actual("<lsx><Request id='42' recipient='EbisuSDK'><GoOnline/></Request></lsx>");}catch(const InvalidMessage&){rejected=true;}
    check(rejected,"actual XML names remain case-sensitive");
    check(f.effects.decrypt_failures==0,"valid encrypted request did not use failure fallback");
    for(const std::string reference:{"prefix&#x100000000;suffix","prefix&#0;suffix","&amp;&#65;&x1f600;"}) {
        const auto id=reference=="&amp;&#65;&x1f600;"?"&A&x1f600;":reference;
        f.settings[id]="exact entity lookup";
        const auto literal=f.process_actual("<LSX><Request id='42' recipient='EbisuSDK'><GetSetting SettingId='"+reference+"'/></Request></LSX>");
        check(literal==wrapped("        <GetSettingResponse Setting=\"exact entity lookup\"/>\n")+std::string(1,'\0'),"entity reference preserves the exact settings key through actual router and serializer");
    }
}
void recovered_license_pipeline() {
    Fixture f(true,true);f.online=0;
    const std::string prefix="<LSX><Request id='42' recipient='EbisuSDK'>",suffix="</Request></LSX>";
    auto reply=f.process_actual(prefix+"<GetAuthCode ClientId=''/>"+suffix);
    check(reply==wrapped("        <ErrorSuccess Code=\"-1577058293\" Description=\"You need to be online to get an AuthCode.\"/>\n")+std::string(1,'\0'),"actual auth handler offline response");
    f.fake_auth=1;
    reply=f.process_actual(prefix+"<GetAuthCode ClientId='fixture-client'/>"+suffix);
    check(reply==wrapped("        <AuthCode value=\"synthetic&lt;&amp;\"/>\n")+std::string(1,'\0'),"actual auth configured fallback/escaping");
    f.license_value="synthetic-license";
    reply=f.process_actual(prefix+"<RequestLicense/>"+suffix);
    check(reply==wrapped("        <RequestLicenseResponse License=\"synthetic-license\"/>\n")+std::string(1,'\0'),"actual request-license cached state");
    check(f.license_effects.statuses==1&&f.license_effects.hashes==1&&f.license_effects.token_reads==0,"cached license still validates DLL before response");
    reply=f.process_actual(prefix+"<InvalidateLicense/>"+suffix);
    check(reply==wrapped("        <ErrorSuccess Code=\"0\" Description=\"License Deleted.\"/>\n")+std::string(1,'\0'),"actual invalidate-license response");
    check(f.license_value.empty()&&f.attempted_files==0&&f.license_blocked==0,"invalidation clears only in-memory token");
    bool invalid=false;try{(void)f.process_actual(prefix+"<RequestLicense/>"+suffix);}catch(const InvalidMessage&){invalid=true;}
    check(invalid&&f.license_blocked==1&&f.attempted_files==2&&f.license_effects.token_reads==2,"missing ticket after two empty synthetic files blocks subsequent request");
    check(f.process_actual(prefix+"<RequestLicense/>"+suffix).empty(),"blocked request emits no wire frame");
}
void actual_reader_no_response() {
    for(bool encrypted:{false,true}) {
        Fixture f(true);f.cipher.set_fixed_key_084c98();const auto key=f.cipher.round_keys();
        const std::string prefix="<LSX><Request id='42' recipient='EbisuSDK'>",suffix="</Request></LSX>";
        check(f.process_actual(prefix+"<QueryChunkStatus/>"+suffix,encrypted).empty(),"actual Router no-response emits no plaintext or AES wire frame");
        check(f.encrypted==encrypted&&f.cipher.round_keys()==key,"no-response retains real AES key and mode");
        auto reply=f.process_actual(prefix+"<GetInternetConnectedState/>"+suffix,encrypted);
        check(!reply.empty()&&reply.back()=='\0',"valid follow-up still produces a framed response");reply.pop_back();
        if(encrypted){msg::hex_decode_03ce30(reply);check(f.cipher.decrypt_042480(reply),"real AES follow-up decrypts");}
        check(reply==wrapped("        <InternetConnectedState connected=\"1\"/>\n"),"actual reader/Router follow-up retains exact valid response");
    }
}

}
int main(){try{plaintext_and_aes();alias_content_and_errors();actual_reader_pipeline();recovered_license_pipeline();actual_reader_no_response();
    std::cout<<"PASS: "<<checks<<" integration checks; actual XML compatibility reader/AES/dispatcher/handlers/serializer and synthetic boundaries; original targets not executed\n";
    return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
