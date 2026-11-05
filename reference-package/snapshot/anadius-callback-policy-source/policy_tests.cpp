#include "policy_test_fixture.hpp"
#include <cerrno>
#include <csignal>
#include <process.h>
#include <limits>
#include "../anadius-shared-log-source/stream.hpp"

namespace {
using namespace b16_test;
void* observed_output=nullptr;
bool expected_clear=false,throw_copy=false;
int safe_error_copy(wchar_t* out,std::size_t count,const wchar_t* input){
    if(expected_clear)require(read(observed_output).empty(),"actual output cleared before selected CRT handler");
    if(throw_copy)throw std::runtime_error("own invalid handler");
    if(!out||!count||!input){errno=EINVAL;if(out&&count)*out=0;return EINVAL;}
    if(std::wcslen(input)+1>count){*out=0;errno=ERANGE;return ERANGE;}
    return ::wcscpy_s(out,count,input);
}
void wide_operations(){
    cp::ByteWide wide;
    require(wide.compare_utf16(L"AbC",u"aBc")==0,"compiler comparison accepts mixed real wide backing");
    require(wide.compare_utf16(u"\u0131\u00e9",u"\u0131\u00e9")==0,"non ASCII units preserved");
    std::array<std::byte,17> bytes;bytes.fill(std::byte{0x55});const char16_t input[]{u'A',0xd83d,0xde03,0};
    require(wide.copy_utf16(bytes.data()+1,5,input)==0,"unaligned output byte bridge");
    require(read(bytes.data()+1)==std::u16string(input),"surrogate code units retained");
    require(bytes.front()==std::byte{0x55}&&bytes[9]==std::byte{0x55},"wide copy preserves prefix and unused tail");
    require(wide.compare_utf16(bytes.data()+1,input)==0,"unaligned input byte bridge");
    wchar_t sdk[8];std::fill(std::begin(sdk),std::end(sdk),L'!');
    require(wide.copy_utf16(sdk,8,u"Hi")==0&&std::wstring(sdk)==L"Hi","real SDK output not type punned");
    require(sdk[3]==L'!',"compiler copy suffix untouched");
    require(wide.copy_utf16(sdk,3,u"Hi")==0&&sdk[2]==0,"exact capacity includes terminator");
    const char16_t embedded[]{u'X',0,u'Y',0};
    require(wide.copy_utf16(sdk,8,embedded)==0&&std::wstring(sdk)==L"X","copy stops at first NUL");
    rejects<std::domain_error>([&]{wide.copy_utf16(sdk,8,sdk);},"overlap is explicit out of domain");
    rejects<std::domain_error>([&]{wide.copy_utf16(sdk,std::size_t(INT_MAX)+1,u"x");},"capacity boundary precedes allocation");
    auto api=cp::compiler_crt_api();api.copy_w=safe_error_copy;cp::ByteWide checked(api);observed_output=sdk;expected_clear=true;
    sdk[0]=L'Q';require(checked.copy_utf16(sdk,2,u"too long")==ERANGE&&sdk[0]==0,"undersize clears actual output and returns CRT range error");
    sdk[0]=L'Q';require(checked.copy_utf16(sdk,8,nullptr)==EINVAL&&sdk[0]==0,"null source clears before handler");
    sdk[0]=L'Q';throw_copy=true;rejects<std::runtime_error>([&]{checked.copy_utf16(sdk,1,u"xx");},"nonreturning handler prefix retained");
    require(sdk[0]==0,"throw preserves actual preclear");throw_copy=false;expected_clear=false;
    require(checked.copy_utf16(nullptr,2,u"x")==EINVAL,"null destination reaches explicit CRT policy");
    require(checked.copy_utf16(sdk,0,u"x")==EINVAL,"zero count reaches CRT policy");
    api.compare_w=nullptr;rejects<std::invalid_argument>([&]{cp::ByteWide invalid(api);},"missing CRT member rejected");
}
void policies(){
    Fixture f;cp::Policy policy(f.view(),f.resources());
    require(policy.original_slots_identity()==&f.driver.runtime.originals,"concrete policy uses final Runtime slots");
    require(policy.owned_handles()==0,"new policy starts without adopted handles");
    const auto handle=policy.allocate_4_04b0bc();require(handle!=0&&policy.owned_handles()==1&&f.memory->outstanding_allocations()==1,"four byte handle owned by same memory provider");
    require(f.heap.memory.blocks.at(reinterpret_cast<void*>(handle))==4,"actual allocator receives four bytes");
    policy.free_04b0f8(handle);require(!policy.owned_handles()&&!f.memory->outstanding_allocations(),"single free releases both ledgers");
    rejects<std::invalid_argument>([&]{policy.free_04b0f8(handle);},"double free rejected");
    rejects<std::invalid_argument>([&]{policy.free_04b0f8(0x1234);},"foreign free rejected");
    f.heap.memory.deny_alloc=true;rejects<std::bad_alloc>([&]{policy.allocate_4_04b0bc();},"allocation failure propagates");f.heap.memory.deny_alloc=false;
    require(!policy.owned_handles()&&!f.memory->outstanding_allocations(),"failed allocation leaves no new live handle");
    require(policy.crt_stricmp("aBc","AbC")==0,"selected compiler narrow comparison");char copy[8]{};
    require(policy.crt_strcpy_s(copy,8,"own")==0&&std::string(copy)=="own","selected compiler narrow copy");
    const std::u16string nul{u'A',0,u'B'};
    require(policy.filesystem_narrow(nul)==std::string("A\0B",3)&&f.conversion->cp==65001,"explicit length conversion preserves NUL");
    f.conversion->locale=1252;f.conversion->ansi=false;(void)policy.filesystem_narrow(u"own");
    require(f.conversion->cp==1,"filesystem OEM page selected");
    f.conversion->ansi=true;(void)policy.filesystem_narrow(u"own");require(f.conversion->cp==0,"filesystem ANSI page selected");
    f.conversion->fail=true;rejects<std::system_error>([&]{policy.filesystem_narrow(u"own");},"conversion failure not silently replaced");f.conversion->fail=false;
    policy.wait_036880(0);policy.wait_steps_036880(0);require(f.driver.runtime.frequency_cache==10000000,"both wait routes use actual Execution");
    policy.log_literal_006880(std::uint64_t{0x1800808e0});policy.log_text_007990(std::string_view("en\0US",5));policy.finish_log_0088e0();
    require(f.output->str()==std::string("replacing the language with ")+std::string("en\0US",5)+"\n","counted log keeps embedded NUL and finish order");
    const auto before=f.output->str();rejects<std::domain_error>([&]{policy.log_literal_006880(std::uint64_t{0x180000001});},"unknown VA never dereferenced");
    require(before==f.output->str(),"unknown literal leaves stream unchanged");
    rejects<std::domain_error>([&]{policy.log_literal_006880(std::string_view("unknown"));},"unknown network literal rejected");
    f.output->str("");policy.log_literal_006880(std::string_view("game connected on port "));policy.log_port_00ad70(3216);policy.log_literal_006880(std::string_view("; redirecting to "));policy.log_port_00ad70(6000);policy.log_finish_0088e0();
    require(f.output->str()=="game connected on port 3216; redirecting to 6000\n","source stream formatting/log sequence");
    policy.dialog_038bc0("own","owned text",0x10);require(f.ui.messages.size()==1&&f.ui.messages[0].text==L"owned text"&&f.ui.messages[0].flags==0x10,"existing dialog adapter with own fake SDK");
    bool terminal=false;try{policy.exit_05cdc0(0xffffffff);}catch(const Terminal& t){terminal=t.code==0xffffffff;}
    require(terminal,"explicit terminal preserves low32 exit code");
    bool foreign=false;std::thread worker([&]{try{policy.log_finish_0088e0();}catch(const std::logic_error&){foreign=true;}});worker.join();require(foreign,"other-thread policy call rejected without PTD adoption");
    auto seeded=f.view();f.callbacks.fabricated.insert(0x1234);rejects<std::invalid_argument>([&]{cp::Policy bad(seeded,f.resources());},"unowned fabricated seed rejected");f.callbacks.fabricated.clear();
    auto factory=cp::make_policy_factory(f.resources());auto copied=factory;auto owned=factory(f.view());require(owned->original_slots_identity()==&f.driver.runtime.originals,"one shot factory keeps original array identity");
    rejects<std::logic_error>([&]{copied(f.view());},"copied policy factory cannot reuse resources");
    auto missing=f.resources();missing.memory.reset();rejects<std::invalid_argument>([&]{cp::Policy bad(f.view(),std::move(missing));},"missing memory rejected");
}
struct BodyBuffer final:std::stringbuf {
    bool& permission;bool revoke=false;
    explicit BodyBuffer(bool& p):permission(p){}
    int sync()override{if(revoke)permission=false;::SetLastError(0x6a55);return 0;}
};
bool* exit_permission=nullptr;bool revoke_exit=false;
void body_throwing_exit(std::uint32_t code){if(revoke_exit)*exit_permission=false;::SetLastError(0x6a66);throw Terminal{code};}
void network_effect_postchecks(){
    Fixture f;bool permission=true;auto buffer=std::make_shared<BodyBuffer>(permission);auto stream=shared_log::make_stream(buffer);
    auto resources=f.resources();resources.stream=stream;resources.exit=body_throwing_exit;
    resources.connect_body_scope=[&]{
        // Probe can reacquire the managed stream: no effect lease may survive
        // into either pre/postcheck. Error pollution is metadata only.
        const auto lease=shared_log::acquire(*stream);::SetLastError(0xdead);return permission;
    };
    cp::Policy policy(f.view(),std::move(resources));
    ::SetLastError(0x5511);policy.log_finish_0088e0();require(::GetLastError()==0x6a55,"postcheck preserves lower flush LastError and runs outside its lease");
    buffer->str("");buffer->revoke=true;std::int32_t v4=16,v6=28;std::uint16_t port=4660;std::uint32_t attempts=0;
    std::array<std::byte,4> address{std::byte{2},std::byte{0},std::byte{0x0c},std::byte{0x90}};const auto old=address;
    rejects<std::logic_error>([&]{network_hook_recovered::connect_00fd00({v4,v6,port,attempts},policy,1,address,16);},"permission revoked by final flush fails before caller counter/port commit");
    require(attempts==0&&address==old&&f.driver.probe->connects==0&&buffer->str()=="game connected on port 3216; redirecting to 4660\n"&&::GetLastError()==0x6a55,
        "completed log prefix retained but counter/port/original remain untouched");
    permission=true;exit_permission=&permission;revoke_exit=false;
    bool identical=false;try{policy.exit_05cdc0(41);}catch(const Terminal& value){identical=value.code==41;}
    require(identical&&::GetLastError()==0x6a66,"valid exceptional postcheck rethrows identical terminal value and retains lower error");
    revoke_exit=true;rejects<std::logic_error>([&]{policy.exit_05cdc0(42);},"invalid exceptional postcheck takes precedence over lower terminal exception");
    require(!permission&&::GetLastError()==0x6a66,"exceptional metadata check does not erase lower outgoing error");
    exit_permission=nullptr;revoke_exit=false;
}
void expected_abort(int){std::_Exit(97);}
int terminal(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);
    Fixture f;
    if(mode=="--live-handle"){cp::Policy policy(f.view(),f.resources());(void)policy.allocate_4_04b0bc();return 98;}
    if(mode=="--returning-exit"){auto r=f.resources();r.exit=returning_exit;cp::Policy policy(f.view(),std::move(r));policy.exit_05cdc0(9);}
    return 98;
}
}
int main(int argc,char** argv){if(argc==2)return terminal(argv[1]);try{wide_operations();policies();network_effect_postchecks();
    for(const auto* mode:{"--live-handle","--returning-exit"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==97,"own terminal child enforces ownership/exit invariant");
    std::cout<<"PASS: "<<b16_test::checks<<" callback policy core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
