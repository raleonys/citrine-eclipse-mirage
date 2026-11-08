// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/platform/hook_host_callbacks.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace citrine_eclipse_mirage::host_callbacks;
namespace {
unsigned checks;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("host callback check failed"); }
struct ProbeException {};
template<class Char> int synthetic_compare(const Char* left, const Char* right) {
    // Fixture policy only. Production comparison remains a mandatory CRT call.
    while (*left && *right) {
        auto lower = [](Char c) { return c >= Char('A') && c <= Char('Z') ? Char(c + ('a' - 'A')) : c; };
        const auto a = lower(*left++), b = lower(*right++);
        if (a != b) return a < b ? -7 : 9;
    }
    return *left == *right ? 0 : (*left ? 9 : -7);
}
template<class Char> bool same_process(const ProcessCall<Char>& a, const ProcessCall<Char>& b) {
    return a.application == b.application && a.command_line == b.command_line &&
        a.process_attributes == b.process_attributes && a.thread_attributes == b.thread_attributes &&
        a.inherit_handles == b.inherit_handles && a.creation_flags == b.creation_flags &&
        a.environment == b.environment && a.current_directory == b.current_directory &&
        a.startup_info == b.startup_info && a.process_information == b.process_information;
}
template<class Char> bool same_folder(const FolderCall<Char>& a, const FolderCall<Char>& b) {
    return a.window == b.window && a.folder_id == b.folder_id && a.token == b.token &&
        a.flags == b.flags && a.path == b.path;
}
struct Mock final : Effects {
    unsigned converts=0, compares_a=0, compares_w=0, processes_a=0, processes_w=0;
    unsigned folders_a=0, folders_w=0, copies_a=0, copies_w=0;
    std::int32_t process_result=-17;
    std::uint32_t folder_result=0x80070005;
    int copy_error=0, override_compare=42;
    bool throw_conversion=false, throw_compare=false, throw_copy=false, throw_original=false;
    std::vector<std::string> events;
    std::u16string converted;
    std::string source_a;
    std::u16string source_w;
    std::size_t capacity=0;
    ProcessCall<char> last_a{};
    ProcessCall<char16_t> last_w{};
    FolderCall<char> last_folder_a{};
    FolderCall<char16_t> last_folder_w{};
    std::string filesystem_narrow(std::u16string_view in) override {
        ++converts; events.push_back("convert"); converted=in;
        if (throw_conversion) throw ProbeException{};
        std::string out; for (auto c:in) out.push_back(c < 128 ? static_cast<char>(c) : '?'); return out;
    }
    int crt_stricmp(const char* a,const char* b) override {
        ++compares_a; events.push_back("compare-a"); source_a=b;
        if (throw_compare) throw ProbeException{};
        return override_compare == 42 ? synthetic_compare(a,b) : override_compare;
    }
    int crt_wcsicmp(const char16_t* a,const char16_t* b) override {
        ++compares_w; events.push_back("compare-w"); source_w=b;
        if (throw_compare) throw ProbeException{};
        return override_compare == 42 ? synthetic_compare(a,b) : override_compare;
    }
    template<class Char> int copy(Char* destination,std::size_t cap,const Char* source) {
        capacity=cap;
        if (throw_copy) throw ProbeException{};
        if (copy_error) return copy_error;
        if (!destination) return 22; // Explicit synthetic invalid-parameter policy.
        const auto length=std::char_traits<Char>::length(source);
        if(length>=cap) { destination[0]=0; return 34; }
        std::copy_n(source,length+1,destination); return 0;
    }
    int crt_strcpy_s(char* destination,std::size_t cap,const char* source) override {
        ++copies_a; events.push_back("copy-a"); source_a=source; return copy(destination,cap,source);
    }
    int crt_wcscpy_s(char16_t* destination,std::size_t cap,const char16_t* source) override {
        ++copies_w; events.push_back("copy-w"); source_w=source; return copy(destination,cap,source);
    }
    std::int32_t original_create_process_a(const ProcessCall<char>& call) override {
        ++processes_a; events.push_back("process-a"); last_a=call;
        if (throw_original) throw ProbeException{}; return process_result;
    }
    std::int32_t original_create_process_w(const ProcessCall<char16_t>& call) override {
        ++processes_w; events.push_back("process-w"); last_w=call;
        if (throw_original) throw ProbeException{}; return process_result;
    }
    std::uint32_t original_folder_a(const FolderCall<char>& call) override {
        ++folders_a; events.push_back("folder-a"); last_folder_a=call;
        if (throw_original) throw ProbeException{}; return folder_result;
    }
    std::uint32_t original_folder_w(const FolderCall<char16_t>& call) override {
        ++folders_w; events.push_back("folder-w"); last_folder_w=call;
        if (throw_original) throw ProbeException{}; return folder_result;
    }
};
template<class Function> void throws_probe(Function fn) {
    bool caught=false; try { fn(); } catch(const ProbeException&) { caught=true; } check(caught);
}
void process_tests() {
    Values values{{"ClientPath",u"C:\\Fixture\\Client.EXE"}};
    const auto before=values;
    char command[]="C:\\Fixture\\Client.EXE --arg";
    char16_t command_w[]=u"C:\\Fixture\\Client.EXE --arg";
    ProcessCall<char> a{"c:\\fixture\\client.exe",command,0x101,0x202,-2,0xfedcba98,0x303,"D:\\",0x404,0x505};
    ProcessCall<char16_t> w{u"c:\\fixture\\client.exe",command_w,0x111,0x222,-3,0xabcdef01,0x333,u"E:\\",0x444,0x555};
    Mock mock;
    check(create_process_a_00f3f0(values,mock,a)==0);
    check(mock.processes_a==0 && mock.compares_a==1 && mock.converts==1);
    check(mock.events==std::vector<std::string>{"convert","compare-a"});
    check(std::string(command)=="C:\\Fixture\\Client.EXE --arg");
    check(create_process_w_00f4d0(values,mock,w)==0);
    check(mock.processes_w==0 && mock.compares_w==1 && mock.converts==1);
    check(values==before);
    a.application="different.exe"; w.application=u"different.exe";
    for(auto result:{std::int32_t{0},std::int32_t{1},std::int32_t{-1},std::numeric_limits<std::int32_t>::min(),std::int32_t{0x12345678}}) {
        Mock original; original.process_result=result;
        check(create_process_a_00f3f0(values,original,a)==result);
        check(same_process(original.last_a,a) && original.processes_a==1);
        check(original.events==std::vector<std::string>{"convert","compare-a","process-a"});
        check(create_process_w_00f4d0(values,original,w)==result);
        check(same_process(original.last_w,w) && original.processes_w==1);
    }
    a.application=nullptr; w.application=nullptr;
    Mock absent; absent.throw_conversion=true; absent.throw_compare=true;
    check(create_process_a_00f3f0(values,absent,a)==absent.process_result);
    check(create_process_w_00f4d0(values,absent,w)==absent.process_result);
    check(absent.converts==0 && absent.compares_a==0 && absent.compares_w==0);
    check(same_process(absent.last_a,a) && same_process(absent.last_w,w));
    a.application=""; w.application=u"";
    Mock empty;
    check(create_process_a_00f3f0({},empty,a)==0);
    check(create_process_w_00f4d0({},empty,w)==0);
    check(empty.processes_a==0 && empty.processes_w==0 && empty.converts==0);
    // Explicit mapped length survives conversion; comparison is C-string based.
    Values nul{{"ClientPath",std::u16string(u"same\0ignored",12)}};
    a.application="same"; w.application=u"same";
    Mock embedded;
    check(create_process_a_00f3f0(nul,embedded,a)==0);
    check(embedded.converted.size()==12 && embedded.converted[4]==0);
    check(create_process_w_00f4d0(nul,embedded,w)==0);
    // Nonzero CRT comparison magnitudes are not confused with equality.
    for (int comparison:{-512,-1,1,512}) {
        Mock forced; forced.override_compare=comparison;
        check(create_process_a_00f3f0(values,forced,a)==forced.process_result);
        check(create_process_w_00f4d0(values,forced,w)==forced.process_result);
    }
    Mock conversion; conversion.throw_conversion=true;
    throws_probe([&]{create_process_a_00f3f0(values,conversion,a);}); check(conversion.processes_a==0);
    Mock comparison; comparison.throw_compare=true;
    throws_probe([&]{create_process_w_00f4d0(values,comparison,w);}); check(comparison.processes_w==0);
    Mock original; original.throw_original=true;
    throws_probe([&]{create_process_a_00f3f0(values,original,a);}); check(original.processes_a==1);
}
void folder_tests() {
    Values values{{"Documents",u"C:\\Synthetic\\Documents"}};
    std::array<char,260> a{}; std::array<char16_t,260> w{};
    FolderCall<char> ac{0x101,5,0x202,0xfedcba98,a.data()};
    FolderCall<char16_t> wc{0x303,5,0x404,0xabcdef01,w.data()};
    Mock mock;
    check(folder_a_00fc00(values,mock,ac)==0 && mock.folders_a==0);
    check(mock.capacity==260 && mock.copies_a==1 && mock.converts==1);
    check(std::string(a.data())=="C:\\Synthetic\\Documents");
    check(mock.events==std::vector<std::string>{"convert","copy-a"});
    check(folder_w_00fc80(values,mock,wc)==0 && mock.folders_w==0);
    check(mock.capacity==260 && mock.copies_w==1 && mock.converts==1);
    check(std::u16string(w.data())==u"C:\\Synthetic\\Documents");
    for(auto id:{std::int32_t{-1},std::int32_t{0},std::int32_t{4},std::int32_t{6},std::int32_t{0x8005},std::int32_t{0x4005},std::numeric_limits<std::int32_t>::min()}) {
        ac.folder_id=id; wc.folder_id=id; Mock fallback; fallback.throw_conversion=true;
        check(folder_a_00fc00(values,fallback,ac)==fallback.folder_result);
        check(folder_w_00fc80(values,fallback,wc)==fallback.folder_result);
        check(fallback.copies_a==0 && fallback.copies_w==0 && fallback.converts==0);
        check(same_folder(ac,fallback.last_folder_a) && same_folder(wc,fallback.last_folder_w));
    }
    ac.folder_id=5; wc.folder_id=5;
    for(const auto& mapping:std::vector<Values>{{},{ {"documents",u"wrong case"} },{{"Documents",u""}}}) {
        Mock fallback;
        check(folder_a_00fc00(mapping,fallback,ac)==fallback.folder_result);
        check(folder_w_00fc80(mapping,fallback,wc)==fallback.folder_result);
        check(fallback.folders_a==1 && fallback.folders_w==1 && fallback.copies_a==0 && fallback.copies_w==0);
        check(fallback.converts==(mapping.contains("Documents")?1u:0u));
    }
    Values leading_nul{{"Documents",std::u16string(u"\0suffix",7)}};
    Mock nul;
    a.fill('X');w.fill(u'X');
    check(folder_a_00fc00(leading_nul,nul,ac)==0 && nul.copies_a==1 && nul.folders_a==0);
    check(a[0]==0 && a[1]=='X' && nul.converted.size()==7);
    check(folder_w_00fc80(leading_nul,nul,wc)==0 && nul.copies_w==1 && nul.folders_w==0);
    check(w[0]==0 && w[1]==u'X');
    // Capacity is a count of destination character units, not byte count.
    for(std::size_t length:{std::size_t{1},std::size_t{258},std::size_t{259},std::size_t{260},std::size_t{261},std::size_t{520}}) {
        Values long_value{{"Documents",std::u16string(length,u'q')}};
        Mock copy;
        a.fill('X');w.fill(u'X');
        const auto expected=static_cast<std::uint32_t>(length>=260);
        check(folder_a_00fc00(long_value,copy,ac)==expected);
        check(copy.capacity==260 && copy.folders_a==0 && copy.copies_a==1);
        check(a[length<260?length:0]==0);
        check(folder_w_00fc80(long_value,copy,wc)==expected);
        check(copy.capacity==260 && copy.folders_w==0 && copy.copies_w==1);
        check(w[length<260?length:0]==0);
    }
    for(int error:{-1,1,22,34,std::numeric_limits<int>::min()}) {
        Mock copy; copy.copy_error=error;
        check(folder_a_00fc00(values,copy,ac)==1);
        check(folder_w_00fc80(values,copy,wc)==1);
        check(copy.folders_a==0 && copy.folders_w==0);
    }
    std::uint32_t result=0xaabbccdd;
    Mock untouched;
    check(!folder_value_a_008db0(values,untouched,0,a.data(),&result)); check(result==0xaabbccdd);
    check(!folder_value_w_008fa0({},untouched,5,w.data(),&result)); check(result==0xaabbccdd);
    ac.path=nullptr; wc.path=nullptr;
    Mock null;
    check(folder_a_00fc00(values,null,ac)==1 && null.copies_a==1 && null.folders_a==0);
    check(folder_w_00fc80(values,null,wc)==1 && null.copies_w==1 && null.folders_w==0);
    Mock throw_copy; throw_copy.throw_copy=true;
    throws_probe([&]{folder_a_00fc00(values,throw_copy,ac);});check(throw_copy.folders_a==0);
    Mock throw_conversion; throw_conversion.throw_conversion=true;
    throws_probe([&]{folder_a_00fc00(values,throw_conversion,ac);});check(throw_conversion.copies_a==0);
    Mock throw_original; throw_original.throw_original=true;
    throws_probe([&]{folder_w_00fc80({},throw_original,wc);});check(throw_original.folders_w==1);
}
struct HostMock final : citrine_eclipse_mirage::host_init::Effects {
    std::int32_t initialize_status=0;
    std::vector<citrine_eclipse_mirage::host_init::Callback> installed;
    std::u16string filesystem_widen_006d70(std::string_view text) override {
        return {text.begin(),text.end()}; // Synthetic ASCII configuration only.
    }
    std::uint64_t version_condition_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t condition) override {
        return mask+type+condition;
    }
    bool verify_version(const citrine_eclipse_mirage::host_init::VersionRequest&,std::uint32_t,std::uint64_t) override { return true; }
    std::int32_t initialize_engine_046a20() override { return initialize_status; }
    Address get_module_handle(std::u16string_view) override { return 0x100; }
    Address get_proc_address(Address,std::string_view) override { return 0x200+installed.size(); }
    std::int32_t create_hook_0466e0(Address,citrine_eclipse_mirage::host_init::Callback callback,Address& original) override {
        installed.push_back(callback);original=0x300+installed.size();return 0;
    }
    std::int32_t enable_hook_046a10(Address) override { return 0; }
    void log_text_006880(std::string_view) override {}
    void log_integer_00af90(std::int32_t) override {}
    void finish_log_0088e0() override {}
};
void host_integration_tests() {
    namespace host=citrine_eclipse_mirage::host_init;
    // Run actual initialization, actual Windows lexical paths, shared map helper
    // implementations, and actual callback decisions. Only OS/CRT edges are mocks.
    citrine_eclipse_mirage::configuration::State configuration{};
    configuration.executable_path=u"C:\\Synthetic\\Game.exe";
    configuration.documents=u"D:\\Synthetic\\Saved";
    configuration.language_registry_key=u"Software\\Synthetic\\Locale";
    configuration.game_attributes["INSTALLED_LANGUAGE"]="fixture-language";
    configuration.registry_spoof=1;
    configuration.block_reruns=1;
    Values values{{"stale",u"discard"},{"Documents",u"old"}};
    std::vector<Address> enabled{0x99};
    std::array<Address,host::callback_count> originals{};
    HostMock host_effects;
    host::initialize_host_00e310({configuration,values,enabled,originals},host_effects);
    check(!values.contains("stale") && values.at("ClientPath")==configuration.executable_path);
    check(values.at("Documents")==configuration.documents);
    check(values.at("LanguageKey")==u"Software\\Synthetic" && values.at("LanguageValue")==u"Locale");
    check(values.at("Language")==u"fixture-language");
    check(enabled.size()==host::callback_count && host_effects.installed.size()==host::callback_count);
    check(host_effects.installed[6]==host::Callback::create_process_a_00f3f0 &&
          host_effects.installed[7]==host::Callback::create_process_w_00f4d0);
    check(host_effects.installed[8]==host::Callback::folder_a_00fc00 &&
          host_effects.installed[9]==host::Callback::folder_w_00fc80);
    check(host_effects.installed[10]==host::Callback::crypt_query_001264 && originals[10]==0x30b);
    check(originals[6]==0x307 && originals[9]==0x30a);
    ProcessCall<char> a{"c:\\synthetic\\game.exe",nullptr,1,2,0,3,4,nullptr,5,6};
    ProcessCall<char16_t> w{u"C:\\SYNTHETIC\\GAME.EXE",nullptr,7,8,0,9,10,nullptr,11,12};
    Mock callbacks;
    check(create_process_a_00f3f0(values,callbacks,a)==0);
    check(create_process_w_00f4d0(values,callbacks,w)==0);
    check(callbacks.processes_a==0 && callbacks.processes_w==0);
    std::array<char,260> path_a{};std::array<char16_t,260> path_w{};
    FolderCall<char> fa{0,5,0,0,path_a.data()};
    FolderCall<char16_t> fw{0,5,0,0,path_w.data()};
    check(folder_a_00fc00(values,callbacks,fa)==0);
    check(folder_w_00fc80(values,callbacks,fw)==0);
    check(std::string(path_a.data())=="D:\\Synthetic\\Saved" &&
          std::u16string(path_w.data())==configuration.documents);
    // A second actual initialization drops stale overrides when configuration
    // removes them. Manually calling a callback doesn't imply it was installed.
    configuration.documents.clear();configuration.block_reruns=0;
    configuration.language_registry_key.clear();host_effects.installed.clear();
    host::initialize_host_00e310({configuration,values,enabled,originals},host_effects);
    check(!values.contains("Documents") && !values.contains("Language"));
    check(configuration.registry_spoof==0 && host_effects.installed.size()==7 && enabled.size()==7);
    check(host_effects.installed.back()==host::Callback::crypt_query_001264);
    Mock fallback;
    check(folder_a_00fc00(values,fallback,fa)==fallback.folder_result);
    check(folder_w_00fc80(values,fallback,fw)==fallback.folder_result);
    check(fallback.folders_a==1 && fallback.folders_w==1);
    // Initialization failure occurs before Documents insertion and hooks.
    configuration.documents=u"F:\\Synthetic\\Other";host_effects.initialize_status=1;
    host_effects.installed.clear();
    host::initialize_host_00e310({configuration,values,enabled,originals},host_effects);
    check(values.at("ClientPath")==configuration.executable_path && !values.contains("Documents"));
    check(enabled.empty() && host_effects.installed.empty());
    check(folder_w_00fc80(values,fallback,fw)==fallback.folder_result && fallback.folders_w==2);
}
}
int main() {
    try { process_tests();folder_tests();host_integration_tests();std::cout<<"host callbacks: "<<checks<<" checks passed (synthetic mocks only)\n"; }
    catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}
