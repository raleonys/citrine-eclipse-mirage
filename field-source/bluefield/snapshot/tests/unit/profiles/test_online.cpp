#include "src/profiles/online.hpp"
#include "src/paths/paths.hpp"
#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>

namespace o = preloader_l::online;
namespace s = preloader_l::status_recovered;
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
    void event(std::string value) {
        calls.push_back(value);
        if (inspect) inspect(value);
        if (fail==value) throw Injected();
    }
    bool called(std::string_view value) const { return std::find(calls.begin(),calls.end(),value)!=calls.end(); }
    std::uint32_t suppress_errors_0450a0() override {event("suppress");const auto previous=mode;mode|=1;return previous;}
    void restore_error_mode(std::uint32_t value) override {event("restore");mode=value;}
    o::Path filesystem_widen_006d70(std::string_view value) override {
        event("widen"); CHECK(value=="preloader_l_online.dll"); CHECK(value.size()==22);
        return o::Path(value.begin(),value.end());
    }
    o::Path parent_path_019820(const o::Path& value) override {event("parent");return preloader_l::paths::parent_path_019820(value);}
    o::Path join_016b00(const o::Path& a,const o::Path& b) override {event("join");return preloader_l::paths::join_016b00(a,b);}
    o::Module load_copy_0450d0(const o::Path& value) override {event("load");loaded_path=value;return loaded;}
    o::Procedure get_proc_address(o::Module module,std::string_view value) override {
        CHECK(module==loaded);event("resolve:"+std::string(value));
        return value=="version_check"?version_proc:factory_proc;
    }
    std::int32_t call_version_check(o::Procedure value) override {CHECK(value==version_proc);event("version");return version;}
    o::Object call_create_online(o::Procedure value,const char16_t* cookie) override {
        CHECK(value==factory_proc);event("create");cookie_copy=cookie;return created;
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
        CHECK(title=="SURGE");CHECK(text=="Online library version mismatch!");CHECK(flags==0x10);event("dialog");
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
        CHECK(state.avatar==u"old-avatar");
        CHECK(!m.called("status-avatar"));CHECK(m.called("write")==cache);
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
static void offline_profile() {
    for(auto status:std::array<std::int32_t,4>{1,2,-1,0x7fffffff})
        for(bool cache:{false,true})for(bool cookie:{false,true})for(bool avatar:{false,true}) {
            State state;Mock m;state.use_last=cache;m.auth_status=status;
            m.statuses[state.cookie]={cookie?2:1,0,0};m.statuses[state.last_avatar]={avatar?2:1,0,0};
            o::worker_035b60(state.refs(),m);
            CHECK(state.online==0);CHECK(state.module==0);CHECK(state.object==0);
            CHECK(m.called("log:authentication failed, ")==(status==1));
            CHECK(m.called("status-cookie")==cache);CHECK(!m.called("status-avatar"));
            CHECK(m.called("read")==bool(cache&&cookie));CHECK(!m.called("write"));CHECK(!m.called("name"));
            CHECK(state.username==(cache&&cookie?"synthetic-name":"old-name"));
            CHECK(state.persona==(cache&&cookie?"synthetic-persona":"old-persona"));
            CHECK(state.user==(cache&&cookie?"synthetic-user":"old-user"));
            CHECK(state.avatar==u"old-avatar");
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
        CHECK(m.mode==0x21);CHECK(!m.called("restore"));CHECK(!m.called("finish"));CHECK(!m.called("free"));
    }
    for(auto fail:{"resolve:version_check","version","resolve:create_online","create","auth-status","log:authentication ok","name","persona","user"}) {
        State state;Mock m;m.fail=fail;
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});
        CHECK(m.mode==0x20);CHECK(state.module==41);CHECK(!m.called("finish"));CHECK(!m.called("destroy"));CHECK(!m.called("free"));
    }
    {
        State state;Mock m;state.use_last=1;m.auth_status=1;m.fail="status-cookie";
        throws<Injected>([&]{o::worker_035b60(state.refs(),m);});CHECK(!m.called("finish"));
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

int main() {
    try {entry_branches();live_profile();offline_profile();split_cases();status_and_cleanup();exceptions();}
    catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<"online: "<<checks<<" checks passed (synthetic mocks only)\n";
}
