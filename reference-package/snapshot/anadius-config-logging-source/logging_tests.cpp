#include "logging_test_fixture.hpp"

namespace {
using namespace b11_test;
void guards(){
    auto output=std::make_shared<OwnedStream>();auto state=std::make_shared<b8_test::LowerState>();
    rejects<std::invalid_argument>([&]{config_logging::ConfigurationLogging s({},std::make_unique<b8_test::Lower>(state));},"null stream rejected");
    rejects<std::invalid_argument>([&]{config_logging::ConfigurationLogging s(output,{});},"null lower rejected");
    rejects<std::invalid_argument>([&]{config_logging::ConfigurationLogging s(output,std::make_unique<b8_test::Lower>(state),{});},"null encoding rejected");
    rejects<std::invalid_argument>([&]{config_logging::ConfigurationLogging s(output,std::make_unique<b8_test::Lower>(state),std::make_unique<encoding_recovered::NativeCalls>(),{});},"null errors rejected");
    require(output->str().empty()&&output->flushes==0,"construction performs no stream I/O");
    std::weak_ptr<OwnedStream> weak=output;
    {
        config_logging::ConfigurationLogging s(output,std::make_unique<b8_test::Lower>(state));output.reset();
        require(!weak.expired(),"stream retained by configuration owner");s.log_config_path(u"owned.cfg");
        require(weak.lock()->str()=="Parsing owned.cfg\n","retained stream remains usable");
    }
    require(weak.expired(),"stream released after owner destruction");
}
void names(){
    struct Case{Path path;std::string name;};
    const Case cases[]={
        {u"C:\\private\\anadius.cfg","anadius.cfg"},{u"C:/private/anadius_override.cfg","anadius_override.cfg"},
        {u"\\\\server\\share\\name.cfg","name.cfg"},{u"C:name.cfg","name.cfg"},
        {u"C:\\private\\",""},{u"/",""},{u"",""},
        {u"dir/ayar_\u0131\u015f\U0001f600.cfg","ayar_\xc4\xb1\xc5\x9f\xf0\x9f\x98\x80.cfg"},
        {Path(u"dir/a\0b.cfg",11),std::string("a\0b.cfg",7)},
        {Path(1,static_cast<char16_t>(0xd800))+u"/valid.cfg","valid.cfg"}
    };
    for(const auto& item:cases){
        Graph g;g.selected.log_config_path(item.path);
        require(g.output->str()=="Parsing "+item.name+"\n","exact prefix plus UTF8 filename and newline");
        require(g.output->flushes==1,"each successful log flushes once");
        require(g.lower->events.empty(),"selected log never invokes lower logger");
        require(g.conversion->locale==0&&g.conversion->ansi==0,"UTF8 route never queries filesystem or locale codepage");
        if(!item.name.empty()){
            require(g.conversion->codepages==std::vector<std::uint32_t>({65001,65001}),"two-pass explicit UTF8 codepage");
            require(g.conversion->flags==std::vector<std::uint32_t>({0x80,0x80}),"strict UTF8 flags retained");
            require(g.conversion->prefixes==std::vector<std::string>({"Parsing ","Parsing "}),"prefix precedes query and fill");
        }else require(g.conversion->inputs.empty(),"empty filename skips conversion calls");
    }
    Graph tail;tail.conversion->inject_tail=true;tail.selected.log_config_path(u"file.cfg");
    require(tail.output->str()==std::string("Parsing Z\0\0\0\n",13),"counted insertion retains first-pass-sized zero tail");
    Graph width;width.output->fill('.');width.output->width(10);width.selected.log_config_path(u"a.cfg");
    require(width.output->str()=="..Parsing a.cfg\n"&&width.output->width()==0,"formatted prefix consumes and resets width");
}
void failures(){
    for(unsigned call:{1U,2U}){
        Graph g;g.conversion->fail_call=call;
        rejects<std::runtime_error>([&]{g.selected.log_config_path(u"a.cfg");},"conversion exception propagates");
        require(g.output->str()=="Parsing "&&g.output->flushes==0,"conversion failure retains prefix without newline or flush");
    }
    Graph invalid;
    rejects<std::system_error>([&]{invalid.selected.log_config_path(Path(1,static_cast<char16_t>(0xd800)));},"invalid UTF16 filename propagates conversion error");
    require(invalid.output->str()=="Parsing "&&invalid.output->flushes==0,"native UTF8 error preserves only prefix");
    Graph prefix;prefix.output->maximum=3;prefix.output->exceptions(std::ios::badbit);
    rejects<std::ios_base::failure>([&]{prefix.selected.log_config_path(u"a.cfg");},"prefix stream exception propagates");
    require(prefix.output->str()=="Par"&&prefix.conversion->inputs.empty(),"prefix exception prevents filename conversion");
    Graph failed;failed.output->setstate(std::ios::failbit);failed.selected.log_config_path(u"a.cfg");
    require(failed.output->str().empty()&&failed.conversion->inputs.size()==2,"nonthrowing failed stream still performs conversion");
    require(failed.output->flushes==0,"failed stream does not fabricate a flush");
    Graph flush;flush.output->fail_flush=true;flush.output->exceptions(std::ios::badbit);
    rejects<std::ios_base::failure>([&]{flush.selected.log_config_path(u"a.cfg");},"flush failure propagates");
    require(flush.output->str()=="Parsing a.cfg\n"&&flush.output->flushes==1,"flush failure preserves prior line output");
}
void forwards(){
    Graph g;require(g.selected.executable_path_03c6d0()==g.lower->executable,"executable route forwarded");
    require(g.selected.program_data_03c920()==g.lower->program&&g.selected.local_app_data_03c6f0()==g.lower->local,"environment paths forwarded");
    require(g.selected.parent_path_019820(u"C:\\dir\\file")==u"C:\\dir","parent forwarded");
    require(g.selected.join_016b00(u"C:\\dir",u"file")==u"C:\\dir\\file","join forwarded");
    require(g.selected.widen_0145d0("value")==u"value"&&g.selected.filesystem_widen_010540("value")==u"value","widen routes forwarded");
    require(g.selected.narrow_path(u"value")=="value"&&g.selected.filename(u"a/b")==u"b","narrow and filename forwarded");
    require(g.selected.resolve_path_0192d0("file",u"C:\\dir\\host.exe",true)==u"C:\\dir\\file","resolve forwarded");
    require(g.selected.crt_is_space(' ')&&g.selected.crt_tolower('A')=='a',"CRT routes forwarded");
    rejects<std::logic_error>([&]{(void)g.selected.load_document(u"unused");},"legacy document boundary remains explicit");
    rejects<std::logic_error>([&]{g.selected.apply_license_key_02a830("synthetic");},"license boundary remains explicit");
    g.selected.create_directories_018e00(u"unused");g.selected.write_embedded_png_03c020(u"unused",configuration::EmbeddedPng::default_user_1800809d0);
    g.selected.remove_04948c(u"unused");configuration::State state{};g.selected.update_achievement_storage_01eb00(state);
    require(g.lower->creates==1&&g.lower->pngs==1&&g.lower->removes==1&&g.lower->persistence==1,"persistence effects forwarded");
    require(g.selected.directory_entries(u"unused").empty(),"directory vector forwarded");
    g.selected.visit_directory_entries(u"unused",{nullptr,[](void*,const Path&){throw std::logic_error("empty visitor must not run");}});
    require(g.lower->enumerations==2,"visitor passes through existing provider extension");
    require(!g.selected.set_environment_variable("synthetic","value")&&g.lower->environment.size()==1,"environment boolean forwarded unchanged");
    require(g.output->str().empty(),"nonlogging routes perform no log I/O");
    bootstrap_backend::Resources resources;resources.configuration=std::make_unique<b8_test::Lower>(g.lower);
    auto bound=config_logging::bind_configuration_logging(std::move(resources),g.output);
    require(dynamic_cast<config_logging::ConfigurationLogging*>(bound.configuration.get())!=nullptr,"Resources helper installs selected wrapper");
    require(!bound.server&&!bound.online&&!bound.host,"binding does not invent production providers");
}
}
int main(){try{guards();names();failures();forwards();std::cout<<"PASS: "<<b11_test::checks<<" configuration logging core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
