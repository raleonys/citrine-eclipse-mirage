// SPDX-License-Identifier: GPL-3.0-only

#include "storage_fixture.hpp"

namespace {
struct ReturnCodeXml final:messages::XmlBackend {
    xml_reader_bridge::Backend actual;bool force_error=false;
    messages::ParseResult parse_c_string(std::string_view raw)override{
        auto parsed=actual.parse_c_string(raw);parsed.returned_code=99;parsed.has_error=force_error;return parsed;
    }
    std::unique_ptr<messages::XmlPrinter> printer()override{return actual.printer();}
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
struct AttributeNode final:messages::XmlNode {
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
    Fixture ignored;ignored.state.steam_app_id="42";ignored.effects.status={1,0,2};ignored.files.bad_output=true;rejects<std::runtime_error>([&]{ignored.storage.update_01eb00(ignored.state);},"atomic writer failure propagates");require(ignored.files.writes.empty(),"no incomplete INI published");
    Fixture fail;fail.state.steam_app_id="42";fail.state.achievements["a"]=record();fail.state.achievements["b"]=record();fail.state.achievements["b"].timestamp="fail";fail.effects.fail_epoch="fail";
    rejects<std::runtime_error>([&]{fail.storage.update_01eb00(fail.state);},"epoch exception propagates");
    require(fail.state.achievements.at("a").field70==11 && fail.state.achievements.at("b").field70==77 && fail.effects.status_paths.empty() && !fail.paths.ini_path.empty(),"epoch failure retains earlier partial update and computed path");
}
void native_fixture(const std::filesystem::path& dir) {
    std::filesystem::create_directories(dir);
    Fixture f;f.state.user_directory=dir.u16string();f.state.achievements_set="own-fixture";f.state.steam_app_id="42";f.effects.status={1,0,2};
    const auto xml=dir/"achievement-own-fixture.xml";
    {std::ofstream out(xml,std::ios::binary|std::ios::trunc);out<<R"(<root><entry Id="native" Count="4" Progress="2" Name="own-file"/></root>)";require(static_cast<bool>(out),"own XML fixture written");}
    const auto parent=dir/"achievement_watcher"/"42";std::filesystem::create_directories(parent);
    file::NativeFactory files;achievement_storage::Storage storage(f.paths,f.xml,f.numbers,f.effects,files);storage.update_01eb00(f.state);
    require(f.state.achievements.at("native").field04==4 && f.state.achievements.at("native").display_name=="own-file","actual own XML file parsed and applied");
    std::ifstream in(parent/"achievements.ini",std::ios::binary);const std::string bytes{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};
    require(bytes=="[]\r\nAchieved=4\r\nUnlockTime=11\r\n\r\n","actual own INI file bytes through read/storage/writer sources");
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires owned fixture directory");xml_updates();numbers();steam_and_failures();native_fixture(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" achievement storage checks; own fixtures only, no original execution\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
