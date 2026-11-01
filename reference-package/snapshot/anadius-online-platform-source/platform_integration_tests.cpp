#include "platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"

namespace {
using namespace b13_test;
void successful_source(){
    Graph g;State state;state.use_last=1;
    g.stats->values[state.cookie]={0,0x80,std::nullopt};g.stats->values[state.last_avatar]={0,0x80,std::nullopt};
    online::load_online_035bd0(state.refs(),*g.selected);
    require(state.module==0x80&&state.object==0x90&&state.online_flag==1,"actual online source retains module/object and marks online");
    require(state.username=="own-user"&&state.persona=="own-persona"&&state.user_id=="own-id","actual source copies controlled foreign getters in order");
    require(g.foreign->events==std::vector<std::string>({"version","factory","status","get-1","get-2","get-3"})&&g.foreign->cookie==state.cookie,"version/factory/vtable boundaries preserve token/order and cookie C string");
    require(g.os->loads==std::vector<std::wstring>({L"C:\\own\\anadius64online.dll"})&&g.os->mode==0x40,"existing parent/join/filesystem-width/copied-load chain restores original mode");
    require(g.output->str()=="online dll found!\nauthentication ok\n"&&g.output->flushes==2,"selected logger emits exact online success lines");
    require(state.avatar==state.last_avatar&&g.stats->queries.size()==2,"cached avatar selection follows regular-file source tests");
    require(g.files->outputs[state.last_profile]=="own-persona;own-id;own-user"&&g.files->reads.empty(),"own memory profile write uses actual binary writer and source field order");
    require(g.foreign->wrong_calls==0&&g.sdk.messages.empty(),"selected policies bypass lower without inventing dialog");
    online::cleanup_0366f0(state.refs(),*g.selected);
    require(state.object==0&&state.module==0&&g.foreign->events.back()=="destroy"&&g.os->unloaded==std::vector<online::Module>({0x80}),"actual cleanup destroys object before releasing module and clears both");
    const auto before=g.foreign->events.size();online::cleanup_0366f0(state.refs(),*g.selected);
    require(g.foreign->events.size()==before&&g.os->unloaded.size()==1&&state.online_flag==1,"repeated cleanup performs no extra calls and does not invent online flag reset");
}
void absent_exports_and_offline(){
    {
        Graph g;State state;g.os->load_hook=[](const wchar_t*){return online::Module{0};};online::worker_035b60(state.refs(),*g.selected);
        require(state.module==0&&state.object==0&&state.online_flag==0&&g.output->str().empty(),"absent module remains offline without fabricated error log");
        require(g.foreign->waits==std::vector<std::int32_t>({1})&&g.foreign->finishes==1&&g.exports.queries.empty(),"actual worker preserves wait1/finish with no export calls");
        require(g.os->mode==0x40&&g.os->unloaded.empty(),"failed native load restores mode and does not release null module");
    }
    {
        Graph g;State state;g.exports.version_missing=true;online::worker_035b60(state.refs(),*g.selected);
        require(g.output->str()=="online dll found!\nversion function not found\n"&&g.foreign->events.empty(),"missing version export logs and skips foreign invocation");
        require(state.module==0&&g.os->unloaded.size()==1&&g.foreign->finishes==1,"offline worker releases retained module before finish");
    }
    {
        Graph g;State state;g.foreign->version=3;bool line_before_ui=false;
        g.sdk.hook=[&](UINT cp,DWORD flags,LPCCH p,int n,LPWSTR out,int cap){line_before_ui=g.output->str()=="online dll found!\n";return ::MultiByteToWideChar(cp,flags,p,n,out,cap);};
        online::worker_035b60(state.refs(),*g.selected);
        require(g.sdk.messages.size()==1&&g.sdk.messages[0].title==L"crack error"&&g.sdk.messages[0].text==L"Online library version mismatch!"&&g.sdk.messages[0].flags==0x10,"version mismatch reaches typed injected dialog endpoint");
        require(line_before_ui&&g.output->str()=="online dll found!\nonline library version mismatch!\n","source version dialog precedes mismatch log");
        require(g.exports.queries.size()==1&&g.foreign->events==std::vector<std::string>({"version"})&&g.os->unloaded.size()==1,"mismatch skips factory and cleans module through source worker");
    }
    {
        Graph g;State state;g.exports.factory_missing=true;online::worker_035b60(state.refs(),*g.selected);
        require(g.output->str()=="online dll found!\nfactory function not found\n"&&g.foreign->events==std::vector<std::string>({"version"}),"missing factory is distinct from failed version");
        require(state.module==0&&state.object==0&&g.sdk.messages.empty(),"missing factory worker cleans module without invented dialog");
    }
    for(const auto& profile:{std::string("cached-persona;cached-id;cached-user"),std::string("too;many;profile;fields")}){
        Graph g;State state;state.use_last=1;g.foreign->status=1;
        g.stats->values[state.cookie]={0,0x80,std::nullopt};g.stats->values[state.last_avatar]={0,0x80,std::nullopt};g.files->inputs[state.last_profile]=profile;
        online::worker_035b60(state.refs(),*g.selected);
        require(g.output->str()=="online dll found!\nauthentication failed, starting offline\n"&&g.output->flushes==2,"offline authentication prefix and line completion stay separate source operations");
        require(state.username==(profile.starts_with("cached")?"cached-user":"seed-user")&&state.avatar==state.last_avatar,"three-field profile guard and earlier avatar assignment preserved");
        require(g.files->reads==std::vector<Path>({state.last_profile})&&g.files->writes.empty(),"offline recovery reads only explicit own memory profile and writes none");
        require(state.object==0&&state.module==0&&g.foreign->events.back()=="destroy"&&g.foreign->finishes==1,"offline worker cleanup completes before ready publication");
    }
}
void failures_and_partial_state(){
    {
        Graph g;State state;g.encoding->fail=true;
        rejects<std::runtime_error>([&]{online::worker_035b60(state.refs(),*g.selected);},"filesystem widening failure propagates after mode suppression");
        require(g.os->mode==0x41&&g.os->loads.empty()&&g.foreign->finishes==0&&g.output->str().empty(),"pre-load exception adds no mode rollback, module call or ready success");
    }
    {
        Graph g;State state;g.output->maximum=3;g.output->exceptions(std::ios::badbit);
        rejects<std::ios_base::failure>([&]{online::load_online_035bd0(state.refs(),*g.selected);},"found-module prefix stream failure remains visible");
        require(state.module==0x80&&state.object==0&&g.os->mode==0x40&&g.exports.queries.empty(),"stream exception occurs after module publication/mode restore and before exports");
        online::cleanup_0366f0(state.refs(),*g.selected);
        require(state.module==0&&g.os->unloaded.size()==1,"explicit source cleanup can release module retained by log exception");
    }
    {
        Graph g;State state;g.foreign->null_getter=2;
        rejects<online::OutOfEvidenceInput>([&]{online::load_online_035bd0(state.refs(),*g.selected);},"null foreign getter stays explicit domain failure");
        require(state.online_flag==1&&state.username=="own-user"&&state.persona=="seed-persona"&&state.user_id=="seed-id","online flag and earlier getter assignment survive partial failure");
        require(g.output->str()=="online dll found!\nauthentication ok\n"&&g.files->writes.empty(),"getter failure follows success log and precedes profile write");
        online::cleanup_0366f0(state.refs(),*g.selected);
    }
    {
        Graph g;State state;g.foreign->object=0;
        rejects<online::OutOfEvidenceInput>([&]{online::load_online_035bd0(state.refs(),*g.selected);},"null factory result is not fabricated into live object");
        require(state.object==0&&state.module==0x80&&state.online_flag==0,"factory failure preserves existing module prefix");
        online::cleanup_0366f0(state.refs(),*g.selected);
    }
    {
        Graph g;State state;state.module=0x80;state.object=0x90;g.foreign->throw_destroy=true;
        rejects<std::runtime_error>([&]{online::cleanup_0366f0(state.refs(),*g.selected);},"destroy boundary exception propagates before source zeroing");
        require(state.object==0x90&&state.module==0x80&&g.os->unloaded.empty(),"failed slot0 does not fabricate released object/module");
        g.foreign->throw_destroy=false;online::cleanup_0366f0(state.refs(),*g.selected);
        require(state.object==0&&state.module==0&&g.os->unloaded.size()==1,"retry after known controlled failure follows original cleanup order");
    }
    {
        Graph g;State state;g.stats->values[state.cookie]={5,0,std::nullopt};
        rejects<std::filesystem::filesystem_error>([&]{online::load_online_035bd0(state.refs(),*g.selected);},"status error uses selected filesystem exception policy");
        require(state.online_flag==1&&state.username=="own-user"&&g.files->writes.empty(),"file status failure retains earlier foreign profile assignments");
        online::cleanup_0366f0(state.refs(),*g.selected);
    }
}
void full_runtime(){
    b4_test::Fixture f;Graph g;auto resources=f.resources();
    resources.online=std::make_unique<Foreign>(g.foreign);f.online=nullptr;
    resources=op::bind_online_platform(std::move(resources),g.output,g.dialog,g.resources());
    f.construct(std::move(resources));f.runtime->publish();
    require(f.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B3/B4 actual Runtime attaches with new online platform composition");
    require(f.joins.size()==3&&g.os->loads.empty()&&g.output->str().empty(),"online worker effects stay pending before join");
    f.timeout_once=true;require(!f.runtime->close(),"join timeout preserves platform resources and owned foreign provider");
    require(g.foreign->events.empty()&&g.os->loads.empty(),"timed-out join does not run pending online callback");
    require(f.runtime->close(),"retry executes actual worker source callbacks and cleanup");
    require(f.runtime->configuration().username=="own-user"&&f.runtime->configuration().persona_id=="own-persona","online source updates actual shared Runtime configuration");
    require(g.output->str()=="online dll found!\nauthentication ok\n"&&g.os->loads.size()==1,"pending online source uses selected platform once");
    require(g.foreign->waits.empty()&&g.foreign->finishes==0&&g.foreign->events.back()=="destroy","B3 intercepts phase methods into shared Execution and later drains online cleanup");
    require(g.os->unloaded==std::vector<online::Module>({0x80})&&g.foreign->wrong_calls==0,"quiescent Runtime cleanup uses selected module release without lower OS fallback");
    const auto before=g.foreign->events.size();require(f.runtime->close()&&g.foreign->events.size()==before,"closed Runtime never repeats provider cleanup");
    f.runtime.reset();require(!bootstrap_native_recovered::is_bound(),"native source entry binding closes after joins and provider cleanup");
}
}
int main(){try{successful_source();absent_exports_and_offline();failures_and_partial_state();full_runtime();std::cout<<"PASS: "<<b13_test::checks<<" online platform integration checks; actual online/worker/cleanup/Runtime source, controlled foreign modules/UI/files\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
