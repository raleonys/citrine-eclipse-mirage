#include "context_test_fixture.hpp"
#include <csignal>

namespace {
using namespace b15_test;
void guards_factory(){
    Driver d;
    rejects<std::invalid_argument>([&]{nc::Owner o(d.view(),seed(),{},permanent_errors,[]{return true;});},"missing policy factory rejected");
    rejects<std::invalid_argument>([&]{nc::Owner o(d.view(),seed(),d.policies(),permanent_errors,{});},"missing quiescence promise rejected");
    rejects<std::invalid_argument>([&]{nc::Owner o(d.view(),seed(),[](nc::PolicyView){return std::unique_ptr<n::OriginalEffects>{};},permanent_errors,[]{return true;});},"null policy owner rejected");
    const auto copied=d.runtime.originals;
    rejects<std::invalid_argument>([&]{nc::Owner o(d.view(),seed(),[&](nc::PolicyView view){return std::make_unique<Policy>(view,d.probe,&copied);},permanent_errors,[]{return true;});},"copied original array rejected even when all values match");
    require(!n::is_bound(),"rejected constructors never publish native Context");
    rejects<std::runtime_error>([&]{nc::Owner o(d.view(),seed(),[](nc::PolicyView)->std::unique_ptr<n::OriginalEffects>{throw std::runtime_error("factory failure");},permanent_errors,[]{return true;});},"factory exception propagates before publication");
    rejects<std::invalid_argument>([&]{(void)nc::make_factory(seed(),{},permanent_errors,[]{return true;});},"factory builder rejects empty policies");
    auto factory=nc::make_factory(seed(),d.policies(),permanent_errors,[]{return true;});auto copy=factory;
    auto owner=factory(d.view());
    rejects<std::logic_error>([&]{(void)copy(d.view());},"copied factory shares one-shot consumption guard");
    require(!n::is_bound()&&owner->close_after_cleanup(),"unpublished owner closes without publishing or calling native entry");
    rejects<std::logic_error>([&]{owner->publish();},"closed owner cannot publish");
}
void exact_context_and_close(){
    Driver d;bool quiescent=false;unsigned proofs=0;
    nc::Owner owner(d.view(),seed(),d.policies(),permanent_errors,[&]{++proofs;return quiescent;});
    require(d.probe->runtime==&d.runtime&&d.probe->config==&d.configuration&&d.probe->execution==&d.execution,"policy receives exact stable backend/config/execution references");
    require(!n::is_bound(),"owner construction stays unpublished");owner.publish();owner.publish();
    require(n::is_bound()&&n::active_calls()==0,"publication is idempotent and produces one native binding");
    require(n::reg_close_entry(reinterpret_cast<HKEY>(0x111))==0&&d.probe->closes==1&&::GetLastError()==0x1234,"typed callback loads original from exact runtime slots and preserves LastError");
    d.runtime.registry_values["ClientPath"]=u"C:\\own\\current.exe";
    PROCESS_INFORMATION info{};info.dwProcessId=77;STARTUPINFOA startup{};
    require(n::process_a_entry("C:\\own\\current.exe",nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&info)==0&&d.probe->processes==0&&info.dwProcessId==77,"Context observes live runtime map without copying");
    d.runtime.registry_values["ClientPath"]=u"C:\\own\\changed.exe";
    require(n::process_a_entry("C:\\own\\current.exe",nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&info)==7&&d.probe->processes==1&&info.dwProcessId==0x12345678,"later map mutation is visible through same native Context");
    d.runtime.registry_values["Documents"]=u"OWN-documents";std::array<char,260> documents{};
    require(n::folder_a_entry(nullptr,5,nullptr,0,documents.data())==0&&std::string(documents.data())=="OWN-documents","native folder callback observes actual values map");
    d.runtime.registry_values["LanguageKey"]=u"Own\\LanguageKey";d.configuration.registry_spoof=0;HKEY key{};
    require(n::reg_open_a_entry(HKEY_CURRENT_USER,"Own\\LanguageKey",0,0,&key)==0&&!d.probe->callbacks->language.contains(0x789),"disabled live config flag excludes language tracking");
    d.configuration.registry_spoof=1;
    require(n::reg_open_a_entry(HKEY_CURRENT_USER,"Own\\LanguageKey",0,0,&key)==0&&d.probe->callbacks->language.contains(0x789),"later flag mutation is visible through Context reference");
    sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(3216);
    d.configuration.server_port=6101;
    require(n::connect_entry(1,reinterpret_cast<const sockaddr*>(&address),16)==-10035&&ntohs(address.sin_port)==6101,"network Context uses actual current configuration port");
    require(d.probe->callbacks->attempts==1&&d.probe->waits==1&&d.probe->connects==1,"callback counters/Execution remain in owned exact domain");
    require(!owner.close_after_cleanup()&&proofs==1&&n::is_bound()&&d.probe->destroyed==0,"false external quiescence retains binding and all policy state");
    quiescent=true;require(owner.close_after_cleanup()&&!n::is_bound()&&proofs==2,"explicit quiescence permits native binding close");
    require(owner.close_after_cleanup()&&proofs==2,"successful close is idempotent");
}
void busy_throw_and_competing_owner(){
    Driver d;bool throw_proof=true;
    nc::Owner owner(d.view(),seed(),d.policies(),permanent_errors,[&]{if(throw_proof)throw std::runtime_error("proof failure");return true;});owner.publish();
    rejects<std::runtime_error>([&]{(void)owner.close_after_cleanup();},"proof exception preserves published owner");
    require(n::is_bound()&&d.probe->destroyed==0,"exception does not fabricate callback shutdown");throw_proof=false;
    auto other=std::make_unique<nc::Owner>(d.view(),seed(),d.policies(),permanent_errors,[]{return true;});
    rejects<Life>([&]{other->publish();},"second native binding rejected without replacing current owner");
    require(n::is_bound()&&other->close_after_cleanup(),"failed publication owner remains safely closable as unpublished");other.reset();
    {std::lock_guard lock(d.probe->gate);d.probe->block=true;d.probe->release=false;}
    std::thread worker([&]{(void)n::reg_close_entry(reinterpret_cast<HKEY>(0x222));});
    {std::unique_lock lock(d.probe->gate);d.probe->cv.wait(lock,[&]{return d.probe->entered;});}
    const bool retained=!owner.close_after_cleanup()&&n::is_bound()&&n::active_calls()==1;
    {std::lock_guard lock(d.probe->gate);d.probe->release=true;}d.probe->cv.notify_all();worker.join();
    require(retained,"in-flight own callback prevents close despite no-new-admissions proof");
    require(owner.close_after_cleanup()&&!n::is_bound(),"retry succeeds after in-flight lease returns");
}
void control_reentry(){
    Driver d;nc::Owner* current=nullptr;
    nc::Owner owner(d.view(),seed(),d.policies(),permanent_errors,[&]{rejects<std::logic_error>([&]{(void)current->close_after_cleanup();},"native owner rejects control reentry");return true;});
    current=&owner;owner.publish();require(owner.close_after_cleanup()&&!n::is_bound(),"outer close remains successful after rejected proof reentry");
}
volatile std::sig_atomic_t abort_code=95;
void expected_abort(int){std::_Exit(abort_code);}
int terminal(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);
    Driver d;
    if(mode=="--open-context"){abort_code=95;nc::Owner owner(d.view(),seed(),d.policies(),permanent_errors,[]{return true;});owner.publish();return 98;}
    if(mode=="--late-context"){abort_code=96;nc::Owner owner(d.view(),seed(),d.policies(),permanent_errors,[]{return true;});owner.publish();owner.close_after_cleanup();(void)n::reg_close_entry(reinterpret_cast<HKEY>(0x111));}
    return 98;
}
}
int main(int argc,char** argv){if(argc==2&&std::string_view(argv[1]).starts_with("--"))return terminal(argv[1]);try{guards_factory();exact_context_and_close();busy_throw_and_competing_owner();control_reentry();
    int code=95;for(const auto* mode:{"--open-context","--late-context"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own terminal child enforces open-owner/late-entry boundary");
    std::cout<<"PASS: "<<b15_test::checks<<" native Context core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
