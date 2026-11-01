#include "identity_test_fixture.hpp"

namespace {
using namespace b21_test;
void body_permission(){
    Fixture f;auto scopes=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();auto* prior=::FlsGetValue(index);
    const auto before=fls_reads.load();require(!scopes->permits_current()&&fls_reads==before,"inactive permission does not call FLS provider");
    scopes->enter();::SetLastError(111);require(scopes->permits_current()&&::GetLastError()==111&&fls_reads==before+2,"borrowed identity probes actual slot and preserves LastError");
    auto* bytes=static_cast<std::byte*>(prior);const auto saved=bytes[1];bytes[1]^=std::byte{1};
    require(scopes->permits_current()&&bytes[1]!=(saved),"borrowed PTD contents are neither snapshotted nor repaired");bytes[1]=saved;
    const auto queried=fls_reads.load();bool foreign=true;b18_test::in_thread([&]{foreign=scopes->permits_current();});
    require(!foreign&&fls_reads==queried&&!scopes->poisoned(),"wrong thread permission rejects without probing or poisoning owner");
    require(scopes->leave()&&::FlsGetValue(index)==prior,"borrowed root remains untouched after actual permission checks");
    b18_test::in_thread([&]{
        scopes->enter();auto* root=::FlsGetValue(index);unsigned metadata=0;
        get_hook=[&]{if(scopes->active()&&!scopes->poisoned()&&scopes->last_failure()==0)++metadata;};
        ::SetLastError(222);const auto allowed=scopes->permits_current();const auto error=::GetLastError();get_hook={};
        require(allowed&&metadata==1&&error==222,"FLS provider can query metadata without either mutex being held");
        crt_ptd_data_recovered::View(root).store<std::uint32_t>(0x20,99);crt_ptd_data_recovered::View(root).store<std::uint32_t>(0x24,5);
        require(scopes->permits_current(),"fresh errno and DOS error mutations remain permitted");
        require(scopes->leave()&&!::FlsGetValue(index),"fresh permission leaves checked cleanup balanced");
    });
}
void cleanup_never_reopens_body(){
    Fixture f;auto scopes=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();
    const auto locale=f.heap.domain->locale_references();
    b18_test::in_thread([&]{
        scopes->enter();auto* root=::FlsGetValue(index);zero_detach=true;
        require(!scopes->leave()&&scopes->last_failure()==0&&::FlsGetValue(index)==root,"known detach false with zero error retains live root");
        auto queried=fls_reads.load();require(!scopes->permits_current()&&fls_reads==queried&&!scopes->poisoned(),"sticky cleanup start denies body without a probe even when error code is zero");zero_detach=false;
        b18_test::deny_root_free=true;require(!scopes->leave()&&::FlsGetValue(index)==ptd::construction_sentinel(),"root free failure retains destroyed root and sentinel");
        queried=fls_reads.load();require(!scopes->permits_current()&&fls_reads==queried,"destroyed-root cleanup retry never grants body permission");b18_test::deny_root_free=false;
        b18_test::deny_set_null=true;const auto freed=f.heap.memory.frees;
        require(!scopes->leave()&&f.heap.memory.frees==freed+1,"successful root free retained before failed final clear");
        queried=fls_reads.load();require(!scopes->permits_current()&&fls_reads==queried,"freed-root clear retry never probes or grants body permission");
        b18_test::deny_set_null=false;require(scopes->leave()&&f.heap.memory.frees==freed+1&&!::FlsGetValue(index),"clear retry repeats neither destructor nor free");
        scopes->enter();require(scopes->permits_current()&&scopes->leave(),"a separate later scope resets cleanup state only after full prior cleanup");
    });
    require(f.heap.domain->locale_references()==locale&&!scopes->active()&&!scopes->poisoned(),"all known cleanup retry paths retain refcount balance");
}
void domain_gate_not_held(){
    Fixture f;auto domain=std::make_shared<foreign_callback::Domain>(f.heap.domain,[]{return true;});foreign_callback::Admission admission(domain);
    admission.host_setup_begin();admission.host_setup_complete();
    b18_test::in_thread([&]{
        admission.enter(native_recovered::Callback::reg_close_00f5b0);unsigned observed=0;
        get_hook=[&]{if(domain->active()&&domain->ready())++observed;};
        require(domain->permits_current()&&observed==1,"foreign Domain metadata gate is released across actual FLS getter");get_hook={};admission.leave(native_recovered::Callback::reg_close_00f5b0);
    });
    require(domain->pause(),"checked domain drains normally after metadata callback");
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,b18_test::expected_abort);Fixture f;
    auto scopes=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();
    b18_test::in_thread([&]{
        scopes->enter();auto* root=::FlsGetValue(index);ptd::Ptd other{};bool nested=false,thrown=false;unsigned code=0;
        if(mode=="--null"){::FlsSetValue(index,nullptr);code=170;}
        if(mode=="--sentinel"){::FlsSetValue(index,ptd::construction_sentinel());code=171;}
        if(mode=="--other"){::FlsSetValue(index,&other);code=172;}
        if(mode=="--snapshot"){crt_ptd_data_recovered::View(root).store<std::uint64_t>(0x48,1);code=173;}
        if(mode=="--getter-throw"){throw_get=true;code=174;}
        if(mode=="--caught-probe-reentry"){get_hook=[&]{nested=!scopes->permits_current();};code=175;}
        const auto frees=f.heap.memory.frees;::SetLastError(321);bool allowed=false;
        try{allowed=scopes->permits_current();}catch(const std::runtime_error&){thrown=true;}
        const auto error=::GetLastError();get_hook={};throw_get=false;
        const bool retained=scopes->active()&&scopes->poisoned()&&f.heap.memory.frees==frees;
        const bool specific=(mode!="--getter-throw"||thrown)&&(mode!="--caught-probe-reentry"||nested);
        if(code&&!allowed&&retained&&specific&&error==321)b18_test::terminate_own(code);
        b18_test::terminate_own(230);
    });return 230;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{
    body_permission();cleanup_never_reopens_body();domain_gate_not_held();
    unsigned code=170;for(const auto* mode:{"--null","--sentinel","--other","--snapshot","--getter-throw","--caught-probe-reentry"})
        require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child proves retained poison before direct terminal success; generic abort cannot pass");
    std::cout<<"PASS: "<<b21_test::checks<<" callback identity core checks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
