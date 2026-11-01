#include "src/runtime/startup.hpp"
#include <algorithm>
#include <cstdio>
#include <deque>
#include <optional>
#include <stdexcept>
#include <tuple>

using namespace preloader_l::startup;
static unsigned checks;
static void require(bool value) { if (++checks && !value) throw std::runtime_error("check "+std::to_string(checks)); }
struct Fixture {
    std::uint8_t language=0,registry=0,lsx=0,show=0,no=0;
    std::map<std::string,std::string,std::less<>> attributes;
    StateReferences refs() { return {language,registry,lsx,show,no,attributes}; }
};
struct FakeInput final : InputPlatform {
    std::optional<std::string> environment;
    std::vector<std::wstring> wide{L"host.exe"};
    std::vector<wchar_t*> pointers;
    std::vector<std::string> trace;
    std::vector<std::tuple<int,int,bool>> conversions;
    std::deque<int> replies;
    std::string assigned_name,assigned_value;
    bool argv_success=true, fail_free=false;
    const char* crt_getenv_05f318(const char* name) override {
        require(std::string(name)=="OriginEmuCmdArgs"); trace.push_back("getenv");
        return environment ? environment->c_str() : nullptr;
    }
    const wchar_t* get_command_line_w() override { trace.push_back("get-command"); return L"host command"; }
    wchar_t** command_line_to_argv_w(const wchar_t* command,int* count) override {
        require(std::wstring(command)==L"host command"); trace.push_back("argv");
        if (!argv_success) return nullptr;
        pointers.clear(); for (auto& item:wide) pointers.push_back(item.data());
        *count=static_cast<int>(wide.size()); return pointers.data();
    }
    void local_free(void* value) override {
        require(value==pointers.data()); trace.push_back("free");
        if (fail_free) throw std::runtime_error("injected free failure");
    }
    int utf8_no_flags(const wchar_t* input,int count,char* output,int capacity) override {
        trace.push_back("convert"); conversions.emplace_back(count,capacity,output!=nullptr);
        if (!replies.empty()) {
            int result=replies.front(); replies.pop_front(); return result;
        }
        if (output) {
            require(capacity>=count);
            for(int i=0;i<count;++i) { require(input[i]<128);output[i]=static_cast<char>(input[i]); }
        }
        return count;
    }
    bool set_environment_variable_a(const char* name,const char* value) override {
        trace.push_back("setenv"); assigned_name=name; assigned_value=value; return false;
    }
};
struct FakeConsole final : ConsolePlatform {
    bool attached=false;
    std::vector<std::string> trace;
    std::vector<std::tuple<std::uintptr_t,std::string,std::string,std::uintptr_t>> reopened;
    unsigned stream_calls=0;
    bool attach_console(std::uint32_t process) override { require(process==0xffffffffu);trace.push_back("attach");return attached; }
    bool allocate_console() override { trace.push_back("allocate");return false; }
    ConsoleHandle console_window() override { trace.push_back("window");return 0x7654; }
    bool move_window(ConsoleHandle handle,int x,int y,int width,int height,bool repaint) override {
        require(handle==0x7654 && x==100 && y==100 && width==800 && height==500 && repaint);
        trace.push_back("move");return false;
    }
    std::uintptr_t stream_052238(unsigned index) override {
        trace.push_back("stream:"+std::to_string(index));return ++stream_calls*0x100+index;
    }
    int reopen_05f024(std::uintptr_t destination,const char* path,const char* mode,std::uintptr_t stream) override {
        trace.push_back("reopen");reopened.emplace_back(destination,path,mode,stream);return 22;
    }
};

static void filter_tests() {
    {
        Fixture f;
        require(filter_arguments_018480(f.refs(),{}).empty());
        require(f.attributes.empty());
        require(filter_arguments_018480(f.refs(),{"unknown","-SHOWLSX","-showLSXextra","-emuGameLang"}).empty());
        require(f.language==0 && f.registry==0 && f.lsx==0 && f.attributes.empty());
    }
    {
        Fixture f;
        auto text=filter_arguments_018480(f.refs(),{"ignore","-showLSX","tail","-emuGameLang","en_US","-showConsole","-noConsole"});
        require(text=="-showLSX -emuGameLang en_US -showConsole -noConsole");
        require(f.lsx==1 && f.show==1 && f.no==1 && f.language==1 && f.registry==1);
        require(f.attributes.at("INSTALLED_LANGUAGE")=="en_US");
    }
    {
        Fixture f;f.attributes["INSTALLED_LANGUAGE"]="old";
        require(filter_arguments_018480(f.refs(),{"-emuGameLang",""})=="-emuGameLang ");
        require(f.attributes.at("INSTALLED_LANGUAGE")=="old" && f.language==1 && f.registry==1);
        require(filter_arguments_018480(f.refs(),{"-emuGameLang","fr_FR","-emuGameLang","de_DE"})=="-emuGameLang fr_FR -emuGameLang de_DE");
        require(f.attributes.at("INSTALLED_LANGUAGE")=="de_DE");
    }
    {
        Fixture f;
        require(filter_arguments_018480(f.refs(),{"-emuGameLang","-noConsole"})=="-emuGameLang -noConsole");
        require(f.no==0 && f.attributes.at("INSTALLED_LANGUAGE")=="-noConsole");
        require(filter_arguments_018480(f.refs(),{"-emuGameLang",""})=="-emuGameLang ");
        Fixture empty;
        (void)filter_arguments_018480(empty.refs(),{"-emuGameLang",""});
        require(empty.attributes.contains("INSTALLED_LANGUAGE") && empty.attributes.at("INSTALLED_LANGUAGE").empty());
    }
    for(unsigned mask=0;mask<8;++mask) {
        Fixture f;std::vector<std::string> input;
        std::string expected;
        const auto add=[&](const char* value) {input.emplace_back(value);if(!expected.empty())expected+=' ';expected+=value;};
        if(mask&1)add("-showLSX");
        if(mask&2)add("-showConsole");
        if(mask&4)add("-noConsole");
        input.emplace_back("unrecognized");
        require(filter_arguments_018480(f.refs(),input)==expected);
        require(f.lsx==((mask&1)!=0) && f.show==((mask&2)!=0) && f.no==((mask&4)!=0));
        require(f.attributes.empty());
    }
}

