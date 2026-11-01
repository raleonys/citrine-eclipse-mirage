#include "stream.hpp"
#include "../anadius-callback-policy-source/policy_test_fixture.hpp"
#include "../anadius-services-completion-source/completion.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Own source-only logging exercise. No native callback entry, module loading,
// real configuration file, socket, UI, or original DLL is invoked here.
namespace {
using namespace anadius;
using namespace std::chrono_literals;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& call,const char* message){bool seen=false;try{call();}catch(const E&){seen=true;}require(seen,message);}

// An independent data lock prevents test-fixture UB if the production gate is
// absent. Inflight is incremented BEFORE that lock, so it cannot conceal two
// concurrent compiler-streambuf invocations. No stringbuf virtual recursion.
class InstrumentedBuffer final:public std::streambuf {
    std::mutex data_;
    std::string text_;
    struct Inflight {
        InstrumentedBuffer& owner;
        explicit Inflight(InstrumentedBuffer& value):owner(value){
            if(owner.inflight.fetch_add(1)!=0)owner.overlap=true;
            ++owner.entries;
            std::this_thread::yield();
        }
        ~Inflight(){owner.inflight.fetch_sub(1);}
    };
protected:
    std::streamsize xsputn(const char* value,std::streamsize count)override{
        Inflight scope(*this);std::lock_guard lock(data_);
        const auto copied=maximum<0?count:std::min(count,maximum);
        if(copied>0)text_.append(value,static_cast<std::size_t>(copied));
        return copied;
    }
    int_type overflow(int_type value=traits_type::eof())override{
        Inflight scope(*this);std::lock_guard lock(data_);
        if(traits_type::eq_int_type(value,traits_type::eof()))return traits_type::not_eof(value);
        if(maximum==0)return traits_type::eof();
        text_.push_back(traits_type::to_char_type(value));return value;
    }
    int sync()override{
        Inflight scope(*this);std::lock_guard lock(data_);++syncs;return fail_flush?-1:0;
    }
public:
    std::atomic<unsigned> inflight=0,entries=0,syncs=0;
    std::atomic<bool> overlap=false;
    // Changed only while every writer is quiescent.
    std::streamsize maximum=-1;
    bool fail_flush=false;
    std::string text(){std::lock_guard lock(data_);return text_;}
};
struct ConversionState {
    unsigned calls=0;
    bool fail_query=false,fail_fill=false,padded=false;
    std::function<void()> query;
};
class OwnEncoding final:public encoding_recovered::Calls {
    std::shared_ptr<ConversionState> state_;
public:
    explicit OwnEncoding(std::shared_ptr<ConversionState> state):state_(std::move(state)){}
    std::uint32_t locale_codepage_0629ec()override{return 65001;}
    bool are_file_apis_ansi()override{return true;}
    int multi_byte_to_wide(std::uint32_t,std::uint32_t,const char*,int,char16_t*,int)override{throw std::logic_error("unused own widening");}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* value,int count,char* out,int capacity,bool* used)override{
        ++state_->calls;
        if(cp!=65001||flags!=0x80||used)throw std::logic_error("unexpected owned UTF8 conversion operands");
        if(!out){if(state_->query)state_->query();if(state_->fail_query)throw std::runtime_error("owned query failure");}
        else if(state_->fail_fill)throw std::runtime_error("owned fill failure");
        if(state_->padded){if(!out)return 4;if(capacity!=4)throw std::logic_error("first-pass capacity lost");out[0]='Z';return 1;}
        if(out){if(capacity!=count)throw std::logic_error("owned conversion capacity");for(int i=0;i<count;++i){if(value[i]>0x7f)throw std::logic_error("own ASCII test input required");out[i]=static_cast<char>(value[i]);}}
        return count;
    }
    std::uint32_t last_error()override{return 1113;}
};
struct Writers {
    std::shared_ptr<InstrumentedBuffer> buffer=std::make_shared<InstrumentedBuffer>();
    std::shared_ptr<shared_log::Stream> stream=shared_log::make_stream(buffer);
    std::shared_ptr<ConversionState> conversion=std::make_shared<ConversionState>();
    std::shared_ptr<b8_test::LowerState> configuration_state=std::make_shared<b8_test::LowerState>();
    std::shared_ptr<b12_test::PhaseState> library_state=std::make_shared<b12_test::PhaseState>();
    std::shared_ptr<b13_test::ForeignState> online_state=std::make_shared<b13_test::ForeignState>();
    std::shared_ptr<library_effects::Dialog> dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());
    std::shared_ptr<b14_test::Engine> engine=std::make_shared<b14_test::Engine>();
    std::shared_ptr<b14_test::Resolver> resolver=std::make_shared<b14_test::Resolver>();
    std::shared_ptr<int> callback_lifetime=std::make_shared<int>(20);
    services_completion::BootstrapLogger bootstrap{stream};
    config_logging::ConfigurationLogging configuration{stream,std::make_unique<b8_test::Lower>(configuration_state),std::make_unique<OwnEncoding>(conversion)};
    library_effects::LibraryEffects libraries{stream,dialog,std::make_unique<b12_test::Phase>(library_state)};
    online_platform::OnlinePlatform online{stream,dialog,std::make_unique<b13_test::Foreign>(online_state)};
    std::shared_ptr<host_platform::Domain> host_domain=std::make_shared<host_platform::Domain>(stream,engine,resolver,callback_lifetime);
    host_platform::HostPlatform host{host_domain};
    host_platform::HookCleanup cleanup{host_domain};
};
std::size_t occurrences(std::string_view text,std::string_view needle){
    std::size_t found=0,at=0;while((at=text.find(needle,at))!=text.npos){++found;at+=needle.size();}return found;
}

