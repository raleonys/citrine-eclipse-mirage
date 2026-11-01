#include "effects_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"

namespace {
using namespace b12_test;
void load_paths(){
    Graph g;Os os;os.load_hook=[](const wchar_t* p){return std::wstring_view(p)==L"missing.dll"?lib::Module{0}:lib::Module{0x601};};
    const std::vector<std::wstring> paths={L"own_\u0131\U0001f600.dll",L"missing.dll",std::wstring(L"a\0b.dll",7),L""};
    const auto loaded=lib::load_list_035260(os,g.effects,paths);
    require(loaded==std::vector<lib::Module>({0x601,0x601,0x601}),"actual source list retains duplicates, skips failed handle, continues");
    require(os.loads==std::vector<std::wstring>({L"own_\u0131\U0001f600.dll",L"missing.dll",L"a",L""}),"synthetic loader sees original c_str NUL boundary including empty path");
    const auto expected=std::string("Loading own_\xc4\xb1\xf0\x9f\x98\x80.dll\nLoading missing.dll\nLoading ")+std::string("a\0b.dll",7)+"\nLoading \n";
    require(g.output->str()==expected&&g.output->flushes==4,"actual UTF8 source conversion and counted logging retain full path bytes");
    require(os.conversions.size()==8,"failed path reconverts after load failure, empty path performs no conversion");
    require(g.sdk.messages.size()==1&&g.sdk.messages[0].title==L"error"&&g.sdk.messages[0].text==L"Can't load missing.dll"&&g.sdk.messages[0].flags==0x10,"load failure reaches concrete dialog boundary with exact title/body/flags");
    require(os.mode==0x40&&os.modes==std::vector<std::uint32_t>({0x41,0x40}),"normal source return restores observed GetErrorMode value");
    require(g.phase->other_calls==0&&g.phase->waits.empty()&&g.phase->finishes==0,"source list only reaches selected output/error/UI policies");
}
void failure_prefixes(){
    for(unsigned fail_at:{1U,2U}){
        Graph g;Os os;unsigned calls=0;os.conversion_hook=[&](const wchar_t* p,int count,char* out,int cap){
            if(++calls==fail_at)return 0;
            return ::WideCharToMultiByte(65001,0x80,p,count,out,cap,nullptr,nullptr);
        };
        rejects<std::system_error>([&]{const std::vector<std::wstring> paths={L"own.dll"};(void)lib::load_list_035260(os,g.effects,paths);},"query/fill library conversion errors reach selected exception policy");
        require(g.output->str()=="Loading "&&g.output->flushes==0,"library conversion error preserves prefix before counted insertion");
        require(os.loads.empty()&&g.sdk.messages.empty()&&os.mode==0x41,"conversion failure prevents loader/UI and preserves observed error-mode prefix");
    }
    {
        Graph g;Os os;g.output->maximum=2;g.output->exceptions(std::ios::badbit);
        rejects<std::ios_base::failure>([&]{const std::vector<std::wstring> paths={L"own.dll"};(void)lib::load_list_035260(os,g.effects,paths);},"prefix stream failure propagates before conversion and loading");
        require(g.output->str()=="Lo"&&os.conversions.empty()&&os.loads.empty()&&os.mode==0x41,"stream error leaves only actual prefix effects");
    }
    {
        Graph g;Os os;g.output->fail_flush=true;g.output->exceptions(std::ios::badbit);
        rejects<std::ios_base::failure>([&]{const std::vector<std::wstring> paths={L"own.dll"};(void)lib::load_list_035260(os,g.effects,paths);},"flush failure precedes loader");
        require(g.output->str()=="Loading own.dll\n"&&os.conversions.size()==2&&os.loads.empty(),"completed failed-flush line is retained without module side effect");
    }
    {
        Graph g;Os os;g.output->setstate(std::ios::failbit);const std::vector<std::wstring> paths={L"own.dll"};
        const auto loaded=lib::load_list_035260(os,g.effects,paths);
        require(loaded.size()==1&&os.loads.size()==1&&g.output->str().empty(),"nonthrowing failed stream does not fabricate source load rejection");
    }
    {
        Graph g;Os os;os.load_hook=[](const wchar_t*){return lib::Module{0};};
        g.sdk.hook=[](UINT,DWORD,LPCCH,int,LPWSTR,int)->int{throw std::runtime_error("controlled dialog conversion outcome");};
        rejects<std::runtime_error>([&]{const std::vector<std::wstring> paths={L"missing.dll",L"later.dll"};(void)lib::load_list_035260(os,g.effects,paths);},"injected UI-boundary exception stops source list");
        require(g.output->str()=="Loading missing.dll\n"&&os.loads.size()==1&&os.conversions.size()==4&&g.sdk.messages.empty(),"failure after reconversion retains line/load prefix and prevents later attempts");
        require(os.mode==0x41,"throwing source path does not add invented error-mode rollback");
    }
}
void phase_source(){
    Graph g;Os os;lib::State state{{L"main.dll"},{L"worker.dll"},{0x777}};
    lib::load_phase_035850(os,g.effects,state,true);
    require(g.phase->waits.empty()&&g.phase->finishes==0,"main phase never invokes worker wait/finish");
    lib::load_phase_035850(os,g.effects,state,false);
    require(g.phase->waits==std::vector<int>({2})&&g.phase->finishes==1,"standalone source phase delegates exact wait2 and final flag");
    require(state.loaded==std::vector<lib::Module>({0x777,0x501,0x502}),"both actual source phases append their successful handles");
    lib::unload_035ab0(os,state);
    require(os.unloaded==state.loaded&&state.loaded.size()==3,"existing unload source retains forward order and storage");
    Graph failed;Os bad;lib::State unchanged{{},{L"bad.dll"},{0x888}};
    bad.conversion_hook=[](const wchar_t*,int,char*,int){return 0;};
    rejects<std::system_error>([&]{lib::load_phase_035850(bad,failed.effects,unchanged,false);},"failed worker phase propagates source conversion error");
    require(failed.phase->waits==std::vector<int>({2})&&failed.phase->finishes==0&&unchanged.loaded==std::vector<lib::Module>({0x888}),"failed phase does not fabricate ready publication or commit result list");
}
void full_runtime(){
    b4_test::Fixture f;Graph g;auto resources=f.resources();
    auto os_owner=std::make_unique<Os>();auto* os=os_owner.get();
    os->load_hook=[](const wchar_t* p){return std::wstring_view(p)==L"worker.dll"?lib::Module{0}:lib::Module{0x900};};
    resources.library_os=std::move(os_owner);f.library_os=nullptr;
    resources=le::bind_library_effects(std::move(resources),g.output,g.dialog);
    f.construct(std::move(resources));f.runtime->publish();
    require(f.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B4 Runtime reaches actual application attach with selected library Effects");
    require(g.output->str()=="Loading main.dll\n"&&f.joins.size()==3,"synchronous main load precedes three pending source worker joins");
    require(f.library_effects->waits==0&&f.library_effects->finishes==0,"B3 execution wrapper has not touched lower phase fixture");
    f.timeout_once=true;
    require(!f.runtime->close()&&f.runtime->phase()==bootstrap_owner::Phase::Stopping,"join timeout retains Runtime and all selected Effects owners");
    require(g.output->str()=="Loading main.dll\n"&&g.sdk.messages.empty(),"timeout does not execute pending worker load or UI");
    require(f.runtime->close(),"retry joins pending actual source callbacks and finishes cleanup");
    require(g.output->str()=="Loading main.dll\nLoading worker.dll\n"&&g.output->flushes==2,"worker phase reaches same stream once during retry");
    require(g.sdk.messages.size()==1&&g.sdk.messages[0].text==L"Can't load worker.dll","failed worker load reaches injected UI endpoint under full Runtime");
    require(f.library_effects->waits==0&&f.library_effects->finishes==0,"B3 binds both phase routes to its actual shared Execution instead of lower throws");
    require(os->unloaded==std::vector<lib::Module>({0x777,0x900}),"source cleanup unloads only retained seed and successful main handle");
    require(f.runtime->close()&&g.output->flushes==2&&os->unloaded.size()==2,"closed Runtime does not repeat logs, UI or cleanup");
    f.runtime.reset();
    require(!bootstrap_native_recovered::is_bound(),"Runtime source route binding removed after quiescent cleanup");
}
}
int main(){try{load_paths();failure_prefixes();phase_source();full_runtime();std::cout<<"PASS: "<<b12_test::checks<<" library Effects integration checks; actual load/phase/Runtime sources, synthetic modules and UI\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
