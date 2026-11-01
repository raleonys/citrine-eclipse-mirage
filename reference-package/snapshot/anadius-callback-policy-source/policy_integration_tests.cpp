#include "policy_test_fixture.hpp"
#include <iomanip>

namespace {
using namespace b16_test;
void native_chain(){
    Fixture f;set_fabrication_originals(f);cp::Policy* policy=nullptr;nc::CallbackSeed* state=nullptr;
    auto factory=cp::make_policy_factory(f.resources());
    auto captured=[&](nc::PolicyView view){state=&view.callbacks;auto result=factory(view);policy=dynamic_cast<cp::Policy*>(result.get());return result;};
    bool quiescent=false;
    nc::Owner owner(f.driver.view(),b15_test::seed(),captured,b15_test::permanent_errors,[&]{return quiescent;});
    require(policy&&state&&!n::is_bound(),"concrete policy created against unpublished actual Context");
    require(policy->original_slots_identity()==&f.driver.runtime.originals,"policy owns actual final original slot references");
    owner.publish();require(n::is_bound(),"own selected Context publication");
    HKEY key{};const auto allocations=f.heap.memory.allocations;
    require(n::reg_open_a_entry(HKEY_CURRENT_USER,"Own\\Client",0,0,&key)==0,"typed original failure reaches concrete fabrication policy");
    const auto handle=reinterpret_cast<std::uintptr_t>(key);
    require(handle&&handle!=0xdead&&state->fabricated.contains(handle)&&state->client.contains(handle),"actual caller output and both tracking sets share owned handle");
    require(policy->owned_handles()==1&&f.memory->outstanding_allocations()==1&&f.heap.memory.allocations==allocations+1,"exact one four byte application allocation");
    require(f.output->str()=="fake key created\n","retained creation literal and flush");
    DWORD type=0,size=128;std::array<char,128> path{};
    require(n::reg_query_a_entry(key,"Path",nullptr,&type,reinterpret_cast<LPBYTE>(path.data()),&size)==0,"actual A query through concrete CRT/encoding/wait policy");
    require(std::string(path.data())=="C:\\own\\target.exe"&&size==18&&type==1,"query output uses caller SDK size/type storage");
    DWORD wtype=0,wsize=128;std::array<wchar_t,64> wpath{};
    require(n::reg_query_w_entry(key,L"Path",nullptr,&wtype,reinterpret_cast<LPBYTE>(wpath.data()),&wsize)==0,"actual W query uses byte bridge");
    require(std::wstring(wpath.data())==L"C:\\own\\target.exe"&&wsize==36&&wtype==1,"wide required size is bytes and output is real wchar_t");
    size=0;require(n::reg_query_a_entry(key,"Path",nullptr,nullptr,nullptr,&size)==0&&size==18,"size only query uses full concrete chain");
    size=2;path[0]='!';require(n::reg_query_a_entry(key,"Path",nullptr,nullptr,reinterpret_cast<LPBYTE>(path.data()),&size)==234&&path[0]=='!'&&size==18,"insufficient capacity prefix retained");
    size=128;require(n::reg_query_a_entry(key,"other",nullptr,nullptr,reinterpret_cast<LPBYTE>(path.data()),&size)==2,"unmatched fabricated query preserves source fallback");
    require(f.output->str().find("fake key queried something else than what I replace\n")!=std::string::npos,"full retained query literal includes observed terminator");
    require(n::reg_close_entry(key)==0&&policy->owned_handles()==0&&f.memory->outstanding_allocations()==0,"fabricated close frees same source memory once");
    require(!state->fabricated.contains(handle)&&!state->client.contains(handle),"successful close clears tracking");
    const auto original_closes=f.driver.probe->closes.load();::SetLastError(0x555);
    require(n::reg_close_entry(reinterpret_cast<HKEY>(0x1234))==0&&f.driver.probe->closes==original_closes+1&&::GetLastError()==0x1234,"own exact typed original preserves LastError");
    std::array<wchar_t,260> folder{};
    require(n::folder_w_entry(nullptr,5,nullptr,0,folder.data())==0&&std::wstring(folder.data())==L"C:\\own\\docs","actual W folder callback reaches concrete byte bridge");
    f.driver.runtime.registry_values["Documents"]=u"changed";
    require(n::folder_w_entry(nullptr,5,nullptr,0,folder.data())==0&&std::wstring(folder.data())==L"changed","live Runtime map remains the same storage");
    STARTUPINFOW startup{};PROCESS_INFORMATION process{};
    require(n::process_w_entry(L"C:\\OWN\\target.exe",nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process)==0,"actual wide process matching uses compiler case comparison");
    sockaddr_in socket{};socket.sin_family=AF_INET;socket.sin_port=htons(3216);
    require(n::connect_entry(1,reinterpret_cast<const sockaddr*>(&socket),16)==-10035&&ntohs(socket.sin_port)==6000&&state->attempts==1,"network typed shim uses concrete wait/log and exact configuration");
    require(f.output->str().find("game connected on port 3216; redirecting to 6000\n")!=std::string::npos,"network log numeric sequence observed");
    require(!owner.close_after_cleanup()&&n::is_bound(),"empty handle ledger does not prove hook quiescence");
    quiescent=true;require(owner.close_after_cleanup()&&!n::is_bound(),"separate explicit quiescence closes Context");
}
void free_failure_prefix(){
    Fixture f;set_fabrication_originals(f);cp::Policy policy(f.view(),f.resources());
    registry_recovered::StateRefs refs{f.driver.runtime.registry_values,f.callbacks.client,f.callbacks.language,f.callbacks.fabricated,f.driver.configuration.registry_spoof};
    registry_recovered::Handle key{};
    require(registry_recovered::open_a_00f800(refs,policy,{0,"Own\\Client",0,0,&key})==0,"own direct source fabricated open for controlled failure");
    deny_free=true;rejects<std::system_error>([&]{registry_recovered::close_00f5b0(refs,policy,key);},"known allocator failure propagates after source erase");deny_free=false;
    require(::GetLastError()==ERROR_ACCESS_DENIED,"failed native free error survives ownership bookkeeping");
    require(!f.callbacks.fabricated.contains(key)&&f.callbacks.client.contains(key),"source erase before free retained without invented rollback");
    require(policy.owned_handles()==1&&f.memory->outstanding_allocations()==1,"failed free retains exact allocator ownership");
    require(f.output->str()=="fake key created\n","failed free stops later deleted log");
    policy.free_04b0f8(key);require(!policy.owned_handles()&&!f.memory->outstanding_allocations(),"explicit owner retry frees retained allocation");
    require(f.callbacks.client.contains(key),"owner retry does not invent unfinished source tracking cleanup");
    f.callbacks.client.clear();
    auto lease=f.memory;require(!f.heap.domain->close(),"policy memory lease prevents domain shutdown");lease.reset();
}
}
int main(){try{native_chain();free_failure_prefix();std::cout<<"PASS: "<<b16_test::checks<<" callback policy integration checks; own SDK shims and same-domain memory, serialized controller\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
