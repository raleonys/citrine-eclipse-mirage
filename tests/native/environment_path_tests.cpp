// SPDX-License-Identifier: GPL-3.0-only

#include "../../src/runtime/application_internal.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace module_path_fixture {
enum class Mode { success, grow, fail, grow_then_fail };
Mode mode=Mode::success;
std::wstring expected;
std::vector<DWORD> capacities;
void require(bool value,const char* message) {
    if(!value)throw std::runtime_error(message);
}
DWORD WINAPI filename(HMODULE module,LPWSTR output,DWORD capacity) {
    require(module==nullptr&&output!=nullptr,"module path requests the current process into owned storage");
    capacities.push_back(capacity);
    const auto call=capacities.size();
    require(call<=3,"module path lookup must make bounded progress for the supplied results");
    require(capacity==(call==1?512u:call==2?1024u:2048u),"module path capacity doubles after a full result");
    if(mode==Mode::fail||(mode==Mode::grow_then_fail&&call==2)) {
        SetLastError(ERROR_MOD_NOT_FOUND);
        return 0;
    }
    if((mode==Mode::grow&&call<3)||mode==Mode::grow_then_fail) {
        // A truncated result may fill the entire buffer without a terminator.
        std::fill_n(output,capacity,L'?');
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return capacity;
    }
    require(expected.size()<capacity,"successful fixture result fits the current capacity");
    std::fill_n(output,capacity,L'!');
    std::copy(expected.begin(),expected.end(),output);
    output[expected.size()]=L'\0';
    // Successful Win32 calls need not clear an earlier last-error value.
    SetLastError(ERROR_ACCESS_DENIED);
    return static_cast<DWORD>(expected.size());
}
void select(Mode next,std::wstring value={}) {
    mode=next;expected=std::move(value);capacities.clear();
}
}

#define GetModuleFileNameW module_path_fixture::filename
#include "../../src/runtime/application_environment.cpp"
#undef GetModuleFileNameW

namespace {
using namespace laureondo;
using namespace module_path_fixture;
struct Fixture {
    configuration::State state{};
    application::Options options;
    file::NativeFactory files;
    xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;
    achievement_runtime::HostFormatter formatter;
    application::detail::Environment environment{state,options,files,xml,paths,clock,formatter};
};
configuration::Path path_value(std::wstring_view value) {
    return {value.begin(),value.end()};
}
void configured_path_bypass() {
    Fixture f;f.options.executable_path=u"D:\\configured\\\u00e9-host.exe";
    select(Mode::fail);
    require(f.environment.executable_path_03c6d0()==f.options.executable_path,
        "configured executable path is returned unchanged");
    require(capacities.empty(),"configured executable path bypasses the native lookup");
}
void growth_and_counted_results() {
    Fixture f;
    std::wstring long_path=L"C:\\fixture\\";
    long_path.append(1480,L'\u00e9');long_path+=L"\\host.exe";
    select(Mode::grow,long_path);
    require(f.environment.executable_path_03c6d0()==path_value(long_path),
        "multiple full nonterminated results are discarded before returning the complete UTF16 path");
    require(capacities==std::vector<DWORD>{512,1024,2048},"both required growth steps were exercised");
    const std::wstring edge(511,L'\u00e9');
    select(Mode::success,edge);
    require(f.environment.executable_path_03c6d0()==path_value(edge)&&capacities==std::vector<DWORD>{512},
        "capacity minus one succeeds without growth or a copied terminator");
    select(Mode::success,L"C:\\short.exe");
    require(f.environment.executable_path_03c6d0()==u"C:\\short.exe"&&capacities==std::vector<DWORD>{512},
        "short result ignores unused buffer bytes and stale last error");
}
void failures_and_retry() {
    Fixture f;
    for(const auto selected:{Mode::fail,Mode::grow_then_fail}) {
        select(selected);bool caught=false;
        try{(void)f.environment.executable_path_03c6d0();}
        catch(const std::system_error& error) {
            caught=error.code().value()==ERROR_MOD_NOT_FOUND&&error.code().category()==std::system_category();
        }
        require(caught,"zero module lookup result reports its own native error instead of earlier truncation status");
        require(capacities==(selected==Mode::fail?std::vector<DWORD>{512}:std::vector<DWORD>{512,1024}),
            "native lookup failure stops immediately at the failing call");
        require(f.options.executable_path.empty(),"failed native lookup cannot publish a configured override");
        select(Mode::success,L"C:\\retry.exe");
        require(f.environment.executable_path_03c6d0()==u"C:\\retry.exe"&&capacities==std::vector<DWORD>{512},
            "another lookup after failure starts from fresh owned storage");
    }
}
}
int main()try {
    configured_path_bypass();growth_and_counted_results();failures_and_retry();
    std::cout<<"PASS: actual Environment module path growth, failure and configured override contracts\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