void six_selected_writers_share_one_stream(){
    Writers graph;
    b16_test::Fixture policy_fixture;
    auto resources=std::make_shared<callback_policy::Resources>(policy_fixture.resources());
    resources->stream=graph.stream;
    constexpr unsigned rounds=64;
    const std::string library_marker("|B12\0T|",7),policy_marker("|B16\0T|",7);
    std::array<std::function<void()>,6> jobs{
        [&]{for(unsigned i=0;i<rounds;++i){auto* token=graph.bootstrap.insert(graph.bootstrap.token(),0x180083cd8ULL);graph.bootstrap.finish(token);}},
        [&]{for(unsigned i=0;i<rounds;++i)graph.configuration.log_config_path(u"C:/own/own.cfg");},
        [&]{for(unsigned i=0;i<rounds;++i){graph.libraries.begin_load_log_0863e0();graph.libraries.append_log_path_and_flush(library_marker);}},
        [&]{for(unsigned i=0;i<rounds;++i){graph.online.log_literal_006880("authentication ok");graph.online.log_finish_0088e0();}},
        [&]{for(unsigned i=0;i<rounds;++i){graph.host.log_text_006880("succeeded");graph.host.log_integer_00af90(26);graph.host.finish_log_0088e0();graph.cleanup.log_literal_006880("disabling hook failed");graph.cleanup.log_finish_0088e0();}},
        [&]{
            // Constructed on its sole controller thread. This is direct B16
            // logging, not foreign admission or native callback execution.
            callback_policy::Policy policy(policy_fixture.view(),std::move(*resources));
            for(unsigned i=0;i<rounds;++i){
                policy.log_literal_006880(std::uint64_t{0x180080528ULL});policy.log_text_007990(policy_marker);policy.finish_log_0088e0();
                policy.log_literal_006880(std::string_view("game connected on port "));policy.log_port_00ad70(3216);policy.log_finish_0088e0();
            }
        }
    };
    std::atomic<unsigned> ready=0;
    std::atomic<bool> start=false;
    std::array<std::exception_ptr,6> failures{};
    std::vector<std::thread> workers;
    for(std::size_t i=0;i<jobs.size();++i)workers.emplace_back([&,i]{
        ready.fetch_add(1);ready.notify_all();start.wait(false);
        try{jobs[i]();}catch(...){failures[i]=std::current_exception();}
    });
    for(auto count=ready.load();count!=jobs.size();count=ready.load())ready.wait(count);
    start=true;start.notify_all();for(auto& worker:workers)worker.join();
    for(const auto& failure:failures)if(failure)std::rethrow_exception(failure);
    require(!graph.buffer->overlap&&graph.buffer->inflight==0&&graph.buffer->entries>0,"all six selected compiler writers serialize streambuf calls on the same owned Stream");
    const auto text=graph.buffer->text();
    for(const auto marker:{std::string_view("LSX emu"),std::string_view("Parsing "),std::string_view("own.cfg"),std::string_view("Loading "),
        std::string_view(library_marker),std::string_view("authentication ok"),std::string_view("succeeded"),std::string_view("disabling hook failed"),
        std::string_view("fake key created"),std::string_view(policy_marker),std::string_view("game connected on port "),std::string_view("3216")})
        require(occurrences(text,marker)==rounds,"every selected adapter preserves each complete counted or C-string insertion");
    require(graph.buffer->syncs==rounds*8&&std::count(text.begin(),text.end(),'\n')==rounds*8,"all eight selected finish routes retain endl newline and flush effects");
    require(graph.conversion->calls==rounds*2,"configuration still performs its own query/fill outside short stream operations");
    require(graph.library_state->other_calls==0&&graph.online_state->wrong_calls==0&&graph.engine->events.empty()&&graph.resolver->seen.empty(),"log tests invoke no lower module, hook engine, native original, or UI provider");
}

