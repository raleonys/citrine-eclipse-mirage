// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/online.hpp"
#include "preloader_l/runtime/delegating_effects.hpp"
#include "preloader_l/core/paths.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <new>

namespace profile_copy_allocation {
struct Failure final : std::bad_alloc {};
struct Observation {
    std::size_t minimum=0;
    unsigned hits=0;
    bool armed=false,fail=false;
};
thread_local Observation* active=nullptr;
class Scope {
    Observation value_;
    Observation* previous_=active;
public:
    Scope(){active=&value_;}
    ~Scope(){active=previous_;}
    Scope(const Scope&)=delete;
    Scope& operator=(const Scope&)=delete;
    void arm(std::size_t minimum,bool fail){value_.minimum=minimum;value_.fail=fail;value_.armed=true;}
    void disarm(){value_.armed=false;}
    unsigned hits() const{return value_.hits;}
};
void observe(std::size_t size){
    // Arm only after the selected getter's fixture bookkeeping. Smaller MSVC
    // Debug iterator proxies remain unselected; the next large allocation is
    // the throwing character-buffer allocation for its prebuilt borrowed text.
    if(active&&active->armed&&size>=active->minimum&&size<active->minimum*2){
        active->armed=false;++active->hits;
        if(active->fail)throw Failure{};
    }
}
}

