// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/message_dispatch_test_fixture.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include <deque>
using namespace message_test;
namespace t=citrine_eclipse_mirage::threads;namespace m=citrine_eclipse_mirage::messages;
namespace {
struct Exit {std::uint32_t code;};
struct Threads:t::Effects {
    std::unique_ptr<t::Context> live;t::Context* slot=nullptr;s::MessageDispatchContext* message=nullptr;
    bool immediate=true;unsigned creates=0,entries=0,frees=0,exits=0;
    t::Context* calloc_context_066848()override{live=std::make_unique<t::Context>();return live.get();}
    void free_0668c0(t::Context* p)override{check(!p,"unchanged05e8e0 constructor retains free(NULL) call");++frees;}
    bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t entry,t::Handle* module)override{check(flags==4&&entry==0x180021770ULL&&module==&live->module,"existing thread source receives opaque021770 token");return false;}
    void run(){check(bool(live)&&!slot,"synthetic CRT context ready before wrapper");try{t::thread_entry_05e7a8(live.get(),*this);r::unexpected();}catch(const Exit& e){check(!e.code&&!live&&!slot,"actual CRT wrapper exits0 after routed application and context lifetime end");}}
    t::Handle create_thread(void* security,std::uint64_t stack,t::Context* context,std::uint32_t flags,std::uint32_t* id)override{check(!security&&!stack&&!flags&&context==live.get()&&id,"actual05e8e0 forwards controlled CreateThread arguments");++creates;if(immediate)run();*id=57;return 0xabc;}
    std::uint32_t last_error()override{r::unexpected();}void write_error_slot_05d724(std::uint32_t)override{r::unexpected();}void invalid_parameter_05c954()override{r::unexpected();}void map_os_error_05d698(std::uint32_t)override{r::unexpected();}
    bool close_handle(t::Handle)override{r::unexpected();}bool free_library(t::Handle)override{r::unexpected();}
    t::Context*& required_thread_slot_064b90()override{return slot;}t::Context* optional_thread_context_064d08()override{return slot;}
    int runtime_policy_06c0d0()override{return 0;}int initialize_runtime_067320(int)override{r::unexpected();}void uninitialize_runtime_06736c()override{r::unexpected();}
    std::uint32_t invoke_entry(std::uintptr_t entry,std::uintptr_t argument)override{++entries;check(slot==live.get(),"actual CRT wrapper publishes thread context before application");return s::route_message_thread_entry(entry,argument,*message);}
    [[noreturn]]void free_library_and_exit_thread(t::Handle,std::uint32_t)override{r::unexpected();}
    [[noreturn]]void exit_thread(std::uint32_t code)override{++exits;slot=nullptr;live.reset();throw Exit{code};}
};
struct ThreadApi:s::MessageThreadApi {
    Fixture& f;explicit ThreadApi(Fixture& f,Threads& t):MessageThreadApi(t),f(f){}
    void construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t p,std::uint64_t n)override{f.memory.construct_message_006b30(out,p,n);if(out.capacity_18+1>=4096){const auto a=s::receive_buffer_address(out);f.headers[f.header_count++]={a-8,b::word(f.base.copied.memory.view(a-8,8))};}}
    s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override{auto* p=f.memory.allocate_frame_04b0bc(n);f.closures[0]={reinterpret_cast<std::uintptr_t>(p),p};return p;}
    std::int32_t detach_04a458(const s::FrameThreadHandle& h)override{return f.memory.detach_04a458(h);}
    [[noreturn]]void thread_error_04a648(int n)override{f.memory.thread_error_04a648(n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{f.memory.free_sized_04b0f8(p,n);}
    [[noreturn]]void invalid_parameter_05c974()override{f.memory.invalid_parameter_05c974();}
    void next_write_02a500(std::uintptr_t)override{r::unexpected();}
    s::WriteFinishStream& begin_log_006880(std::uintptr_t a,std::uintptr_t b)override{return f.memory.begin_log_006880(a,b);}
    s::WriteFinishStream& append_007990(s::WriteFinishStream& stream,std::uintptr_t p,std::uint64_t n)override{return f.memory.append_007990(stream,p,n);}
    std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t n)override{return f.memory.widen_01e510(p,n);}
    void put_00fe90(s::WriteFinishStream& stream,std::uint8_t n)override{f.memory.put_00fe90(stream,n);}
    void flush_00e1e0(s::WriteFinishStream& stream)override{f.memory.flush_00e1e0(stream);}
};
struct Cipher:m::Cipher {
    bool fail=false;unsigned decrypts=0;
    void construct_0423d0_in_place()override{r::unexpected();}void set_fixed_key_084c98()override{r::unexpected();}void set_key_042780(const m::Key&)override{r::unexpected();}
    bool decrypt_042480(std::string&)override{++decrypts;return !fail;}bool encrypt_042610(std::string&)override{return true;}
};
struct Factory:m::CryptoFactory {std::unique_ptr<m::Cipher> create_0423d0()override{r::unexpected();}};
struct Random:m::Random {m::Key challenge_bytes_034cf0()override{r::unexpected();}m::Key key_from_response_034c30(std::string_view)override{r::unexpected();}};
struct Events:m::EventQueue {
    bool nonempty_relaxed()const override{return false;}int lock_04a6ac()override{r::unexpected();}std::int32_t& lock_count_4c()override{r::unexpected();}bool nonempty_locked()const override{r::unexpected();}std::string copy_front_locked()const override{r::unexpected();}void pop_front_locked()override{r::unexpected();}void unlock_04a6b4()override{r::unexpected();}
};
struct Effects:m::Effects {
    unsigned commands=0,bad_xml=0,failed_decrypt=0;
    int atoi_05ee08(std::string_view)override{r::unexpected();}
    bool registered_handler(std::uint32_t,const m::XmlNode&,m::XmlPrinter& p)override{++commands;p.open("SyntheticAnswer");p.attribute("value","fixture");p.close();return true;}
    [[noreturn]]void invalid_message_02bd30_04d228()override{throw Failure{66};}[[noreturn]]void thread_error_04a648(int)override{r::unexpected();}
    void log_received(std::string_view)override{}void log_response(std::string_view)override{}void log_event(std::string_view)override{r::unexpected();}
    void log_bad_xml(std::string_view,std::string_view)override{++bad_xml;}void log_decryption_failure(std::string_view)override{++failed_decrypt;}
};
struct Loop {
    Fixture f;Threads threads;ThreadApi thread_api{f,threads};
    s::FrameReadContext frame{f.base.buffer,f.base.base.completion,thread_api,f.base.owners,f.base.categories,f.headers,f.base.terminal,{0x1111},nullptr};
    s::MessageDispatchContext message{frame,f.api,f.owners,f.copies,f.closures};
    citrine_eclipse_mirage::xml_reader_bridge::Backend xml;Cipher cipher;Factory factory;Random random;Events events;Effects effects;
    std::uint8_t encrypted=0,show=0;std::deque<s::MessageWriteTask> queued;std::string decoded_input;unsigned rearms=0;
    Loop(){threads.message=&message;
        f.core_hook=[&](auto member,auto& response,auto& input){
            check(member==0x120f0,"core compatibility adapter binds same owner+f0 identity");
            auto owned=f.text(input);const auto result=m::process_message_033d70({encrypted,show,cipher,events},owned,xml,factory,random,effects);decoded_input=owned;
            // Selected normal-return string adapter on owned test spans; does
            // not claim native MSVC string allocation or exception mutation ABI.
            if(owned!=f.text(input)){f.free_string(input);f.seed(f.extra,owned);input=f.extra.value;}
            f.seed(f.response,result);response=f.response.value;
        };
        f.hook=[&](auto e){if(e=="schedule"){queued.push_back(*f.api.task);f.api.task->response_08.size_10=0;f.api.task->response_08.capacity_18=15;f.api.task->response_08.storage_00[0]=0;f.api.task->object_28=0;f.api.task->control_30=nullptr;}};
    }
    void start(std::string input,bool immediate){
        threads.immediate=immediate;f.base.seed(input+std::string("\0tail",5),std::max<std::uint64_t>(15,input.size()+5));
        f.base.base.handler.words_88_98[2]=0x12000;
        frame_test::restart_capture=[&](auto owner){check(owner==0x12000,"R4 rearm retains source owner");++rearms;};
        s::finish_frame_read_025480(f.base.base.handler,f.base.base.incoming,input.size()+1,frame,&frame_test::capture_restart);
        check(f.base.base.buffer.text()=="tail"&&rearms==1,"actual R4 consumes frame and preserves trailing input after synthetic thread detach");
        if(!immediate){check(f.memory.closure&&queued.empty()&&!f.api.processes,"deferred thread owns copied message after input is consumed");threads.run();}
        check(!f.memory.closure&&threads.creates==1&&threads.entries==1&&threads.exits==1&&threads.frees==1,"actual05e8e0/05e7a8/021770/028d30 composition closes closure lifetime in both timing orders");
    }
    void consume(){for(auto& task:queued){check(task.function_00==0x180028270ULL&&task.control_30==&f.base.owner,"controlled021420 owns exact write task and acquired reference");f.free_string(task.response_08);auto* c=task.control_30;check(c->strong_08.fetch_sub(1)==2,"last queued reference returns to original owner");task.control_30=nullptr;}queued.clear();check(f.base.owner.strong_08==1&&!f.base.owner.disposes,"owned write consumer balances source reference without deleting original owner");}
};
void integration(){
    const std::string xml="<LSX><Request id=\"synthetic\" recipient=\"fixture\"><GetConfig/></Request></LSX>";
    for(bool immediate:{false,true})for(unsigned mode=0;mode<5;++mode){Loop l;std::string input=xml;if(mode==1){l.encrypted=1;m::hex_encode_03d190(input);}if(mode==2){l.encrypted=1;l.cipher.fail=true;m::hex_encode_03d190(input);}if(mode==3)input="<LSX>";if(mode==4)input="";
        l.start(input,immediate);
        if(mode==4){check(l.queued.empty()&&!l.f.api.processes&&l.f.api.empty==1,"empty frame bypasses core and writer while trampoline still cleans closure");}
        else {check(l.queued.size()==1&&l.f.api.processes==1&&l.f.api.schedules==1&&l.f.base.owner.strong_08==2,"every nonempty request schedules a response, including empty failure response");
            auto output=l.f.text(l.queued.front().response_08);if(mode==0)check(output.find("SyntheticAnswer")!=std::string::npos&&output.back()==0&&l.effects.commands==1,"actual033d70/parser/serializer emits NUL-terminated plaintext response");
            if(mode==1){check(l.decoded_input==xml&&l.cipher.decrypts==1&&!output.empty()&&output.back()==0,"actual hex decoding mutates owned input and response is hex encoded");output.pop_back();m::hex_decode_03ce30(output);check(output.find("SyntheticAnswer")!=std::string::npos,"encrypted response independently decodes to expected XML");}
            if(mode==2)check(output.empty()&&l.effects.failed_decrypt==1,"decryption failure still dispatches zero-size response");if(mode==3)check(output.empty()&&l.effects.bad_xml==1,"bad XML still dispatches zero-size response");
        }
        l.consume();
    }
}
}
int main(){try{integration();std::cout<<"PASS: "<<checks<<" socket message dispatch integration checks; actual025480/05e8e0/05e7a8/021770/028d30/033d70 source, compatibility XML parser/serializer, immediate and deferred synthetic thread, owned021420 response adapter\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
