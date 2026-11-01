#include "storage.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include "../anadius-config-startup-source/startup.hpp"
#include "../anadius-path-source/paths.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace anadius;
namespace {
unsigned checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
using Path=configuration::Path;
Path wide(std::string_view s){return {s.begin(),s.end()};}
Path join(const Path& a,const Path& b){return path_recovered::join_016b00(a,b);}
configuration::Achievement record(std::string name={}){return {5,9,8,"old-grant",std::move(name),"steam-id",77};}
struct Input final:file_recovered::Input {
    std::string bytes;bool exists;
    Input(std::string b,bool e):bytes(std::move(b)),exists(e){}
    std::uint32_t initial_state()const override{return exists?0:1;}
    void seek_03b610(std::int64_t p,file_recovered::Seek)override{require(p==0,"seek zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{std::copy(bytes.begin(),bytes.end(),out.begin());return bytes.size();}
    void close_03ae70()override{}
};
struct Output final:file_recovered::Output {
    std::string& bytes;bool bad;
    Output(std::string& b,bool fail):bytes(b),bad(fail){}
    std::uint32_t initial_state()const override{return bad?1:0;}
    void write_03be80(std::string_view value)override{bytes.assign(value);}
    void close_03ae70()override{}
};
struct Files final:file_recovered::Factory {
    std::map<Path,std::string> input,output;
    std::vector<Path> reads,writes;
    bool bad_output=false;
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& p,std::uint32_t m,std::uint32_t s)override{
        require(m==0x20 && s==0x40,"input flags");reads.push_back(p);const auto it=input.find(p);return std::make_unique<Input>(it==input.end()?"":it->second,it!=input.end());
    }
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t m,std::uint32_t s)override{
        require(m==0x20 && s==0x40,"output flags");writes.push_back(p);return std::make_unique<Output>(output[p],bad_output);
    }
};
struct Effects final:achievement_storage::Effects {
    std::vector<std::string> widened,epochs,events;
    std::vector<Path> directories,status_paths;
    status_recovered::Status status{2,0,0};
    std::string fail_epoch;
    Path filesystem_widen_006d70(std::string_view value)override{widened.emplace_back(value);return wide(value);}
    std::int64_t timestamp_to_epoch_041e10(std::string_view value)override{
        epochs.emplace_back(value);events.emplace_back("epoch");if(value==fail_epoch)throw std::runtime_error("synthetic epoch failure");return value=="negative"?-1:11;
    }
    status_recovered::Status status_01ea20(const Path& path,std::uint32_t flags)override{require(flags==3,"status flags3");status_paths.push_back(path);events.emplace_back("status");return status;}
    [[noreturn]]void filesystem_error_018130(std::string_view op,std::uint32_t error,const Path&)override{require(op=="status" && error!=0,"status error contract");events.emplace_back("error");throw std::runtime_error("synthetic status failure");}
    void create_directories_018e00(const Path& path)override{directories.push_back(path);events.emplace_back("mkdir");}
};
struct Fixture {
    configuration::State state{};achievement_storage::Paths paths;xml_reader_bridge::Backend xml;achievement_storage::HostCrtNumbers numbers;Effects effects;Files files;
    achievement_storage::Storage storage{paths,xml,numbers,effects,files};
    Fixture(){state.user_directory=u"USER";}
};
struct ReturnCodeXml final:messages_recovered::XmlBackend {
    xml_reader_bridge::Backend actual;bool force_error=false;
    messages_recovered::ParseResult parse_c_string(std::string_view raw)override{
        auto parsed=actual.parse_c_string(raw);parsed.returned_code=99;parsed.has_error=force_error;return parsed;
    }
    std::unique_ptr<messages_recovered::XmlPrinter> printer()override{return actual.printer();}
};
void xml_updates() {
    Fixture f;f.paths.xml_path=u"retained.xml";f.paths.ini_path=u"retained.ini";f.state.achievements["known"]=record("configuration-name");
    f.files.input[f.paths.xml_path]=R"(<AnyRoot><Ignored Count="8"/><Unexpected Id="known" Count="-2" Progress="0xFFFFFFFF" Grant="file-grant" Name="must-not-replace"/><Whatever Id="new" Name="first-name" Count="12tail" Progress="0X10"/><Whatever Id="new" Name="second-name"/><Empty Id="" Grant="" Name=""/></AnyRoot>)";
    f.storage.update_01eb00(f.state);
    require(f.files.reads==std::vector<Path>{u"retained.xml"},"empty set retains and reads prior XML path");
    require(f.paths.ini_path==u"retained.ini" && f.state.achievements.at("known").field70==77 && f.effects.status_paths.empty(),"empty SteamAppId retains old INI/epochs and skips status");
    const auto& known=f.state.achievements.at("known");
    require(known.field00==5 && known.field04==-2 && known.field08==-1,"XML changes Count/Progress low32 but keeps configured total");
    require(known.timestamp=="file-grant" && known.display_name=="configuration-name" && known.steam_id=="steam-id","Grant replaces and configured Name/SteamId retained");
    const auto& added=f.state.achievements.at("new");
    require(added.field00==1 && added.field04==0 && added.field08==0,"duplicate row with absent numbers resets both to zero");
    require(added.timestamp=="0000-00-00T00:00:00" && added.display_name=="first-name" && added.steam_id.empty(),"unknown record defaults and first nonempty Name only");
    require(f.state.achievements.contains("") && f.state.achievements.at("").timestamp.empty(),"present empty ID/Grant accepted");
    require(f.state.achievements.size()==3,"missing Id is skipped");
    Fixture n;n.state.achievements_set="fixture-set";n.paths.xml_path=u"old.xml";
    const auto expected=join(u"USER",u"achievement-fixture-set.xml");
    n.files.input[expected]=R"(<root><a Id="id" Count="12tail" Progress=" 0X10"/><b Id="bad" Count="invalid" Progress="-7suffix"/><c Id="fill" Name=""/><d Id="fill" Name="later"/></root>)";
    n.storage.update_01eb00(n.state);
    require(n.paths.xml_path==expected && n.effects.widened==std::vector<std::string>{"achievement-fixture-set.xml"},"nonempty set recomputes XML filename/path");
    require(n.state.achievements.at("id").field04==12 && n.state.achievements.at("id").field08==16,"signed decimal suffix and whitespace hexadecimal");
    require(n.state.achievements.at("bad").field04==0 && n.state.achievements.at("bad").field08==-7,"invalid default and signed suffix");
    require(n.state.achievements.at("fill").display_name=="later","empty Name permits later fill");
    Fixture nul;nul.paths.xml_path=u"nul.xml";nul.files.input[nul.paths.xml_path]=R"(<r><a Id="prefix"/></r>)";nul.files.input[nul.paths.xml_path].push_back('\0');nul.files.input[nul.paths.xml_path]+="<broken";
    nul.storage.update_01eb00(nul.state);require(nul.state.achievements.contains("prefix"),"reader original C-string boundary ignores bytes after NUL");
    for(const auto* raw:{"<broken", "", "<r/>"}){
        Fixture empty;empty.paths.xml_path=u"x.xml";empty.files.input[empty.paths.xml_path]=raw;empty.state.steam_app_id="42";empty.state.achievements["seed"]=record();
        empty.storage.update_01eb00(empty.state);require(empty.state.achievements.size()==1 && empty.state.achievements.at("seed").field04==9,"invalid/empty/root-without-child does not mutate XML fields");
        require(empty.effects.epochs.size()==1 && empty.effects.status_paths.size()==1,"XML failure does not skip independent Steam branch");
    }
    for(const bool error:{false,true}) {
        Fixture probe;ReturnCodeXml proxy;proxy.force_error=error;probe.paths.xml_path=u"proxy.xml";
        probe.files.input[probe.paths.xml_path]=R"(<root><entry Id="proxy-id"/></root>)";
        achievement_storage::Storage storage(probe.paths,proxy,probe.numbers,probe.effects,probe.files);storage.update_01eb00(probe.state);
        require(probe.state.achievements.contains("proxy-id")==!error,"document error state controls import; parser numeric return code ignored");
    }
}
struct AttributeNode final:messages_recovered::XmlNode {
    std::optional<std::string> value;
    std::string_view name()const override{return "fixture";}
    const XmlNode* first_element()const override{return nullptr;}
    const XmlNode* next_element()const override{return nullptr;}
    std::optional<std::string_view> attribute(std::string_view)const override{return value?std::optional<std::string_view>(*value):std::nullopt;}
};
struct SpyNumbers final:achievement_storage::Numbers {
    unsigned spaces=0;std::string original,route;
    bool is_space(unsigned char c)override{++spaces;return c==' ';}
    std::optional<std::uint32_t> scan_decimal(std::string_view s)override{original=s;route="decimal";return 17;}
    std::optional<std::uint32_t> scan_hex(std::string_view s)override{original=s;route="hex";return 19;}
};
void numbers() {
    AttributeNode node;achievement_storage::HostCrtNumbers host;
    for(const auto& [text,expected]:std::vector<std::pair<std::string,std::uint32_t>>{{"12tail",12},{"-1",0xffffffffU},{"2147483647",0x7fffffffU},{"-2147483648",0x80000000U},{" 0Xf",15},{"0xffffffff",0xffffffffU},{"-0x10",0},{"+0x10",0},{"invalid",91},{"",91}}){
        node.value=text;require(achievement_storage::attribute_003b70(node,"Count",91,host)==expected,"host compatibility bounded numeric fixture");
    }
    node.value.reset();require(achievement_storage::attribute_003b70(node,"Count",91,host)==91,"absent attribute fallback");
    SpyNumbers spy;node.value=" 0x10";
    require(achievement_storage::attribute_003b70(node,"Count",0,spy)==19 && spy.route=="hex" && spy.original==" 0x10","prefix detection passes untrimmed original string");
    spy=SpyNumbers{};node.value=std::string(1,static_cast<char>(0xa0))+"0x10";
    require(achievement_storage::attribute_003b70(node,"Count",0,spy)==17 && spy.route=="decimal" && spy.spaces==0,"high-bit byte bypasses CRT whitespace query and selects decimal");
    spy=SpyNumbers{};node.value=std::string("0x10\0ignored",12);
    require(achievement_storage::attribute_003b70(node,"Count",0,spy)==19 && spy.original=="0x10","arbitrary backend attribute obeys original C-string limit");
}
void steam_and_failures() {
    Fixture f;f.state.steam_app_id="42";f.state.achievements["z"]=record();f.state.achievements["a"]=record();f.state.achievements["a"].timestamp="negative";
    f.storage.update_01eb00(f.state);
    const auto ini=join(join(join(u"USER",u"achievement_watcher"),u"42"),u"achievements.ini");
    require(f.paths.ini_path==ini && f.effects.widened==std::vector<std::string>{"achievements.ini","42","achievement_watcher"},"INI path and exact conversion order");
    require(f.effects.epochs==std::vector<std::string>{"negative","old-grant"} && f.state.achievements.at("a").field70==0xffffffffffffffffULL,"epoch traversal lexical and low64 signed result");
    require(f.files.writes.empty() && f.effects.directories.empty(),"existing regular file kind2 skips writer");
    for(const auto kind:{-1,0,1,2,3,9}){
        Fixture value;value.state.steam_app_id="42";value.state.achievements["key"]=record();value.effects.status={kind,0,5};
        if(kind==1 || kind==9){value.storage.update_01eb00(value.state);require(value.files.writes.size()==1,"status mask allows errors for kinds1/9 and writes");}
        else {rejects<std::runtime_error>([&]{value.storage.update_01eb00(value.state);},"other error statuses throw");require(value.files.writes.empty() && value.state.achievements.at("key").field70==11,"epochs happen before status error and no write");}
    }
    Fixture write;write.state.steam_app_id="42";write.effects.status={1,0,2};write.state.achievements["key"]=record();write.storage.update_01eb00(write.state);
    require(write.files.output.at(ini)=="[steam-id]\r\nAchieved=9\r\nCurProgress=8\r\nMaxProgress=5\r\nUnlockTime=11\r\n\r\n","actual writer exact output bytes");
    require(write.effects.events==std::vector<std::string>{"epoch","status","mkdir"},"epoch/status/directory/write sequence");
    Fixture ignored;ignored.state.steam_app_id="42";ignored.effects.status={1,0,2};ignored.files.bad_output=true;ignored.storage.update_01eb00(ignored.state);require(ignored.files.writes.size()==1,"writer false initial-state result ignored");
    Fixture fail;fail.state.steam_app_id="42";fail.state.achievements["a"]=record();fail.state.achievements["b"]=record();fail.state.achievements["b"].timestamp="fail";fail.effects.fail_epoch="fail";
    rejects<std::runtime_error>([&]{fail.storage.update_01eb00(fail.state);},"epoch exception propagates");
    require(fail.state.achievements.at("a").field70==11 && fail.state.achievements.at("b").field70==77 && fail.effects.status_paths.empty() && !fail.paths.ini_path.empty(),"epoch failure retains earlier partial update and computed path");
}
struct Configuration final:achievement_storage::ConfigurationDependencies {
    std::vector<std::pair<std::string,std::string>> environment;
    std::vector<std::string> seen_names;
    explicit Configuration(achievement_storage::Storage& storage):ConfigurationDependencies(storage){}
    Path executable_path_03c6d0()override{return u"C:\\fixture\\host.exe";}
    Path parent_path_019820(const Path& p)override{return path_recovered::parent_path_019820(p);}
    Path join_016b00(const Path& a,const Path& b)override{return join(a,b);}
    Path widen_0145d0(std::string_view s)override{return wide(s);}
    Path filesystem_widen_010540(std::string_view s)override{return wide(s);}
    std::string narrow_path(const Path& p)override{return {p.begin(),p.end()};}
    Path resolve_path_0192d0(std::string_view s,const Path&,bool)override{return wide(s);}
    Path program_data_03c920()override{return u"PROGRAM";}
    Path local_app_data_03c6f0()override{return u"LOCAL";}
    configuration::Node load_document(const Path&)override{throw std::logic_error("old document callback bypassed");}
    void log_config_path(const Path&)override{}
    bool crt_is_space(unsigned char c)override{return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c)override{return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("license effect forbidden");}
    void create_directories_018e00(const Path&)override{}
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng)override{}
    std::vector<Path> directory_entries(const Path&)override{return {};}
    Path filename(const Path& p)override{return p;}
    void remove_04948c(const Path&)override{throw std::logic_error("remove forbidden");}
    bool set_environment_variable(std::string_view key,std::string_view value)override{environment.emplace_back(key,value);return false;}
};
void startup_integration() {
    Fixture f;Configuration deps(f.storage);
    f.files.input[u"C:\\fixture\\anadius.cfg"]=R"("Config2" {"Game" {"Version" "1" "ContentId" "fixture-content" "Language" "tr"} "Emulator" {"SDK" "1.2"} "Achievements" {"AchievementsSet" "fixture-set" "AchievementIDs" "known" "SteamAppId" "42" "SteamIdPrefix" "steam_"}})";
    f.files.input[u"C:\\fixture\\anadius_override.cfg"]=R"("Config2" {"User" {"Username" "override-user"}})";
    const auto xml=join(join(join(u"LOCAL",u"anadius"),u"LSX emu"),u"achievement-fixture-set.xml");
    f.files.input[xml]=R"(<root><entry Id="known" Count="3" Name="from-file" Grant="file-time"/><entry Id="new" Count="2"/></root>)";
    config_startup::initialize_configuration(f.state,deps,f.files);
    require(f.state.username=="override-user" && f.state.sdk=="1,2","real Config2 startup before persistence");
    require(f.files.reads==std::vector<Path>{u"C:\\fixture\\anadius.cfg",u"C:\\fixture\\anadius_override.cfg",xml},"base/override then actual persistence XML read");
    require(f.state.achievements.at("known").field04==3 && f.state.achievements.at("known").display_name=="from-file" && f.state.achievements.at("new").field04==2,"final persistence method delegates to actual storage");
    require(f.state.achievements.at("known").steam_id=="steam_known" && f.state.achievements.at("new").steam_id.empty(),"new XML IDs do not retroactively inherit earlier Steam prefix pass");
    require(f.effects.epochs==std::vector<std::string>{"file-time","0000-00-00T00:00:00"} && deps.environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","fixture-content"},{"EAGameLocale","tr"}},"real storage completes before final environment boundaries");
}
void native_fixture(const std::filesystem::path& dir) {
    std::filesystem::create_directories(dir);
    Fixture f;f.state.user_directory=dir.u16string();f.state.achievements_set="own-fixture";f.state.steam_app_id="42";f.effects.status={1,0,2};
    const auto xml=dir/"achievement-own-fixture.xml";
    {std::ofstream out(xml,std::ios::binary|std::ios::trunc);out<<R"(<root><entry Id="native" Count="4" Progress="2" Name="own-file"/></root>)";require(static_cast<bool>(out),"own XML fixture written");}
    const auto parent=dir/"achievement_watcher"/"42";std::filesystem::create_directories(parent);
    file_recovered::NativeFactory files;achievement_storage::Storage storage(f.paths,f.xml,f.numbers,f.effects,files);storage.update_01eb00(f.state);
    require(f.state.achievements.at("native").field04==4 && f.state.achievements.at("native").display_name=="own-file","actual own XML file parsed and applied");
    std::ifstream in(parent/"achievements.ini",std::ios::binary);const std::string bytes{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
    require(bytes=="[]\r\nAchieved=4\r\nUnlockTime=11\r\n\r\n","actual own INI file bytes through read/storage/writer sources");
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires owned fixture directory");xml_updates();numbers();steam_and_failures();startup_integration();native_fixture(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" achievement storage checks; own fixtures only, no original execution\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