namespace cookie_lifetime {
const void* tracked=nullptr;
const std::uintptr_t* object=nullptr;
std::uintptr_t expected=0;
unsigned released=0;
bool committed=false;
void release(void* pointer) noexcept {
    if(pointer&&pointer==tracked){
        tracked=nullptr;++released;committed=object&&*object==expected;
    }
    std::free(pointer);
}
}
void* operator new(std::size_t size){profile_copy_allocation::observe(size);if(auto* p=std::malloc(size?size:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t size){return ::operator new(size);}
void operator delete(void* p) noexcept {cookie_lifetime::release(p);}
void operator delete[](void* p) noexcept {cookie_lifetime::release(p);}
void operator delete(void* p,std::size_t) noexcept {cookie_lifetime::release(p);}
void operator delete[](void* p,std::size_t) noexcept {cookie_lifetime::release(p);}

namespace o = preloader_l::online;
namespace s = preloader_l::status;
namespace f = preloader_l::file;
namespace e = preloader_l::encoding;
namespace l = preloader_l::system_services::libraries;
static unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) throw std::runtime_error("Check failed at line " + std::to_string(__LINE__) + ": " #x); } while(false)
struct Injected : std::runtime_error { Injected() : std::runtime_error("synthetic failure") {} };
template<class Exception, class F> void throws(F body) {
    bool caught = false;
    try { body(); } catch (const Exception&) { caught = true; }
    CHECK(caught);
}
struct State {
    o::Path exe=u"C:\\fixture\\app.exe", cookie=u"C:\\fixture\\cookie.synthetic",
        profile=u"C:\\fixture\\profile.synthetic", last_avatar=u"C:\\fixture\\avatar.synthetic", avatar=u"old-avatar";
    o::Module module=0;
    o::Object object=0;
    std::string username="old-name", persona="old-persona", user="old-user";
    std::uint8_t use_last=0, online=0;
    o::StateRefs refs() { return {exe,cookie,profile,last_avatar,avatar,module,object,username,persona,user,use_last,online}; }
};
struct Mock : o::Effects {
    std::vector<std::string> calls;
    std::string fail, written, read_value="synthetic-persona;synthetic-user;synthetic-name";
    o::Path loaded_path, cookie_copy;
    std::map<o::Path,s::Status> statuses;
    std::uint32_t mode=0x20;
    o::Module loaded=41;
    o::Procedure version_proc=51, factory_proc=61;
    o::Object created=71;
    std::int32_t version=2, auth_status=0;
    std::string name="provider-name", persona="provider-persona", user="provider-user";
    bool null_name=false, null_persona=false, null_user=false, free_result=true, write_result=true;
    std::function<void(std::string_view)> inspect;
    std::function<void(const char16_t*)> inspect_cookie;
    void event(std::string value) {
        calls.push_back(value);
        if (inspect) inspect(value);
        if (fail==value) throw Injected();
    }
    bool called(std::string_view value) const { return std::find(calls.begin(),calls.end(),value)!=calls.end(); }
    std::uint32_t suppress_errors_0450a0() override {event("suppress");const auto previous=mode;mode|=1;return previous;}
    void restore_error_mode(std::uint32_t value) override {event("restore");mode=value;}
    o::Path filesystem_widen_006d70(std::string_view value) override {
        event("widen"); CHECK(value=="preloader_l_online.dll"); CHECK(value.size()==0x16);
        return o::Path(value.begin(),value.end());
    }
    o::Path parent_path_019820(const o::Path& value) override {event("parent");return preloader_l::path::parent_path_019820(value);}
    o::Path join_016b00(const o::Path& a,const o::Path& b) override {event("join");return preloader_l::path::join_016b00(a,b);}
    o::Module load_copy_0450d0(const o::Path& value) override {event("load");loaded_path=value;return loaded;}
    o::Procedure get_proc_address(o::Module module,std::string_view value) override {
        CHECK(module==loaded);event("resolve:"+std::string(value));
        return value=="version_check"?version_proc:factory_proc;
    }
    std::int32_t call_version_check(o::Procedure value) override {CHECK(value==version_proc);event("version");return version;}
    o::Object call_create_online(o::Procedure value,const char16_t* cookie) override {
        CHECK(value==factory_proc);event("create");cookie_copy=cookie;
        if(inspect_cookie)inspect_cookie(cookie);
        return created;
    }
    std::int32_t status_virtual_08(o::Object value) override {CHECK(value==created);event("auth-status");return auth_status;}
    const char* username_virtual_10(o::Object value) override {CHECK(value==created);event("name");return null_name?nullptr:name.c_str();}
    const char* persona_virtual_18(o::Object value) override {CHECK(value==created);event("persona");return null_persona?nullptr:persona.c_str();}
    const char* user_id_virtual_20(o::Object value) override {CHECK(value==created);event("user");return null_user?nullptr:user.c_str();}
    void destroy_virtual_00(o::Object value) override {CHECK(value!=0);event("destroy");}
    bool free_library(o::Module value) override {CHECK(value!=0);event("free");return free_result;}
    s::Status status_01ea20(const o::Path& value,std::uint32_t flags) override {
        CHECK(flags==3); event(value==u"C:\\fixture\\cookie.synthetic"?"status-cookie":"status-avatar");
        const auto it=statuses.find(value);return it==statuses.end()?s::Status{1,0xffff,2}:it->second;
    }
    [[noreturn]] void filesystem_error_018130(std::string_view operation,std::uint32_t code,const o::Path&) override {
        CHECK(operation=="status");CHECK(code!=0);event("filesystem-error");throw Injected();
    }
    std::string read_binary_03b3f0(const o::Path& path) override {CHECK(path==u"C:\\fixture\\profile.synthetic");event("read");return read_value;}
    bool write_binary_03c1d0(const o::Path& path,std::string_view value) override {
        CHECK(path==u"C:\\fixture\\profile.synthetic");event("write");written=value;return write_result;
    }
    void log_literal_006880(std::string_view value) override {event("log:"+std::string(value));}
    void log_finish_0088e0() override {event("flush");}
    void dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags) override {
        CHECK(title=="crack error");CHECK(text=="Online library version mismatch!");CHECK(flags==0x10);event("dialog");
    }
    void wait_steps_036880(std::int32_t count) override {CHECK(count==1);event("wait");}
    void finish_flag_036820() override {event("finish");}
};
static void compare_calls(const Mock& m,std::initializer_list<const char*> expected) {
    std::vector<std::string> wanted;
    for(auto value:expected)wanted.emplace_back(value);
    CHECK(m.calls==wanted);
}
static void entry_branches() {
    {
        State state;Mock m;m.loaded=0;o::worker_035b60(state.refs(),m);
        compare_calls(m,{"wait","suppress","widen","parent","join","load","restore","finish"});
        CHECK(m.mode==0x20);CHECK(state.module==0);CHECK(state.online==0);
        CHECK(m.loaded_path==u"C:\\fixture\\preloader_l_online.dll");
    }
    {
        State state;Mock m;m.version_proc=0;o::worker_035b60(state.refs(),m);
        compare_calls(m,{"wait","suppress","widen","parent","join","load","restore",
            "log:online dll found!","flush","resolve:version_check","log:version function not found","flush","free","finish"});
        CHECK(state.module==0);CHECK(state.object==0);
    }
    for(auto version:std::array<std::int32_t,5>{-1,0,1,3,0x7fffffff}) {
        State state;Mock m;m.version=version;o::worker_035b60(state.refs(),m);
        CHECK(m.called("dialog"));CHECK(!m.called("resolve:create_online"));CHECK(m.called("free"));CHECK(m.called("finish"));
        CHECK(state.module==0);CHECK(state.object==0);CHECK(state.online==0);
    }
    {
        State state;Mock m;m.factory_proc=0;o::worker_035b60(state.refs(),m);
        CHECK(m.called("log:factory function not found"));CHECK(!m.called("create"));CHECK(m.called("free"));CHECK(state.module==0);
    }
    // Existing state is not proactively reset when the loader fails.
    {
        State state;Mock m;state.online=1;state.object=99;m.loaded=0;o::worker_035b60(state.refs(),m);
        CHECK(state.online==1);CHECK(state.object==99);CHECK(!m.called("destroy"));CHECK(m.called("finish"));
    }
    {
        State state;Mock m;state.cookie=std::u16string(u"cookie\0suffix",13);o::load_online_035bd0(state.refs(),m);
        CHECK(m.cookie_copy==u"cookie");CHECK(state.cookie.size()==13);
    }
}
static void live_profile() {
    for(bool cache:{false,true})for(bool cookie:{false,true})for(bool avatar:{false,true}) {
        State state;Mock m;state.use_last=cache;
        m.statuses[state.cookie]={cookie?2:1,0,0};m.statuses[state.last_avatar]={avatar?2:1,0,0};
        m.write_result=false;
        m.inspect=[&](std::string_view event) {if(event=="log:authentication ok"||event=="name")CHECK(state.online==1);};
        o::worker_035b60(state.refs(),m);
        CHECK(state.online==1);CHECK(state.module==41);CHECK(state.object==71);
        CHECK(state.username=="provider-name");CHECK(state.persona=="provider-persona");CHECK(state.user=="provider-user");
        CHECK(state.avatar==(cookie&&avatar?state.last_avatar:u"old-avatar"));
        CHECK(m.called("status-avatar")==cookie);CHECK(m.called("write")==cache);
        if(cache)CHECK(m.written=="provider-persona;provider-user;provider-name");
        CHECK(!m.called("destroy"));CHECK(!m.called("free"));CHECK(m.calls.back()=="finish");
    }
    {
        State state;Mock m;m.name=std::string("visible\0hidden",14);m.persona="";m.user="";state.use_last=1;
        o::load_online_035bd0(state.refs(),m);
        CHECK(state.username=="visible");CHECK(state.persona.empty());CHECK(state.user.empty());CHECK(m.written==";;visible");
    }
    for(auto missing:std::array<int,3>{0,1,2}) {
        State state;Mock m;m.null_name=missing==0;m.null_persona=missing==1;m.null_user=missing==2;
        throws<o::OutOfEvidenceInput>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(state.online==1);CHECK(state.object==71);CHECK(state.module==41);CHECK(!m.called("destroy"));CHECK(!m.called("finish"));
        CHECK(state.username==(missing==0?"old-name":"provider-name"));
        CHECK(state.persona==(missing<=1?"old-persona":"provider-persona"));
        CHECK(state.user=="old-user");
    }
    {
        State state;Mock m;m.created=0;
        throws<o::OutOfEvidenceInput>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(state.object==0);CHECK(state.module==41);CHECK(state.online==0);CHECK(!m.called("auth-status"));CHECK(!m.called("finish"));
    }
}
static void profile_copy_allocation_failures() {
    const std::array<const char*,3> getters{"name","persona","user"};
    const std::array<const char*,19> prefix{"wait","suppress","widen","parent","join","load","restore",
        "log:online dll found!","flush","resolve:version_check","version","resolve:create_online","create",
        "auth-status","log:authentication ok","flush","name","persona","user"};
    for(bool fail:{false,true})for(bool cache:{false,true})for(unsigned selected=0;selected<getters.size();++selected){
        State state;Mock mock;state.use_last=cache;
        mock.name=std::string(4096,'N');mock.persona=std::string(8192,'P');mock.user=std::string(12288,'U');
        const std::array<std::string,3> expected{mock.name,mock.persona,mock.user};
        const std::array<std::string,3> initial{state.username,state.persona,state.user};
        mock.statuses[state.cookie]={2,0,0};mock.statuses[state.last_avatar]={2,0,0};
        profile_copy_allocation::Scope allocation;
        mock.inspect=[&](std::string_view event){
            if(event==getters[selected])allocation.arm(expected[selected].size()+1,fail);
        };
        bool caught=false;
        try{o::worker_035b60(state.refs(),mock);}
        catch(const profile_copy_allocation::Failure&){caught=true;}
        allocation.disarm(); // Assertions and explicit cleanup must not inject.
        CHECK(caught==fail);CHECK(allocation.hits()==1);
        CHECK(state.online==1&&state.object==mock.created&&state.module==mock.loaded);
        const std::array<std::string,3> copied{state.username,state.persona,state.user};
        for(unsigned i=0;i<copied.size();++i)CHECK(copied[i]==(fail&&i>=selected?initial[i]:expected[i]));
        std::vector<std::string> calls(prefix.begin(),prefix.begin()+(fail?17+selected:prefix.size()));
        if(!fail){
            calls.emplace_back("status-cookie");calls.emplace_back("status-avatar");
            if(cache)calls.emplace_back("write");
            calls.emplace_back("finish");
            if(cache)CHECK(mock.written==expected[1]+';'+expected[2]+';'+expected[0]);
        }
        // Exact failure prefix excludes every later getter/status/cache/ready
        // effect and any automatic destruction or module release.
        CHECK(mock.calls==calls);
        o::cleanup_0366f0(state.refs(),mock);o::cleanup_0366f0(state.refs(),mock);
        calls.emplace_back("destroy");calls.emplace_back("free");
        CHECK(mock.calls==calls&&state.object==0&&state.module==0&&state.online==1);
        CHECK((std::array<std::string,3>{state.username,state.persona,state.user}==copied));
    }
    std::cout<<"PASS: profile-copy allocation failure/control at three getters with cache on/off (synthetic effects)\n";
}
static void offline_profile() {
    for(auto status:std::array<std::int32_t,4>{1,2,-1,0x7fffffff})
        for(bool cache:{false,true})for(bool cookie:{false,true})for(bool avatar:{false,true}) {
            State state;Mock m;state.use_last=cache;m.auth_status=status;
            m.statuses[state.cookie]={cookie?2:1,0,0};m.statuses[state.last_avatar]={avatar?2:1,0,0};
            o::worker_035b60(state.refs(),m);
            CHECK(state.online==0);CHECK(state.module==0);CHECK(state.object==0);
            CHECK(m.called("log:authentication failed, ")==(status==1));
            CHECK(m.called("status-cookie")==cache);CHECK(m.called("status-avatar")==bool(cache&&cookie));
            CHECK(m.called("read")==bool(cache&&cookie));CHECK(!m.called("write"));CHECK(!m.called("name"));
            CHECK(state.username==(cache&&cookie?"synthetic-name":"old-name"));
            CHECK(state.persona==(cache&&cookie?"synthetic-persona":"old-persona"));
            CHECK(state.user==(cache&&cookie?"synthetic-user":"old-user"));
            CHECK(state.avatar==(cache&&cookie&&avatar?state.last_avatar:u"old-avatar"));
            CHECK(m.calls[m.calls.size()-3]=="destroy");CHECK(m.calls[m.calls.size()-2]=="free");CHECK(m.calls.back()=="finish");
        }
    for(const auto& profile:std::vector<std::string>{"","single","one;two","one;two;three;four",";;",";two;","one;;three",std::string("one\0x;two;three\0y",17)}) {
        State state;Mock m;state.use_last=1;m.auth_status=1;m.statuses[state.cookie]={2,0,0};m.read_value=profile;
        o::worker_035b60(state.refs(),m);
        const auto fields=o::split_profile_fields_03cf00(profile);
        if(fields.size()==3) {CHECK(state.persona==fields[0]);CHECK(state.user==fields[1]);CHECK(state.username==fields[2]);}
        else {CHECK(state.persona=="old-persona");CHECK(state.user=="old-user");CHECK(state.username=="old-name");}
    }
    {
        State state;Mock m;state.use_last=1;m.auth_status=1;m.statuses[state.cookie]={2,0,0};
        o::load_online_035bd0(state.refs(),m);
        const auto auth=std::find(m.calls.begin(),m.calls.end(),"log:authentication failed, ");
        CHECK(auth!=m.calls.end());CHECK(*(auth+1)=="log:starting offline");CHECK(*(auth+2)=="flush");
    }
}
static void split_cases() {
    CHECK(o::split_profile_fields_03cf00("")==std::vector<std::string>{""});
    CHECK((o::split_profile_fields_03cf00(";;")==std::vector<std::string>{"","",""}));
    // Independent reconstruction property over all short binary strings: join
    // fields must restore every byte, with exactly delimiter-count + 1 fields.
    for(unsigned length=0;length<=6;++length) {
        const unsigned total=1u<<(length*2);
        for(unsigned seed=0;seed<total;++seed) {
            std::string value;unsigned cursor=seed;
            for(unsigned i=0;i<length;++i){value.push_back(std::array<char,4>{'a',';',char(0),char(0xff)}[cursor&3]);cursor>>=2;}
            const auto fields=o::split_profile_fields_03cf00(value);
            CHECK(fields.size()==static_cast<std::size_t>(std::count(value.begin(),value.end(),';'))+1);
            std::string rebuilt;
            for(std::size_t i=0;i<fields.size();++i){if(i)rebuilt+=';';rebuilt+=fields[i];}
            CHECK(rebuilt==value);
        }
    }
}
static void status_and_cleanup() {
    for(std::int32_t kind=-2;kind<=12;++kind)for(auto error:std::array<std::uint32_t,3>{0,5,0xffffffff}) {
        State state;Mock m;m.statuses[state.cookie]={kind,0,error};
        if(error && kind!=1 && kind!=9) {
            throws<Injected>([&]{(void)o::exists_035b00(state.cookie,m);});CHECK(m.called("filesystem-error"));
        } else {CHECK(o::exists_035b00(state.cookie,m)==(kind==2));CHECK(!m.called("filesystem-error"));}
    }
    for(bool object:{false,true})for(bool module:{false,true})for(bool free_result:{false,true}) {
        State state;Mock m;state.object=object?88:0;state.module=module?99:0;state.online=1;m.free_result=free_result;
        o::cleanup_0366f0(state.refs(),m);
        CHECK(state.object==0);CHECK(state.module==0);CHECK(state.online==1);
        CHECK(m.called("destroy")==object);CHECK(m.called("free")==module);
        if(object&&module)compare_calls(m,{"destroy","free"});
    }
    {
        State state;Mock m;state.object=88;state.module=99;m.fail="destroy";
        throws<Injected>([&]{o::cleanup_0366f0(state.refs(),m);});CHECK(state.object==88);CHECK(state.module==99);CHECK(!m.called("free"));
    }
    {
        State state;Mock m;state.object=88;state.module=99;m.fail="free";
        throws<Injected>([&]{o::cleanup_0366f0(state.refs(),m);});CHECK(state.object==0);CHECK(state.module==99);
    }
}
static void exceptions() {
    for(auto fail:{"widen","parent","join","load"}) {
        State state;Mock m;m.fail=fail;
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(m.mode==0x20);CHECK(m.called("restore"));CHECK(!m.called("finish"));CHECK(!m.called("free"));
        CHECK(std::count(m.calls.begin(),m.calls.end(),"restore")==1);
        CHECK(state.module==0);
    }
    {
        State state;Mock m;m.fail="join";
        m.inspect=[](std::string_view event) {
            if(event=="restore")throw std::logic_error("synthetic restoration failure");
        };
        // Restoration must not replace the initiating path failure.
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(m.called("restore"));CHECK(!m.called("finish"));
    }
    for(auto fail:{"resolve:version_check","version","resolve:create_online","create","auth-status","log:authentication ok","name","persona","user","status-cookie"}) {
        State state;Mock m;m.fail=fail;
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(m.mode==0x20);CHECK(state.module==41);CHECK(!m.called("finish"));CHECK(!m.called("destroy"));CHECK(!m.called("free"));
    }
    {
        State state;Mock m;m.auth_status=1;m.fail="destroy";
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});CHECK(state.object==71);CHECK(state.module==41);CHECK(!m.called("finish"));
    }
    {
        State state;Mock m;m.fail="wait";throws<Injected>([&]{o::worker_035b60(state.refs(),m);});
        compare_calls(m,{"wait"});CHECK(state.module==0);
    }
}

