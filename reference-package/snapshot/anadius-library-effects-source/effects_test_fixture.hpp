#pragma once
#include "effects.hpp"
#include "../anadius-config-logging-source/logging_test_fixture.hpp"
#include <functional>
#include <map>
#include <system_error>

namespace b12_test {
using namespace anadius;namespace le=library_effects;namespace lib=recovered::libraries;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct Conversion {UINT cp;DWORD flags;std::string input;int capacity;bool destination;};
struct Message {HWND window;std::wstring text,title;UINT flags;};
struct Ui {
    std::vector<Conversion> conversions;std::vector<Message> messages;
    std::function<int(UINT,DWORD,LPCCH,int,LPWSTR,int)> hook;
    int result=IDOK;
};
inline thread_local Ui* ui=nullptr;
inline int WINAPI convert(UINT cp,DWORD flags,LPCCH p,int count,LPWSTR out,int capacity){
    ui->conversions.push_back({cp,flags,std::string(p,p+count),capacity,out!=nullptr});
    return ui->hook?ui->hook(cp,flags,p,count,out,capacity) : ::MultiByteToWideChar(cp,flags,p,count,out,capacity);
}
inline int WINAPI message(HWND window,LPCWSTR text,LPCWSTR title,UINT flags){
    ui->messages.push_back({window,std::wstring(text),std::wstring(title),flags});return ui->result;
}
inline le::DialogApi dialog_api(){return {convert,message};}
struct PhaseState {std::vector<int> waits;unsigned finishes=0,other_calls=0;};
class Phase final:public lib::Effects {
    std::shared_ptr<PhaseState> state_;
    [[noreturn]] void wrong(){++state_->other_calls;throw std::logic_error("selected library policy reached lower unexpectedly");}
public:
    explicit Phase(std::shared_ptr<PhaseState> state):state_(std::move(state)){}
    void begin_load_log_0863e0()override{wrong();}
    void append_log_path_and_flush(std::string_view)override{wrong();}
    void message_038bc0(std::string,std::string,std::uint32_t)override{wrong();}
    [[noreturn]] void throw_runtime_00cd70(int)override{wrong();}
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t)override{wrong();}
    void wait_steps_036880(int count)override{state_->waits.push_back(count);}
    void finish_flag_036820()override{++state_->finishes;}
};
struct Graph {
    Ui sdk;
    std::shared_ptr<b11_test::OwnedStream> output=std::make_shared<b11_test::OwnedStream>();
    std::shared_ptr<le::Dialog> dialog=std::make_shared<le::Dialog>(dialog_api());
    std::shared_ptr<PhaseState> phase=std::make_shared<PhaseState>();
    le::LibraryEffects effects{output,dialog,std::make_unique<Phase>(phase)};
    Graph(){ui=&sdk;}
    ~Graph(){ui=nullptr;}
};
// All module operations are synthetic. Native UTF8 conversion may run only on
// owned test strings; no LoadLibrary/FreeLibrary or MessageBoxW is executed.
struct Os final:lib::SystemCalls {
    std::uint32_t mode=0x40;std::vector<std::uint32_t> modes;
    std::vector<std::wstring> loads;std::vector<lib::Module> unloaded;
    std::vector<std::wstring> conversions;std::vector<std::string> events;
    std::function<lib::Module(const wchar_t*)> load_hook;
    std::function<int(const wchar_t*,int,char*,int)> conversion_hook;
    std::uint32_t error=1113;
    std::uint32_t get_error_mode()override{events.emplace_back("get-mode");return mode;}
    std::uint32_t set_error_mode(std::uint32_t value)override{auto old=mode;mode=value;modes.push_back(value);events.emplace_back("set-mode");return old;}
    lib::Module load_library(const wchar_t* path)override{loads.emplace_back(path);events.emplace_back("load");return load_hook?load_hook(path):0x500+loads.size();}
    bool free_library(lib::Module value)override{unloaded.push_back(value);return false;}
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* p,int count,char* out,int cap,bool* used)override{
        if(cp!=65001||flags!=0x80||used)throw std::logic_error("unexpected library UTF8 conversion operands");
        conversions.emplace_back(p,p+count);events.emplace_back("convert");
        return conversion_hook?conversion_hook(p,count,out,cap) : ::WideCharToMultiByte(cp,flags,p,count,out,cap,nullptr,nullptr);
    }
    std::uint32_t last_error()override{return error;}
};
} // namespace b12_test
