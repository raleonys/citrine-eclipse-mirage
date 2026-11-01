#include "iteration_test_fixture.hpp"
#include <cctype>

namespace {
using namespace b9_test;
enum class Fault {none,late_next,narrow,remove};
struct NativePlan {
    Fault fault=Fault::none;std::wstring wanted=L"early_AVATAR_fixture.png";
    std::filesystem::path early;HANDLE handle=INVALID_HANDLE_VALUE;
    unsigned firsts=0,nexts=0,closes=0,dispositions=0;bool early_deleted_at_next=false;
};
NativePlan* native_plan=nullptr;
HANDLE WINAPI ordered_first(LPCWSTR pattern,FINDEX_INFO_LEVELS level,LPVOID output,FINDEX_SEARCH_OPS search,LPVOID filter,DWORD flags){
    auto& plan=*native_plan;++plan.firsts;
    auto handle=::FindFirstFileExW(pattern,level,output,search,filter,flags);
    if(handle==INVALID_HANDLE_VALUE)return handle;
    if(plan.fault!=Fault::none){
        auto* data=static_cast<WIN32_FIND_DATAW*>(output);
        while(std::wstring_view(data->cFileName)!=plan.wanted){
            if(!::FindNextFileW(handle,data)){const auto error=::GetLastError();::FindClose(handle);::SetLastError(error);return INVALID_HANDLE_VALUE;}
        }
    }
    plan.handle=handle;return handle;
}
BOOL WINAPI controlled_next(HANDLE handle,LPWIN32_FIND_DATAW output){
    auto& plan=*native_plan;++plan.nexts;
    if(plan.fault==Fault::late_next){
        plan.early_deleted_at_next=!std::filesystem::exists(plan.early);::SetLastError(ERROR_READ_FAULT);return FALSE;
    }
    return ::FindNextFileW(handle,output);
}
BOOL WINAPI controlled_close(HANDLE handle){++native_plan->closes;return ::FindClose(handle);}
BOOL WINAPI controlled_disposition(HANDLE handle,FILE_INFO_BY_HANDLE_CLASS level,LPVOID input,DWORD bytes){
    auto& plan=*native_plan;++plan.dispositions;
    if(plan.fault==Fault::remove){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return ::SetFileInformationByHandle(handle,level,input,bytes);
}
cs::FilesystemApi native_api(){
    auto api=cs::native_filesystem_api();api.find_first_ex=ordered_first;api.find_next=controlled_next;api.find_close=controlled_close;api.set_info=controlled_disposition;return api;
}
class NarrowCalls final:public encoding_recovered::Calls {
    encoding_recovered::NativeCalls actual_;
public:
    std::uint32_t locale_codepage_0629ec()override{return actual_.locale_codepage_0629ec();}
    bool are_file_apis_ansi()override{return actual_.are_file_apis_ansi();}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* p,int n,char16_t* out,int cap)override{return actual_.multi_byte_to_wide(cp,flags,p,n,out,cap);}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* p,int n,char* out,int cap,bool* used)override{
        if(native_plan->fault==Fault::narrow&&n>=0&&std::u16string_view(p,static_cast<std::size_t>(n)).find(u"_AVATAR_")!=std::u16string_view::npos)
            throw std::runtime_error("controlled source filename conversion failure");
        return actual_.wide_to_multi_byte(cp,flags,p,n,out,cap,used);
    }
    std::uint32_t last_error()override{return actual_.last_error();}
};
b8_test::LowerState* paths=nullptr;
DWORD copy_path(const Path& p,LPWSTR out,DWORD cap,bool environment){
    if(p.size()>=cap){if(!environment)std::copy_n(p.begin(),cap,out);return static_cast<DWORD>(p.size()+(environment?1:0));}
    std::copy(p.begin(),p.end(),out);out[p.size()]=0;return static_cast<DWORD>(p.size());
}
DWORD WINAPI module_path(HMODULE module,LPWSTR out,DWORD cap){require(!module&&out&&cap,"configuration uses current-process module path");return copy_path(paths->executable,out,cap,false);}
DWORD WINAPI environment_path(LPCWSTR name,LPWSTR out,DWORD cap){
    const auto key=std::wstring_view(name);require(key==L"ProgramData"||key==L"LocalAppData","configuration retains environment variable identities");
    return copy_path(key==L"ProgramData"?paths->program:paths->local,out,cap,true);
}
BOOL WINAPI environment_set(LPCSTR name,LPCSTR value){paths->environment.emplace_back(name,value);return FALSE;}
struct Graph {
    NativePlan plan;
    std::shared_ptr<b8_test::LowerState> lower=std::make_shared<b8_test::LowerState>();
    std::shared_ptr<cs::Filesystem> fs;
    std::unique_ptr<bootstrap_providers::Configuration> selected;
    cs::ConfigurationPersistence* persistence=nullptr;
    Graph(const std::filesystem::path& root,Fault fault){
        plan.fault=fault;native_plan=&plan;paths=lower.get();fs=std::make_shared<cs::Filesystem>(native_api());
        lower->executable=(root/"game"/"host.exe").u16string();lower->local=(root/"data").u16string();lower->program=(root/"program").u16string();
        // Every forwarding layer occurs before the concrete streaming layer.
        auto directory=std::make_unique<cs::ConfigurationFilesystem>(fs,std::make_unique<b8_test::Lower>(lower));
        auto storage=std::make_unique<cs::ConfigurationPersistence>(fs,achievement_storage::Paths{},std::move(directory));persistence=storage.get();
        auto platform=std::make_unique<bootstrap_policies::ConfigurationPlatform>(std::move(storage),std::make_unique<NarrowCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),
            bootstrap_policies::ConfigPlatformApi{module_path,environment_path,::GetLastError,::SetLastError});
        selected=std::make_unique<bootstrap_providers::Configuration>(std::move(platform),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),
            bootstrap_providers::ConfigApi{&::isspace,&::tolower,environment_set});
    }
    ~Graph(){selected.reset();fs.reset();paths=nullptr;native_plan=nullptr;}
};
constexpr std::string_view base_cfg=R"("Config2" {"Game" {"Version" "native-source-version" "ContentId" "own" "Language" "tr"} "Emulator" {"SDK" "1.2.3"} "User" {"Username" "base"}})";
constexpr std::string_view override_cfg=R"("Config2" {"User" {"Username" "override"}})";
void configuration_graph(const std::filesystem::path& root,Fault fault){
    using b8_test::write_own;using b8_test::read_own;
    Graph graph(root,fault);const auto game=root/"game",user=root/"data"/"anadius"/"LSX emu";
    graph.fs->create_directories(game.u16string());graph.fs->create_directories(user.u16string());
    write_own(game/"anadius.cfg",base_cfg);write_own(game/"anadius_override.cfg",override_cfg);
    graph.plan.early=user/"early_AVATAR_fixture.png";
    write_own(graph.plan.early,"owned early avatar");write_own(user/"keep.txt","owned keep");
    configuration::State state{};file_recovered::NativeFactory files;bool caught=false;int error=0;
    try{config_startup::initialize_configuration(state,*graph.selected,files);}
    catch(const std::system_error& e){caught=true;error=e.code().value();}
    catch(const std::runtime_error&){caught=true;error=-1;}
    require(state.username=="override"&&state.user_directory==user.u16string(),"actual FilePass base/override and path prefix reach one state before streaming");
    require(graph.lower->pngs==0&&graph.lower->enumerations==0&&graph.lower->removes==0,"full four-wrapper graph bypasses coarse lower PNG/vector/remove policies");
    require(std::filesystem::exists(user/"anadius.png")&&std::filesystem::exists(user/"default.png"),"actual PNG prefix completes before streaming failure or success");
    require(read_own(user/"keep.txt")=="owned keep","streaming config never changes nonmatching own file");
    require(graph.plan.firsts==1&&graph.plan.closes==1,"full wrapper graph opens and closes one real native enumeration");
    if(fault==Fault::late_next){
        require(caught&&error==ERROR_READ_FAULT&&graph.plan.nexts==1,"controlled later native increment failure reaches actual source exception boundary");
        require(graph.plan.early_deleted_at_next&&!std::filesystem::exists(graph.plan.early),"real early avatar deletion is visible inside later FindNext before that error returns");
        require(graph.plan.dispositions==1&&state.sdk=="1.2.3"&&graph.lower->environment.empty(),"late error retains delete prefix and stops source normalization/environment");
        require(graph.persistence->paths().xml_path.empty(),"later achievement policy is not fabricated after failed enumeration");
    }else if(fault==Fault::narrow){
        require(caught&&error==-1&&graph.plan.nexts==0,"actual source narrow failure occurs before native iterator increment");
        require(std::filesystem::exists(graph.plan.early)&&graph.plan.dispositions==0,"failed filename conversion cannot delete the unprocessed file");
        require(state.sdk=="1.2.3"&&graph.lower->environment.empty(),"narrow failure stops later source mutations");
    }else if(fault==Fault::remove){
        require(caught&&error==ERROR_ACCESS_DENIED&&graph.plan.nexts==0,"real delete handle disposition failure blocks next directory increment");
        require(std::filesystem::exists(graph.plan.early)&&graph.plan.dispositions==1,"failed real delete preserves own file and occurs exactly once");
        require(state.sdk=="1.2.3"&&graph.lower->environment.empty(),"delete failure preserves pre-normalization source prefix");
    }else{
        require(!caught&&!std::filesystem::exists(graph.plan.early),"successful full graph filters and deletes matching own avatar during live iteration");
        require(state.sdk=="1,2,3"&&graph.lower->environment.size()==2,"successful stream allows existing normalization and final environment sequence");
        require(graph.lower->environment==std::vector<std::pair<std::string,std::string>>({{"ContentId","own"},{"EAGameLocale","tr"}}),"post-stream environment operands remain source-derived");
    }
}
class FailureBootstrap final:public recovered::BootstrapDependencies {
    bootstrap_owner::Services& services_;
public:
    enum class Mode {create,identifier,detach};
    Mode mode;std::vector<std::pair<void*,std::size_t>> payloads;unsigned begins=0,closes=0;
    FailureBootstrap(bootstrap_owner::Services& services,Mode m):services_(services),mode(m){}
    int register_exit(recovered::RegisteredCallback)override{return 0;}
    void install_terminate(recovered::RegisteredCallback)override{}
    void setup_environment_019910()override{}
    void setup_console_038760()override{}
    void* log_insert_006880(void* s,std::uintptr_t)override{return s;}
    void log_finish_0088e0(void*)override{}
    void load_configuration_01c870()override{}
    void* allocate_04b0bc(std::size_t n)override{auto* p=services_.allocate(n);payloads.emplace_back(p,n);return p;}
    void delete_04b0f8(void* p,std::size_t n)override{services_.deallocate(p,n);}
    static void noop(){}
    static void boolean(bool){}
    recovered::NoArgumentFunction worker(recovered::Worker)override{return noop;}
    recovered::BooleanFunction boolean_worker_035850()override{return boolean;}
    recovered::Handle begin_thread_05e8e0(recovered::ThreadEntry,void*,std::uint32_t* id)override{++begins;*id=mode==Mode::identifier?0:7;return mode==Mode::create?0:0x999;}
    bool close_handle(recovered::Handle h)override{require(h==0x999,"synthetic detach receives only own opaque thread token");++closes;return false;}
    [[noreturn]] void throw_thread_error_04a648(int i)override{services_.thread_error(i);std::terminate();}
    void between_workers_00e310()override{}
    void guard_return_017a30()override{}
    void cleanup_threads_04a47c()override{}
    void cleanup_035ab0()override{}
    void cleanup_0366f0()override{}
    void cleanup_0102b0()override{}
    void terminate_log_038920()override{}
    [[noreturn]] void unexpected_terminate_return_trap()override{throw std::logic_error("unused terminate");}
    void cleanup_test_payloads(){for(const auto& [p,n]:payloads)services_.deallocate(p,n);payloads.clear();}
};
void bootstrap_failure_routes(){
    b7_test::DomainFixture domain;auto memory=domain.domain->make_memory();b2_test::Track track;
    {
        bootstrap_policies::SourceServices services(memory,std::make_unique<b2_test::Backend>(track));
        for(const auto mode:{FailureBootstrap::Mode::create,FailureBootstrap::Mode::identifier,FailureBootstrap::Mode::detach}){
            FailureBootstrap deps(services,mode);recovered::Bootstrap bootstrap(deps);bool caught=false;
            try{bootstrap.initialize();}catch(const std::system_error& e){caught=e.code().value()==(mode==FailureBootstrap::Mode::create?11:22);}
            require(caught&&deps.begins==1,"actual Bootstrap selects retained index6 for create failure and index1 for id/detach failure");
            require(deps.closes==(mode==FailureBootstrap::Mode::detach?1u:0u),"source short-circuit avoids detach on create/identifier failure");
            require(memory->outstanding_allocations()==1,"source failed creation retains payload rather than inventing production rollback");
            deps.cleanup_test_payloads();require(memory->outstanding_allocations()==0,"test explicitly releases its synthetic retained failure payload");
        }
    }
    memory.reset();require(domain.domain->close(),"Bootstrap failure tests close actual owned heap/PTD graph");
}
}
int main(int argc,char** argv){try{
    const auto root=own_root(argc,argv);
    configuration_graph(root/"late",Fault::late_next);configuration_graph(root/"narrow",Fault::narrow);
    configuration_graph(root/"remove",Fault::remove);configuration_graph(root/"success",Fault::none);
    bootstrap_failure_routes();
    std::cout<<"PASS: "<<b9_test::total()<<" live-iteration integration checks; real own file deletion before injected increment/conversion/remove failures, four-wrapper FilePass routes and Bootstrap indexed thread errors; no original execution\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