// Concrete source composition uses only these synthetic boundaries. No native
// library, filesystem, conversion, provider, or original target is invoked.
struct LibraryCalls : l::SystemCalls {
    std::uint32_t mode=0x40;std::wstring loaded;std::vector<o::Module> freed;
    std::uint32_t get_error_mode() override{return mode;}
    std::uint32_t set_error_mode(std::uint32_t value) override{const auto old=mode;mode=value;return old;}
    l::Module load_library(const wchar_t* value) override{loaded=value;return 41;}
    bool free_library(l::Module value)override{freed.push_back(value);return false;}
    int wide_to_narrow(std::uint32_t,std::uint32_t,const wchar_t*,int,char*,int,bool*)override{throw std::logic_error("unexpected conversion");}
    std::uint32_t last_error() override{throw std::logic_error("unexpected last error");}
};
struct Stats : s::Stats {
    s::StatsResult query(const o::Path&,std::uint32_t flags)override{CHECK(flags==3);return {0,0,{}};}
};
struct Encoding : e::Calls {
    unsigned calls=0;
    std::uint32_t locale_codepage_0629ec()override{return 65001;}
    bool are_file_apis_ansi()override{throw std::logic_error("UTF8 locale takes priority");}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* input,int count,char16_t* output,int capacity)override{
        CHECK(cp==65001);CHECK(flags==8);++calls;
        if(output){CHECK(capacity==count);for(int i=0;i<count;++i)output[i]=static_cast<unsigned char>(input[i]);}
        return count;
    }
    int wide_to_multi_byte(std::uint32_t,std::uint32_t,const char16_t*,int,char*,int,bool*)override{throw std::logic_error("unexpected conversion");}
    std::uint32_t last_error()override{throw std::logic_error("unexpected error");}
};
struct Errors : e::Errors {
    [[noreturn]]void runtime_00cd70(int)override{throw std::logic_error("unexpected runtime error");}
    [[noreturn]]void filesystem_00cdb0(std::uint32_t)override{throw std::logic_error("unexpected filesystem error");}
};
struct Input : f::Input {
    std::string data="cache-persona;cache-user;cache-name";bool& closed;
    explicit Input(bool& c):closed(c){}
    std::uint32_t initial_state()const override{return 0;}
    void seek_03b610(std::int64_t offset,f::Seek)override{CHECK(offset==0);}
    std::uint64_t position_sum_03ba50()override{return data.size();}
    std::int32_t peek_043790()override{throw std::logic_error("read_binary does not peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{CHECK(out.size()==data.size());std::copy(data.begin(),data.end(),out.begin());return out.size();}
    void close_03ae70()override{closed=true;}
};
struct Output : f::Output {
    std::string& written;bool& closed;
    Output(std::string& w,bool& c):written(w),closed(c){}
    std::uint32_t initial_state()const override{return 0;}
    void write_03be80(std::string_view value)override{written=value;}
    void close_03ae70()override{closed=true;}
};
struct Files : f::Factory {
    std::string written;bool input_closed=false,output_closed=false;
    std::unique_ptr<f::Input> open_input_039f70(const o::Path& path,std::uint32_t mode,std::uint32_t share)override{
        CHECK(path==u"C:\\fixture\\profile.synthetic");CHECK(mode==0x20);CHECK(share==0x40);return std::make_unique<Input>(input_closed);
    }
    std::unique_ptr<f::Output> open_output_03a1a0(const o::Path& path,std::uint32_t mode,std::uint32_t share)override{
        CHECK(path==u"C:\\fixture\\profile.synthetic");CHECK(mode==0x20);CHECK(share==0x40);return std::make_unique<Output>(written,output_closed);
    }
};
#if !defined(PRELOADER_L_PORTABLE_ONLINE_TESTS)
static void composition() {
    for(bool success:{false,true}) {
        State state;Mock remaining;LibraryCalls libraries;Stats stats;Encoding encoding;Errors errors;Files files;
        state.use_last=1;remaining.auth_status=success?0:1;
        o::DelegatingEffects effects(remaining,libraries,stats,encoding,errors,files);
        o::worker_035b60(state.refs(),effects);
        CHECK(libraries.loaded==L"C:\\fixture\\preloader_l_online.dll");CHECK(libraries.mode==0x40);CHECK(encoding.calls==2);
        CHECK(state.online==static_cast<std::uint8_t>(success));CHECK(state.avatar==state.last_avatar);CHECK(remaining.called("finish"));
        CHECK(!remaining.called("widen"));CHECK(!remaining.called("load"));CHECK(!remaining.called("read"));CHECK(!remaining.called("write"));
        if(success) {
            CHECK(files.output_closed);CHECK(!files.input_closed);CHECK(files.written=="provider-persona;provider-user;provider-name");
            CHECK(libraries.freed.empty());CHECK(state.module==41);CHECK(state.object==71);
        } else {
            CHECK(files.input_closed);CHECK(!files.output_closed);CHECK(state.persona=="cache-persona");CHECK(state.user=="cache-user");CHECK(state.username=="cache-name");
            CHECK((libraries.freed==std::vector<o::Module>{41}));CHECK(state.module==0);CHECK(state.object==0);
        }
    }
}
#endif
static void cookie_release_before_status() {
    for(int failure:{0,1,2}) {
        State state;Mock mock;state.cookie.assign(512,u'c');
        struct Reset {~Reset(){cookie_lifetime::tracked=nullptr;cookie_lifetime::object=nullptr;}} reset;
        cookie_lifetime::released=0;cookie_lifetime::committed=false;
        cookie_lifetime::object=&state.object;cookie_lifetime::expected=failure==2?0:mock.created;
        mock.inspect_cookie=[&](const char16_t* cookie){
            CHECK(cookie!=state.cookie.data());cookie_lifetime::tracked=cookie;
            if(failure==2)throw Injected();
        };
        mock.inspect=[&](std::string_view event){
            if(event=="auth-status"){
                CHECK(cookie_lifetime::released==1&&cookie_lifetime::committed);
                if(failure==1)throw Injected();
            }
        };
        if(failure) throws<Injected>([&]{o::load_online_035bd0(state.refs(),mock);});
        else o::load_online_035bd0(state.refs(),mock);
        CHECK(cookie_lifetime::released==1&&cookie_lifetime::committed);
        CHECK(state.cookie.size()==512&&state.cookie.front()==u'c');
        CHECK(state.object==(failure==2?0:mock.created));
    }
}
int main() {
    try {entry_branches();live_profile();profile_copy_allocation_failures();offline_profile();split_cases();status_and_cleanup();exceptions();
#if !defined(PRELOADER_L_PORTABLE_ONLINE_TESTS)
        composition();
#endif
        cookie_release_before_status();}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"online: "<<checks<<" checks passed (synthetic mocks only)\n";
}
