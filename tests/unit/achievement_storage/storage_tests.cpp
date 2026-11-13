// SPDX-License-Identifier: GPL-3.0-only

#include "storage_fixture.hpp"
#include <limits>

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
    Fixture references;references.paths.xml_path=u"owned-entity-fixture.xml";
    references.files.input[references.paths.xml_path]=R"(<R><A Id="pre&#x100000000;suffix" Count="4" Name="&amp;&#65;&x1f600;"/><A Id="pre&#0;suffix" Count="2"/></R>)";
    references.storage.update_01eb00(references.state);
    require(references.state.achievements.size()==2 && references.state.achievements.at("pre&#x100000000;suffix").field04==4,"oversized reference preserves entire achievement key");
    require(references.state.achievements.at("pre&#0;suffix").field04==2,"NUL reference cannot truncate an achievement key");
    require(references.state.achievements.at("pre&#x100000000;suffix").display_name=="&A&x1f600;","unknown entity after compaction cannot corrupt persisted achievement name");
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
void bounded_numeric_overflow() {
    achievement_storage::HostCrtNumbers host;
    AttributeNode node;
    // Out-of-range scanf conversions are undefined, not a documented way
    // to wrap the result. They now use the existing fallback instead.
    for(const auto* raw:{"2147483648","-2147483649",
                         "9999999999999999999999999999",
                         "-9999999999999999999999999999"}) {
        require(!host.scan_decimal(raw),"out-of-range signed decimal is rejected");
        node.value=raw;
        require(achievement_storage::attribute_003b70(node,"Count",91,host)==91,
                "invalid decimal attribute returns caller fallback");
    }
    for(const auto* raw:{"0x100000000","0xfffffffff",
                         "0xffffffffffffffffffffffff","-0x100000000"}) {
        require(!host.scan_hex(raw),"out-of-range unsigned hex is rejected");
        node.value=raw;
        if(raw[0]!='-')require(
            achievement_storage::attribute_003b70(node,"Count",91,host)==91,
            "invalid hex attribute returns caller fallback");
    }
    require(host.scan_decimal("-2147483648")==0x80000000U &&
            host.scan_decimal("2147483647")==0x7fffffffU,
            "both signed decimal limits remain supported exactly");
    require(host.scan_hex("0xffffffff")==0xffffffffU &&
            host.scan_hex("  -0x10tail")==0xfffffff0U,
            "hexadecimal upper limit and optional negative sign keep low32 bits");
    require(host.scan_hex("+0x10suffix")==16U &&
            host.scan_decimal("  -12tail")==static_cast<std::uint32_t>(-12),
            "optional signs, leading whitespace and trailing content remain compatible");
    require(!host.scan_decimal("  +") && !host.scan_hex("- 0x1"),
            "missing digits and separated sign do not parse");
    for(const auto* raw:{"++1","-+1","+-0","--0",
                         "++0x1","-+0x1","+-0x0","--0x0"})
        require(!host.scan_decimal(raw)&&!host.scan_hex(raw),
                "a numeric prefix accepts at most one optional sign");
    node.value=std::string("13\0ignored",10);
    require(achievement_storage::attribute_003b70(node,"Count",91,host)==13,
            "existing XML C-string boundary ignores bytes after embedded NUL");

    // Exercise the actual XML -> storage -> numeric adapter path. Invalid
    // Count/Progress values must not corrupt fields or change import order.
    Fixture f;
    f.paths.xml_path=u"bounded-numbers.xml";
    f.files.input[f.paths.xml_path]=
        R"(<root><entry Id="overflow" Count="2147483648" Progress="0x100000000"/>
        <entry Id="minimum" Count="-2147483648" Progress="0xffffffff"/>
        <entry Id="normal" Count="12tail" Progress=" 0X10"/></root>)";
    f.storage.update_01eb00(f.state);
    require(f.state.achievements.size()==3,
            "all three XML entries are imported despite invalid numeric fields");
    require(f.state.achievements.at("overflow").field04==0 &&
            f.state.achievements.at("overflow").field08==0,
            "overflow Count and Progress use the established fallback zero");
    require(f.state.achievements.at("minimum").field04==
                std::numeric_limits<std::int32_t>::min() &&
            f.state.achievements.at("minimum").field08==-1,
            "signed decimal minimum and full-width hexadecimal survive XML import");
    require(f.state.achievements.at("normal").field04==12 &&
            f.state.achievements.at("normal").field08==16,
            "ordinary suffixed decimal and prefixed hexadecimal remain unchanged");
}
void achievement_path_components() {
    // A11-09: Config2 sets are filenames and Steam app IDs are a single
    // directory leaf. No original or game files are opened by these fixtures.
    const std::vector<std::string> unsafe{
        ".", "..", "../outside", "..\\..\\outside", "nested/../outside",
        "C:stream", "C:\\victim", "\\\\host\\share", std::string("42\0outside",10),
        "a<b", "a>b", "a\"b", "a|b", "a?b", "a*b",
        std::string("x\n",2),std::string("x\t",2),std::string("x\1z",3)
    };
    for(const auto& value:unsafe)for(const bool steam:{false,true}) {
        Fixture f;
        f.paths.xml_path=u"kept.xml";f.paths.ini_path=u"kept.ini";
        f.state.achievements["retained"]=record();
        if(steam)f.state.steam_app_id=value;
        else f.state.achievements_set=value;
        rejects<std::invalid_argument>([&]{f.storage.update_01eb00(f.state);},
            "path components cannot escape the achievement storage root");
        require(f.paths.xml_path==u"kept.xml"&&f.paths.ini_path==u"kept.ini"&&
                f.files.reads.empty()&&f.files.writes.empty()&&
                f.effects.widened.empty()&&f.effects.status_paths.empty()&&
                f.effects.directories.empty()&&f.effects.epochs.empty()&&
                f.state.achievements.at("retained").field70==77,
            "rejected identifier has zero path, IO, status or achievement side effects");
    }
    // A raw Steam app ID is a Windows directory name. Unlike the prefixed
    // achievement XML filename, it must not alias a device or normalized leaf.
    for(const auto& value:std::vector<std::string>{
            "CON","con.txt","PRN","AUX","NUL","COM1","com9.foo",
            "LPT1","lpt9","42 "," 42","42.","CON .txt",
            "CONIN$","conin$","CONIN$.txt","CONIN$ .txt",
            "CONOUT$","conout$","CONOUT$.txt","CONOUT$ .txt"}) {
        Fixture f;
        f.paths.xml_path=u"kept.xml";f.paths.ini_path=u"kept.ini";
        f.state.steam_app_id=value;f.state.achievements["retained"]=record();
        rejects<std::invalid_argument>([&]{f.storage.update_01eb00(f.state);},
            "Win32 reserved or ambiguous watcher directory must fail closed");
        require(f.paths.xml_path==u"kept.xml"&&f.paths.ini_path==u"kept.ini"&&
                f.files.reads.empty()&&f.files.writes.empty()&&
                f.effects.widened.empty()&&f.effects.status_paths.empty()&&
                f.effects.directories.empty()&&f.effects.epochs.empty()&&
                f.state.achievements.at("retained").field70==77,
            "invalid Win32 watcher leaf has zero persistence and state effects");
    }
    Fixture device_mixed;
    device_mixed.state.achievements_set="legitimate";
    device_mixed.state.steam_app_id="NUL";
    device_mixed.files.input[join(u"USER",u"achievement-legitimate.xml")]=
        R"(<root><record Id="must-not-import" Count="1"/></root>)";
    rejects<std::invalid_argument>([&]{device_mixed.storage.update_01eb00(device_mixed.state);},
        "device-name watcher rejected before otherwise-valid XML import");
    require(device_mixed.files.reads.empty()&&device_mixed.paths.xml_path.empty()&&
            !device_mixed.state.achievements.contains("must-not-import"),
        "invalid watcher leaf cannot publish partial XML state");
    // Positive watcher controls retained from PR #281: matching a prefix is
    // insufficient to classify an otherwise valid leaf as a device name.
    for(const char* ok:{"console","com10","lpt","NULLED","legitimate.v1","COM0","42\x7f",
                        "CONIN","CONOUT","xCONIN$","CONIN$_","CONOUT$_"}) {
        Fixture kept;kept.state.steam_app_id=ok;
        kept.storage.update_01eb00(kept.state);
        require(kept.paths.ini_path==join(join(join(u"USER",u"achievement_watcher"),wide(ok)),u"achievements.ini"),
            "names that only resemble a device stay a single watcher directory");
    }
    // Set IDs are embedded in a filename, not used as raw directory names.
    // Verify actual XML import so a directory-only restriction cannot be
    // accidentally extended to these safe prefixed/suffixed filenames.
    for(const char* set:{"CON","CON.txt","COM1","42.","42 ","CONIN$","CONOUT$","CONIN$.txt"}) {
        Fixture prefixed_device;
        prefixed_device.state.achievements_set=set;
        const auto prefixed_xml=join(u"USER",wide(std::string("achievement-")+set+".xml"));
        prefixed_device.files.input[prefixed_xml]=R"(<root><record Id="preserved" Count="7"/></root>)";
        prefixed_device.storage.update_01eb00(prefixed_device.state);
        require(prefixed_device.paths.xml_path==prefixed_xml&&
                prefixed_device.files.reads==std::vector<Path>{prefixed_xml}&&
                prefixed_device.state.achievements.at("preserved").field04==7,
            "safe prefixed XML filename preserves set identity and imports its record");
    }

    // A bad second component must not partially import a good XML set.
    Fixture mixed;
    mixed.state.achievements_set="legitimate";
    mixed.state.steam_app_id="..\\..\\outside";
    mixed.files.input[join(u"USER",u"achievement-legitimate.xml")]=
        R"(<root><record Id="must-not-load" Count="1"/></root>)";
    rejects<std::invalid_argument>([&]{mixed.storage.update_01eb00(mixed.state);},
        "invalid watcher app id is rejected before otherwise-valid XML import");
    require(mixed.files.reads.empty()&&mixed.paths.xml_path.empty()&&
            !mixed.state.achievements.contains("must-not-load"),
        "both identifier guards run before document and state publication");

    Fixture valid;
    valid.state.achievements_set="legitimate.v1";
    valid.state.steam_app_id="42-test";
    const auto xml=join(u"USER",u"achievement-legitimate.v1.xml");
    const auto ini=join(join(join(u"USER",u"achievement_watcher"),u"42-test"),u"achievements.ini");
    valid.files.input[xml]="<root/>";
    valid.storage.update_01eb00(valid.state);
    require(valid.paths.xml_path==xml&&valid.paths.ini_path==ini&&
            valid.files.reads==std::vector<Path>{xml}&&
            valid.effects.widened==std::vector<std::string>{"achievement-legitimate.v1.xml",
                                                           "achievements.ini","42-test","achievement_watcher"}&&
            valid.effects.status_paths==std::vector<Path>{ini},
        "ordinary identifiers retain exact filename, conversion and status order");
}
void converted_watcher_directories() {
    // Selected file-code-page responses, not a claim of native Win32 execution.
    // Exercise all documented superscript digits via single-byte and UTF-8
    // source strings, with no extension and both device-extension spellings.
    for(const char* prefix:{"COM","lpt"})for(const char16_t digit:{u'\u00b9',u'\u00b2',u'\u00b3'})
        for(const bool utf8:{false,true})for(const char* suffix:{"",".txt"," .txt"}) {
            std::string raw(prefix);if(utf8)raw+='\xc2';raw+=static_cast<char>(digit);raw+=suffix;
            auto decoded=wide(prefix);decoded+=digit;decoded+=wide(suffix);
            Fixture f;f.state.achievements_set="legitimate";f.state.steam_app_id=raw;
            f.paths.xml_path=u"kept.xml";f.paths.ini_path=u"kept.ini";
            f.state.achievements["retained"]=record();
            f.effects.converted_paths.emplace(raw,decoded);
            f.files.input[join(u"USER",u"achievement-legitimate.xml")]=
                R"(<root><record Id="must-not-import" Count="1"/></root>)";
            rejects<std::invalid_argument>([&]{f.storage.update_01eb00(f.state);},
                "decoded COM/LPT superscript device must be rejected");
            require(f.paths.xml_path==u"kept.xml"&&f.paths.ini_path==u"kept.ini"&&
                    f.files.reads.empty()&&f.files.writes.empty()&&
                    f.effects.status_paths.empty()&&f.effects.directories.empty()&&f.effects.epochs.empty()&&
                    !f.state.achievements.contains("must-not-import")&&
                    f.state.achievements.at("retained").field70==77&&
                    f.effects.widened==std::vector<std::string>{"achievement-legitimate.xml","achievements.ini",raw},
                "decoded device preflight preserves conversion order and rejects before IO or publication");
        }
    // The native Win32 observation shows console aliases remain device names
    // with extensions and spaces before an extension, not ordinary directories.
    for(const char* prefix:{"CONIN$","CONOUT$"})for(const char* suffix:{"",".txt"," .txt"}) {
        const auto decoded=wide(std::string(prefix)+suffix);
        Fixture f;f.state.achievements_set="legitimate";f.state.steam_app_id="converted-console";
        f.paths.xml_path=u"kept.xml";f.paths.ini_path=u"kept.ini";
        f.state.achievements["retained"]=record();
        f.effects.converted_paths.emplace(f.state.steam_app_id,decoded);
        f.files.input[join(u"USER",u"achievement-legitimate.xml")]=
            R"(<root><record Id="must-not-import" Count="1"/></root>)";
        rejects<std::invalid_argument>([&]{f.storage.update_01eb00(f.state);},"decoded console alias rejected before XML import");
        require(f.paths.xml_path==u"kept.xml"&&f.paths.ini_path==u"kept.ini"&&
                f.files.reads.empty()&&f.files.writes.empty()&&f.effects.status_paths.empty()&&
                f.effects.directories.empty()&&f.effects.epochs.empty()&&
                !f.state.achievements.contains("must-not-import")&&
                f.state.achievements.at("retained").field70==77&&
                f.effects.widened==std::vector<std::string>{"achievement-legitimate.xml","achievements.ini","converted-console"},
                "decoded console refusal preserves conversion order and every retained side effect");
    }
    const std::vector<std::pair<std::string,Path>> ordinary{
        {std::string("COM")+'\xb0',u"COM\u00b0"},
        {std::string("COM\xc2\xb0"),u"COM\u00b0"},
        // The same raw B9 byte maps to a box-drawing character in a different
        // code page. Do not reject it merely because CP1252 would map it to ¹.
        {std::string("COM")+'\xb9',u"COM\u2563"},
        {std::string("COM")+'\xb9'+"0",u"COM\u00b90"},
        {std::string("COM\xe2\x81\xb4"),u"COM\u2074"},
        {std::string("safe\xe2\x80\xaename"),u"safe\u202ename"}
    };
    for(const auto& [raw,decoded]:ordinary) {
        Fixture f;f.state.achievements_set="CON";f.state.steam_app_id=raw;
        f.effects.converted_paths.emplace(raw,decoded);
        f.files.input[join(u"USER",u"achievement-CON.xml")]=
            R"(<root><record Id="preserved" Count="7"/></root>)";
        f.storage.update_01eb00(f.state);
        require(f.state.achievements.at("preserved").field04==7&&
                f.paths.ini_path==join(join(join(u"USER",u"achievement_watcher"),decoded),u"achievements.ini")&&
                f.effects.widened==std::vector<std::string>{"achievement-CON.xml","achievements.ini",raw,"achievement_watcher"},
            "ordinary decoded names preserve XML import, exact directory and original conversion sequence");
    }
    const std::vector<std::string> conversions{
        "achievement-legitimate.xml","achievements.ini","42","achievement_watcher"};
    for(std::size_t index=0;index<conversions.size();++index) {
        Fixture f;f.state.achievements_set="legitimate";f.state.steam_app_id="42";
        f.paths.xml_path=u"kept.xml";f.paths.ini_path=u"kept.ini";
        f.state.achievements["retained"]=record();f.effects.fail_widen=conversions[index];
        f.files.input[join(u"USER",u"achievement-legitimate.xml")]=
            R"(<root><record Id="must-not-import" Count="1"/></root>)";
        rejects<std::runtime_error>([&]{f.storage.update_01eb00(f.state);},"conversion preflight failure propagates");
        require(f.paths.xml_path==u"kept.xml"&&f.paths.ini_path==u"kept.ini"&&
                f.files.reads.empty()&&f.files.writes.empty()&&f.effects.status_paths.empty()&&
                f.effects.epochs.empty()&&!f.state.achievements.contains("must-not-import")&&
                f.state.achievements.at("retained").field70==77&&
                f.effects.widened==std::vector<std::string>(conversions.begin(),conversions.begin()+index+1),
            "every failed conversion stops before file IO, retained-path updates or achievement import");
    }
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
    // Actual filesystem controls for permitted names; the Effects conversion
    // is selected explicitly so this is independent of the machine ANSI page.
    const std::vector<std::pair<std::string,Path>> permitted{
        {"CONIN",u"CONIN"},{"CONOUT",u"CONOUT"},{"xCONIN$",u"xCONIN$"},
        {"CONIN$_",u"CONIN$_"},{"CONOUT$_",u"CONOUT$_"},
        {"COM\xe2\x81\xb4",u"COM\u2074"},
        {"safe\xe2\x80\xaename",u"safe\u202ename"}};
    for(std::size_t i=0;i<permitted.size();++i) {
        const auto& [raw,decoded]=permitted[i];
        const auto owned=dir/("positive-"+std::to_string(i));
        const auto directory=owned/"achievement_watcher"/std::filesystem::path(decoded);
        std::filesystem::create_directories(directory);
        require(std::filesystem::is_directory(directory),"permitted leaf is an actual owned directory");
        Fixture positive;positive.state.user_directory=owned.u16string();
        positive.state.achievements_set="CONIN$";positive.state.steam_app_id=raw;
        positive.effects.converted_paths.emplace(raw,decoded);positive.effects.status={1,0,2};
        {std::ofstream out(owned/"achievement-CONIN$.xml",std::ios::binary|std::ios::trunc);
         out<<R"(<root><entry Id="native" Count="4"/></root>)";
         require(static_cast<bool>(out),"safely prefixed console XML fixture written");}
        achievement_storage::Storage actual(positive.paths,positive.xml,positive.numbers,positive.effects,files);
        actual.update_01eb00(positive.state);
        require(positive.state.achievements.at("native").field04==4&&
                positive.paths.ini_path==(directory/"achievements.ini").u16string(),
                "permitted leaf and wrapped console set preserve native storage import and path");
        std::ifstream result(directory/"achievements.ini",std::ios::binary);
        const std::string written{std::istreambuf_iterator<char>(result),std::istreambuf_iterator<char>()};
        require(written=="[]\r\nAchieved=4\r\nUnlockTime=11\r\n\r\n",
                "permitted leaf retains exact INI bytes through native filesystem writes");
    }
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires owned fixture directory");xml_updates();numbers();bounded_numeric_overflow();achievement_path_components();converted_watcher_directories();steam_and_failures();native_fixture(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" achievement storage checks; own fixtures only, no original execution\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
