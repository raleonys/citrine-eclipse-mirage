// SPDX-License-Identifier: GPL-3.0-only

#include "src/runtime/application_internal.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

// Exercise the shipping anonymous LicenseEffects adapter, not a replacement
// Effects timestamp. The linked Environment and host UTC formatter are real.
#include "src/runtime/application.cpp"

namespace {
using namespace citrine_eclipse_mirage;
namespace fs=std::filesystem;
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Directory {
    fs::path path=fs::temp_directory_path()/("surge-ticket-time-"+
        std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    Directory(){require(fs::create_directory(path),"fresh owned ticket directory");}
    ~Directory(){std::error_code error;fs::remove_all(path,error);}
};
struct Fixture {
    configuration::State state=application::initial_state();application::Options options;
    file::NativeFactory files;xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    execution::MutexState mutex{};std::uint8_t a=0,b=0,c=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("unexpected execution error");}};
    std::int64_t frequency=0;
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    application::Provider provider{state,env,files,execution,b};
    application::LicenseEffects effects{env,provider,files};
};
void timestamp_shape(std::string_view stamp,bool filename){
    require(stamp.size()==19,"UTC timestamp has all six fields");
    for(std::size_t i=0;i<stamp.size();++i){
        if(i==4||i==7)require(stamp[i]=='-',"date separators preserved");
        else if(i==10)require(stamp[i]==(filename?'_':'T'),"selected date/time separator");
        else if(i==13||i==16)require(stamp[i]==(filename?'-':':'),"selected clock separators");
        else require(stamp[i]>='0'&&stamp[i]<='9',"timestamp fields contain decimal digits");
    }
}
void ticket_output(){
    Fixture f;Directory directory;
    const auto stamp=f.effects.current_time_041b60_null_true();
    timestamp_shape(stamp,true);
    const auto filename="Denuvo_ticket_"+stamp+".txt";
    require(filename.find_first_of("<>:\"/\\|?*")==std::string::npos,"ticket basename contains no reserved filename character");
    const auto path=directory.path/fs::path(filename);
    constexpr std::string_view bytes="owned request-ticket fixture\r\n";
    require(f.effects.write_ticket_03c1d0(path.u16string(),bytes),"real native ticket writer accepts timestamp basename");
    std::ifstream input(path,std::ios::binary);
    const std::string observed{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
    require(observed==bytes,"ticket file contains every requested byte");
    require(fs::is_regular_file(path)&&fs::file_size(path)==bytes.size(),"ticket is an ordinary file at the complete requested basename");
    // Separate calls may straddle a second or midnight. Assert each format,
    // without assuming a frozen wall clock or comparing different instants.
    timestamp_shape(f.env.current_time_041b60(),false);
    require(f.provider.module==0&&f.provider.object==0,"timestamp/output needs no foreign provider image");
}
}
int main()try{ticket_output();std::cout<<"PASS: "<<checks<<" license ticket time checks; actual adapter/UTC/native output, owned fixture only\n";}
catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