void formatting_counted_data_and_persistent_state(){
    {
        Writers graph;
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->fill('.');graph.stream->width(10);}
        graph.configuration.log_config_path(u"a.cfg");
        require(graph.buffer->text()=="..Parsing a.cfg\n","configuration formatted prefix consumes shared width without changing source insertion kind");
        {const auto lease=shared_log::acquire(*graph.stream);require(graph.stream->width()==0,"compiler insertion consumes width on the persistent underlying stream");}
    }
    {
        Writers graph;
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->fill('.');graph.stream->width(10);}
        graph.libraries.begin_load_log_0863e0();
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->width(5);}
        graph.libraries.append_log_path_and_flush(std::string_view("x\0y",3));
        require(graph.buffer->text()==std::string("..Loading ..")+std::string("x\0y",3)+"\n","library counted formatted insertion retains embedded NUL and shared width/fill");
    }
    {
        Writers graph;b16_test::Fixture fixture;auto resources=fixture.resources();resources.stream=graph.stream;
        callback_policy::Policy policy(fixture.view(),std::move(resources));
        {const auto lease=shared_log::acquire(*graph.stream);*graph.stream<<std::hex<<std::showbase;graph.stream->fill('.');graph.stream->width(6);}
        policy.log_port_00ad70(26);policy.finish_log_0088e0();graph.host.log_integer_00af90(26);graph.host.finish_log_0088e0();
        policy.log_text_007990(std::string_view("\0T",2));policy.finish_log_0088e0();
        require(graph.buffer->text()==std::string("..0x1a\n0x1a\n")+std::string("\0T\n",3),"callback uint16 and host int32 keep shared numeric flags; counted language text keeps NUL");
    }
    {
        Writers graph;graph.conversion->padded=true;graph.configuration.log_config_path(u"own.cfg");
        require(graph.buffer->text()==std::string("Parsing Z\0\0\0\n",13),"config counted insertion retains first-pass-sized zero tail after a shorter fill");
    }
}

