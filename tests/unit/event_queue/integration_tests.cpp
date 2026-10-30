#include "preloader_l/runtime/event_adapters.hpp"
#include "preloader_l/runtime/achievement_runtime.hpp"
#include "preloader_l/xml/reader.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace eq=preloader_l::event_queue_recovered;
namespace msg=preloader_l::messages_recovered;
namespace content=preloader_l::content_handlers_recovered;
namespace ar=preloader_l::achievement_runtime;
namespace cfg=preloader_l::configuration;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected event test provider");}
struct Mutex final:eq::Mutex {
    std::vector<std::string>& trace;bool held=false;std::int32_t count=1;
    explicit Mutex(std::vector<std::string>& t):trace(t){}
    int lock_04a6ac()override{require(!held,"event mutex not double locked");trace.push_back("lock");held=true;return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{require(held,"unlock held event mutex");trace.push_back("unlock");held=false;}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
};
struct Runtime final:eq::Initialization {
    std::vector<std::string>& trace;std::int32_t tls=std::numeric_limits<std::int32_t>::min(),epoch=tls;
    explicit Runtime(std::vector<std::string>& t):trace(t){}
    std::int32_t thread_epoch()override{return tls;}
    void header_04b838(std::int32_t& guard)override{trace.push_back("header");require(guard==0,"synthetic cold guard0");guard=-1;}
    void footer(std::int32_t& guard)override{trace.push_back("footer");guard=++epoch;tls=epoch;}
    void construct_mutex_04a698(eq::Mutex&,std::uint32_t flags)override{require(flags==2,"mutexflags2");trace.push_back("construct-mutex");}
    int register_atexit(eq::Cleanup id)override{trace.push_back(id==eq::Cleanup::queue_07e000?"atexit-queue":"atexit-mutex");return 0;}
};
struct Storage final:eq::Storage {
    eq::OwnedStorage owned;Mutex& mutex;std::vector<std::string>& trace;bool expected_seed_locked=false;
    Storage(Mutex& m,std::vector<std::string>& t):mutex(m),trace(t){}
    void construct_seed_copy(std::string_view value)override{require(mutex.held==expected_seed_locked,"seed initialization lock position");trace.push_back("seed");owned.construct_seed_copy(value);}
    bool nonempty_relaxed()const override{trace.push_back("relaxed");return owned.nonempty_relaxed();}
    bool nonempty_locked()const override{require(mutex.held,"locked nonempty under event mutex");return owned.nonempty_locked();}
    std::string copy_front_locked()const override{require(mutex.held,"front copy under event mutex");trace.push_back("copy");return owned.copy_front_locked();}
    void push_persisted_locked(std::string& value,const std::function<void()>& persist)override {
        require(mutex.held,"transaction under event mutex");trace.push_back("push");owned.push_persisted_locked(value,persist);
    }
    bool front_available_locked()override{require(mutex.held,"delivery availability under mutex");return owned.front_available_locked();}
    std::uint64_t reserve_front_locked()override{require(mutex.held,"reservation under mutex");return owned.reserve_front_locked();}
    bool finish_reserved(std::uint64_t token,bool success)noexcept override{return owned.finish_reserved(token,success);}
    void push_move_034660(std::string& value)override{require(mutex.held,"producer push under event mutex");trace.push_back("push");owned.push_move_034660(value);}
    void pop_front_locked()override{require(mutex.held,"consumer pop under event mutex");trace.push_back("pop");owned.pop_front_locked();}
};
struct Clock final:ar::Clock {
    Mutex& achievement;explicit Clock(Mutex& m):achievement(m){}
    std::int64_t time_05eeb8()override{require(achievement.held,"grant clock under achievement mutex");return 123;}
    std::optional<ar::BrokenDown> gmtime_05fffc(std::int64_t)override{return ar::BrokenDown{0,0,0,1,0,100,6,0,0};}
};
struct Directories final:preloader_l::achievement_writer::Effects {void create_directories_018e00(const cfg::Path&)override{unused();}};
struct Files final:preloader_l::file_recovered::Factory {
    std::unique_ptr<preloader_l::file_recovered::Input> open_input_039f70(const cfg::Path&,std::uint32_t,std::uint32_t)override{unused();}
    std::unique_ptr<preloader_l::file_recovered::Output> open_output_03a1a0(const cfg::Path&,std::uint32_t,std::uint32_t)override{unused();}
};
struct ContentEffects final:ar::PersistenceEffects {
    using PersistenceEffects::PersistenceEffects;
    std::int32_t atoi_05ee08(std::string_view)override{return 1;}
    void unknown_entitlement_field(std::string_view)override{unused();}
    cfg::Path filesystem_widen_006d70(std::string_view)override{unused();}
    cfg::Path path_append_016b50(const cfg::Path&,const cfg::Path&)override{unused();}
    bool online_avatar_virtual_28(const cfg::Path&)override{unused();}
    bool copy_overwrite_048d20(const cfg::Path&,const cfg::Path&)override{unused();}
    std::string path_to_utf8_031ca0(const cfg::Path&)override{unused();}
    void log_saved_size_085530(std::int32_t)override{unused();}
};
struct Cipher final:msg::Cipher {
    Mutex& mutex;std::vector<std::string>& trace;bool fail_response=false,fail_event=false,fail_decrypt=false;std::vector<std::string> encrypted;
    Cipher(Mutex& m,std::vector<std::string>& t):mutex(m),trace(t){}
    void construct_0423d0_in_place()override{unused();}
    void set_fixed_key_084c98()override{unused();}
    void set_key_042780(const msg::Key&)override{unused();}
    bool decrypt_042480(std::string& value)override{value="<LSX><Request id=\"1\" recipient=\"EbisuSDK\"><GetConfig/></Request></LSX>";return !fail_decrypt;}
    bool encrypt_042610(std::string& value)override{
        const bool event=!encrypted.empty();require(mutex.held==event,"response before lock and event encryption under lock");trace.push_back(event?"encrypt-event":"encrypt-response");encrypted.push_back(value);return !(event?fail_event:fail_response);
    }
};
struct Factory final:msg::CryptoFactory {std::unique_ptr<msg::Cipher> create_0423d0()override{unused();}};
struct Random final:msg::Random {msg::Key challenge_bytes_034cf0()override{unused();}msg::Key key_from_response_034c30(std::string_view)override{unused();}};
struct Effects final:msg::Effects {
    std::vector<std::string> events;
    int atoi_05ee08(std::string_view)override{return 0;}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode&,msg::XmlPrinter& printer)override{require(rva!=0,"registered message handler reached");printer.open("ok");printer.close();return true;}
    [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{}
    void log_response(std::string_view)override{}
    void log_event(std::string_view value)override{events.emplace_back(value);}
    void log_bad_xml(std::string_view,std::string_view)override{unused();}
    void log_decryption_failure(std::string_view)override{}
};
struct Fixture {
    std::vector<std::string> trace,achievement_trace;Mutex mutex{trace},achievement{achievement_trace};Runtime runtime{trace};Storage storage{mutex,trace};eq::Guards guards{0,0};eq::Context context{guards,runtime,mutex,storage};
    eq::IgoQueue igo{context};eq::ContentQueue content_queue{context};eq::MessageQueue messages{context};
    cfg::State state{};preloader_l::achievement_storage::Paths paths;Clock clock{achievement};ar::HostFormatter formatter;Directories directories;Files files;ContentEffects content_effects{state,paths,clock,formatter,directories,files};
    preloader_l::xml_reader_bridge::Backend xml;Factory factory;Random random;Effects effects;std::uint8_t encrypted=1,show=1,online=0;std::int32_t saved=0;
    Fixture(){state.achievements_set="set";state.game_attributes["DISPLAY_NAME"]="Game";state.achievements.emplace("a",cfg::Achievement{1,0,0,"old","Award","1",0});}
    void show_igo(){auto response=content_effects.printer_0019b0();require(preloader_l::misc_handlers_recovered::show_igo_032b10(response->xml(),igo),"real IGO handler succeeds");}
    void grant(){auto response=content_effects.printer_0019b0();const auto refs=ar::bind_state(state,paths,online,saved,achievement,content_queue);content::RequestTree request;request.attributes={{"AchievementId","a"}};require(content::grant_achievement_030010(request,response->xml(),refs,content_effects),"real grant handler succeeds");require(!achievement.held && !mutex.held,"grant releases both mutexes");}
    std::string process(Cipher& cipher,msg::EventDelivery* delivery=nullptr){std::string input=encrypted?"00":"<LSX><Request id=\"1\" recipient=\"EbisuSDK\"><GetConfig/></Request></LSX>";return msg::process_message_033d70({encrypted,show,cipher,messages,delivery},input,xml,factory,random,effects);}
};
std::vector<std::string> frames(std::string_view wire){std::vector<std::string> result;std::size_t pos=0;while(pos<wire.size()){const auto end=wire.find('\0',pos);require(end!=std::string_view::npos,"wire frames have NUL terminator");std::string frame(wire.substr(pos,end-pos));msg::hex_decode_03ce30(frame);result.push_back(std::move(frame));pos=end+1;}return result;}
std::vector<std::string> cleanup_trace(const std::vector<std::string>& trace){std::vector<std::string> result;for(const auto& entry:trace)if(entry.starts_with("atexit-"))result.push_back(entry);return result;}
void cold_consumer(){
    Fixture f;Cipher cipher(f.mutex,f.trace);const auto wire=f.process(cipher);const auto values=frames(wire);
    require(values.size()==2 && values[1]==eq::initial_down_event,"cold encrypted response consumes its lazy initial DOWN event");
    require(cipher.encrypted.size()==2 && f.effects.events==std::vector<std::string>{std::string(eq::initial_down_event)},"exact one event logged and encrypted");
    require(!f.storage.owned.nonempty_relaxed() && !f.mutex.held,"single initial event popped and mutex released");
    const auto seed=std::find(f.trace.begin(),f.trace.end(),"seed"),lock=std::find(f.trace.begin(),f.trace.end(),"lock");require(seed<lock,"consumer initializes queue before mutex acquisition");
    require(cleanup_trace(f.trace)==std::vector<std::string>{"atexit-queue","atexit-mutex"},"cold consumer registers queue cleanup before mutex cleanup");
}
void shared_producers(){
    Fixture f;f.storage.expected_seed_locked=true;f.show_igo();f.grant();
    require(cleanup_trace(f.trace)==std::vector<std::string>{"atexit-mutex","atexit-queue"},"cold producer registers mutex cleanup before queue cleanup");
    std::vector<std::string> delivered;
    for(unsigned i=0;i<3;++i){Cipher cipher(f.mutex,f.trace);const auto values=frames(f.process(cipher));require(values.size()==2,"one queued event per encrypted response");delivered.push_back(values[1]);}
    require(delivered[0]==eq::initial_down_event && delivered[1]==eq::initial_down_event,"cold IGO has seed DOWN plus explicit DOWN");
    require(delivered[2].find("<Achievement Id=\"a\"")!=std::string::npos && delivered[2].find("Count=\"1\" Progress=\"1\"")!=std::string::npos,"same queue carries actual grant XML event after IGO events");
    require(!f.storage.owned.nonempty_relaxed(),"shared producer events fully consumed");
    Cipher empty_cipher(f.mutex,f.trace);require(frames(f.process(empty_cipher)).size()==1 && empty_cipher.encrypted.size()==1,"empty hot queue adds no event frame");
    require(std::count(f.trace.begin(),f.trace.end(),"seed")==1 && std::count(f.trace.begin(),f.trace.end(),"construct-mutex")==1,"shared getters initialize once across all adapters");
    Fixture grant;grant.storage.expected_seed_locked=true;grant.grant();Cipher first(grant.mutex,grant.trace);require(frames(grant.process(first))[1]==eq::initial_down_event,"cold grant first emits initial DOWN");Cipher second(grant.mutex,grant.trace);require(frames(grant.process(second))[1].find("<Achievement Id=\"a\"")!=std::string::npos,"cold grant next emits completion event");
}
void delivery_reservations(){
    Fixture f;Cipher first(f.mutex,f.trace);msg::EventDelivery pending;
    const auto wire=f.process(first,&pending);
    require(frames(wire).size()==2&&pending.pending(),"prepared event remains reserved before send");
    Cipher parallel(f.mutex,f.trace);msg::EventDelivery other;
    require(frames(f.process(parallel,&other)).size()==1&&!other.pending(),"another session cannot duplicate a reserved event");
    require(pending.finish(false),"failed send releases reservation without dequeue");
    Cipher retry(f.mutex,f.trace);msg::EventDelivery retried;
    require(frames(f.process(retry,&retried))[1]==eq::initial_down_event,"next session retries the exact retained plaintext event");
    require(retried.finish(true)&&retried.finish(true),"full send commits once; repeated finish is harmless");
    Cipher after(f.mutex,f.trace);msg::EventDelivery empty;
    require(frames(f.process(after,&empty)).size()==1&&!f.storage.owned.nonempty_relaxed(),"completed event retires at next locked queue access");
    Fixture failed;Cipher bad(failed.mutex,failed.trace);bad.fail_event=true;msg::EventDelivery no_event;
    require(frames(failed.process(bad,&no_event)).size()==1&&!no_event.pending(),"failed encryption releases reservation");
    Cipher good(failed.mutex,failed.trace);msg::EventDelivery restored;
    require(frames(failed.process(good,&restored)).size()==2,"encryption failure preserves event for retry");
    require(!failed.mutex.held&&!f.mutex.held,"all reservation branches release queue mutex");
}
void failure_modes(){
    {Fixture f;Cipher cipher(f.mutex,f.trace);cipher.fail_event=true;require(frames(f.process(cipher)).size()==1,"failed event encryption omits event frame");require(!f.storage.owned.nonempty_relaxed() && !f.mutex.held,"failed event encryption still pops exactly one and unlocks");}
    {Fixture f;Cipher cipher(f.mutex,f.trace);cipher.fail_response=true;require(f.process(cipher).empty(),"failed response encryption returns empty");require(f.guards.queue_0a39a0==0 && f.guards.mutex_0a3970==0 && f.trace==std::vector<std::string>{"encrypt-response"},"response failure does not initialize or lock queue");}
    {Fixture f;Cipher cipher(f.mutex,f.trace);cipher.fail_decrypt=true;require(f.process(cipher).empty() && f.trace.empty() && f.guards.queue_0a39a0==0,"decrypt failure never accesses event queue");}
    {Fixture f;f.encrypted=0;Cipher cipher(f.mutex,f.trace);const auto wire=f.process(cipher);require(!wire.empty() && wire.back()=='\0' && cipher.encrypted.empty() && f.trace.empty() && f.guards.queue_0a39a0==0,"unencrypted response leaves queue uninitialized");}
}
}
int main(){try{cold_consumer();shared_producers();failure_modes();delivery_reservations();std::cout<<"PASS: "<<checks<<" event queue integration checks; shared synthetic guards and controlled cipher only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
