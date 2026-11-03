// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/memory_printer.hpp"
#include <algorithm>
#include <cstdio>
#include <map>
#include <stdexcept>
using namespace preloader_l::xml;
static unsigned checks;
static void require(bool v){++checks;if(!v)throw std::runtime_error("check "+std::to_string(checks));}
static std::string terminated(std::string value){value.push_back('\0');return value;}
struct Reader final : AttributeReader {
    std::map<std::string,std::string,std::less<>> values;
    const char* attribute(preloader_l::handlers::Request request,const char* key) override {
        require(request==77);auto it=values.find(key);return it==values.end()?nullptr:it->second.c_str();
    }
};
struct Backend final : preloader_l::messages::XmlBackend {
    preloader_l::messages::ParseResult parse_c_string(std::string_view) override {
        throw std::logic_error("Synthetic printer test has no XML reader");
    }
    std::unique_ptr<preloader_l::messages::XmlPrinter> printer() override {
        return std::make_unique<MemoryPrinter>();
    }
};
struct Random final : preloader_l::messages::Random {
    preloader_l::messages::Key challenge_bytes_034cf0() override {return {};}
    preloader_l::messages::Key key_from_response_034c30(std::string_view) override {throw std::logic_error("unused");}
};

static void shapes() {
    MemoryPrinter empty;require(empty.buffer_including_terminator()==std::string(1,'\0'));
    MemoryPrinter p;p.open("Root");p.attribute("key","a&<>\"'");p.close();
    require(p.buffer_including_terminator()==terminated("<Root key=\"a&amp;&lt;&gt;&quot;&apos;\"/>\n"));
    require(p.depth()==0);
    MemoryPrinter tree;tree.open("R");tree.open("A");tree.close();tree.open("B");tree.push_text("t<&>\"'");tree.close();tree.close();
    require(tree.bytes_without_terminator()=="<R>\n    <A/>\n    <B>t&lt;&amp;&gt;\"'</B>\n</R>\n");
    MemoryPrinter mixed;mixed.open("R");mixed.push_text("x");mixed.open("Child");mixed.close();mixed.push_text("y");mixed.close();
    require(mixed.bytes_without_terminator()=="<R>x<Child/>y</R>\n");
    MemoryPrinter roots;roots.open("R");roots.close();roots.open("S");roots.close();
    require(roots.bytes_without_terminator()=="<R/>\n\n<S/>\n");
}
static void byte_semantics() {
    MemoryPrinter p;p.open(std::string_view("R\0bad",5));p.attribute(std::string_view("k\0bad",5),std::string_view("v\0bad",5));p.push_text(std::string_view("t\0bad",5));p.close();
    require(p.bytes_without_terminator()=="<R k=\"v\">t</R>\n");
    MemoryPrinter raw;raw.open("R");raw.attribute("x",std::string("\xff\r\n",3));raw.push_text("&amp;");raw.close();
    require(raw.bytes_without_terminator()==std::string("<R x=\"\xff\r\n\">&amp;amp;</R>\n"));
    MemoryPrinter cdata;cdata.open("R");cdata.push_text("a]]>b<&",true);cdata.close();
    require(cdata.bytes_without_terminator()=="<R><![CDATA[a]]>b<&]]></R>\n");
    MemoryPrinter outside;outside.attribute("k","v");
    require(outside.bytes_without_terminator()==" k=\"v\""); // original does not validate nesting here
    MemoryPrinter compact;compact.open_element("R",true);compact.open_element("C",true);compact.close_element(true);compact.close_element(true);
    require(compact.bytes_without_terminator()=="<R><C/></R>");
    MemoryPrinter mixed_flags;mixed_flags.open_element("R",true);mixed_flags.open_element("C",false);mixed_flags.close_element(false);mixed_flags.close_element(false);
    require(mixed_flags.bytes_without_terminator()=="<R>    <C/>\n</R>\n");
    for(unsigned byte=1;byte<256;++byte) {
        MemoryPrinter one;one.open("R");one.push_text(std::string(1,static_cast<char>(byte)));one.close();
        const std::string expected=byte=='&'?"&amp;":byte=='<'?"&lt;":byte=='>'?"&gt;":std::string(1,static_cast<char>(byte));
        require(one.bytes_without_terminator()=="<R>"+expected+"</R>\n");
    }
}
static void actual_handler_and_greeting() {
    using namespace preloader_l::handlers;
    Attributes game,settings;Profile profile{};std::uint8_t online=0,pretend=7;
    StateReferences state{game,settings,profile,online,pretend};
    Reader reader;MemoryPrinter connected;HandlerWriter connected_writer(connected,reader);
    require(get_connected_02f2d0(connected_writer,state));
    require(connected.bytes_without_terminator()=="<InternetConnectedState connected=\"1\"/>\n");
    settings["A&B"]="<value>\"'";reader.values["SettingId"]="A&B";
    MemoryPrinter setting;HandlerWriter setting_writer(setting,reader);
    require(get_setting_02f7c0(77,setting_writer,state));
    require(setting.bytes_without_terminator()=="<GetSettingResponse Setting=\"&lt;value&gt;&quot;&apos;\"/>\n");
    MemoryPrinter config;HandlerWriter config_writer(config,reader);
    require(get_config_02e5f0(config_writer));
    const auto bytes=config.bytes_without_terminator();
    require(bytes.starts_with("<GetConfigResponse>\n    <Service Name=\"EbisuSDK\" Facility=\"SDK\"/>\n"));
    require(bytes.ends_with("    <Service Name=\"EbisuSDK\" Facility=\"CONTENT\"/>\n</GetConfigResponse>\n"));
    require(std::count(bytes.begin(),bytes.end(),'\n')==36);
    Backend backend;Random random;
    const auto greeting=preloader_l::messages::initial_message_02d540(backend,random,"sdk<&");
    require(greeting==terminated("<LSX>\n    <Event sender=\"EALS\">\n        <Challenge version=\"sdk&lt;&amp;\" build=\"release\" key=\"00000000000000000000000000000000\"/>\n    </Event>\n</LSX>\n"));
}
int main(){shapes();byte_semantics();actual_handler_and_greeting();std::printf("PASS: %u checks; concrete recovered serializer; synthetic readers/random only\n",checks);}
