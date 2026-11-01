#include "policies_test_fixture.hpp"
#include <limits>

namespace {
using namespace b7_test;
void configuration_sdk_paths(){
    ConfigFixture f;
    require(f.sdk.module_capacities.empty()&&f.sdk.names.empty(),"constructing configuration platform performs no SDK read");
    ::SetLastError(0x55);require(f.config->executable_path_03c6d0()==u"C:\\fixture\\host.exe","SDK executable path is copied into owned UTF16 state");
    require(f.sdk.module_capacities==std::vector<DWORD>{260}&&f.sdk.incoming_errors==std::vector<DWORD>{0x55}&&::GetLastError()==0x702,"initial capacity and actual call LastError retained");
    f.sdk.module_capacities.clear();f.sdk.incoming_errors.clear();f.sdk.executable.assign(260,L'x');
    require(f.config->executable_path_03c6d0()==configuration::Path(260,u'x'),"exact-capacity module result is retried without losing final code unit");
    require(f.sdk.module_capacities==std::vector<DWORD>({260,520}),"module path grows by retained 260 units");
    f.sdk.module_capacities.clear();f.sdk.incoming_errors.clear();f.sdk.executable.assign(790,L'y');::SetLastError(0x66);
    require(f.config->executable_path_03c6d0()==configuration::Path(790,u'y'),"long executable path survives repeated fixed-step growth");
    require(f.sdk.module_capacities==std::vector<DWORD>({260,520,780,1040})&&f.sdk.incoming_errors==std::vector<DWORD>({0x66,0x702,0x703,0x704}),"each module retry receives preceding SDK LastError");
    f.sdk.no_module=true;require(f.config->executable_path_03c6d0().empty(),"zero executable result remains empty rather than fabricated fallback");
    f.sdk.program.assign(1100,L'p');f.sdk.environment_capacities.clear();f.sdk.incoming_errors.clear();
    require(f.config->program_data_03c920()==configuration::Path(1100,u'p'),"ProgramData environment path retains all UTF16 units");
    require(f.sdk.environment_capacities==std::vector<DWORD>({260,520,780,1040,1300}),"environment required-size result does not replace retained fixed-step growth");
    require(f.sdk.names==std::vector<std::wstring>(5,L"ProgramData")&&::GetLastError()==0x726,"exact retained ProgramData name and last call status preserved");
    require(f.config->local_app_data_03c6f0()==u"LOCAL"&&f.sdk.names.back()==L"LocalAppData","LocalAppData uses exact environment variable instead of known-folder API");
    f.sdk.no_environment=true;require(f.config->program_data_03c920().empty()&&f.config->local_app_data_03c6f0().empty(),"missing environment values stay empty for both routes");
    const auto native=p::native_config_platform_api();require(native.module_filename==::GetModuleFileNameW&&native.environment==::GetEnvironmentVariableW&&native.last_error==::GetLastError&&native.set_last_error==::SetLastError,"native SDK table selects real imports without invoking them");
}
void configuration_encoding(){
    ConfigFixture f;auto& enc=*f.encoding;
    const std::string input("a\0B",3);
    require(f.config->widen_0145d0(input)==configuration::Path(u"a\0B",3),"UTF8 widen preserves explicit byte length including embedded NUL");
    require(enc.narrow_calls.size()==2&&enc.narrow_calls[0].cp==65001&&enc.narrow_calls[0].flags==8&&enc.narrow_calls[0].size==3&&enc.narrow_calls[1].capacity==3&&enc.cp_calls==0,"UTF8 widen uses recovered two-pass codepage65001/flags8 without locale query");
    const auto prior=enc.narrow_calls.size();require(f.config->widen_0145d0({}).empty()&&enc.narrow_calls.size()==prior,"empty UTF8 widen avoids conversion boundary");
    enc.short_fill=true;require(f.config->widen_0145d0("abcd")==configuration::Path(u"abc\0",4),"short successful fill retains first-pass output size and untouched zero suffix");enc.short_fill=false;
    enc.fail=true;bool failure=false;try{f.config->widen_0145d0("bad");}catch(const std::system_error& e){failure=e.code().value()==1113;}
    require(failure,"first-pass conversion failure preserves actual error code");enc.fail=false;enc.fail_fill=true;failure=false;
    try{f.config->widen_0145d0("bad");}catch(const std::system_error& e){failure=e.code().value()==1113;}
    require(failure,"second-pass conversion failure does not return partially initialized success");enc.fail_fill=false;
    require(f.config->narrow_path(configuration::Path(u"q\0R",3))==std::string("q\0R",3),"narrow path preserves explicit UTF16 length including embedded NUL");
    require(enc.cp_calls==1&&enc.ansi_calls==0&&enc.wide_calls.size()==2&&enc.wide_calls[0].cp==65001&&enc.wide_calls[0].flags==0x80&&!enc.wide_calls[0].has_default,"UTF8 filesystem codepage retains source strict conversion operands");
    const auto wide_count=enc.wide_calls.size();require(f.config->narrow_path({}).empty()&&enc.cp_calls==2&&enc.wide_calls.size()==wide_count,"empty narrow path still queries filesystem codepage");
    enc.cp=1252;enc.ansi=true;enc.wide_calls.clear();require(f.config->narrow_path(u"ansi")=="ansi"&&enc.wide_calls.front().cp==0&&enc.wide_calls.front().flags==0x400&&enc.wide_calls.front().has_default,"nonUTF8 ANSI file mode selects codepage0 and default-character detection");
    enc.ansi=false;enc.wide_calls.clear();require(f.config->narrow_path(u"oem")=="oem"&&enc.wide_calls.front().cp==1,"OEM file mode selects codepage1 instead of guessed UTF8");
    enc.used_default=true;failure=false;try{f.config->narrow_path(u"lossy");}catch(const std::system_error& e){failure=e.code().value()==0x459;}
    require(failure,"default-character substitution remains explicit encoding failure");enc.used_default=false;enc.cp=65001;enc.fallback_once=true;enc.wide_calls.clear();
    require(f.config->narrow_path(u"fallback")=="fallback"&&enc.wide_calls.size()==3&&enc.wide_calls[0].flags==0x80&&enc.wide_calls[1].flags==0&&enc.wide_calls[2].flags==0x80,"existing invalid-flags fallback is retained per conversion pass");
}
void configuration_resolution_and_limits(){
    ConfigFixture f;
    require(f.config->resolve_path_0192d0("assets\\avatar.png",u"C:\\fixture\\host.exe",true)==u"C:\\fixture\\assets\\avatar.png","relative resolution uses recovered executable parent and path join");
    require(f.config->resolve_path_0192d0("D:\\absolute.png",u"C:\\fixture\\host.exe",true)==u"D:\\absolute.png","absolute drive path bypasses relative prefix");
    require(f.config->resolve_path_0192d0("\\rooted.png",u"C:\\fixture\\host.exe",true)==u"C:\\rooted.png","rooted path retains executable drive through recovered join");
    require(f.config->resolve_path_0192d0("relative.png",u"C:\\fixture\\host.exe",false)==u"relative.png","disabled-relative policy returns converted path unchanged");
    require(f.config->resolve_path_0192d0("\\\\server\\share\\a",u"C:\\fixture\\host.exe",true)==u"\\\\server\\share\\a","UNC root is recognized by existing absolute-path algorithm");
    f.config->log_config_path(u"anadius.cfg");require(f.lower->logs==1,"unselected configuration logging remains explicit lower policy");
    rejects<std::logic_error>([&]{f.config->load_document(u"anything");},"unselected document parser policy is never silently fabricated");
    rejects<std::logic_error>([&]{f.config->apply_license_key_02a830("fixture");},"unselected licensing policy remains explicit");
    f.lower->fail_mkdir=true;rejects<std::runtime_error>([&]{f.config->create_directories_018e00(u"LOCAL\\anadius\\LSX emu");},"lower persistence/directory failure propagates through configuration platform");
    rejects<std::invalid_argument>([&]{p::ConfigurationPlatform invalid({});},"configuration platform rejects absent lower policy");
    auto bad=path_api();bad.environment=nullptr;
    rejects<std::invalid_argument>([&]{p::ConfigurationPlatform invalid(std::make_unique<b5_test::Policies>(f.events),std::make_unique<Encoding>(),std::make_unique<encoding_recovered::PortableErrors>(),bad);},"configuration platform rejects incomplete SDK table before effects");
    rejects<std::invalid_argument>([&]{p::ConfigurationPlatform invalid(std::make_unique<b5_test::Policies>(f.events),{},std::make_unique<encoding_recovered::PortableErrors>(),path_api());},"configuration platform rejects absent conversion provider");
    rejects<std::invalid_argument>([&]{p::ConfigurationPlatform invalid(std::make_unique<b5_test::Policies>(f.events),std::make_unique<Encoding>(),{},path_api());},"configuration platform rejects absent error provider");
}
void application_memory(){
    DomainFixture f;auto memory=f.domain->make_memory();
    require(f.domain->provider_leases()==1&&memory->outstanding_allocations()==0,"application memory retains one actual domain lease before allocation");
    auto* value=memory->allocate(19);require(value&&memory->outstanding_allocations()==1&&f.sdk.size()==2,"application byte allocation uses same observed process heap as main PTD");
    std::memset(value,0x5a,19);memory->deallocate(value,999);
    require(memory->outstanding_allocations()==0&&f.sdk.size()==1&&f.sdk.valid,"observed sized delete ignores size and frees exact owned process-heap block");
    rejects<std::invalid_argument>([&]{memory->deallocate(value,19);},"double free is rejected before native heap call");
    int foreign=0;rejects<std::invalid_argument>([&]{memory->deallocate(&foreign,sizeof(foreign));},"foreign pointer is rejected before native heap call");
    const auto frees=f.sdk.frees.load();memory->deallocate(nullptr,17);require(f.sdk.frees==frees,"null delete has no native free side effect");
    auto* zero=memory->allocate(0);require(zero&&memory->outstanding_allocations()==1,"zero-sized new produces owned nonnull minimum allocation");memory->deallocate(zero,0);
    auto* retry=memory->allocate(7);f.sdk.deny_free=true;bool error=false;
    try{memory->deallocate(retry,7);}catch(const std::system_error& e){error=e.code().value()==ERROR_ACCESS_DENIED;}
    const auto native_error=::GetLastError();
    require(error&&native_error==ERROR_ACCESS_DENIED&&memory->outstanding_allocations()==1&&f.sdk.size()==2&&f.domain->errno_cell().load()==13,"known HeapFree failure preserves native LastError, retains ownership and maps same-domain errno before explicit retry");
    f.sdk.deny_free=false;memory->deallocate(retry,0);require(memory->outstanding_allocations()==0&&f.sdk.size()==1,"retry frees retained failed-release allocation exactly once");
    f.sdk.deny_alloc=true;rejects<std::bad_alloc>([&]{memory->allocate(33);},"missing new handler exhaustion throws selected compiler bad_alloc");
    require(f.domain->errno_cell().load()==12&&memory->outstanding_allocations()==0,"allocation failure writes same-domain error without phantom ticket");f.sdk.deny_alloc=false;
    require(!f.domain->close()&&f.domain->phase()==d::Phase::Stopping&&f.sdk.size()==1,"live allocator lease blocks actual domain teardown even with no blocks");
    rejects<std::logic_error>([&]{f.domain->make_memory();},"stopping domain cannot admit new memory provider");
    memory.reset();require(f.domain->provider_leases()==0&&f.domain->close()&&f.sdk.size()==0&&f.sdk.valid,"allocator destruction releases lease before real FLS/main-PTD cleanup");
}
void allocation_new_handler_and_services(){
    DomainFixture f(true);auto memory=f.domain->make_memory();f.sdk.deny_alloc=true;
    auto* value=memory->allocate(27);require(value&&f.sdk.handlers==1&&f.sdk.handler_bytes==27,"recovered operator-new loop invokes selected same-domain handler with exact request");memory->deallocate(value,27);
    b2_test::Track track;auto lower=std::make_unique<b2_test::Backend>(track);auto* raw=lower.get();
    {
        p::SourceServices services(memory,std::move(lower));
        raw->guard_hook=[]{throw std::logic_error("lower guard must not run");};services.guard_return();
        require(b4_test::absent(track.events,"guard-return"),"observed final guard RET is concrete and bypasses lower policy");
        value=services.allocate(8);require(value&&raw->allocated.empty()&&memory->outstanding_allocations()==1,"Services allocation reaches shared recovered heap rather than lower allocator");
        services.deallocate(value,8);require(raw->deleted.empty()&&memory->outstanding_allocations()==0,"Services delete reaches matching domain ownership");
        auto* stream=reinterpret_cast<void*>(0x1234);require(services.log_insert(stream,0x5678)==stream,"remaining log policy preserves opaque identity");services.log_finish(stream);
        bool mapped_error=false;std::string error_context;
        try{services.thread_error(6);}catch(const std::system_error& e){
            mapped_error=e.code().value()==11&&e.code().category()==std::generic_category();error_context=e.what();
        }
        require(mapped_error,"B9 thread-error route selects retained code11 and rebuilt generic category instead of lower provider");
        require(error_context.find("resource unavailable try again")!=std::string::npos,"B9 thread-error retains exact source context in rebuilt exception");
        require(b4_test::pos(track.events,"bootstrap-log-part")<b4_test::pos(track.events,"bootstrap-log-finish"),"selected policy binding does not replace remaining log sequence");
    }
    rejects<std::invalid_argument>([&]{p::SourceServices invalid({},std::make_unique<b2_test::Backend>(track));},"Services binding rejects absent application-memory provider");
    rejects<std::invalid_argument>([&]{p::SourceServices invalid(memory,{});},"Services binding rejects absent remaining policy owner");
    memory.reset();require(f.domain->close()&&f.sdk.valid&&f.sdk.size()==0,"service adapter lifetime retains no hidden allocation or domain lease");
}
int terminal_exit=90;
int terminal(const char* mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::set_terminate([]{std::_Exit(terminal_exit);});
    DomainFixture f;auto memory=f.domain->make_memory();
    if(std::string_view(mode)=="--outstanding"){memory->allocate(8);terminal_exit=90;}
    else if(std::string_view(mode)=="--unknown-allocate"){
        terminal_exit=91;f.sdk.throw_after_alloc=true;bool caught=false;try{memory->allocate(8);}catch(const std::runtime_error&){caught=true;}
        if(!caught)std::_Exit(89);f.sdk.throw_after_alloc=false;
    }else{
        terminal_exit=92;auto* value=memory->allocate(8);f.sdk.throw_after_free=true;bool caught=false;try{memory->deallocate(value,8);}catch(const std::runtime_error&){caught=true;}
        if(!caught)std::_Exit(89);f.sdk.throw_after_free=false;
    }
    memory.reset();std::_Exit(89);
}
}
int main(int argc,char** argv){if(argc==2)return terminal(argv[1]);try{
    configuration_sdk_paths();configuration_encoding();configuration_resolution_and_limits();application_memory();allocation_new_handler_and_services();
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--outstanding",static_cast<const char*>(nullptr))==90,"outstanding application allocation prevents unsafe provider destruction in own child");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--unknown-allocate",static_cast<const char*>(nullptr))==91,"unknown native allocation outcome remains terminal rather than guessed rollback in own child");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--unknown-free",static_cast<const char*>(nullptr))==92,"unknown native free outcome remains terminal rather than double-free retry in own child");
    std::cout<<"PASS: "<<b7_test::count()<<" bootstrap-policy core checks; fixed-step SDK paths, source encoding/resolution, same-domain operator new/delete and explicit release/error policies; original targets never executed\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
