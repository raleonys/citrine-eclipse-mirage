// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/environment.hpp"
#include "src/files/atomic_io.hpp"
#include "src/files/binary_io.hpp"
#include "tests/support/files/packet_binary_io.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
namespace fs=std::filesystem;
namespace current=preloader_l::files;
namespace packet=preloader_l::test_packet_binary_io;
static unsigned checks=0;
static void check(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
static void put(const fs::path& p,std::string_view text){fs::create_directories(p.parent_path());std::ofstream out(p,std::ios::binary);out.write(text.data(),text.size());check(bool(out),"fixture write failed");}
static std::string get(const fs::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
struct PacketOutput final:packet::Output {
 std::vector<std::string>& trace;unsigned state;
 PacketOutput(std::vector<std::string>& t,unsigned s):trace(t),state(s){}
 ~PacketOutput(){trace.push_back("destroy");}
 unsigned initial_state()const override{trace.push_back("initial");return state;}
 void write_03be80(std::string_view)override{trace.push_back("write");}
 void close_03ae70()override{trace.push_back("close");}
};
struct PacketFactory final:packet::Factory {
 std::vector<std::string> trace;unsigned state=2;
 std::unique_ptr<packet::Input> open_input_039f70(const packet::Path&,unsigned,unsigned)override{throw std::logic_error("unused input");}
 std::unique_ptr<packet::Output> open_output_03a1a0(const packet::Path&,unsigned mode,unsigned share)override{check(mode==0x20&&share==0x40,"packet mode/share");trace.push_back("open");return std::make_unique<PacketOutput>(trace,state);}
};
struct TracedOutput final:current::Output {
 std::unique_ptr<current::Output> real;std::vector<std::string>& trace;
 TracedOutput(std::unique_ptr<current::Output> r,std::vector<std::string>& t):real(std::move(r)),trace(t){}
 ~TracedOutput(){trace.push_back("destroy");}
 unsigned initial_state()const override{trace.push_back("initial");return real->initial_state();}
 void write_03be80(std::string_view value)override{trace.push_back("write");real->write_03be80(value);}
 void close_03ae70()override{trace.push_back("close");real->close_03ae70();}
};
struct TracedFactory final:current::Factory {
 current::NativeFactory native;std::vector<std::string> trace;
 explicit TracedFactory(const preloader_l::RuntimePaths& owner):native(owner){}
 std::unique_ptr<current::Input> open_input_039f70(const current::Path&,unsigned,unsigned)override{throw std::logic_error("unused input");}
 std::unique_ptr<current::Output> open_output_03a1a0(const current::Path& p,unsigned mode,unsigned share)override{trace.push_back("open");return std::make_unique<TracedOutput>(native.open_output_03a1a0(p,mode,share),trace);}
};
int main(){try{
 auto root=fs::temp_directory_path()/("preloader_l_collision_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 struct Cleanup{fs::path p;~Cleanup(){std::error_code e;fs::remove_all(p,e);}}cleanup{root};
 auto owner=preloader_l::RuntimePaths::from_system(root/"module/preloader_l.dll",root/"game/game.exe",root/"local",root/"program");
#ifdef _WIN32
 const auto pid=GetCurrentProcessId();
#else
 const auto pid=getpid();
#endif
 const auto target=owner.data()/"output.bin";put(target,"OLD");
 auto stage=[&](unsigned n){return target.parent_path()/(".preloader_l-"+std::to_string(pid)+"-"+std::to_string(n)+".tmp");};
 auto occupy=[&](unsigned begin,unsigned end){for(unsigned n=begin;n<end;++n)put(stage(n),"PREEXISTING-"+std::to_string(n));};
 // This executable has not used any AtomicFileWriter yet, so its private
 // sequence starts at zero. Real exclusive-create collisions, no syscall mock.
 occupy(0,100);bool failed=false;
 try{preloader_l::AtomicFileWriter out(target,&owner);}catch(const std::system_error& e){failed=true;check(e.code()==std::errc::file_exists,"exhaustion must identify existing stage names");}
 check(failed,"initial stage exhaustion escaped IO error classification");check(get(target)=="OLD","failed direct setup changed target");
 occupy(100,200);current::NativeFactory native(owner);
 {auto stream=native.open_output_03a1a0(target.u16string(),0x20,0x40);check(stream->initial_state()==2,"collision setup must report source failbit2");}
 occupy(200,300);PacketFactory expected;TracedFactory actual(owner);
 const auto expected_result=packet::write_binary_03c1d0(target.u16string(),"PAYLOAD",expected);
 const auto actual_result=current::write_binary_03c1d0(target.u16string(),"PAYLOAD",actual);
 check(!expected_result&&actual_result==expected_result,"real collision output must match packet initial-failure false branch");
 check(actual.trace==expected.trace,"initial failure unexpectedly wrote/closed payload");check(get(target)=="OLD","failed binary output changed target");
 // A free final attempt still succeeds: the retained budget is not shortened.
 occupy(300,399);expected.trace.clear();expected.state=0;actual.trace.clear();
 check(current::write_binary_03c1d0(target.u16string(),"NEW",actual)==packet::write_binary_03c1d0(target.u16string(),"NEW",expected),"last permitted attempt lost success");
 check(actual.trace==expected.trace&&get(target)=="NEW","success source trace or commit bytes differ");
 check(!fs::exists(stage(399)),"committed owned stage leaked");
 for(unsigned n=0;n<399;++n)check(get(stage(n))=="PREEXISTING-"+std::to_string(n),"unowned collision file altered");
 std::cout<<"PASS real stage collision/source caller checks="<<checks<<";399 preexisting files preserved; Windows execution NOT_RUN unless run there\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
