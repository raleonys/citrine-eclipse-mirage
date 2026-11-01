#include "effects.hpp"
#include "../anadius-xml-source/memory_printer.hpp"
#include <windows.h>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

// Actual information handlers/printer and UTC algorithm on owned test inputs;
// no Graph, original image, real file, clock query or provider lower is used.
namespace {
using namespace anadius;namespace ie=information_effects;namespace h=handlers_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Buffer final:std::stringbuf {
    std::vector<std::string> writes;unsigned flushes=0;std::function<void()> write_hook,flush_hook;
    std::streamsize xsputn(const char* text,std::streamsize size)override{writes.emplace_back(text,static_cast<std::size_t>(size));const auto result=std::stringbuf::xsputn(text,size);if(write_hook)write_hook();return result;}
    int sync()override{++flushes;if(flush_hook)flush_hook();return std::stringbuf::sync();}
};
struct Trace {
    unsigned time=0,gmtime=0,formatted=0,checkpoints=0;bool deny=false,gmtime_null=false;std::string pattern;
    std::function<void()> time_hook,format_hook;
    static void check(void* raw){auto& t=*static_cast<Trace*>(raw);++t.checkpoints;if(t.deny)throw std::logic_error("selected Information admission denied");}
};
struct Clock final:achievement_runtime::Clock {
    Trace& trace;explicit Clock(Trace& value):trace(value){}
    std::int64_t time_05eeb8()override{++trace.time;if(trace.time_hook)trace.time_hook();return 1577836800;}
    std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t value)override{++trace.gmtime;require(value==1577836800,"UTC forwards supplied clock value to gmtime");if(trace.gmtime_null)return std::nullopt;return achievement_runtime::BrokenDown{0,0,0,1,0,120,3,0,0};}
};
struct Formatter final:achievement_runtime::Formatter {
    Trace& trace;explicit Formatter(Trace& value):trace(value){}
    std::string put_time_03d610(const achievement_runtime::BrokenDown& value,std::string_view pattern)override{++trace.formatted;trace.pattern=pattern;require(value.year==120&&value.month==0&&value.day==1,"UTC formatter receives copied controlled tm fields");if(trace.format_hook)trace.format_hook();return "2020-01-01T00:00:00";}
};
struct Fixture {
    Trace trace;std::shared_ptr<Buffer> buffer=std::make_shared<Buffer>();std::shared_ptr<std::ostream> stream=std::make_shared<std::ostream>(buffer.get());
    std::shared_ptr<Clock> clock=std::make_shared<Clock>(trace);std::shared_ptr<Formatter> formatter=std::make_shared<Formatter>(trace);std::unique_ptr<ie::Information> information;
    ie::Options options(){ie::Options value;value.lifetime=buffer;value.stream=stream;value.clock=clock;value.formatter=formatter;return value;}
    Fixture():information(std::make_unique<ie::Information>(options())){}
    void checkpoint(){information->bind_content_checkpoint({&trace,Trace::check});}
    ~Fixture(){information->clear_content_checkpoint();information.reset();}
};
struct Reader final:xml_recovered::AttributeReader {
    const char* attribute(h::Request request,const char* key)override{require(request==0x91&&std::string_view(key)=="index","Actual profile handler reads selected index input");return "0";}
};
void actual_handlers_and_time(){
    Fixture f;require(f.buffer->str().empty()&&f.buffer->writes.empty()&&!f.buffer->flushes&&!f.trace.time&&!f.trace.gmtime&&!f.trace.formatted,"Information construction owns inputs passively without logging or clock callbacks");
    h::Attributes game{{"DISPLAY_NAME","own<&"}},settings;h::Profile profile{std::string("user<&\0tail",11),"persona","uid","US","USD","2","true","false","false"};std::uint8_t online=0,pretend=0;h::StateReferences state{game,settings,profile,online,pretend};
    xml_recovered::MemoryPrinter printer;Reader reader;xml_recovered::HandlerWriter xml(printer,reader);bool flushed_before_xml=false;
    f.buffer->flush_hook=[&]{flushed_before_xml=printer.bytes_without_terminator().empty()&&f.buffer->str()==std::string("profile name: ")+profile.username+"\n";};
    require(h::get_profile_02f530(0x91,xml,state,*f.information),"Actual get_profile executes concrete Information logging and HandlerWriter");
    require(flushed_before_xml&&f.buffer->flushes==1&&f.buffer->writes==std::vector<std::string>{"profile name: ",profile.username},"Exact prefix and counted username including NUL are flushed with newline before any response XML");
    require(printer.bytes_without_terminator().find("GetProfileResponse")!=std::string::npos&&printer.bytes_without_terminator().find("Persona=\"user&lt;&amp;\"")!=std::string::npos,"Actual MemoryPrinter retains native username C-string boundary and XML escaping independently of counted logging");f.buffer->flush_hook={};
    xml_recovered::MemoryPrinter all;xml_recovered::HandlerWriter all_xml(all,reader);require(h::get_all_game_info_02da90(all_xml,state,*f.information),"Actual get_all_game_info invokes concrete current-time effect");
    require(all.bytes_without_terminator().find("DisplayName=\"own&lt;&amp;\"")!=std::string::npos&&all.bytes_without_terminator().find("SystemTime=\"2020-01-01T00:00:00\"")!=std::string::npos&&f.trace.time==1&&f.trace.gmtime==1&&f.trace.formatted==1&&f.trace.pattern=="%Y-%m-%dT%H:%M:%S","Existing UTC helper uses output-null normal mode and supplied clock/tm/formatter in actual response");
    f.trace.gmtime_null=true;xml_recovered::MemoryPrinter missing;xml_recovered::HandlerWriter missing_xml(missing,reader);require(h::get_all_game_info_02da90(missing_xml,state,*f.information)&&missing.bytes_without_terminator().find("SystemTime=\"\"")!=std::string::npos&&f.trace.time==2&&f.trace.gmtime==2&&f.trace.formatted==1,"Null gmtime produces empty SystemTime without invoking formatter or inventing a timestamp");
}
void primitive_checkpoints_and_reentry(){
    {Fixture f;f.checkpoint();f.buffer->write_hook=[&]{f.trace.deny=true;};rejects<std::logic_error>([&]{f.information->log_profile_name_084f48("unreached-name");},"Stream-prefix postcheck stops on swallowed admission denial");require(f.buffer->writes==std::vector<std::string>{"profile name: "}&&f.buffer->str()=="profile name: "&&!f.buffer->flushes,"Denied first stream primitive prevents username/newline/flush");}
    {Fixture f;f.checkpoint();f.trace.time_hook=[&]{f.trace.deny=true;};rejects<std::logic_error>([&]{(void)f.information->current_time_041b60();},"Clock-time postcheck rejects before later opaque primitives");require(f.trace.time==1&&!f.trace.gmtime&&!f.trace.formatted,"Denied time callback cannot reach gmtime or formatter");}
    {Fixture f;unsigned denied=0;f.trace.format_hook=[&]{try{f.information->log_profile_name_084f48("nested");}catch(const std::logic_error&){++denied;}};rejects<std::logic_error>([&]{(void)f.information->current_time_041b60();},"Swallowed formatter reentry remains sticky at outer postcheck");require(denied==1&&f.trace.time==1&&f.trace.gmtime==1&&f.trace.formatted==1&&f.buffer->str().empty(),"Formatter reentry cannot perform nested logging or return successful outer time");rejects<std::logic_error>([&]{(void)f.information->current_time_041b60();},"Unknown/reentrant provider denies subsequent calls");require(f.trace.time==1,"Sticky denial prevents repeated clock effects");}
}
struct FiberCall {
    void* controller;ie::Information& information;bool rejected=false;
    static VOID WINAPI run(void* raw){auto& call=*static_cast<FiberCall*>(raw);try{(void)call.information.current_time_041b60();}catch(const std::logic_error&){call.rejected=true;}::SwitchToFiber(call.controller);std::terminate();}
};
void worker_and_fiber_affinity(){
    const bool converted=!::IsThreadAFiber();void* controller=converted ? ::ConvertThreadToFiber(nullptr) : ::GetCurrentFiber();require(controller!=nullptr,"Own controller fiber available");
    {Fixture f;bool rejected=false;std::thread other([&]{try{f.information->log_profile_name_084f48("foreign");}catch(const std::logic_error&){rejected=true;}});other.join();require(rejected&&f.buffer->str().empty(),"Foreign worker rejects before any stream primitive");
        FiberCall call{controller,*f.information};auto* fiber=::CreateFiber(0,FiberCall::run,&call);require(fiber!=nullptr,"Own secondary fiber created");::SwitchToFiber(fiber);::DeleteFiber(fiber);require(call.rejected&&!f.trace.time&&!f.trace.gmtime&&!f.trace.formatted,"Same-thread foreign fiber rejects before clock effects");f.information->log_profile_name_084f48("original");require(f.buffer->str()=="profile name: original\n","Rejected foreign access does not poison original worker/fiber use");}
    if(converted)require(::ConvertFiberToThread()!=0,"Own controller returns to ordinary thread after provider retirement");
}
void attached_retirement_and_required_inputs(){
    Fixture f;auto invalid=f.options();invalid.stream.reset();rejects<std::invalid_argument>([&]{ie::Information bad(std::move(invalid));},"Information requires an owned stream before provider work");require(!f.trace.time&&f.buffer->str().empty(),"Invalid construction remains passive");
    Trace trace;ie::Information* facade=nullptr;unsigned retired=0,denied=0;
    auto probe=[&]{++retired;try{(void)facade->current_time_041b60();}catch(const std::logic_error&){++denied;}try{facade->log_profile_name_084f48("retiring");}catch(const std::logic_error&){++denied;}};
    ie::Options options;options.lifetime=std::shared_ptr<void>(new int(7),[&](void* p){probe();delete static_cast<int*>(p);});
    options.stream=std::shared_ptr<std::ostream>(new std::ostringstream,[&](std::ostream* p){probe();delete static_cast<std::ostringstream*>(p);});
    options.clock=std::shared_ptr<achievement_runtime::Clock>(new Clock(trace),[&](achievement_runtime::Clock* p){probe();delete p;});
    options.formatter=std::shared_ptr<achievement_runtime::Formatter>(new Formatter(trace),[&](achievement_runtime::Formatter* p){probe();delete p;});
    auto owned=std::make_unique<ie::Information>(std::move(options));facade=owned.get();require(retired==0&&!trace.time&&!trace.formatted,"Sole owned leaf/lifetime captures remain alive during passive construction");owned.reset();
    require(retired==4&&denied==8&&!trace.time&&!trace.gmtime&&!trace.formatted,"All last-owner deleters see attached Retiring guard and reject both routes before touching retired leaves");
}
}
int main(){try{actual_handlers_and_time();primitive_checkpoints_and_reentry();worker_and_fiber_affinity();attached_retirement_and_required_inputs();std::cout<<"PASS: "<<checks<<" Information effects checks; own handlers/XML/clock and bounded admission/retirement only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
