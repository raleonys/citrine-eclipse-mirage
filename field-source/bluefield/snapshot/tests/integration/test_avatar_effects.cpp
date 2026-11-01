// SPDX-License-Identifier: GPL-3.0-only
#include "tests/support/handlers/packet_image.hpp"
#include "src/application/avatars.hpp"
#include "src/application/avatar_assets.hpp"
#include "src/files/atomic_io.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace c=preloader_l::content;
namespace packet=preloader_l::test_packet_avatar;
namespace fs=std::filesystem;
static unsigned checks=0;
static void check(bool ok,const char* what){++checks;if(!ok)throw std::runtime_error(what);}
struct Injected {};
struct Xml final:c::Xml {
    std::vector<std::pair<std::string,std::string>> attrs;
    int depth=0;
    const char* attribute_002cc0(preloader_l::handlers::Request,const char*) override{throw std::logic_error("snapshot required");}
    void open_004080(const char*,bool) override{++depth;}
    void attribute_005560(const char* k,const char* v) override{attrs.emplace_back(k,v);}
    void close_virtual_48(bool) override{--depth;}
    std::string resource()const{for(auto& [k,v]:attrs)if(k=="ResourcePath")return v;return {};}
};
struct Mutex final:c::Mutex {
    std::int32_t n=0;
    int lock_04a6ac()override{throw std::logic_error("image must not lock achievements");}
    std::int32_t& count_4c()override{return n;}
    void unlock_04a6b4()override{throw std::logic_error("unexpected unlock");}
    [[noreturn]]void thread_error_04a648(int)override{throw std::logic_error("unexpected thread error");}
};
struct Queue final:c::EventQueue {
    Mutex mutex;
    c::Mutex& mutex_02c620()override{return mutex;}
    bool initialized_locked()const override{return false;}
    void initialize_locked(std::string)override{throw std::logic_error("unexpected event");}
    void push_locked_034660(std::string)override{throw std::logic_error("unexpected event");}
    std::unique_ptr<c::PendingEvent> prepare_event(std::string,std::string)override{throw std::logic_error("unexpected event");}
};
struct State {
    preloader_l::handlers::Attributes game;
    c::AchievementMap achievements;
    std::string set,content,user="own";
    c::Path achievement,selected=u"selected-custom.png",fallback=u"default.png",directory=u"data",last=u"data\\last_avatar";
    std::uint8_t online=1;
    std::int32_t saved=0;
    Mutex mutex;Queue queue;
    c::StateReferences refs(){return{{},{},game,achievements,set,content,user,achievement,selected,fallback,directory,last,online,saved,mutex,queue};}
};
static std::string narrow(const c::Path& p){return preloader_l::path_utf8(p);}
struct Effects final:c::Effects {
    std::vector<std::string> trace;
    std::string fail;
    bool provider=true,copy=true,disk=false;
    std::int32_t width=416;
    preloader_l::AvatarStore* avatars=nullptr;
    const preloader_l::RuntimePaths* paths=nullptr;
    void event(std::string s){trace.push_back(s);if(fail==s.substr(0,s.find(':')))throw Injected{};}
    std::int32_t atoi_05ee08(std::string_view v)override{event("atoi:"+std::string(v));return width;}
    void unknown_entitlement_field(std::string_view)override{throw std::logic_error("unexpected entitlement");}
    std::string utc_time_041b60(std::uint64_t&)override{throw std::logic_error("unexpected timestamp");}
    std::unique_ptr<c::DocumentWriter> printer_0019b0()override{throw std::logic_error("unexpected printer");}
    void write_xml_03c1d0(const c::Path&,std::string_view)override{throw std::logic_error("unexpected xml write");}
    void save_steam_01f9a0(const c::AchievementMap&)override{throw std::logic_error("unexpected achievement");}
    c::Path filesystem_widen_006d70(std::string_view v)override{event("widen:"+std::string(v));return preloader_l::filesystem_path(v);}
    c::Path path_append_016b50(const c::Path& a,const c::Path& b)override{event("append:"+narrow(b));return a+u"\\"+b;}
    c::Path avatar_cache_path(const c::Path& root,std::string_view owner,std::string_view spelling)override{
        if(paths)return preloader_l::model_path(paths->avatar_cache(owner,spelling));
        return path_append_016b50(root,filesystem_widen_006d70("_AVATAR_"+std::string(owner)+"_"+std::string(spelling)));
    }
    c::Path resolve_image(const c::Path& p)override{if(fail=="resolve")throw Injected{};return avatars?avatars->resolve(p):p;}
    bool online_avatar_virtual_28(const c::Path& p,std::string_view owner,std::int32_t size)override{
        event("provider:"+narrow(p)+":"+std::string(owner)+":"+std::to_string(size));
        if(provider&&disk)preloader_l::atomic_write(preloader_l::native_path(p),"provider-image",paths);
        return provider;
    }
    bool copy_overwrite_048d20(const c::Path& a,const c::Path& b)override{
        event("copy:"+narrow(a)+":"+narrow(b));
        if(copy&&disk)preloader_l::atomic_write(preloader_l::native_path(b),preloader_l::read_file(a,true),paths);
        return copy;
    }
    std::string path_to_utf8_031ca0(const c::Path& p)override{return narrow(p);}
    void log_saved_size_085530(std::int32_t n)override{event("log:"+std::to_string(n));}
};
static c::RequestTree request(std::string owner="own",std::string width="00416"){
    return{{{"Width",width},{"Height","arbitrary"},{"ImageId","avatar:"+owner}},{}};
}
static void source_matrix(){
    for(bool own:{false,true})for(bool online:{false,true})for(bool provider:{false,true})for(bool copy:{false,true})
    for(auto width:std::array<std::int32_t,10>{-416,-1,0,207,208,415,416,832,1248,std::numeric_limits<std::int32_t>::max()})
    for(auto saved:std::array<std::int32_t,6>{-1,0,1,2,4,std::numeric_limits<std::int32_t>::max()}){
        State source,actual;source.saved=actual.saved=saved;source.online=actual.online=online;
        Effects se,ae;se.provider=ae.provider=provider;se.copy=ae.copy=copy;se.width=ae.width=width;
        const std::string owner=own?"own":"other";auto q=request(owner);Xml sx,ax;
        packet::Effects adapter{se,owner};
        check(packet::query_image_0314f0(q,sx,source.refs(),adapter),"packet admission");
        check(c::query_image_0314f0(q,ax,actual.refs(),ae),"candidate admission");
        check(ae.trace==se.trace,"packet provider/copy/log order differs");
        check(actual.saved==source.saved,"packet saved-size differs");
        check(actual.selected==source.selected,"selected path mutated");
        check(ax.resource()==(own?narrow(actual.selected):sx.resource()),"visible selection differs");
        check(ax.depth==0&&sx.depth==0,"XML closure differs");
        const bool should_save=online&&own&&provider&&saved<width/208&&copy;
        check(actual.saved==(should_save?width/208:saved),"copy success must precede saved-size publication");
    }
}
static void failures_and_admission(){
    for(const char* fail:{"atoi","widen","append","provider","copy","log"}){
        State source,actual;Effects se,ae;se.fail=ae.fail=fail;Xml sx,ax;
        packet::Effects adapter{se,"own"};bool st=false,at=false;
        try{packet::query_image_0314f0(request(),sx,source.refs(),adapter);}catch(Injected&){st=true;}
        try{c::query_image_0314f0(request(),ax,actual.refs(),ae);}catch(Injected&){at=true;}
        check(st&&at,"injected failure not propagated");check(se.trace==ae.trace,"failure effect order differs");
        check(source.saved==actual.saved,"failure saved-size differs");check(actual.saved==(ae.fail=="log"?2:0),"failure publication order");
        check(ax.attrs.empty(),"response emitted before failing effects");
    }
    State state;Effects e;e.fail="resolve";Xml x;bool thrown=false;
    try{c::query_image_0314f0(request(),x,state.refs(),e);}catch(Injected&){thrown=true;}
    check(thrown&&state.saved==2,"late resolve failure does not undo source cache effects");
    for(auto missing:{"Width","Height","ImageId"}){
        State a;Effects effects;Xml xml;auto q=request();std::erase_if(q.attributes,[&](auto& pair){return pair.first==missing;});
        check(!c::query_image_0314f0(q,xml,a.refs(),effects),"incomplete image request accepted");check(effects.trace.empty(),"incomplete request effect");
    }
    for(const std::string id:{"unqualified","avatar:","avatar:other:tail"}){
        State a;Effects effects;Xml xml;auto q=request();q.attributes[2].second=id;
        check(c::query_image_0314f0(q,xml,a.refs(),effects),"image ID admission narrowed");
        check(id!="unqualified"||effects.trace.empty(),"unqualified ID calls provider");
    }
}
static void disk_and_selection(){
    auto root=fs::temp_directory_path()/("preloader_l_avatar_effects_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup{fs::path p;~Cleanup(){std::error_code ec;fs::remove_all(p,ec);}}cleanup{root};
    for(int mode:{0,1,2}){
        const bool custom=mode==1,sidecar=mode==2;
        auto base=root/std::to_string(mode);
        auto paths=preloader_l::RuntimePaths::from_system(base/"module/preloader_l.dll",base/"game/game.exe",base/"local",base/"program");
        preloader_l::AvatarStore avatars(paths);State state;state.selected=avatars.logical_default();state.last=paths.model(preloader_l::RuntimeFile::last_avatar);
        state.directory=preloader_l::model_path(paths.data());state.fallback=avatars.logical_default();
        if(custom){state.selected=preloader_l::model_path(base/"user/custom.png");preloader_l::atomic_write(preloader_l::native_path(state.selected),"custom-image",nullptr);}
        if(sidecar)preloader_l::atomic_write(paths.sidecar_avatar(),"sidecar-image",nullptr);
        Effects e;e.disk=true;e.paths=&paths;e.avatars=&avatars;
        check(!fs::exists(paths.avatar_cache("own","00416")),"eager provider cache");
        check(!fs::exists(paths.default_avatar()),"eager official fallback");
        Xml x;check(c::query_image_0314f0(request(),x,state.refs(),e),"disk image request");
        check(preloader_l::read_file(preloader_l::model_path(paths.avatar_cache("own","00416")),true)=="provider-image","provider file missing");
        check(preloader_l::read_file(state.last,true)=="provider-image","last-avatar copy missing");
        check(state.saved==2,"disk saved-size missing");
        auto selected=custom?preloader_l::native_path(state.selected):(sidecar?paths.sidecar_avatar():paths.default_avatar());
        check(x.resource()==preloader_l::path_utf8(preloader_l::model_path(selected)),"disk visible choice changed");
        if(custom){check(preloader_l::read_file(state.selected,true)=="custom-image","custom bytes changed");check(!fs::exists(paths.default_avatar()),"custom forces fallback write");}
        else if(sidecar){check(preloader_l::read_file(preloader_l::model_path(selected),true)=="sidecar-image","sidecar bytes changed");check(!fs::exists(paths.default_avatar()),"sidecar forces fallback write");}
        else check(preloader_l::read_file(preloader_l::model_path(selected),true)==std::string(reinterpret_cast<const char*>(preloader_l::assets::surge_png),sizeof preloader_l::assets::surge_png),"provider replaces official fallback");
        e.width=208;Xml smaller;check(c::query_image_0314f0(request("own","208"),smaller,state.refs(),e),"smaller request");
        check(state.saved==2,"smaller request lowers cached size");
    }
}
int main(){try{source_matrix();failures_and_admission();disk_and_selection();std::cout<<"PASS avatar source effects checks="<<checks<<"; real provider ABI NOT_RUN\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL unexpected exception\n";return 1;}}
