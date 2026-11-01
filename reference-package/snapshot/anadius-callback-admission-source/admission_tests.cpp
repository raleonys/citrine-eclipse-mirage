#include "admission_test_fixture.hpp"

namespace {
using namespace b17_test;
bootstrap_backend::CallbackAttachment* closing_owner=nullptr;
bool active_close_rejected=false;
LSTATUS WINAPI close_inside_callback(HKEY){active_close_rejected=!closing_owner->close_after_cleanup();return 0;}
void controller(){
    ca::Controller admission;::SetLastError(0x3333);
    require(!admission.active()&&!admission.stopped()&&::GetLastError()==0x3333,"initial gate preserves caller error");
    admission.enter(n::Callback::reg_close_00f5b0);
    require(admission.active()&&::GetLastError()==0x3333,"entry admits controller without retaining mutex");
    rejects(ca::Fault::Reentry,[&]{admission.enter(n::Callback::folder_w_00fc80);},"same thread nested callback rejected");
    require(!admission.stop()&&!admission.stopped()&&admission.active(),"stop cannot consume an active callback");
    bool foreign=false,stop=false,observed=false;
    std::thread other([&]{
        observed=admission.active();
        try{admission.enter(n::Callback::reg_close_00f5b0);}catch(const ca::Rejected& e){foreign=e.fault==ca::Fault::ForeignThread;}
        try{admission.stop();}catch(const ca::Rejected& e){stop=e.fault==ca::Fault::ForeignThread;}
    });other.join();
    require(observed&&foreign&&stop,"foreign thread returns promptly while controller body is active");
    require(admission.active()&&!admission.stopped(),"foreign rejection leaves controller entry intact");
    admission.leave(n::Callback::reg_close_00f5b0);
    require(!admission.active()&&::GetLastError()==0x3333,"leave releases active entry preserving error");
    require(admission.stop()&&admission.stopped()&&admission.stop(),"quiescent stop is permanent and idempotent");
    rejects(ca::Fault::Stopped,[&]{admission.enter(n::Callback::reg_close_00f5b0);},"stopped controller cannot admit again");
}
void wrapper_order_and_legacy(){
    b15_test::Driver d;bool destroyed=false;probe_enters=probe_leaves=0;leave_saw_lease=false;
    d.runtime.originals[3]=reinterpret_cast<std::uintptr_t>(&own_close);
    {
        auto admission=std::make_unique<ProbeAdmission>(&destroyed);
        nc::Owner owner(d.view(),b15_test::seed(),d.policies(),b15_test::permanent_errors,[]{return true;},std::move(admission));
        require(!destroyed&&!n::is_bound(),"owned admission constructed unpublished");owner.publish();
        ::SetLastError(0x4567);
        require(n::reg_close_entry(reinterpret_cast<HKEY>(0x111))==7,"native wrapper forwards original result");
        require(original_incoming==0x4567&&::GetLastError()==0x7654,"enter and leave effects do not alter API error state");
        require(probe_enters==1&&probe_leaves==1&&leave_saw_lease,"admission leaves exactly once before binding lease release");
        require(owner.close_after_cleanup()&&!destroyed&&!n::is_bound(),"binding closes before owned admission destruction");
    }
    require(destroyed,"admission owned through Context and binding destruction");
    {
        nc::Owner legacy(d.view(),b15_test::seed(),d.policies(),b15_test::permanent_errors,[]{return true;});legacy.publish();
        require(n::reg_close_entry(reinterpret_cast<HKEY>(0x111))==7&&probe_enters==1,"null admission keeps existing source path");
        require(legacy.close_after_cleanup(),"legacy close unchanged");
    }
}
void factory(){
    b15_test::Driver d;bool proof=false,throws=false;unsigned proofs=0;
    auto f=ca::make_factory(b15_test::seed(),d.policies(),b15_test::permanent_errors,[&]{++proofs;if(throws)throw std::runtime_error("proof");return proof;});auto copy=f;
    auto owner=f(d.view());require(!n::is_bound(),"selected admission factory does not publish");owner->publish();
    require(!owner->close_after_cleanup()&&n::is_bound()&&proofs==1,"false proof retains open admission and Context");
    require(n::reg_close_entry(reinterpret_cast<HKEY>(0x111))==0,"controller callback remains admissible after false proof");
    throws=true;bool rejected=false;try{owner->close_after_cleanup();}catch(const std::runtime_error&){rejected=true;}
    require(rejected&&n::reg_close_entry(reinterpret_cast<HKEY>(0x111))==0,"throwing proof retains usable admission");throws=false;proof=true;
    require(owner->close_after_cleanup()&&!n::is_bound()&&proofs==3,"successful proof permits stop and binding close");
    bool twice=false;try{copy(d.view());}catch(const std::logic_error&){twice=true;}require(twice,"copied outer factory consumes admission once");
    require(owner->close_after_cleanup()&&proofs==3,"closed owner does not rerun proof or stop");
}
void active_close(){
    b15_test::Driver d;d.runtime.originals[3]=reinterpret_cast<std::uintptr_t>(&close_inside_callback);
    unsigned proofs=0;auto f=ca::make_factory(b15_test::seed(),d.policies(),b15_test::permanent_errors,[&]{++proofs;return true;});
    auto owner=f(d.view());closing_owner=owner.get();active_close_rejected=false;owner->publish();
    require(n::reg_close_entry(reinterpret_cast<HKEY>(1))==0&&active_close_rejected&&n::is_bound(),"close inside original cannot stop active admission or release Context");
    require(owner->close_after_cleanup()&&proofs==2&&!n::is_bound(),"close retry succeeds after callback admission leaves");closing_owner=nullptr;
}
void expected_abort(int){std::_Exit(145);}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);
    b15_test::Driver d;expected_unwind=false;
    if(mode.starts_with("--stopped-")){
        expected_callback=static_cast<std::size_t>(std::stoi(std::string(mode.substr(10))));expected_code=120+static_cast<int>(expected_callback);expected_fault=ca::Fault::Stopped;
        auto admission=std::make_unique<ca::Controller>();auto* gate=admission.get();
        nc::Owner owner(d.view(),b15_test::seed(),d.policies(),permanent_errors,[]{return true;},std::move(admission));owner.publish();gate->stop();bad_entry(expected_callback);
    }else if(mode=="--foreign"){
        expected_callback=0;expected_code=140;expected_fault=ca::Fault::ForeignThread;
        auto f=ca::make_factory(b15_test::seed(),d.policies(),permanent_errors,[]{return true;});auto owner=f(d.view());owner->publish();
        std::thread other([]{bad_entry(0);});other.join();
    }else if(mode=="--reentry"){
        expected_callback=9;expected_code=141;expected_fault=ca::Fault::Reentry;
        d.runtime.originals[3]=reinterpret_cast<std::uintptr_t>(&reentering_close);
        auto f=ca::make_factory(b15_test::seed(),d.policies(),permanent_errors,[]{return true;});auto owner=f(d.view());owner->publish();(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));
    }else if(mode=="--unwind"){
        expected_callback=3;expected_code=142;expected_unwind=true;probe_enters=probe_leaves=0;leave_saw_lease=false;
        d.runtime.originals[3]=reinterpret_cast<std::uintptr_t>(&throwing_close);
        nc::Owner owner(d.view(),b15_test::seed(),d.policies(),permanent_errors,[]{return true;},std::make_unique<ProbeAdmission>());owner.publish();(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));
    }else if(mode=="--bad-leave"){
        ca::Controller admission;admission.enter(n::Callback::reg_close_00f5b0);admission.leave(n::Callback::folder_w_00fc80);
    }
    return 214;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{
    controller();wrapper_order_and_legacy();factory();active_close();
    for(unsigned i=0;i<10;++i){const auto mode="--stopped-"+std::to_string(i);require(_spawnl(_P_WAIT,argv[0],argv[0],mode.c_str(),static_cast<const char*>(nullptr))==120+i,"all ten native entries reject before inaccessible body buffers");}
    int code=140;for(const auto* mode:{"--foreign","--reentry","--unwind"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own terminal child proves foreign/reentry/unwind boundary");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--bad-leave",static_cast<const char*>(nullptr))==145,"wrong admission release is terminal");
    std::cout<<"PASS: "<<b17_test::checks<<" callback admission core checks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
