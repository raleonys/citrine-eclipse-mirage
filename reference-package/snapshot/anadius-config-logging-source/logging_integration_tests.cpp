#include "logging_test_fixture.hpp"
#include "../anadius-services-completion-source/completion.hpp"

namespace {
using namespace b11_test;
constexpr std::string_view base=R"("Config2" {"Game" {"Version" "1.2.3" "ContentId" "synthetic-own" "Language" "tr"} "User" {"Username" "base-user"} "Content" {"a" {"Name" "Alpha"} "b" {"Name" "Beta"}} "Achievements" {"AchievementIDs" "a;b" "AchievementsSet" "own"}})";
constexpr std::string_view over=R"("Config2" {"User" {"Username" "override-user"} "Content" {"ignored" {"Name" "Never"}}})";
struct MemoryGraph:Graph {
    std::shared_ptr<b8_test::MemoryFilesState> fs=std::make_shared<b8_test::MemoryFilesState>();
    b8_test::MemoryFiles files{fs};config_startup::FilePass pass{files};configuration::State state{};
    MemoryGraph(){
        state.executable_path=u"C:\\own\\host.exe";
        fs->inputs[u"C:\\own\\anadius.cfg"]=base;fs->inputs[u"C:\\own\\anadius_override.cfg"]=over;
    }
    void apply(bool override_pass){pass.load_and_apply(state,override_pass,selected);}
};
void source_pass(){
    MemoryGraph g;services_completion::BootstrapLogger bootstrap(g.output);
    auto* token=bootstrap.insert(reinterpret_cast<void*>(services_completion::BootstrapLogger::stream_identity),0x180083cd8ULL);
    token=bootstrap.insert(token,0x180086720ULL);token=bootstrap.insert(token,0x180086718ULL);bootstrap.finish(token);
    g.apply(false);
    require(g.state.username=="base-user"&&g.state.content_id=="synthetic-own","actual FilePass parser and mutable application reach one state");
    require(g.state.content.size()==2&&g.state.achievements.size()==2,"ordered collection application is preserved");
    require(g.output->str()=="LSX emu version 2.0.1.0\nParsing anadius.cfg\n","B10 and B11 share one explicitly owned stream under serialized use");
    g.apply(true);
    require(g.state.username=="override-user"&&g.state.content.size()==2,"override reaches selected logger and keeps base-only collections");
    require(g.output->str()=="LSX emu version 2.0.1.0\nParsing anadius.cfg\nParsing anadius_override.cfg\n","base and override produce exact ordered lines");
    require(g.output->flushes==3&&g.lower->events.empty(),"one flush per line and no lower logger calls");
    require(g.fs->reads==std::vector<Path>({u"C:\\own\\anadius.cfg",u"C:\\own\\anadius_override.cfg"}),"source file path selection retained");
    require(g.fs->writes.empty(),"FilePass logging does not write configuration or credentials");
}
void failure_prefixes(){
    {
        MemoryGraph g;g.fs->inputs[u"C:\\own\\anadius.cfg"]="\"Config2\" {";
        rejects<parser::ParseError>([&]{g.apply(false);},"malformed base propagates parser exception");
        require(g.output->str().empty()&&g.state.username.empty(),"base parser failure precedes logging and application");
    }
    {
        MemoryGraph g;g.apply(false);g.fs->inputs[u"C:\\own\\anadius_override.cfg"]="\"Config2\" {";
        rejects<parser::ParseError>([&]{g.apply(true);},"malformed override propagates parser exception");
        require(g.output->str()=="Parsing anadius.cfg\n"&&g.state.username=="base-user","override parser failure retains successful base prefix");
    }
    {
        MemoryGraph g;g.apply(false);g.output->maximum=3;g.output->exceptions(std::ios::badbit);
        rejects<std::ios_base::failure>([&]{g.apply(true);},"override prefix stream failure propagates");
        require(g.output->str()=="Parsing anadius.cfg\nPar"&&g.state.username=="base-user","partial override log precedes and prevents override application");
        require(g.conversion->inputs.size()==2,"failed override prefix never starts its filename conversion");
    }
    {
        MemoryGraph g;g.conversion->fail_call=1;g.state.username="seed";
        rejects<std::runtime_error>([&]{g.apply(false);},"conversion failure propagates from source FilePass");
        require(g.output->str()=="Parsing "&&g.state.username=="seed"&&g.state.content_id.empty(),"conversion failure occurs after parse but before any config assignment");
    }
    {
        MemoryGraph g;g.fs->inputs[u"C:\\own\\anadius.cfg"]=R"("Config2" {"Game" {"ContentId" "updated-before-failure" "KeyForLicense" "synthetic-test-input" "Language" "fr"}})";
        g.state.game_attributes["INSTALLED_LANGUAGE"]="seed";
        rejects<std::logic_error>([&]{g.apply(false);},"unselected license subsystem remains an explicit failure");
        require(g.output->str()=="Parsing anadius.cfg\n","license boundary is reached only after completed configuration log");
        require(g.state.content_id=="updated-before-failure"&&g.state.game_attributes["INSTALLED_LANGUAGE"]=="seed","application prefix retained without fabricated rollback after license failure");
        require(g.fs->writes.empty(),"controlled license fixture performs no token or license write");
    }
    {
        MemoryGraph g;g.fs->inputs.clear();g.apply(false);
        require(g.output->str()=="Parsing anadius.cfg\n","source initial stream failure takes its observed empty-byte parse path");
        require(g.fs->reads.size()==1&&g.fs->writes.empty(),"missing synthetic input does not fabricate a file write");
    }
    {
        MemoryGraph g;g.fs->null_input=true;
        rejects<std::logic_error>([&]{g.apply(false);},"invalid null factory result stays an explicit exception");
        require(g.output->str().empty(),"factory contract failure is not swallowed as a logged empty document");
    }
}
void own_native_files(const std::filesystem::path& directory){
    // Caller-selected output fixture directory only; never inspect user CFGs.
    std::filesystem::create_directories(directory);
    b8_test::write_own(directory/"anadius.cfg",base);b8_test::write_own(directory/"anadius_override.cfg",over);
    auto output=std::make_shared<OwnedStream>();auto lower=std::make_shared<b8_test::LowerState>();
    lower->executable=(directory/"own-host.exe").u16string();lower->program=(directory/"program").u16string();lower->local=(directory/"local").u16string();
    config_logging::ConfigurationLogging logging(output,std::make_unique<b8_test::Lower>(lower));
    file_recovered::NativeFactory files;configuration::State state{};
    config_startup::initialize_configuration(state,logging,files);
    require(state.username=="override-user"&&state.content.size()==2,"actual own disk files reach complete source initializer");
    require(output->str()=="Parsing anadius.cfg\nParsing anadius_override.cfg\n"&&output->flushes==2,"native file path logs exact filenames in source order");
    require(std::find(lower->events.begin(),lower->events.end(),"log-base")==lower->events.end()&&std::find(lower->events.begin(),lower->events.end(),"log-override")==lower->events.end(),"post-configuration lower policy stays separate from selected logging");
    require(lower->pngs==2&&lower->persistence==1&&lower->environment.size()==2,"same source initializer continues into controlled post-configuration effects");
    require(b8_test::read_own(directory/"anadius.cfg")==base&&b8_test::read_own(directory/"anadius_override.cfg")==over,"own input bytes unchanged by logging and initialization");
}
std::filesystem::path own_root(const char* argument){
    namespace fs=std::filesystem;
    const auto expected=fs::weakly_canonical(fs::path(__FILE__).parent_path().parent_path()/"source-workspace"/"build"/"b11-fixtures");
    const auto requested=fs::weakly_canonical(fs::path(argument));
    if(!b8_test::same_windows_path(expected,requested))throw std::invalid_argument("B11 fixture root must resolve inside source-workspace/build/b11-fixtures");
    fs::create_directories(requested);
    const auto child=requested/("logging-"+std::to_string(::GetCurrentProcessId())+"-"+std::to_string(::GetTickCount64()));
    if(!fs::create_directory(child))throw std::runtime_error("fresh fixture child required");
    b8_test::fixture_root=fs::weakly_canonical(child);return b8_test::fixture_root;
}
}
int main(int argc,char** argv){try{if(argc!=2)throw std::invalid_argument("owned fixture output directory required");source_pass();failure_prefixes();own_native_files(own_root(argv[1]));std::cout<<"PASS: "<<b11_test::checks<<" configuration logging integration checks; actual FilePass, base/override failure prefixes and own files\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
