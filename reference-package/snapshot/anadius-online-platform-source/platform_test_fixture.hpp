#pragma once
#include "platform.hpp"
#include "../anadius-library-effects-source/effects_test_fixture.hpp"
#include <functional>

namespace b13_test {
using namespace anadius;namespace op=online_platform;namespace online=online_recovered;
using Path=configuration::Path;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct ForeignState {
    std::vector<std::string> events;std::vector<std::int32_t> waits;
    std::int32_t version=2,status=0;online::Object object=0x90;
    std::string username="own-user",persona="own-persona",user_id="own-id";
    Path cookie;unsigned finishes=0,wrong_calls=0;
    unsigned null_getter=0,throw_getter=0;bool throw_destroy=false;
};
class Foreign final:public online::Effects {
    std::shared_ptr<ForeignState> state_;
    [[noreturn]] void wrong(){++state_->wrong_calls;throw std::logic_error("selected online policy reached lower unexpectedly");}
    void object(online::Object value){if(value!=state_->object)throw std::logic_error("unexpected controlled object token");}
    const char* getter(online::Object value,unsigned number,const std::string& text){
        object(value);state_->events.push_back("get-"+std::to_string(number));
        if(state_->throw_getter==number)throw std::runtime_error("controlled getter failure");
        return state_->null_getter==number?nullptr:text.c_str();
    }
public:
    explicit Foreign(std::shared_ptr<ForeignState> state):state_(std::move(state)){}
    std::uint32_t suppress_errors_0450a0()override{wrong();}
    void restore_error_mode(std::uint32_t)override{wrong();}
    Path filesystem_widen_006d70(std::string_view)override{wrong();}
    Path parent_path_019820(const Path&)override{wrong();}
    Path join_016b00(const Path&,const Path&)override{wrong();}
    online::Module load_copy_0450d0(const Path&)override{wrong();}
    online::Procedure get_proc_address(online::Module,std::string_view)override{wrong();}
    std::int32_t call_version_check(online::Procedure p)override{if(p!=0x81)throw std::logic_error("version token");state_->events.emplace_back("version");return state_->version;}
    online::Object call_create_online(online::Procedure p,const char16_t* cookie)override{if(p!=0x82)throw std::logic_error("factory token");state_->events.emplace_back("factory");state_->cookie=cookie;return state_->object;}
    std::int32_t status_virtual_08(online::Object o)override{object(o);state_->events.emplace_back("status");return state_->status;}
    const char* username_virtual_10(online::Object o)override{return getter(o,1,state_->username);}
    const char* persona_virtual_18(online::Object o)override{return getter(o,2,state_->persona);}
    const char* user_id_virtual_20(online::Object o)override{return getter(o,3,state_->user_id);}
    void destroy_virtual_00(online::Object o)override{object(o);state_->events.emplace_back("destroy");if(state_->throw_destroy)throw std::runtime_error("controlled destroy failure");}
    bool free_library(online::Module)override{wrong();}
    status_recovered::Status status_01ea20(const Path&,std::uint32_t)override{wrong();}
    [[noreturn]] void filesystem_error_018130(std::string_view,std::uint32_t,const Path&)override{wrong();}
    std::string read_binary_03b3f0(const Path&)override{wrong();}
    bool write_binary_03c1d0(const Path&,std::string_view)override{wrong();}
    void log_literal_006880(std::string_view)override{wrong();}
    void log_finish_0088e0()override{wrong();}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{wrong();}
    void wait_steps_036880(std::int32_t count)override{state_->waits.push_back(count);}
    void finish_flag_036820()override{++state_->finishes;}
};
struct EncodingState {unsigned locales=0,ansi=0;std::vector<std::uint32_t> cps,flags;bool fail=false;};
class Encoding final:public encoding_recovered::Calls {
    std::shared_ptr<EncodingState> state_;encoding_recovered::NativeCalls actual_;
public:
    explicit Encoding(std::shared_ptr<EncodingState> state):state_(std::move(state)){}
    std::uint32_t locale_codepage_0629ec()override{++state_->locales;return 65001;}
    bool are_file_apis_ansi()override{++state_->ansi;return true;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* p,int count,char16_t* out,int capacity)override{
        state_->cps.push_back(cp);state_->flags.push_back(flags);
        if(state_->fail)throw std::runtime_error("controlled filesystem widening failure");
        return actual_.multi_byte_to_wide(cp,flags,p,count,out,capacity);
    }
    int wide_to_multi_byte(std::uint32_t,std::uint32_t,const char16_t*,int,char*,int,bool*)override{throw std::logic_error("unexpected online narrowing");}
    std::uint32_t last_error()override{return actual_.last_error();}
};
struct StatsState {
    std::map<Path,status_recovered::StatsResult> values;
    std::vector<std::pair<Path,std::uint32_t>> queries;
};
class Stats final:public status_recovered::Stats {
    std::shared_ptr<StatsState> state_;
public:
    explicit Stats(std::shared_ptr<StatsState> state):state_(std::move(state)){}
    status_recovered::StatsResult query(const Path& p,std::uint32_t flags)override{
        state_->queries.emplace_back(p,flags);auto found=state_->values.find(p);
        return found==state_->values.end()?status_recovered::StatsResult{2,0,std::nullopt}:found->second;
    }
};
struct Lookup {
    std::vector<std::pair<std::uintptr_t,std::string>> queries;bool version_missing=false,factory_missing=false;
};
inline thread_local Lookup* lookup=nullptr;
inline FARPROC WINAPI get_proc(HMODULE module,LPCSTR name){
    lookup->queries.emplace_back(reinterpret_cast<std::uintptr_t>(module),name);
    if(std::string_view(name)=="version_check")return reinterpret_cast<FARPROC>(lookup->version_missing?0:0x81);
    if(std::string_view(name)=="create_online")return reinterpret_cast<FARPROC>(lookup->factory_missing?0:0x82);
    throw std::logic_error("unexpected native export lookup");
}
struct Graph {
    b12_test::Ui sdk;Lookup exports;
    std::shared_ptr<b11_test::OwnedStream> output=std::make_shared<b11_test::OwnedStream>();
    std::shared_ptr<library_effects::Dialog> dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());
    std::shared_ptr<ForeignState> foreign=std::make_shared<ForeignState>();
    std::shared_ptr<EncodingState> encoding=std::make_shared<EncodingState>();
    std::shared_ptr<StatsState> stats=std::make_shared<StatsState>();
    std::shared_ptr<b8_test::MemoryFilesState> files=std::make_shared<b8_test::MemoryFilesState>();
    b12_test::Os* os=nullptr;std::unique_ptr<op::OnlinePlatform> selected;
    op::Resources resources(){
        op::Resources r;auto native=std::make_unique<b12_test::Os>();os=native.get();
        native->load_hook=[](const wchar_t*){return online::Module{0x80};};r.os=std::move(native);
        r.encoding=std::make_unique<Encoding>(encoding);r.stats=std::make_unique<Stats>(stats);
        r.files=std::make_unique<b8_test::MemoryFiles>(files);r.get_proc=get_proc;return r;
    }
    Graph(){b12_test::ui=&sdk;lookup=&exports;selected=std::make_unique<op::OnlinePlatform>(output,dialog,std::make_unique<Foreign>(foreign),resources());}
    ~Graph(){selected.reset();lookup=nullptr;b12_test::ui=nullptr;}
};
struct State {
    Path executable=u"C:\\own\\host.exe",cookie=u"OWN-cookie",last_profile=u"OWN-profile",last_avatar=u"OWN-avatar",avatar=u"seed-avatar";
    online::Module module=0;online::Object object=0;
    std::string username="seed-user",persona="seed-persona",user_id="seed-id";
    std::uint8_t use_last=0,online_flag=0;
    online::StateRefs refs(){return {executable,cookie,last_profile,last_avatar,avatar,module,object,username,persona,user_id,use_last,online_flag};}
};
} // namespace b13_test
