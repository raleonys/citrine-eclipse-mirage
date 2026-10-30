#include "preloader_l/net/session_handler.hpp"
#include "preloader_l/net/session_routing.hpp"
#include "preloader_l/runtime/achievement_runtime.hpp"
#include "preloader_l/runtime/event_adapters.hpp"
#include "preloader_l/codec/crypto.hpp"
#include "preloader_l/xml/reader.hpp"
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <stdexcept>
namespace msg=preloader_l::messages_recovered;
namespace srv=preloader_l::server_recovered;
namespace cfg=preloader_l::configuration;
namespace content=preloader_l::content_handlers_recovered;
namespace info=preloader_l::handlers_recovered;
namespace misc=preloader_l::misc_handlers_recovered;
namespace dispatch=preloader_l::dispatch_recovered;
namespace eq=preloader_l::event_queue_recovered;
namespace ar=preloader_l::achievement_runtime;
namespace crypto=preloader_l::crypto_recovered;
namespace bridge=preloader_l::session_bridge;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected session fixture dependency");}
struct Mutex final:content::Mutex {
    bool held=false;std::int32_t count=1;
    int lock_04a6ac()override{require(!held,"fixture mutex single owner");held=true;return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{require(held,"unlock held fixture mutex");held=false;}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
};
struct Initialization final:eq::Initialization {
    std::int32_t tls=std::numeric_limits<std::int32_t>::min(),epoch=tls;
    std::int32_t thread_epoch()override{return tls;}
    void header_04b838(std::int32_t& g)override{require(g==0,"cold fixture guard");g=-1;}
    void footer(std::int32_t& g)override{g=++epoch;tls=epoch;}
    void construct_mutex_04a698(eq::Mutex&,std::uint32_t flags)override{require(flags==2,"event mutex constructor flags");}
    int register_atexit(eq::Cleanup)override{return 0;} // Records no process callback.
};
struct Clock final:ar::Clock {
    std::int64_t time_05eeb8()override{return 946684800;}
    std::optional<ar::BrokenDown> gmtime_05fffc(std::int64_t)override{return ar::BrokenDown{0,0,0,1,0,100,6,0,0};}
};
struct Directories final:preloader_l::achievement_writer::Effects {void create_directories_018e00(const cfg::Path&)override{unused();}};
struct Files final:preloader_l::file_recovered::Factory {
    std::unique_ptr<preloader_l::file_recovered::Input> open_input_039f70(const cfg::Path&,std::uint32_t,std::uint32_t)override{unused();}
    std::unique_ptr<preloader_l::file_recovered::Output> open_output_03a1a0(const cfg::Path&,std::uint32_t,std::uint32_t)override{unused();}
};
struct ContentEffects final:ar::PersistenceEffects {
    using PersistenceEffects::PersistenceEffects;
    std::int32_t atoi_05ee08(std::string_view value)override{return std::atoi(std::string(value).c_str());}
    void unknown_entitlement_field(std::string_view)override{unused();}
    cfg::Path filesystem_widen_006d70(std::string_view)override{unused();}
    cfg::Path path_append_016b50(const cfg::Path&,const cfg::Path&)override{unused();}
    bool online_avatar_virtual_28(const cfg::Path&)override{unused();}
    bool copy_overwrite_048d20(const cfg::Path&,const cfg::Path&)override{unused();}
    std::string path_to_utf8_031ca0(const cfg::Path&)override{unused();}
    void log_saved_size_085530(std::int32_t)override{unused();}
};
struct Information final:info::Effects {
    void log_profile_name_084f48(std::string_view)override{unused();}
    std::string current_time_041b60()override{unused();}
};
struct NoResponse final:misc::NoResponse {[[noreturn]]void throw_no_response_02bcd0_04d228()override{unused();}};
struct Licenses final:dispatch::LicenseHandlers {
    bool auth_code_02e3b0(const msg::XmlNode&,info::Xml&)override{unused();}
    bool request_license_0320c0(const msg::XmlNode&,info::Xml&)override{unused();}
    bool invalidate_license_030c50(const msg::XmlNode&,info::Xml&)override{unused();}
};
struct Time final:crypto::TimeSource {std::int64_t unix_seconds_05eeb8()override{return 946684800;}};
struct Effects final:bridge::RoutedEffects {
    unsigned received=0,responses=0,decrypt_failures=0;std::vector<std::string> events;
    using RoutedEffects::RoutedEffects;
    int atoi_05ee08(std::string_view value)override{return std::atoi(std::string(value).c_str());}
    [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{++received;}
    void log_response(std::string_view)override{++responses;}
    void log_event(std::string_view event)override{events.emplace_back(event);}
    void log_bad_xml(std::string_view,std::string_view)override{unused();}
    void log_decryption_failure(std::string_view)override{++decrypt_failures;}
};
struct Scheduler final:srv::Scheduling {
    std::deque<std::function<void()>> io,workers,writes;unsigned launches=0;
    void post_io(std::function<void()> f)override{io.push_back(std::move(f));}
    void launch_detached(std::function<void()> f)override{++launches;workers.push_back(std::move(f));}
    void dispatch_write(std::function<void()> f)override{writes.push_back(std::move(f));}
    void log_error(std::string_view,srv::Error)override{unused();}
    [[noreturn]]void raise_write_error(srv::Error)override{unused();}
    void one(std::deque<std::function<void()>>& q){auto fn=std::move(q.front());q.pop_front();fn();}
};
struct Transport final:srv::Transport {
    std::function<void(srv::Error,std::string)> receive;srv::Completion send;
    std::size_t requested=0;std::string greeting,wire,pending;unsigned sync_calls=0,sends=0,empty_sends=0;
    srv::WriteResult synchronous_send(std::span<const char> bytes)override{++sync_calls;require(bytes.size()<=65536,"greeting chunks capped");const auto count=std::min(bytes.size(),std::size_t{5});greeting.append(bytes.data(),count);return {{},count};}
    void async_receive(std::size_t size,std::function<void(srv::Error,std::string)> f)override{require(!receive,"one pending receive");requested=size;receive=std::move(f);}
    void async_send(std::span<const char> bytes,srv::Completion f)override{require(!send,"one pending async send");++sends;if(bytes.empty())++empty_sends;pending.assign(bytes.data(),bytes.size());send=std::move(f);}
    void feed(std::string bytes){require(static_cast<bool>(receive) && bytes.size()<=requested,"feed respects pending receive capacity");auto fn=std::move(receive);receive={};fn({},std::move(bytes));}
    void complete(){require(static_cast<bool>(send),"pending async send");const auto count=std::min(pending.size(),std::size_t{13});wire.append(pending.data(),count);auto fn=std::move(send);send={};fn({},count);}
};
cfg::State fixture_state(){
    cfg::State state{};state.game_attributes["DISPLAY_NAME"]="Synthetic Game";
    state.content.push_back({"fixture-id","A & B","1.2","READY"});
    state.achievements_set="set";state.achievements.emplace("award",cfg::Achievement{1,0,0,"old","Award","1",0});
    return state;
}
struct Fixture {
    cfg::State state=fixture_state();info::Attributes settings;info::Profile profile{};std::uint8_t online=0,pretend=0,encrypted=7,unknown=9,show=1;std::int32_t saved=0;std::string version="fixture-v1";
    Mutex achievement,event_mutex;eq::Guards guards{0,0};Initialization initialization;eq::OwnedStorage storage;eq::Context context{guards,initialization,event_mutex,storage};eq::ContentQueue content_events{context};eq::IgoQueue igo{context};eq::MessageQueue message_events{context};
    preloader_l::achievement_storage::Paths paths;Clock clock;ar::HostFormatter formatter;Directories directories;Files files;ContentEffects content_effects{state,paths,clock,formatter,directories,files};
    preloader_l::xml_reader_bridge::Backend xml;Information information;NoResponse no_response;Licenses licenses;
    dispatch::Router router;Effects effects;crypto::Cipher cipher,client;crypto::CryptoFactory factory;Time time;crypto::Random random{time};bridge::MessageHandler handler;Transport transport;Scheduler scheduler;srv::Session session;
    Fixture():router({state.game_attributes,settings,profile,online,pretend},ar::bind_state(state,paths,online,saved,achievement,content_events),{xml,information,igo,no_response,content_effects,licenses}),effects(router),handler({encrypted,unknown,show,version,cipher,message_events},xml,factory,random,effects),session(transport,scheduler,handler){
        client.set_fixed_key_084c98();
    }
    void drive(){unsigned budget=10000;while(!scheduler.io.empty() || !scheduler.workers.empty() || !scheduler.writes.empty() || transport.send){require(budget--!=0,"finite fixture scheduler progress");if(!scheduler.io.empty())scheduler.one(scheduler.io);else if(!scheduler.workers.empty())scheduler.one(scheduler.workers);else if(!scheduler.writes.empty())scheduler.one(scheduler.writes);else transport.complete();}}
    static std::string request(std::string_view id,std::string_view command){return "<LSX><Request id='"+std::string(id)+"' recipient='EbisuSDK'>"+std::string(command)+"</Request></LSX>";}
    std::string encode(std::string value){require(client.encrypt_042610(value),"actual client AES encrypt");msg::hex_encode_03d190(value);value.push_back('\0');return value;}
    std::vector<std::string> take_frames(bool decrypt){std::vector<std::string> result;std::size_t pos=0;while(pos<transport.wire.size()){const auto end=transport.wire.find('\0',pos);require(end!=std::string::npos,"complete NUL-delimited output frame");auto frame=transport.wire.substr(pos,end-pos);if(decrypt){msg::hex_decode_03ce30(frame);require(client.decrypt_042480(frame),"actual independent client AES decrypt");}result.push_back(std::move(frame));pos=end+1;}transport.wire.clear();return result;}
};
void full_session(){
    Fixture f;require(f.encrypted==0 && f.unknown==0,"Session constructor reset reaches shared handler state");f.unknown=8;f.version="live-v2";f.session.start_025720();
    require(f.encrypted==0 && f.unknown==0 && f.transport.sync_calls>1,"accepted start second reset and partial greeting writeall");
    require(f.transport.greeting.back()=='\0' && f.transport.greeting.find('\0')==f.transport.greeting.size()-1,"wire greeting retains one final NUL");
    require(f.transport.greeting.find("version=\"live-v2\"")!=std::string::npos,"greeting reads current version reference");
    const auto greeting=f.xml.parse_c_string(f.transport.greeting);require(!greeting.has_error && greeting.document->first_element()->name()=="LSX","actual XML reader validates greeting");
    auto challenge=Fixture::request("c","<ChallengeResponse key='fixture-key' version='2'/>");challenge.push_back('\0');
    for(std::size_t i=0;i<challenge.size();++i){f.transport.feed(challenge.substr(i,1));if(i+1<challenge.size())require(f.scheduler.workers.empty() && f.transport.wire.empty(),"no worker before fragmented NUL delimiter");}
    require(f.scheduler.io.size()==1 && f.scheduler.workers.empty(),"frame completion posted rather than inline");f.drive();auto accepted=f.take_frames(false);
    require(accepted.size()==1 && accepted[0].find("<ChallengeAccepted response=")!=std::string::npos && f.encrypted==0,"plaintext challenge accepted before encrypted transition");
    auto expected_response=std::string("fixture-key");require(f.client.encrypt_042610(expected_response),"independent fixed-key challenge AES");msg::hex_encode_03d190(expected_response);require(accepted[0].find(expected_response)!=std::string::npos,"actual fixed-key challenge response golden value");
    auto igo=Fixture::request("i","<ShowIGO/>");igo.push_back('\0');f.transport.feed(igo);f.drive();auto plain=f.take_frames(false);require(plain.size()==1 && plain[0].find("ErrorSuccess")!=std::string::npos,"actual RoutedEffects IGO dispatch");
    require(f.storage.nonempty_relaxed() && f.effects.events.empty(),"plaintext IGO queues two DOWN events without consuming");
    f.transport.feed(f.encode(Fixture::request("q1","<QueryContent/>")));f.drive();auto first=f.take_frames(true);
    require(f.encrypted==1 && first.size()==2 && first[0].find("contentID=\"fixture-id\"")!=std::string::npos && first[0].find("A &amp; B")!=std::string::npos,"automatic encrypted transition reaches real Router/content serializer/AES");
    require(first[1]==eq::initial_down_event && f.storage.nonempty_relaxed(),"first encrypted response consumes exactly one of two queued DOWNs");
    auto joined=std::string(1,'\0')+f.encode(Fixture::request("q2","<QueryContent/>"))+f.encode(Fixture::request("g","<GrantAchievement AchievementId='award'/>"));
    f.transport.feed(joined);require(f.scheduler.workers.empty(),"coalesced receive still posts IO completion");f.drive();auto frames=f.take_frames(true);
    require(frames.size()==4 && frames[0].find("id=\"q2\"")!=std::string::npos && frames[1]==eq::initial_down_event && frames[2].find("id=\"g\"")!=std::string::npos,"coalesced requests preserve response/event FIFO, empty frame adds no response");
    require(frames[3].find("Achievement Id=\"award\"")!=std::string::npos && frames[3].find("Count=\"1\" Progress=\"1\"")!=std::string::npos,"actual grant queues and emits serialized completion event");
    require(f.state.achievements.at("award").field70==946684800 && !f.storage.nonempty_relaxed(),"grant shares state and consumes completion event once");
    require(f.session.queued_messages()==0 && f.session.buffered_input().empty() && !f.event_mutex.held && !f.achievement.held && f.transport.sends>10,"partial async sends drained with stable fixture lifetime and released locks");
    f.transport.feed(std::string("00\0",3));f.drive();require(f.effects.decrypt_failures==1 && f.transport.wire.empty() && f.transport.empty_sends==1,"decryption failure still dispatches empty response through Session write queue");
    require(f.effects.responses==5 && f.effects.events.size()==3,"actual logging sees five valid responses and three delivered events");
}
void isolated_sessions(){
    Fixture a,b;a.encrypted=1;a.unknown=3;b.encrypted=1;b.unknown=4;
    crypto::Key key{};key.fill(0x5a);b.cipher.set_key_042780(key);const auto before=b.cipher.round_keys();
    a.handler.reset_02bc70();
    require(a.encrypted==0 && a.unknown==0 && b.encrypted==1 && b.unknown==4,"one session reset leaves other flag cells untouched");
    require(b.cipher.round_keys()==before,"one session reset leaves other cipher state untouched");
    require(&a.storage!=&b.storage && a.guards.queue_0a39a0==0 && b.guards.queue_0a39a0==0,"fixture sessions explicitly own distinct queue/global domains");
}
}
int main(){try{full_session();isolated_sessions();std::cout<<"PASS: "<<checks<<" session integration checks; actual XML/AES/Router/handlers and memory transport only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
