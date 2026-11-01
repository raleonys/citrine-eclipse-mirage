#include "move.hpp"
#include "../anadius-session-source/handler.hpp"
#include "../anadius-session-source/routing.hpp"
#include "../anadius-socket-source/cipher_session.hpp"
#include "../anadius-config-defaults-source/defaults.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <new>

// Own EXE allocator observer: exact long greeting buffer only, never reads
// released storage. All metadata preserves the synthetic API's LastError.
namespace greeting_lifetime {
constinit std::atomic<void*> address=nullptr;
constinit std::atomic<bool> released=false;
constinit std::atomic<unsigned> deletions=0;
void* allocate(std::size_t size){const auto error=::GetLastError();void* value=std::malloc(size?size:1);::SetLastError(error);if(!value)throw std::bad_alloc();return value;}
void release(void* value)noexcept{const auto error=::GetLastError();void* expected=value;if(value&&address.compare_exchange_strong(expected,nullptr)){released.store(true);++deletions;}std::free(value);::SetLastError(error);}
void arm(const char* value)noexcept{const auto error=::GetLastError();released.store(false);deletions.store(0);address.store(const_cast<char*>(value));::SetLastError(error);}
void disarm()noexcept{const auto error=::GetLastError();address.store(nullptr);::SetLastError(error);}
}
void* operator new(std::size_t size){return greeting_lifetime::allocate(size);}
void* operator new[](std::size_t size){return greeting_lifetime::allocate(size);}
void operator delete(void* value)noexcept{greeting_lifetime::release(value);}
void operator delete[](void* value)noexcept{greeting_lifetime::release(value);}
void operator delete(void* value,std::size_t)noexcept{greeting_lifetime::release(value);}
void operator delete[](void* value,std::size_t)noexcept{greeting_lifetime::release(value);}