void conversion_does_not_hold_stream_gate(){
    Writers graph;
    std::promise<void> entered,finished;auto entry=entered.get_future();auto completion=finished.get_future();
    bool prefix_seen=false;std::exception_ptr worker_failure,configuration_failure;
    graph.conversion->query=[&]{
        prefix_seen=graph.buffer->text()=="Parsing ";entered.set_value();
        if(completion.wait_for(2s)!=std::future_status::ready)throw std::runtime_error("conversion held shared stream gate across another source writer");
        completion.get();
    };
    std::thread other([&]{
        try{
            if(entry.wait_for(2s)!=std::future_status::ready)throw std::runtime_error("own conversion was not reached");
            graph.libraries.begin_load_log_0863e0();graph.libraries.append_log_path_and_flush("own-provider");finished.set_value();
        }catch(...){worker_failure=std::current_exception();try{finished.set_exception(worker_failure);}catch(...){}}
    });
    try{graph.configuration.log_config_path(u"own.cfg");}catch(...){configuration_failure=std::current_exception();}
    other.join();if(worker_failure)std::rethrow_exception(worker_failure);if(configuration_failure)std::rethrow_exception(configuration_failure);
    require(prefix_seen,"configuration prefix is committed before conversion provider runs");
    require(graph.buffer->text()=="Parsing Loading own-provider\nown.cfg\n","another source writer completes while conversion provider is still active, before config suffix");
    require(!graph.buffer->overlap&&graph.buffer->syncs==2,"conversion overlap needs no callback-wide or phase-wide stream lock");
}

void failure_prefixes_and_recovery(){
    for(bool fill_failure:{false,true}){
        Writers graph;graph.conversion->fail_query=!fill_failure;graph.conversion->fail_fill=fill_failure;
        rejects<std::runtime_error>([&]{graph.configuration.log_config_path(u"own.cfg");},"controlled conversion failure remains visible to caller");
        require(graph.buffer->text()=="Parsing "&&graph.buffer->syncs==0,"query/fill failure preserves only source prefix without invented newline or flush");
        graph.libraries.begin_load_log_0863e0();graph.libraries.append_log_path_and_flush("after");
        require(graph.buffer->text()=="Parsing Loading after\n","a different selected writer can acquire after a conversion exception");
    }
    {
        Writers graph;graph.buffer->maximum=3;
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->exceptions(std::ios::badbit);}
        rejects<std::ios_base::failure>([&]{graph.configuration.log_config_path(u"own.cfg");},"selected source prefix write failure propagates compiler ostream exception");
        require(graph.buffer->text()=="Par"&&graph.conversion->calls==0&&graph.buffer->syncs==0,"failed partial prefix stops conversion and preserves committed bytes");
        {const auto lease=shared_log::acquire(*graph.stream);require(graph.stream->bad(),"badbit remains on persistent selected stream after failed output");graph.stream->exceptions(std::ios::goodbit);graph.stream->clear();}
        graph.buffer->maximum=-1;graph.online.log_literal_006880("authentication ok");graph.online.log_finish_0088e0();
        require(graph.buffer->text()=="Parauthentication ok\n","explicit caller recovery enables another writer; guard does not replay failed bytes");
    }
    {
        Writers graph;graph.buffer->fail_flush=true;
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->exceptions(std::ios::badbit);}
        graph.libraries.begin_load_log_0863e0();
        rejects<std::ios_base::failure>([&]{graph.libraries.append_log_path_and_flush("x");},"selected library endl flush failure propagates");
        require(graph.buffer->text()=="Loading x\n"&&graph.buffer->syncs==1,"flush exception preserves complete prior line and one attempted sync");
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->exceptions(std::ios::goodbit);graph.stream->clear();}
        graph.buffer->fail_flush=false;graph.cleanup.log_literal_006880("succeeded");graph.cleanup.log_finish_0088e0();
        require(graph.buffer->text()=="Loading x\nsucceeded\n"&&graph.buffer->syncs==2,"HookCleanup acquires released stream gate after an endl exception");
    }
    {
        Writers graph;
        {const auto lease=shared_log::acquire(*graph.stream);graph.stream->setstate(std::ios::failbit);}
        graph.configuration.log_config_path(u"own.cfg");
        require(graph.buffer->text().empty()&&graph.buffer->syncs==0&&graph.conversion->calls==2,"nonthrowing failed compiler stream still performs source conversion without fabricated output");
        {const auto lease=shared_log::acquire(*graph.stream);require(graph.stream->fail(),"new synchronization does not silently clear a preexisting failed stream");}
    }
}
}
int main(){
    try{
        six_selected_writers_share_one_stream();formatting_counted_data_and_persistent_state();conversion_does_not_hold_stream_gate();failure_prefixes_and_recovery();
        std::cout<<"PASS: "<<checks<<" shared log integration checks; six selected source writers, direct controller Policy logging only\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
