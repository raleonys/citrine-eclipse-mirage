// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/startup.hpp"
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>

namespace {
using namespace laureondo::startup;
unsigned checks=0;
void require(bool value,const char* why) {
    ++checks;
    if(!value)throw std::runtime_error(why);
}
struct State {
    std::uint8_t language=0,registry=0,lsx=0,show=0,no=0;
    std::map<std::string,std::string,std::less<>> attributes;
    StateReferences refs(){return {language,registry,lsx,show,no,attributes};}
};
// Use the actual Win32 tokenizer, UTF-8 adapter and LocalFree. Supply only the
// command/environment inputs and observe the write, without changing this
// process's environment or opening a console.
struct Input final:InputPlatform {
    NativeInputPlatform native;
    std::wstring command;
    std::optional<std::string> environment;
    void* allocation=nullptr;
    unsigned gets=0,commands=0,parses=0,conversions=0,frees=0,writes=0;
    std::string written;
    Input(std::wstring text,std::optional<std::string> value)
        :command(std::move(text)),environment(std::move(value)){}
    ~Input(){if(allocation)native.local_free(allocation);}
    const char* crt_getenv_05f318(const char* key) override {
        require(std::string_view(key)=="OriginEmuCmdArgs","exact startup environment name");
        ++gets;return environment?environment->c_str():nullptr;
    }
    const wchar_t* get_command_line_w() override {
        ++commands;return command.c_str();
    }
    wchar_t** command_line_to_argv_w(const wchar_t* text,int* count) override {
        require(text==command.c_str()&&!allocation,"tokenizer receives the owned command once");
        ++parses;
        auto result=native.command_line_to_argv_w(text,count);
        allocation=result;
        require(result&&*count>0,"real Win32 command line has an executable argument");
        return result;
    }
    void local_free(void* value) override {
        require(value&&value==allocation,"release the exact Win32 argv allocation");
        ++frees;allocation=nullptr;native.local_free(value);
    }
    int utf8_no_flags(const wchar_t* text,int count,char* output,int capacity) override {
        ++conversions;
        return native.utf8_no_flags(text,count,output,capacity);
    }
    bool set_environment_variable_a(const char* key,const char* value) override {
        require(std::string_view(key)=="OriginEmuCmdArgs","write the filtered startup environment name");
        ++writes;written=value;return false; // The existing source ignores BOOL.
    }
};
struct Expected {
    std::optional<std::string> language;
    bool lsx=false,show=false,no=false;
    std::string retained;
};
void state_is(const State& state,const Input& input,const Expected& expected) {
    require(state.language==expected.language.has_value()&&
            state.registry==expected.language.has_value(),"language argument publishes both flags");
    require(state.lsx==expected.lsx&&state.show==expected.show&&state.no==expected.no,
            "only recognized unconsumed command tokens publish flags");
    require(state.attributes.size()==(expected.language?1u:0u),"no extra game attributes are introduced");
    if(expected.language)
        require(state.attributes.at("INSTALLED_LANGUAGE")==*expected.language,
                "decoded language bytes survive into the actual application state");
    require(input.written==expected.retained,"filtered environment preserves the existing byte format");
    require(input.gets==1&&input.writes==1,"one environment read and one observed write");
}
void command_case(std::wstring_view command,const Expected& expected) {
    // Both missing and empty OriginEmuCmdArgs select native command-line input.
    for(const auto& environment:std::vector<std::optional<std::string>>{std::nullopt,std::string{}}) {
        State state;Input input{std::wstring(command),environment};
        setup_environment_019910(state.refs(),input);
        state_is(state,input,expected);
        require(input.commands==1&&input.parses==1&&input.frees==1&&!input.allocation,
                "native fallback tokenizes once and frees the exact argv owner");
    }
}
void command_lines() {
    command_case(LR"("C:\Program Files\host.exe" -emuGameLang "en US" -showLSX)",
                 {std::string("en US"),true,false,false,"-emuGameLang en US -showLSX"});
    command_case(LR"(host.exe -emuGameLang "" -showConsole)",
                 {std::string{},false,true,false,"-emuGameLang  -showConsole"});
    command_case(L"host.exe -emuGameLang \"tr_\u0130 \u4e2d \U0001f680\" -noConsole",
                 {std::string("tr_\xc4\xb0 \xe4\xb8\xad \xf0\x9f\x9a\x80"),false,false,true,
                  "-emuGameLang tr_\xc4\xb0 \xe4\xb8\xad \xf0\x9f\x9a\x80 -noConsole"});
    command_case(LR"(host.exe -emuGameLang "a\\\"b" -showLSX)",
                 {std::string(R"(a\"b)"),true,false,false,R"(-emuGameLang a\"b -showLSX)"});
    command_case(LR"(host.exe -emuGameLang "C:\folder with space\\")",
                 {std::string(R"(C:\folder with space\)"),false,false,false,
                  R"(-emuGameLang C:\folder with space\)"});
    command_case(L"host.exe\t-emuGameLang\t\"fr FR\"\t-noConsole",
                 {std::string("fr FR"),false,false,true,"-emuGameLang fr FR -noConsole"});
    command_case(LR"(-noConsole -showLSX)",{std::nullopt,true,false,false,"-showLSX"});
    command_case(LR"(host.exe -emuGameLang "first value" -emuGameLang last -showLSXextra)",
                 {std::string("last"),false,false,false,"-emuGameLang first value -emuGameLang last"});
    command_case(LR"(host.exe -emuGameLang fr"_FR" -showConsole)",
                 {std::string("fr_FR"),false,true,false,"-emuGameLang fr_FR -showConsole"});
}
void environment_precedence() {
    {
        State state;Input input{L"host.exe -noConsole","-emuGameLang \"en US\" -showLSX"};
        setup_environment_019910(state.refs(),input);
        state_is(state,input,{std::string("\"en"),true,false,false,"-emuGameLang \"en -showLSX"});
        require(!input.commands&&!input.parses&&!input.conversions&&!input.frees,
                "nonempty environment keeps literal-space parsing and bypasses native argv");
    }
    {
        State state;Input input{L"host.exe -noConsole"," "};
        setup_environment_019910(state.refs(),input);
        state_is(state,input,{std::nullopt,false,false,false,""});
        require(!input.commands&&!input.parses&&!input.conversions&&!input.frees,
                "space-only environment still overrides the command line");
    }
}
}
int main()try {
    command_lines();environment_precedence();
    std::cout<<"PASS: "<<checks<<" real Win32 argv/UTF8 startup checks; controlled environment, no UI\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