static void input_tests() {
    for(auto env:std::vector<std::optional<std::string>>{std::nullopt,std::string{}}) {
        Fixture f;FakeInput platform;platform.environment=env;
        platform.wide={L"-noConsole",L"-emuGameLang",L"a b",L"-showConsole"};
        setup_environment_019910(f.refs(),platform);
        require(platform.assigned_name=="OriginEmuCmdArgs");
        require(platform.assigned_value=="-emuGameLang a b -showConsole");
        require(f.no==0 && f.show==1 && f.attributes.at("INSTALLED_LANGUAGE")=="a b");
        require(platform.trace.front()=="getenv" && platform.trace.back()=="setenv");
        require(std::count(platform.trace.begin(),platform.trace.end(),"free")==1);
    }
    {
        Fixture f;FakeInput p;p.environment="-emuGameLang  en_US -showLSX\t-noConsole -showConsole";
        f.attributes["INSTALLED_LANGUAGE"]="old";
        setup_environment_019910(f.refs(),p);
        require(p.assigned_value=="-emuGameLang  -showConsole");
        require(f.attributes.at("INSTALLED_LANGUAGE")=="old" && f.lsx==0 && f.no==0 && f.show==1);
        require(p.trace==std::vector<std::string>{"getenv","setenv"});
    }
    {
        Fixture f;FakeInput p;p.environment="-emuGameLang \"en US\" -showLSX";
        setup_environment_019910(f.refs(),p);
        require(p.assigned_value=="-emuGameLang \"en -showLSX");
        require(f.attributes.at("INSTALLED_LANGUAGE")=="\"en" && f.lsx==1);
    }
    {
        Fixture f;FakeInput p;p.environment=" ";
        p.wide={L"host.exe",L"-showLSX"};
        setup_environment_019910(f.refs(),p);
        require(p.assigned_value.empty() && f.lsx==0);
        require(p.trace==std::vector<std::string>{"getenv","setenv"});
    }
    {
        Fixture f;FakeInput p;p.argv_success=false;
        setup_environment_019910(f.refs(),p);
        require(p.assigned_value.empty() && f.attributes.empty());
        require(p.trace==std::vector<std::string>{"getenv","get-command","argv","setenv"});
    }
    {
        FakeInput p;
        require(narrow_argument_0453d0(p,L"").empty() && p.conversions.empty());
        p.replies={3,0};
        auto result=narrow_argument_0453d0(p,L"abc");
        require(result==std::string(3,'\0'));
        require(p.conversions.size()==2 && std::get<1>(p.conversions[1])==3);
        p.replies={0,0};
        require(narrow_argument_0453d0(p,L"abc").empty());
        require(p.conversions.size()==4 && std::get<1>(p.conversions[3])==0);
    }
}

static void console_tests() {
    for(bool no:{false,true}) for(bool show:{false,true}) for(bool attached:{false,true}) {
        Fixture f;FakeConsole p;f.no=no;f.show=show;p.attached=attached;
        setup_console_038760(f.refs(),p);
        if(no) {require(p.trace.empty());continue;}
        if(!attached && !show) {require(p.trace==std::vector<std::string>{"attach"});continue;}
        if(attached) {
            require(p.trace==std::vector<std::string>{"attach","stream:2","stream:2","reopen"});
            require(p.reopened.size()==1);
            require(p.reopened[0]==std::tuple<std::uintptr_t,std::string,std::string,std::uintptr_t>{0x202,"conout$","w",0x102});
        } else {
            require(p.trace==std::vector<std::string>{"attach","allocate","window","move","stream:0","stream:0","reopen","stream:1","stream:1","reopen","stream:2","stream:2","reopen"});
            require(p.reopened.size()==3);
            require(p.reopened[0]==std::tuple<std::uintptr_t,std::string,std::string,std::uintptr_t>{0x200,"conin$","r",0x100});
            require(p.reopened[1]==std::tuple<std::uintptr_t,std::string,std::string,std::uintptr_t>{0x401,"conout$","w",0x301});
            require(p.reopened[2]==std::tuple<std::uintptr_t,std::string,std::string,std::uintptr_t>{0x602,"conout$","w",0x502});
        }
    }
}
int main() {filter_tests();input_tests();console_tests();std::printf("PASS: %u checks; synthetic input/console only; no real environment writes or UI\n",checks);}
