#include "../../src/runtime/application_internal.hpp"
#include <iostream>
#include <string>
namespace dialog {
std::wstring body,title;unsigned calls=0;
int WINAPI show(HWND,LPCWSTR text,LPCWSTR caption,UINT flags){
    if(flags!=MB_OK)std::terminate();body=text;title=caption;++calls;return IDOK;
}
}
#define MessageBoxW dialog::show
#include "../../src/runtime/application_environment.cpp"
#undef MessageBoxW

int main()try{
    using namespace preloader_l;
    configuration::State state{};application::Options options;
    file_recovered::NativeFactory files;xml_reader_bridge::Backend xml;achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    env.unknown_entitlement_field("QueryEntitlements unknown attr: \xc3\xa9|value");
    if(dialog::calls!=1||dialog::title!=L"SURGE"||dialog::body!=L"QueryEntitlements unknown attr: \u00e9|value")
        throw std::runtime_error("actual dialog lost the supplied field/value diagnostic");
    std::cout<<"PASS: actual Environment dialog forwards counted UTF8 diagnostic and product title\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
