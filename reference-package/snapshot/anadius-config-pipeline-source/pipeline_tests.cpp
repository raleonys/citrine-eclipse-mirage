#include "pipeline.hpp"
#include "../anadius-config-order-source/order.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <optional>

using namespace anadius;
namespace {
int checks=0;
void require(bool ok,const char* reason) {
    ++checks;
    if(!ok)throw std::runtime_error(reason);
}
template<class Exception,class F> void rejects(F action,const char* reason) {
    bool caught=false;
    try {action();} catch(const Exception&) {caught=true;}
    require(caught,reason);
}
struct Spec {
    std::string bytes;
    std::uint32_t initial=0;
    std::optional<std::uint64_t> measured;
    std::size_t read_limit=std::numeric_limits<std::size_t>::max();
    std::string fail;
};
struct MemoryInput final:file_recovered::Input {
    Spec spec;
    std::vector<std::string>& events;
    MemoryInput(Spec s,std::vector<std::string>& e):spec(std::move(s)),events(e) {}
    ~MemoryInput() override {events.emplace_back("destroy");}
    void event(std::string name) {
        events.push_back(name);
        if(spec.fail==name)throw std::runtime_error("synthetic stream failure");
    }
    std::uint32_t initial_state()const override {events.emplace_back("state");return spec.initial;}
    void seek_03b610(std::int64_t offset,file_recovered::Seek way) override {
        require(offset==0,"seek offset");
        event(way==file_recovered::Seek::end?"seek-end":"seek-begin");
    }
    std::uint64_t position_sum_03ba50() override {event("tell");return spec.measured.value_or(spec.bytes.size());}
    std::int32_t peek_043790() override {throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> destination) override {
        event("read");
        const auto count=std::min({destination.size(),spec.bytes.size(),spec.read_limit});
        std::copy_n(spec.bytes.begin(),count,destination.begin());
        return count;
    }
    void close_03ae70() override {event("close");}
};
struct MemoryFactory final:file_recovered::Factory {
    std::map<configuration::Path,Spec> files;
    std::vector<configuration::Path> opened;
    std::vector<std::string> events;
    bool null_result=false;
    std::unique_ptr<file_recovered::Input> open_input_039f70(
        const configuration::Path& path,std::uint32_t mode,std::uint32_t share) override {
        require(mode==0x20 && share==0x40,"original stream open flags");
        opened.push_back(path);events.emplace_back("open");
        if(null_result)return {};
        return std::make_unique<MemoryInput>(files.at(path),events);
    }
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(
        const configuration::Path&,std::uint32_t,std::uint32_t) override {
        throw std::logic_error("document pipeline must not write");
    }
};
struct InvalidOrder final:parser::BinaryIterationOrder {
    std::vector<std::string> order_for(const parser::Node&,const std::vector<std::string>&)const override {
        return {"missing-key"};
    }
};
struct NoOrder final:parser::BinaryIterationOrder {
    std::vector<std::string> order_for(const parser::Node&,const std::vector<std::string>&)const override {
        throw std::logic_error("unselected tree must not request order");
    }
};
configuration::Path wide(std::string_view s) {return {s.begin(),s.end()};}
struct Effects final:config_pipeline::ScalarDependencies {
    std::vector<configuration::Path> logs;
    std::vector<std::string> filename_conversions;
    explicit Effects(const config_pipeline::DocumentPipeline& p):ScalarDependencies(p) {}
    configuration::Path executable_path_03c6d0() override {throw std::logic_error("not invoked");}
    configuration::Path parent_path_019820(const configuration::Path& p) override {
        require(p==u"C:/synthetic/host.exe","caller executable identity");return u"C:/synthetic";
    }
    configuration::Path join_016b00(const configuration::Path& a,const configuration::Path& b) override {return a+u"/"+b;}
    configuration::Path widen_0145d0(std::string_view s) override {return wide(s);}
    configuration::Path filesystem_widen_010540(std::string_view s) override {
        filename_conversions.emplace_back(s);return wide(s);
    }
    std::string narrow_path(const configuration::Path&) override {throw std::logic_error("not invoked");}
    configuration::Path resolve_path_0192d0(std::string_view s,const configuration::Path&,bool relative) override {
        require(relative,"path flag");return wide(s);
    }
    configuration::Path program_data_03c920() override {return u"SYNTHETIC_PROGRAM_DATA";}
    configuration::Path local_app_data_03c6f0() override {throw std::logic_error("not invoked");}
    void log_config_path(const configuration::Path& p) override {logs.push_back(p);}
    bool crt_is_space(unsigned char c) override {return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c) override {return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view) override {throw std::logic_error("license effect forbidden in fixtures");}
    void create_directories_018e00(const configuration::Path&) override {throw std::logic_error("OS effect forbidden");}
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng) override {throw std::logic_error("OS effect forbidden");}
    std::vector<configuration::Path> directory_entries(const configuration::Path&) override {throw std::logic_error("OS effect forbidden");}
    configuration::Path filename(const configuration::Path&) override {throw std::logic_error("not invoked");}
    void remove_04948c(const configuration::Path&) override {throw std::logic_error("OS effect forbidden");}
    void update_achievement_storage_01eb00(configuration::State&) override {throw std::logic_error("OS effect forbidden");}
    bool set_environment_variable(std::string_view,std::string_view) override {throw std::logic_error("OS effect forbidden");}
};
void stream_and_parser() {
    MemoryFactory f;
    config_order::SelectedCopyOrder order;
    config_pipeline::DocumentPipeline pipeline(f,order);
    const configuration::Path path=u"synthetic-input";
    f.files[path].bytes="\xef\xbb\xbf\"Config2\" { \"Game\" { \"Name\" \"first\" \"Name\" \"last\" } }";
    auto document=pipeline.read(path);
    require(document.child("Config2").child("Game").text("Name")=="last","BOM and scalar duplicate last-wins");
    require(f.events==std::vector<std::string>{"open","state","seek-end","tell","seek-begin","read","close","destroy"},"stream order");
    f.files.clear();f.events.clear();
    require(document.child("Config2").child("Game").text("Name")=="last","owned result after parser/input destroyed");
    f.files[path].bytes=R"("Config2" {"Game" {"Name" "discarded"}} "Config2" {"User" {"Username" "retained"}})";
    document=pipeline.read(path);
    require(document.child("Config2").child("Game").children.empty(),"duplicate object replaces subtree");
    require(document.child("Config2").child("User").text("Username")=="retained","last complete object");
    f.files[path].bytes=R"("Config2" {"Game" {"Version" "fresh"}})";
    document=pipeline.read(path);
    require(document.child("Config2").child("User").children.empty(),"fresh parser per call");
    for(const auto& bad:std::vector<std::string>{
        "\xef\xbb\xbf\xef\xbb\xbf\"Config2\" {}",std::string("\xff\xfe",2),
        "\"Config2\" {\"Game\" {\"Name\" \"unterminated",std::string("\0",1)}) {
        f.files[path]=Spec{};f.files[path].bytes=bad;
        rejects<parser::ParseError>([&]{(void)pipeline.read(path);},"raw parse failure propagates, BOM only once");
    }
    f.files[path]=Spec{};f.files[path].bytes="\xef\xbb\xbf?";
    try {(void)pipeline.read(path);require(false,"parse expected");}
    catch(const parser::ParseError& e){require(e.position.byte==0 && e.position.column==1,"error offset after single BOM");}
    f.files[path]=Spec{};f.files[path].bytes=R"("Config2" {"User" {"Username" "raw)";
    f.files[path].bytes.push_back('\0');f.files[path].bytes+="bytes\"}}";
    document=pipeline.read(path);
    require(document.child("Config2").child("User").text("Username")==std::string("raw\0bytes",9),"quoted embedded NUL preserved");
    f.files[path]=Spec{};f.files[path].initial=1;f.files[path].bytes="invalid but never read";
    f.events.clear();document=pipeline.read(path);
    require(document.child("Config2").children.empty(),"initial bad stream becomes empty parser input");
    require(f.events==std::vector<std::string>{"open","state","destroy"},"bad initial stream skips read and explicit close");
    f.files[path]=Spec{};document=pipeline.read(path);
    require(document.child("Config2").children.empty(),"empty file succeeds");
    f.files[path].bytes=R"("Config2" {})";f.files[path].measured=f.files[path].bytes.size()+2;
    rejects<parser::ParseError>([&]{(void)pipeline.read(path);},"short-read zero-tail is not trimmed into valid document");
    for(const auto* fail:{"seek-end","tell","seek-begin","read","close"}) {
        f.files[path]=Spec{};f.files[path].fail=fail;f.events.clear();
        rejects<std::runtime_error>([&]{(void)pipeline.read(path);},"stream exception propagated");
        require(f.events.back()=="destroy","stream destroyed on exception");
        require(std::count(f.events.begin(),f.events.end(),"close")==std::string_view(fail).ends_with("close"),"no extra close on earlier failure");
    }
    f.null_result=true;
    rejects<std::invalid_argument>([&]{(void)pipeline.read(path);},"null stream provider contract rejected");
    f.null_result=false;
    NoOrder no_order;config_pipeline::DocumentPipeline missing(f,no_order);
    f.files[path]=Spec{};f.files[path].bytes=R"("Other" {"a" "1" "b" "2" "c" "3" "d" "4" "e" "5" "f" "6" "g" "7" "h" "8" "i" "9"})";
    require(missing.read(path).child("Config2").children.empty(),"missing Config2 does not convert unrelated tree");
    f.files[path].bytes+=R"( "Config2" {"User" {"Username" "selected"}})";
    require(pipeline.read(path).child("Config2").child("User").text("Username")=="selected","unselected large tree does not request order");
    InvalidOrder invalid;config_pipeline::DocumentPipeline broken(f,invalid);
    rejects<std::logic_error>([&]{(void)broken.read(path);},"invalid provider order rejected");
    f.files[path].bytes=R"("Config2" {"a" "1" "b" "2" "i" "3" "j" "4"})";
    document=pipeline.read(path);
    require(document.child("Config2").iteration_order==std::vector<std::string>{"a","i","b","j"},"first-copy collision order differs from lexical/source order");
}
void application_bridge() {
    MemoryFactory f;config_order::SelectedCopyOrder order;
    config_pipeline::DocumentPipeline pipeline(f,order);Effects effects(pipeline);
    const configuration::Path base=u"C:/synthetic/anadius.cfg",over=u"C:/synthetic/anadius_override.cfg";
    f.files[base].bytes=R"("Config2" {"Game" {"Name" "fixture-game" "Version" "1.2"} "Emulator" {"ServerPort" "4001suffix" "SDK" "a.b"} "User" {"Username" "first-user"}})";
    f.files[over].bytes=R"("Config2" {"Game" {"Name" "ignored-override-name"} "Emulator" {"ServerPort" "random"} "User" {"Username" "second-user"}})";
    configuration::State state{};state.executable_path=u"C:/synthetic/host.exe";
    configuration::load_and_apply(state,false,effects);
    require(state.game_attributes.at("DISPLAY_NAME")=="fixture-game","base parsed scalar applied");
    require(state.game_attributes.at("INSTALLED_VERSION")=="1.2","base version applied");
    require(state.server_port==4001 && state.username=="first-user" && state.sdk=="a.b","base emulator/user parsed scalar applied");
    configuration::load_and_apply(state,true,effects);
    require(state.game_attributes.at("DISPLAY_NAME")=="fixture-game","override preserves base-only game field");
    require(state.server_port==0 && state.username=="second-user","override updates selected scalars");
    require(f.opened==std::vector<configuration::Path>{base,over},"actual base/override filename selection");
    require(effects.logs==f.opened,"one log per successful read before application");
    for(const auto* bad:{"\"Config2\" {", "\xef\xbb\xbf\xef\xbb\xbf"}) {
        f.files[base].bytes=bad;const auto logs=effects.logs.size();
        rejects<parser::ParseError>([&]{configuration::load_and_apply(state,false,effects);},"parse throws before application");
        require(effects.logs.size()==logs && state.username=="second-user","parse failure leaves log/state unchanged");
    }
    for(const auto* collection:{"Content","Entitlements"}) {
        f.files[base].bytes="\"Config2\" {\""+std::string(collection)+"\" {\"item\" {\"Name\" \"fixture\"}}}";
        const auto logs=effects.logs.size();
        rejects<config_pipeline::UnsupportedCollectionOrder>([&]{configuration::load_and_apply(state,false,effects);},"unresolved final collection order explicitly rejected");
        require(effects.logs.size()==logs && state.content.empty() && state.entitlements.empty(),"collection rejection precedes log/apply");
        require(!pipeline.read(base).child("Config2").child(collection).children.empty(),"first-copy inspection still available");
        f.files[over]=f.files[base];
        rejects<config_pipeline::UnsupportedCollectionOrder>([&]{configuration::load_and_apply(state,true,effects);},"conservative bridge bound also applies to override");
    }
    for(const auto* collection:{"AchievementNames","AchievementTotals","SteamIdOverrides"}) {
        f.files[base].bytes="\"Config2\" {\"Achievements\" {\""+std::string(collection)+"\" {\"id\" \"value\"}}}";
        rejects<config_pipeline::UnsupportedCollectionOrder>([&]{configuration::load_and_apply(state,false,effects);},"achievement ordered childmap rejected");
    }
    f.files[base].bytes=R"("Config2" {"User" {"a" "1" "b" "2" "c" "3" "d" "4" "e" "5" "f" "6" "g" "7" "h" "8" "i" "9"}})";
    const auto logs=effects.logs.size();
    rejects<config_order::UnsupportedOrder>([&]{configuration::load_and_apply(state,false,effects);},"growth beyond evidence bound explicitly rejected");
    require(effects.logs.size()==logs,"unsupported order before log");
    f.files[base]=Spec{};f.files[base].fail="read";
    rejects<std::runtime_error>([&]{configuration::load_and_apply(state,false,effects);},"I/O exception through actual application outer path");
    require(effects.logs.size()==logs && state.username=="second-user","I/O failure before log/apply");
    f.files[base]=Spec{};f.files[base].bytes=R"("Config2" {"User" {"Username" "must-not-apply"}})";
    InvalidOrder invalid;config_pipeline::DocumentPipeline broken(f,invalid);Effects broken_effects(broken);
    rejects<std::logic_error>([&]{configuration::load_and_apply(state,false,broken_effects);},"provider order failure through outer application");
    require(broken_effects.logs.empty() && state.username=="second-user","order failure before log/apply");
}
void native_fixture(const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    const auto path=directory/"config-pipeline-generated-fixture.cfg";
    {
        std::ofstream file(path,std::ios::binary|std::ios::trunc);
        file<<"\xef\xbb\xbf\"Config2\" {\"User\" {\"Username\" \"own-generated-fixture\"}}";
        require(static_cast<bool>(file),"own fixture written");
    }
    file_recovered::NativeFactory files;config_order::SelectedCopyOrder order;
    config_pipeline::DocumentPipeline pipeline(files,order);
    const auto document=pipeline.read(path.u16string());
    require(document.child("Config2").child("User").text("Username")=="own-generated-fixture","native compatibility stream -> actual parser/order -> owned result");
}
} // namespace
int main(int argc,char** argv) {
    try {
        if(argc!=2)throw std::invalid_argument("requires own fixture directory argument");
        stream_and_parser();application_bridge();native_fixture(std::filesystem::path(argv[1]));
        std::cout<<"PASS: "<<checks<<" Config2 pipeline checks; generated fixture only; original targets never executed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