namespace {
using namespace anadius;namespace socket=socket_recovered;namespace server=server_recovered;namespace msg=messages_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected synthetic session dependency");}
struct Control final:socket::Control {
    unsigned destroys=0,deletes=0;Control(){strong=1;weak=1;resource=0x9876;}
    void destroy_resource()override{++destroys;}
    void delete_control()override{++deletes;}
};
struct NullControl final:socket::ControlFactory {socket::Control* allocate_04b0bc(std::uint32_t)override{return nullptr;}};
struct Gate final:socket::SocketGate {
    unsigned enters=0,leaves=0;bool held=false;
    void enter()override{if(held)throw std::logic_error("Unexpected recursive synthetic list gate");held=true;++enters;}
    void leave()override{if(!held)throw std::logic_error("Unmatched synthetic list gate");held=false;++leaves;}
};
struct Lifecycle final:socket::LifecycleEffects {
    socket::SocketService& service;std::vector<socket::Socket> closed;unsigned cleanups=0;
    explicit Lifecycle(socket::SocketService& value):service(value){}
    socket::SocketService& resolve_service_0239c0(socket::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(socket::SocketService& selected,socket::SocketObject& object)override{
        if(&selected!=&service)throw std::logic_error("Synthetic close service identity");
        if(object.state.socket!=socket::invalid_socket){closed.push_back(object.state.socket);object.state.socket=socket::invalid_socket;}
        NullControl empty;socket::replace_owner_0237f0(object.cancellation,0,empty);
    }
    void executor_cleanup(socket::SocketObject&,std::uintptr_t)override{++cleanups;}
};
struct SocketChain {
    Gate gate;socket::SocketService service{gate};Lifecycle lifecycle{service};Control control;
    std::array<socket::AcceptorState,3> states;
    socket::SocketObject pending{states[0]},temporary{states[1]},owned{states[2]};unsigned moves=0;bool temporary_destroyed=false,closed=false;
    static void executor(void* context,socket::ExecutorFields& destination,socket::ExecutorFields& source,std::uintptr_t table){
        auto& self=*static_cast<SocketChain*>(context);if(self.gate.held||table!=0x180084928ULL)throw std::logic_error("Synthetic executor slot8 projection");
        ++self.moves;destination.context=source.context;destination.zero_74=source.zero_74;destination.self_reference_80=true;
    }
    explicit SocketChain(bool move=true){
        socket::initialize_0211b0(pending,{0x1234},lifecycle);pending.state.socket=0xabc;pending.state.socket_state=0x11;
        pending.state.protocol_40=23;pending.state.byte_44=0x7a;pending.state.slot_48=0x1111;pending.state.slot_50=0x2222;pending.state.slot_58=0x3333;pending.state.slot_60=0x4444;
        pending.cancellation={0x9876,&control};
        if(move){socket::ExecutorMove lower{this,executor};socket::move_socket_023d20(temporary,pending,lower);socket::move_socket_023d20(owned,temporary,lower);}
    }
    void destroy_temporary(){socket::destroy_024db0(temporary,lifecycle);temporary_destroyed=true;}
    void shutdown(){
        if(closed)return;
        if(temporary.service&&!temporary_destroyed)destroy_temporary();
        if(owned.service)socket::destroy_024db0(owned,lifecycle);
        socket::destroy_024db0(pending,lifecycle);closed=true;
    }
    ~SocketChain(){if(!closed){try{shutdown();}catch(...){std::terminate();}}}
};
void two_moves_and_temporary_cleanup(){
    SocketChain chain;
    require(chain.moves==2&&chain.owned.state.socket==0xabc&&chain.pending.state.socket==socket::invalid_socket&&chain.temporary.state.socket==socket::invalid_socket,"Actual two023d20/028950 moves transfer sole live socket pending to temporary to owned");
    require(chain.service.head==&chain.owned&&chain.owned.next==&chain.temporary&&chain.temporary.next==&chain.pending&&chain.pending.previous==&chain.temporary,"Each move inserts a new list node; invalid pending and temporary remain registered");
    require(chain.owned.cancellation.control==&chain.control&&chain.owned.cancellation.resource==0x9876&&!chain.pending.cancellation.control&&!chain.temporary.cancellation.control&&chain.control.strong==1&&chain.control.weak==1&&!chain.control.destroys,"SC cancellation transfers leave one owned control reference without premature destruction");
    require(chain.owned.state.protocol_40==23&&chain.owned.state.byte_44==0x7a&&chain.owned.state.slot_48==0x1111&&chain.owned.state.slot_50==0x2222&&chain.owned.state.slot_58==0x3333&&chain.owned.state.slot_60==0x4444,"Actual moves preserve accepted protocol/options while both moved sources reset to protocol2 defaults");
    chain.destroy_temporary();
    require(chain.service.head==&chain.owned&&chain.owned.next==&chain.pending&&chain.pending.previous==&chain.owned&&!chain.temporary.next&&!chain.temporary.previous&&chain.lifecycle.closed.empty(),"Existing024db0 removes only invalid temporary node without closing transferred live handle");
    require(chain.owned.executor.context.value==0x1234&&chain.owned.executor.table_78==0x180084928ULL&&chain.owned.executor.table_90==0x180084860ULL&&chain.gate.enters==chain.gate.leaves&&!chain.gate.held,"Required executor payload provider remains explicit and source list gate is balanced");
    chain.shutdown();
    require(!chain.service.head&&chain.lifecycle.closed==std::vector<socket::Socket>{0xabc}&&chain.control.destroys==1&&chain.control.deletes==1&&chain.lifecycle.cleanups==3,"Explicit final destruction closes live owned socket once and releases cancellation once across all three nodes");
}
struct Mutex final:content_handlers_recovered::Mutex {
    std::int32_t count=1;int lock_04a6ac()override{unused();}std::int32_t& count_4c()override{return count;}void unlock_04a6b4()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
};
struct ContentEvents final:content_handlers_recovered::EventQueue {
    Mutex mutex;content_handlers_recovered::Mutex& mutex_02c620()override{return mutex;}bool initialized_locked()const override{unused();}void initialize_locked(std::string)override{unused();}void push_locked_034660(std::string)override{unused();}
};
struct MessageEvents final:msg::EventQueue {
    bool nonempty_relaxed()const override{return false;}int lock_04a6ac()override{unused();}std::int32_t& lock_count_4c()override{unused();}bool nonempty_locked()const override{unused();}std::string copy_front_locked()const override{unused();}void pop_front_locked()override{unused();}void unlock_04a6b4()override{unused();}
};
struct ContentEffects final:content_handlers_recovered::Effects {
    std::int32_t atoi_05ee08(std::string_view)override{unused();}void unknown_entitlement_field(std::string_view)override{unused();}std::string utc_time_041b60(std::uint64_t&)override{unused();}
    std::unique_ptr<content_handlers_recovered::DocumentWriter> printer_0019b0()override{unused();}void write_xml_03c1d0(const configuration::Path&,std::string_view)override{unused();}void save_steam_01f9a0(const content_handlers_recovered::AchievementMap&)override{unused();}
    configuration::Path filesystem_widen_006d70(std::string_view)override{unused();}configuration::Path path_append_016b50(const configuration::Path&,const configuration::Path&)override{unused();}
    bool online_avatar_virtual_28(const configuration::Path&)override{unused();}bool copy_overwrite_048d20(const configuration::Path&,const configuration::Path&)override{unused();}std::string path_to_utf8_031ca0(const configuration::Path&)override{unused();}void log_saved_size_085530(std::int32_t)override{unused();}
};
struct Information final:handlers_recovered::Effects {void log_profile_name_084f48(std::string_view)override{unused();}std::string current_time_041b60()override{unused();}};
struct Igo final:misc_handlers_recovered::IgoEffects {
    int lock_02c620_04a6ac()override{unused();}std::int32_t& lock_count_4c()override{unused();}void acquire_event_queue_02c400()override{unused();}void enqueue_034660(std::string&&)override{unused();}void unlock_04a6b4()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
};
struct NoResponse final:misc_handlers_recovered::NoResponse {[[noreturn]]void throw_no_response_02bcd0_04d228()override{unused();}};
struct Licenses final:dispatch_recovered::LicenseHandlers {
    bool auth_code_02e3b0(const msg::XmlNode&,handlers_recovered::Xml&)override{unused();}bool request_license_0320c0(const msg::XmlNode&,handlers_recovered::Xml&)override{unused();}bool invalidate_license_030c50(const msg::XmlNode&,handlers_recovered::Xml&)override{unused();}
};
struct Time final:crypto_recovered::TimeSource {std::int64_t unix_seconds_05eeb8()override{return 946684800;}};
struct MessageEffects final:session_bridge::RoutedEffects {
    unsigned routed=0;using RoutedEffects::RoutedEffects;
    int atoi_05ee08(std::string_view value)override{return std::atoi(std::string(value).c_str());}
    [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{}void log_response(std::string_view)override{++routed;}void log_event(std::string_view)override{unused();}void log_bad_xml(std::string_view,std::string_view)override{unused();}void log_decryption_failure(std::string_view)override{unused();}
};
struct CountingHandler final:server::Handler {
    server::Handler& lower;unsigned resets=0,greetings=0,messages=0;
    explicit CountingHandler(server::Handler& value):lower(value){}
    void reset_02bc70()override{++resets;lower.reset_02bc70();}
    std::string initial_message_02d540()override{++greetings;return lower.initial_message_02d540();}
    std::string process_message_033d70(std::string_view value)override{++messages;return lower.process_message_033d70(value);}
};
struct WriteFault {server::Error error;};
struct Scheduler final:server::Scheduling {
    std::deque<std::function<void()>> io,workers,writes;unsigned accepts=0,raised=0;std::vector<std::string> errors;
    void post_io(std::function<void()> value)override{io.push_back(std::move(value));}void launch_detached(std::function<void()> value)override{workers.push_back(std::move(value));}void dispatch_write(std::function<void()> value)override{writes.push_back(std::move(value));}
    void log_error(std::string_view operation,server::Error)override{errors.emplace_back(operation);}
    [[noreturn]]void raise_write_error(server::Error error)override{++raised;throw WriteFault{error};}
    static void one(std::deque<std::function<void()>>& queue){auto value=std::move(queue.front());queue.pop_front();value();}
};
struct Transport final:server::Transport {
    SocketChain& chain;std::function<void(server::Error,std::string)> receive;server::Completion send;std::span<const char> borrowed;
    std::string greeting,wire;std::size_t requested=0;unsigned sync_calls=0,reads=0,sends=0;bool first_read_saw_freed=false;DWORD first_read_error=0;server::Error sync_error;
    explicit Transport(SocketChain& value):chain(value){}
    void live(){if(chain.owned.state.socket!=0xabc)throw std::logic_error("Caller must retain transferred socket during semantic session callbacks");}
    server::WriteResult synchronous_send(std::span<const char> bytes)override{
        live();if(sync_calls++==0)greeting_lifetime::arm(bytes.data());const auto count=std::min(bytes.size(),sync_error.value?std::size_t{3}:std::size_t{17});greeting.append(bytes.data(),count);::SetLastError(0x4040);return {sync_error,count};
    }
    void async_receive(std::size_t size,std::function<void(server::Error,std::string)> value)override{
        live();if(receive)throw std::logic_error("Only one pending own receive");if(reads++==0){first_read_saw_freed=greeting_lifetime::released.load();first_read_error=::GetLastError();}requested=size;receive=std::move(value);
    }
    void async_send(std::span<const char> bytes,server::Completion value)override{live();if(send)throw std::logic_error("Only one pending own send");++sends;borrowed=bytes;send=std::move(value);}
    void feed(std::string bytes,server::Error error={}){if(!receive||bytes.size()>requested)throw std::logic_error("Own feed outside requested domain");auto value=std::move(receive);receive={};value(error,std::move(bytes));}
    void complete(){live();if(!send)throw std::logic_error("Own pending send missing");const auto count=std::min(borrowed.size(),std::size_t{13});wire.append(borrowed.data(),count);borrowed={};auto value=std::move(send);send={};value({},count);}
};
configuration::State state(){auto value=configuration_defaults::make_configuration_defaults_036d20();value.content.push_back({"OWN-id","OWN & Content","1.2","READY"});return value;}
struct App {
    SocketChain chain;configuration::State config=state();handlers_recovered::Profile profile{};std::uint8_t online=0,show=1;std::int32_t saved=0;configuration::Path achievement_file=u"OWN-unused";
    ContentEvents content_events;MessageEvents message_events;ContentEffects content_effects;Information information;Igo igo;NoResponse no_response;Licenses licenses;
    xml_reader_bridge::Backend xml;dispatch_recovered::Router router;MessageEffects effects;
    socket::SessionCipher cipher;socket::SessionCipherFactory factory;Time time;crypto_recovered::Random random{time};std::string version="before";
    session_bridge::MessageHandler actual;CountingHandler handler;Transport transport{chain};Scheduler scheduler;server::Session session;
    App():router({config.game_attributes,config.emulator_attributes,profile,online,config.pretend_connected},
        {config.content,config.entitlements,config.game_attributes,config.achievements,config.achievements_set,config.content_id,config.user_id,achievement_file,config.avatar,config.default_avatar,config.user_directory,config.last_avatar_path,online,saved,content_events.mutex,content_events},
        {xml,information,igo,no_response,content_effects,licenses}),effects(router),actual({cipher.encrypted_c4,cipher.unknown_c5,show,version,cipher,message_events},xml,factory,random,effects),handler(actual),session(transport,scheduler,handler){chain.destroy_temporary();}
    void drive(){unsigned budget=10000;while(!scheduler.io.empty()||!scheduler.workers.empty()||!scheduler.writes.empty()||transport.send){if(!budget--)throw std::logic_error("Own synthetic drain budget");if(!scheduler.io.empty())Scheduler::one(scheduler.io);else if(!scheduler.workers.empty())Scheduler::one(scheduler.workers);else if(!scheduler.writes.empty())Scheduler::one(scheduler.writes);else transport.complete();}}
    static std::string request(std::string_view command){return "<LSX><Request id='OWN' recipient='EbisuSDK'>"+std::string(command)+"</Request></LSX>";}
};
void semantic_session_with_transferred_socket(){
    App app;require(app.handler.resets==1&&app.cipher.encrypted_c4==0&&app.cipher.unknown_c5==0,"Caller-owned semantic Session constructor supplies exactly first actual MessageHandler/cipher reset");
    app.version.assign(8192,'V');app.cipher.encrypted_c4=7;app.cipher.unknown_c5=9;app.session.start_025720();
    require(app.handler.resets==2&&app.handler.greetings==1&&app.cipher.encrypted_c4==0&&app.cipher.unknown_c5==0&&app.transport.sync_calls>1,"Accepted start performs second reset and actual long live-version greeting write-all before read");
    require(app.transport.greeting.back()==0&&app.transport.greeting.find('\0')==app.transport.greeting.size()-1&&app.transport.greeting.find("version=\""+app.version+"\"")!=std::string::npos,"Actual XML greeting carries current long version and one framing NUL across partial sends");
    const auto parsed=app.xml.parse_c_string(app.transport.greeting);require(!parsed.has_error&&parsed.document->first_element()->name()=="LSX","Real compatibility XML reader validates actual MemoryPrinter greeting");
    auto challenge=App::request("<ChallengeResponse key='OWN-key' version='2'/>");challenge.push_back(0);app.transport.feed(challenge);app.drive();
    require(app.transport.wire.find("<ChallengeAccepted response=")!=std::string::npos&&app.handler.messages==1&&app.cipher.native().mode_00==2,"Actual MessageHandler challenge selects same SessionCipher key state before encrypted requests");
    app.transport.wire.clear();crypto_recovered::Cipher client;client.set_fixed_key_084c98();auto request=App::request("<QueryContent/>");if(!client.encrypt_042610(request))throw std::logic_error("Own client cipher failed");msg::hex_encode_03d190(request);request.push_back(0);
    app.transport.feed(request);app.drive();auto response=app.transport.wire;if(response.empty()||response.back()!=0)throw std::logic_error("Own response lacks frame");response.pop_back();msg::hex_decode_03ce30(response);const auto decrypted=client.decrypt_042480(response);
    require(decrypted&&response.find("contentID=\"OWN-id\"")!=std::string::npos&&response.find("OWN &amp; Content")!=std::string::npos&&app.cipher.encrypted_c4==1,"Transferred socket semantic pipeline reaches actual XML/Router/content serializer and coherent AES response");
    require(app.handler.messages==2&&app.session.queued_messages()==0&&app.session.buffered_input().empty()&&app.transport.sends>1,"Caller-retained Session drains detached/write queues and partial borrowed-span sends without replacing actual FIFO algorithms");
    app.transport.feed({}, {2,server::ErrorCategory::asio_misc});app.drive();
    require(!app.transport.receive&&!app.transport.send&&app.scheduler.io.empty()&&app.scheduler.workers.empty()&&app.scheduler.writes.empty()&&app.scheduler.errors.empty(),"Explicit EOF stops rearmed receive; all raw callback captures are drained while caller-owned references remain live");
    app.chain.shutdown();
    require(app.chain.lifecycle.closed==std::vector<socket::Socket>{0xabc}&&!app.chain.service.head&&app.chain.control.destroys==1&&app.chain.control.deletes==1,"Only after synthetic drain caller explicitly closes transferred socket and cancellation owner once");
    const auto deleted=greeting_lifetime::deletions.load();const auto freed=greeting_lifetime::released.load();greeting_lifetime::disarm();
    require(freed&&deleted==1&&app.transport.first_read_saw_freed,"Actual greeting heap buffer must be deleted exactly once before first async receive is submitted");
    require(app.transport.first_read_error==0x4040,"Allocator greeting-lifetime observer preserves final synthetic write LastError at initial read boundary");
}
struct AcceptServer final:server::ServerBackend {
    unsigned accepts=0;
    void initialize_context_0241b0()override{unused();}server::ParsedAddress parse_address_028690(std::uint16_t,std::string_view)override{unused();}void construct_acceptor_020e20(const server::Endpoint&,bool,int)override{unused();}void initialize_pending_socket_0211b0()override{unused();}server::LocalEndpoint local_endpoint()override{unused();}bool set_environment_variable(std::string_view,std::string_view)override{unused();}
    void async_accept_021b00()override{++accepts;}server::Error run_context_0298b0()override{unused();}void destroy_pending_socket_024db0()override{unused();}void destroy_acceptor_024db0()override{unused();}void destroy_context()override{unused();}[[noreturn]]void raise_error(server::Error,std::string_view)override{unused();}
};
void error_prefixes(){
    {SocketChain chain(false);AcceptServer server;Scheduler scheduler;server::accept_completion_025720({10061,server::ErrorCategory::system},nullptr,server,scheduler);
     require(server.accepts==1&&scheduler.errors==std::vector<std::string>{"accept"}&&chain.moves==0&&chain.pending.state.socket==0xabc&&chain.service.head==&chain.pending,"Actual semantic accept-error path logs/reaccepts without moving socket or dereferencing an absent prepared Session");chain.shutdown();}
    {App app;app.version.assign(2048,'E');app.transport.sync_error={7,server::ErrorCategory::system};bool caught=false;try{app.session.start_025720();}catch(const WriteFault& fault){caught=fault.error.value==7;}
     const auto still_live=app.chain.owned.state.socket==0xabc;app.chain.shutdown();greeting_lifetime::disarm();
     require(caught&&app.handler.resets==2&&app.handler.greetings==1&&app.transport.greeting.size()==3&&app.transport.sync_calls==1,"Greeting write error preserves completed byte prefix and both resets without an invented write retry");
     require(app.transport.reads==0&&!app.transport.receive&&app.scheduler.raised==1&&still_live&&app.chain.lifecycle.closed.size()==1,"Source start exception does not submit reads or implicitly close socket; caller later performs explicit cleanup");}
}
}
int main(){
    try{two_moves_and_temporary_cleanup();semantic_session_with_transferred_socket();error_prefixes();std::cout<<"PASS: "<<checks<<" socket move integration checks; actual two-move lifecycle and caller-retained semantic session pipeline\n";return 0;}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
