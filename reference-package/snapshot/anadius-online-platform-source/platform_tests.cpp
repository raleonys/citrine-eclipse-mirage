#include "platform_test_fixture.hpp"

namespace {
using namespace b13_test;
void guards(){
    Graph g;
    rejects<std::invalid_argument>([&]{op::OnlinePlatform e({},g.dialog,std::make_unique<Foreign>(g.foreign));},"null stream rejected");
    rejects<std::invalid_argument>([&]{op::OnlinePlatform e(g.output,{},std::make_unique<Foreign>(g.foreign));},"null dialog rejected");
    rejects<std::invalid_argument>([&]{op::OnlinePlatform e(g.output,g.dialog,{});},"null foreign provider rejected");
    for(unsigned field=0;field<6;++field){
        auto r=g.resources();
        switch(field){case 0:r.os.reset();break;case 1:r.encoding.reset();break;case 2:r.errors.reset();break;case 3:r.stats.reset();break;case 4:r.files.reset();break;default:r.get_proc=nullptr;}
        rejects<std::invalid_argument>([&]{op::OnlinePlatform e(g.output,g.dialog,std::make_unique<Foreign>(g.foreign),std::move(r));},"every missing owned resource is rejected");
    }
    require(g.output->str().empty()&&g.exports.queries.empty()&&g.sdk.messages.empty()&&g.files->reads.empty()&&g.files->writes.empty(),"construction and rejected resources perform no external I/O");
    auto output=std::make_shared<b11_test::OwnedStream>();auto dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());
    std::weak_ptr<b11_test::OwnedStream> output_ref=output;std::weak_ptr<library_effects::Dialog> dialog_ref=dialog;
    {op::OnlinePlatform e(output,dialog,std::make_unique<Foreign>(g.foreign),g.resources());output.reset();dialog.reset();
     e.log_literal_006880("authentication ok");e.log_finish_0088e0();
     require(output_ref.lock()->str()=="authentication ok\n"&&!dialog_ref.expired(),"selected policy retains stream buffer and dialog owners");}
    require(output_ref.expired()&&dialog_ref.expired(),"dependency owners released after platform owner");
    bootstrap_backend::Resources backend;backend.online=std::make_unique<Foreign>(g.foreign);
    auto bound=op::bind_online_platform(std::move(backend),g.output,g.dialog,g.resources());
    require(dynamic_cast<op::OnlinePlatform*>(bound.online.get())!=nullptr,"Resources binder installs selected online policy");
    require(!bound.server&&!bound.host&&!bound.hook_cleanup,"binder invents no server/host/hook providers");
}
void os_paths_exports(){
    Graph g;auto& e=*g.selected;
    require(e.suppress_errors_0450a0()==0x40&&g.os->mode==0x41,"suppress returns GetErrorMode result and applies bit1");
    e.restore_error_mode(0xaabbccdd);require(g.os->modes==std::vector<std::uint32_t>({0x41,0xaabbccdd}),"restore passes complete mode and ignores set return");
    require(e.filesystem_widen_006d70("dir_\xc4\xb1") ==u"dir_\u0131","filesystem widening reuses owned encoding source");
    require(g.encoding->cps==std::vector<std::uint32_t>({65001,65001})&&g.encoding->flags==std::vector<std::uint32_t>({8,8}),"filesystem policy uses selected codepage and strict flags8, distinct from dialog");
    require(g.encoding->locales==1&&g.encoding->ansi==0,"UTF8 filesystem locale selects no ANSI API query");
    require(e.filesystem_widen_006d70("").empty()&&g.encoding->locales==2&&g.encoding->cps.size()==2,"empty widening still selects codepage but skips conversion");
    require(e.parent_path_019820(u"C:\\own\\host.exe")==u"C:\\own","source parent extraction retained");
    require(e.join_016b00(u"C:\\own",u"file")==u"C:\\own\\file","source Windows join retained");
    const Path path(u"C:\\own\\a\0suffix.dll",19);
    require(e.load_copy_0450d0(path)==0x80&&g.os->loads.back()==L"C:\\own\\a","typed copied wchar path observes c_str truncation without normalization");
    require(path.size()==19&&path[7]==u'a'&&path[8]==0,"load adapter does not mutate original UTF16 input");
    require(e.get_proc_address(0x80,"version_check")==0x81&&e.get_proc_address(0x80,"create_online")==0x82,"typed SDK lookup returns opaque procedure tokens");
    require(g.exports.queries==std::vector<std::pair<std::uintptr_t,std::string>>({{0x80,"version_check"},{0x80,"create_online"}}),"lookup uses exact module/name operands");
    g.exports.version_missing=true;require(e.get_proc_address(0x80,"version_check")==0,"null export result is preserved");
    rejects<std::domain_error>([&]{(void)e.get_proc_address(0x80,"other");},"unobserved export name rejected before SDK");
    rejects<std::domain_error>([&]{(void)e.get_proc_address(0x80,std::string_view("version_check\0x",15));},"borrowed embedded-NUL name rejected rather than scanned");
    require(g.exports.queries.size()==3,"unsupported names cause no SDK query");
    require(!e.free_library(0x80)&&g.os->unloaded==std::vector<online::Module>({0x80}),"release boolean and token forwarded without implicit policy");
    require(g.foreign->events.empty()&&g.foreign->wrong_calls==0,"OS/path/export operations bypass mandatory foreign calls");
}
void files_status(){
    Graph g;auto& e=*g.selected;
    g.stats->values[u"regular"]={0,0x80,std::nullopt};g.stats->values[u"directory"]={0,0x10,std::nullopt};
    require(e.status_01ea20(u"regular",3).kind==2&&e.status_01ea20(u"directory",3).kind==3,"existing status source distinguishes regular and directory");
    require(e.status_01ea20(u"missing",3).kind==1,"missing-file status preserves source classification");
    require(g.stats->queries==std::vector<std::pair<Path,std::uint32_t>>({{u"regular",3},{u"directory",3},{u"missing",3}}),"status query path and flags preserved");
    g.stats->values[u"reparse"]={0,0x400,std::nullopt};
    rejects<status_recovered::OutOfEvidenceStatus>([&]{(void)e.status_01ea20(u"reparse",3);},"unknown reparse tag is not fabricated");
    try{e.filesystem_error_018130("status",0x80000005,u"fixture");}catch(const std::filesystem::filesystem_error& error){
        require(static_cast<std::uint32_t>(error.code().value())==0x80000005&&&error.code().category()==&std::system_category(),"selected filesystem exception preserves full code/category");
        require(error.path1()==std::filesystem::path(u"fixture"),"selected filesystem exception retains supplied path");
    }
    const std::string bytes("a\0b",3);g.files->inputs[u"input"]=bytes;
    require(e.read_binary_03b3f0(u"input")==bytes,"existing binary reader retains owned embedded NUL bytes");
    require(e.read_binary_03b3f0(u"absent").empty(),"nonzero initial stream state follows original empty-byte path");
    require(e.write_binary_03c1d0(u"output",bytes)&&g.files->outputs[u"output"]==bytes,"existing binary writer retains exact bytes and boolean");
    g.files->bad_output=true;require(!e.write_binary_03c1d0(u"bad","x")&&!g.files->outputs.contains(u"bad"),"bad initial output stream is not fabricated success");
    g.files->bad_output=false;g.files->throw_write=true;
    rejects<std::runtime_error>([&]{(void)e.write_binary_03c1d0(u"failed","x");},"file write exception remains visible");
    g.files->null_input=true;rejects<std::invalid_argument>([&]{(void)e.read_binary_03b3f0(u"invalid");},"invalid null input factory contract propagates");
    require(g.foreign->wrong_calls==0,"file/status policies never delegate selected work to lower");
}
void logging_dialog_foreign(){
    Graph g;auto& e=*g.selected;
    for(const auto* text:{"online dll found!","version function not found","online library version mismatch!","factory function not found","authentication failed, ","starting offline","authentication ok"}){
        const auto before=g.output->str();e.log_literal_006880(text);e.log_finish_0088e0();
        require(g.output->str()==before+text+"\n","each observed literal uses exact C-string insertion and newline");
    }
    require(g.output->flushes==7,"one flush per explicit log finish");
    const auto before=g.output->str();rejects<std::domain_error>([&]{e.log_literal_006880(std::string_view("authentication ok\0suffix",24));},"unobserved literal/NUL extension rejected before output");
    require(g.output->str()==before,"rejected literal leaves stream unchanged");
    e.dialog_038bc0("crack error","Online library version mismatch!",0x10);
    require(g.sdk.messages.size()==1&&g.sdk.messages[0].title==L"crack error"&&g.sdk.messages[0].text==L"Online library version mismatch!"&&g.sdk.messages[0].flags==0x10,"online dialog reuses selected B12 policy with exact arguments");
    require(g.sdk.conversions.size()==4&&g.sdk.conversions[0].flags==0,"dialog keeps flags0 instead of filesystem flags8");
    require(e.call_version_check(0x81)==2&&e.call_create_online(0x82,u"own-cookie")==0x90,"export calls stay explicit owned lower contracts");
    require(e.status_virtual_08(0x90)==0&&std::string(e.username_virtual_10(0x90))=="own-user"&&std::string(e.persona_virtual_18(0x90))=="own-persona"&&std::string(e.user_id_virtual_20(0x90))=="own-id","all four value virtual calls stay lower and preserve returns");
    e.destroy_virtual_00(0x90);e.wait_steps_036880(1);e.finish_flag_036820();
    require(g.foreign->events==std::vector<std::string>({"version","factory","status","get-1","get-2","get-3","destroy"})&&g.foreign->cookie==u"own-cookie","foreign token/order/string contracts forwarded without dereference");
    require(g.foreign->waits==std::vector<std::int32_t>({1})&&g.foreign->finishes==1&&g.foreign->wrong_calls==0,"phase methods remain exact owned lower operations");
}
}
int main(){try{guards();os_paths_exports();files_status();logging_dialog_foreign();std::cout<<"PASS: "<<b13_test::checks<<" online platform core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
