#include "platform_test_fixture.hpp"

namespace {
using namespace b14_test;
void guards_lifetime(){
    Graph g;
    rejects<std::invalid_argument>([]{hp::HostPlatform h({});},"null host domain rejected");
    rejects<std::invalid_argument>([]{hp::HookCleanup c({});},"null cleanup domain rejected");
    rejects<std::invalid_argument>([&]{hp::Domain d({},g.engine,g.resolver,g.callback_lifetime);},"null stream rejected");
    rejects<std::invalid_argument>([&]{hp::Domain d(g.output,{},g.resolver,g.callback_lifetime);},"null engine rejected");
    rejects<std::invalid_argument>([&]{hp::Domain d(g.output,g.engine,{},g.callback_lifetime);},"null resolver rejected");
    rejects<std::invalid_argument>([&]{hp::Domain d(g.output,g.engine,g.resolver,{});},"null explicit callback lifetime rejected");
    for(unsigned field=0;field<6;++field){auto r=g.resources();switch(field){case 0:r.encoding.reset();break;case 1:r.errors.reset();break;case 2:r.api.condition=nullptr;break;case 3:r.api.verify=nullptr;break;case 4:r.api.module=nullptr;break;default:r.api.procedure=nullptr;}
        rejects<std::invalid_argument>([&]{hp::Domain d(g.output,g.engine,g.resolver,g.callback_lifetime,std::move(r));},"incomplete encoding/SDK resource rejected");}
    require(g.engine->events.empty()&&g.output->str().empty()&&g.plan.modules.empty()&&g.plan.verifies==0,"constructors and rejected inputs perform no engine/SDK/log operation");
    auto stream=std::make_shared<b11_test::OwnedStream>();auto engine=std::make_shared<Engine>();auto resolver=std::make_shared<Resolver>();auto lifetime=std::make_shared<int>(23);
    std::weak_ptr<b11_test::OwnedStream> weak_stream=stream;std::weak_ptr<Engine> weak_engine=engine;std::weak_ptr<int> weak_lifetime=lifetime;
    auto domain=std::make_shared<hp::Domain>(stream,engine,resolver,lifetime,g.resources());
    auto host=std::make_unique<hp::HostPlatform>(domain);auto cleanup=std::make_unique<hp::HookCleanup>(domain);
    stream.reset();engine.reset();resolver.reset();lifetime.reset();domain.reset();
    host->log_text_006880("succeeded");host->finish_log_0088e0();host.reset();
    require(!weak_stream.expired()&&!weak_engine.expired()&&!weak_lifetime.expired(),"cleanup keeps shared domain and callback lifetime after host owner dies");
    require(weak_stream.lock()->str()=="succeeded\n"&&weak_engine.lock()->events.empty(),"owner destruction does not invent engine shutdown");
    cleanup.reset();require(weak_stream.expired()&&weak_engine.expired()&&weak_lifetime.expired(),"all dependencies released after final shared wrapper");
    bootstrap_backend::Resources resources;auto bound=hp::bind_host_platform(std::move(resources),g.domain);
    require(dynamic_cast<hp::HostPlatform*>(bound.host.get())&&dynamic_cast<hp::HookCleanup*>(bound.hook_cleanup.get()),"binder supplies both wrappers from shared domain");
    require(!bound.online&&!bound.server&&!bound.remaining,"binder invents no foreign/server/Services providers");
}
void version_abi(){
    Graph g;hi::VersionRequest request;
    request.size=0x11c;request.major=0x12345678;request.minor=0x87654321;request.build=0x11223344;request.platform=0xaabbccdd;
    for(std::size_t i=0;i<request.service_pack_text.size();++i)request.service_pack_text[i]=static_cast<char16_t>(0x400+i);
    request.service_pack_major=0x1234;request.service_pack_minor=0x5678;request.suite=0xabcd;request.product_type=0x91;request.reserved=0xe7;
    require(g.host->verify_version(request,0xfedcba98,0x12345678abcdef90ULL),"full BOOL0x100 is true rather than lowbyte false");
    const auto& v=g.plan.version;
    require(v.dwOSVersionInfoSize==request.size&&v.dwMajorVersion==request.major&&v.dwMinorVersion==request.minor&&v.dwBuildNumber==request.build&&v.dwPlatformId==request.platform,"five DWORD fields copied without truncation");
    require(std::equal(request.service_pack_text.begin(),request.service_pack_text.end(),std::begin(v.szCSDVersion)),"all128codeunits copied into typed WCHAR storage including final unit");
    require(v.wServicePackMajor==0x1234&&v.wServicePackMinor==0x5678&&v.wSuiteMask==0xabcd&&v.wProductType==0x91&&v.wReserved==0xe7,"WORD/BYTE fields copied at SDK offsets");
    require(g.plan.types==0xfedcba98&&g.plan.mask==0x12345678abcdef90ULL,"complete type mask and64bit condition mask preserved");
    g.plan.result=FALSE;require(!g.host->verify_version(hi::VersionRequest{},0x23,0),"FALSE return preserved");
    require(g.plan.version.dwMajorVersion==6&&g.plan.version.dwMinorVersion==2&&g.plan.version.wServicePackMajor==0&&g.plan.version.szCSDVersion[127]==0,"default request contains observed6.2/SP0 and zero fields");
    const auto mask=g.host->version_condition_mask(0xabcdef0000000000ULL,VER_MAJORVERSION,VER_GREATER_EQUAL);
    require(mask==::VerSetConditionMask(0xabcdef0000000000ULL,VER_MAJORVERSION,VER_GREATER_EQUAL)&&g.plan.masks.back().mask==0xabcdef0000000000ULL,"typed condition API preserves full incoming mask");
}
void lookup_widen(){
    Graph g;
    for(const auto* name:{u"ws2_32.dll",u"KernelBase.dll",u"advapi32.dll",u"Kernel32.dll",u"shell32.dll"})require(g.host->get_module_handle(name)==0x8000,"each observed module uses typed lookup");
    g.plan.missing_module=L"ws2_32.dll";require(g.host->get_module_handle(u"ws2_32.dll")==0,"missing module result retained without LoadLibrary fallback");
    const auto modules=g.plan.modules.size();rejects<std::domain_error>([&]{(void)g.host->get_module_handle(u"");},"empty module does not become current executable");
    rejects<std::domain_error>([&]{(void)g.host->get_module_handle(u"unknown.dll");},"unobserved module domain rejected");
    require(g.plan.modules.size()==modules,"rejected module inputs never reach SDK");
    for(const auto* name:{"connect","RegOpenKeyExA","RegOpenKeyExW","RegCloseKey","RegQueryValueExA","RegQueryValueExW","CreateProcessA","CreateProcessW","SHGetFolderPathA","SHGetFolderPathW"})require(g.host->get_proc_address(0x8000,name)!=0,"all ten observed export names supported");
    g.plan.missing_proc="connect";require(g.host->get_proc_address(0x8000,"connect")==0,"missing procedure result preserved");
    const auto procedures=g.plan.procedures.size();rejects<std::domain_error>([&]{(void)g.host->get_proc_address(0x8000,std::string_view("connect\0x",9));},"NUL-extended unknown export rejected before scan");
    require(g.plan.procedures.size()==procedures&&g.plan.procedures.front().first==0x8000,"lookup module operand retained and unknown names cause no SDK call");
    require(g.host->filesystem_widen_006d70("own_\xc4\xb1")==u"own_\u0131","filesystem widening uses existing encoding source");
    require(g.encoding->flags==std::vector<std::uint32_t>({8,8})&&g.encoding->cps==std::vector<std::uint32_t>({65001,65001}),"filesystem codepage/strict flags retained");
}
void engine_and_logs(){
    Graph g;g.engine->init=1;require(g.host->initialize_engine_046a20()==1,"already-initialized engine result stays failure code1");
    std::uintptr_t original=0xabc;
    require(g.host->create_hook_0466e0(0x1234,hi::Callback::connect_00fd00,original)==0&&original==0x101234,"existing adapter copies original on success");
    require(g.engine->detours.back()==0x90000&&g.resolver->seen.back()==hi::Callback::connect_00fd00,"engine gets explicit rebuilt resolver address rather than original VA");
    g.engine->create_results[1]=8;original=0xabc;
    require(g.host->create_hook_0466e0(0x2234,hi::Callback::reg_open_a_00f800,original)==8&&original==0x102234,"original output survives failed normal create return");
    g.engine->throw_create=true;original=0xabc;rejects<std::runtime_error>([&]{(void)g.host->create_hook_0466e0(0x3234,hi::Callback::reg_open_w_00f960,original);},"injected create exception propagates");
    require(original==0xabc,"existing adapter does not invent copyback after unknown throwing engine outcome");
    g.engine->throw_create=false;g.engine->enable_results[0]=5;require(g.host->enable_hook_046a10(0x1234)==5,"enable result retained");
    g.engine->disable_results[0]=6;g.engine->uninit=2;
    require(g.cleanup->disable_hook_046a00(0x1234)==6&&g.cleanup->uninitialize_engine_046ac0()==2,"cleanup uses same selected engine and preserves errors");
    g.host->log_text_006880("MinHook initialization ");g.host->log_text_006880("failed");g.host->finish_log_0088e0();
    g.host->log_text_006880("Hooking function ");g.host->log_text_006880("connect");g.host->log_text_006880(" failed; error ");g.host->log_integer_00af90(8);g.host->finish_log_0088e0();
    g.cleanup->log_literal_006880("MinHook uninitialization ");g.cleanup->log_literal_006880("failed");g.cleanup->log_finish_0088e0();
    require(g.output->str()=="MinHook initialization failed\nHooking function connect failed; error 8\nMinHook uninitialization failed\n"&&g.output->flushes==3,"host and cleanup share exact formatted output stream");
    const auto before=g.output->str();rejects<std::domain_error>([&]{g.host->log_text_006880("unknown");},"unknown literal rejected before output");require(g.output->str()==before,"unsupported literal does not modify output");
    g.output->str("");*g.output<<std::hex<<std::showbase;g.host->log_integer_00af90(26);require(g.output->str()=="0x1a","integer insertion uses selected compiler stream formatting rather than decimalizing");
    Graph bad;bad.output->maximum=3;bad.output->exceptions(std::ios::badbit);rejects<std::ios_base::failure>([&]{bad.host->log_text_006880("failed");},"stream exception remains visible");
    require(bad.output->str()=="fai"&&bad.output->flushes==0,"stream exception preserves partial prefix");
}
}
int main(){try{guards_lifetime();version_abi();lookup_widen();engine_and_logs();std::cout<<"PASS: "<<b14_test::checks<<" host platform core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
