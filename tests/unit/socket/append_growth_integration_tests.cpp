// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/append_growth_test_fixture.hpp"
using namespace core_test;
using container_test::active_container;
using container_test::borrowed_containers;
using owned_test::active_owned;
namespace {
struct Loop;
void message_copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& in){out.payload_00=in.payload_00;out.state_18=in.state_18;}
struct Send:s::AsyncSendApi {
    Loop& f;explicit Send(Loop& f):f(f){}
    s::AsyncSendResult send(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override;
};
struct Effects:s::ComposedWriteEffects,s::IoCompletionEffects {
    Loop& f;explicit Effects(Loop& f):f(f){}
    std::unique_ptr<s::AsyncSendOperation> operation;s::DispatcherOperationBinding packet;
    s::AsyncSendOperation& project_async_send(s::OperationStorage&)override;
    void construct_environment_023dd0(s::IoHandlerEnvironment&,std::uintptr_t)override;
    void finish_write_025a70(s::SendHandler&,s::Error&)override;
    s::Category misc_category_0285b0()override{return {0x2222};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{r::unexpected();}
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override;
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{r::unexpected();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{r::unexpected();}
};
struct Finish:s::WriteFinishApi {
    Loop& f;explicit Finish(Loop& f):f(f){}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
    [[noreturn]]void invalid_parameter_05c974()override{r::unexpected();}
    void next_write_02a500(std::uintptr_t)override;
    s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{r::unexpected();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{r::unexpected();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{r::unexpected();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{r::unexpected();}void flush_00e1e0(s::WriteFinishStream&)override{r::unexpected();}
};
struct App:s::NativeMessageApi {
    message_test::Fixture& m;App(s::ResponseDispatchContext& c,s::NativeMessageContext& core,message_test::Fixture& m):NativeMessageApi(c,core),m(m){}
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{m.api.destroy_empty_message_00aa50(value);}
    void cleanup_exit_waits_04a47c()override{r::unexpected();}
};
struct Loop {
    native_message_test::Fixture native;core_test::Fixture& core=native.core;container_test::ContainerFixture& containers=core.containers;owned_test::OwnedFixture& ownership=containers.ownership;response_test::Fixture& f=containers.f;r::Fixture io;Send send{*this};Effects effects{*this};Finish finish{*this};
    s::AsyncSendService service{&io.entry};s::SendSocketImplementation implementation{77,0,0,nullptr};
    std::array<s::SendStreamBinding,1> streams{{{0x12010,&service,&implementation,0x12078}}};
    s::ComposedWriteContext write{io.environment,io.worker,io.memory,send,io.completion,io.errors,effects,streams};
    s::QueuedWriteOwner owner{0x12000,&f.m.base.owner,&f.queue,&streams[0]};
    std::array<s::QueuedWriteOwnerBinding,1> owners{{{0x12000,&owner}}};
    s::WriteFinishContext finished{effects,finish,f.owners,{},containers.bytes.headers};
    append_test::Api append_api{native.strings,native.api};s::NativeMessageContext append_context{native.bindings,core.show,native.strings,native.mutation,native.locals,append_api};
    App app{containers.response,append_context,f.m};s::MessageDispatchContext message{f.m.frame,app,f.m.owners,f.m.copies,f.m.closures};
    std::vector<std::string> received;std::string current;std::vector<std::uint32_t> lengths;
    unsigned sends=0,completions=0,finishes=0;bool finish_pending=false;
    Loop(){f.queued.owners=owners;f.queued.write=&write;f.invoke_borrowed=true;f.m.base.owner.strong_08=1;f.m.base.projection.object_00=0x12000;
        f.m.source.executor_28=&f.first_executor;f.m.copies[2]={&f.live,&message_copy};
        f.m.copy_hook=[&](auto& to,auto&,unsigned n){if(n==1)to.operations_10=&f.live;};
    }
    void dispatch(std::string payload,bool owned,unsigned mode=0,bool event=false,bool invoke=true){
        response_test::active=&f;active_owned=&ownership;message_test::running=&f.m;f.moves=f.queries=f.m.copy_calls=0;
        active_container=&containers;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed_containers;f.take_owned=true;f.invoke_owned=invoke;
        core.effects.answer=payload;core.encrypted=mode==1;core.show=0;auto input=request_xml+std::string("\0tail",5);
        if(mode==2){input.insert(4," version=\"\"");native.api.codes={7};native.api.errors={1};}
        auto expected=input;if(mode==2)expected.erase(expected.find("version=\"\""),10);
        native.seed(mode==1||mode==3?hex(input):input);
        if(event)native.event(std::string_view("event\0tail",10));
        core.effects.handling=[&,expected](){check(core.input()==expected,"native033d70 callback sees decoded/erased request before handler");};
        s::dispatch_message_028d30(0x12000,f.m.request.value,message);
        check(f.m.request.value.size_10==0&&f.m.request.live_blocks()==0,"native033d70 feeds actual028d30 cleanup with original live request");
    }
    void drain(){
        for(unsigned steps=0;effects.operation&&steps<32;++steps){
            auto operation=std::move(effects.operation);auto handler=operation->io.handler;operation->io.handler.control_98=nullptr;
            const auto bytes=operation->buffer_48.size;
            // Controlled completion adapter, not native027a20/IOCP/worker.
            // Actual storage release and021a80/0261d0/025a70 are executed.
            s::release_io_storage_026a00(operation->io.storage,0xd0,io.environment.storage,io.worker,io.memory);
            operation.reset();io.entry.work_count_30.fetch_sub(1);
            handler.error={0,{0x2222}};handler.transferred=bytes;++completions;
            s::invoke_send_021a80(handler,effects);
            auto* c=handler.control_98;if(c&&c->strong_08.fetch_sub(1)==1){c->dispose_slot_00();if(c->weak_0c.fetch_sub(1)==1)c->delete_slot_08();}
        }
        check(!effects.operation&&!f.queue.count_e8&&!f.queue.offset_e0&&f.m.base.owner.strong_08==1,"actual send/finish/next loop drains queue and restores initial shared reference");
        check(io.memory.allocations.size()==io.memory.frees&&!io.entry.work_count_30,"actual send storage allocation/release balanced under controlled consumer");
    }
};
s::AsyncSendOperation& Effects::project_async_send(s::OperationStorage& storage){operation=std::make_unique<s::AsyncSendOperation>(storage);packet={&operation->io.operation,{},0,&operation->io.operation.overlapped};operation->packet=&packet;return *operation;}
void Effects::construct_environment_023dd0(s::IoHandlerEnvironment&,std::uintptr_t allocator){check(allocator==0x12078,"real queued-write/send forwards owner+78 allocator identity");}
void Effects::finish_write_025a70(s::SendHandler& handler,s::Error& error){++f.finishes;f.received.push_back(f.current);f.current.clear();s::finish_write_025a70(handler,error,f.finished);}
void Effects::send_body_0261d0(s::SendHandler& handler,s::Error& error,std::uint64_t n,std::uint32_t flag){s::composed_write_0261d0(handler,error,n,flag,f.write);}
void Finish::free_sized_04b0f8(std::uintptr_t data,std::uint64_t n){f.containers.bytes.free_sized_04b0f8(data,n);}
void Finish::next_write_02a500(std::uintptr_t owner){s::start_queued_write_02a500(owner,f.f.queued);}
s::AsyncSendResult Send::send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& overlapped){
    check(socket==77&&!flags&&!overlapped.Internal&&!overlapped.InternalHigh&&!overlapped.Pointer&&!overlapped.hEvent,"actual021fd0 invokes injected AsyncSendApi with initialized OVERLAPPED");
    const auto index=(f.f.queue.map_size_d8-1)&f.f.queue.offset_e0;const auto* head=f.f.queue.map_d0[index];const auto data=head->size_10?s::receive_buffer_address(*head):0;
    const auto address=reinterpret_cast<std::uintptr_t>(buffer.buf);check(address>=data&&address-data<=head->size_10&&buffer.len<=65536&&buffer.len<=head->size_10-(address-data),"actual send pointer/progress and64KiB cap stay within queued source string");
    const auto text=f.containers.bytes.text(*head);f.current+=text.substr(address-data,buffer.len);f.lengths.push_back(buffer.len);++f.sends;return {-1,997,0};
}
std::string answer(std::string_view value,bool encrypted){
    std::string result="<LSX>\n    <Response id=\"bridge\" sender=\"fixture\">\n        <BridgeAnswer value=\""+std::string(value)+"\"/>\n    </Response>\n</LSX>\n";
    if(encrypted)result=hex(result);result.push_back(0);return result;
}
void direct_closed(){
    for(unsigned n:{15U,31U,4095U}){native_message_test::Fixture f;append_test::Api api{f.strings,f.api};s::NativeMessageContext context{f.bindings,f.core.show,f.strings,f.mutation,f.locals,api};f.seed(request_xml);f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(n,'Q'));return out;};s::process_native_message_033d70(0x120f0,f.core.response,f.core.request(),context);check(f.output()==std::string(n,'Q')+std::string(1,0)&&!f.api.nul_grows&&f.core.containers.bytes.copies.back().count==n,"plain response at exact capacity reaches actual012350 COPY/character/NUL through final provider");}
    {native_message_test::Fixture f;append_test::Api api{f.strings,f.api};s::NativeMessageContext context{f.bindings,f.core.show,f.strings,f.mutation,f.locals,api};f.core.encrypted=1;f.seed(hex(request_xml));f.event(std::string(5000,'E'));f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(15,'Q'));return out;};s::process_native_message_033d70(0x120f0,f.core.response,f.core.request(),context);const auto& copies=f.core.containers.bytes.copies;check(f.output()==native_message_test::framed(std::string(15,'Q'),true)+native_message_test::framed(std::string(5000,'E'),true)&&!f.api.append_grows&&copies[copies.size()-2].count==31&&copies.back().count==10001&&!f.api.queue.count_20&&f.api.unlocks==1,"actual007e30 two COPYs append full encoded event/NUL while locked; original/copy event released");}
    {native_message_test::Fixture f;append_test::Api api{f.strings,f.api};s::NativeMessageContext context{f.bindings,f.core.show,f.strings,f.mutation,f.locals,api};f.seed(request_xml);f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(15,'Q'));f.core.containers.f.m.response.api.null_allocate=true;return out;};response_test::rejects<s::UnsupportedInput>([&]{s::process_native_message_033d70(0x120f0,f.core.response,f.core.request(),context);});check(f.locals.response.size_10==16&&f.locals.response.capacity_18==31&&f.locals.response.storage_00[0]=='Q'&&!f.api.destroys&&f.output().empty(),"closed012350 small NULL failure exposes actual new metadata before bounded COPY, no output move/document cleanup");}
}
void integration(){
    for(bool owned:{false,true})for(unsigned mode:{0U,1U,2U,3U}){Loop l;std::vector<std::string> payloads={"alpha","",std::string(150000,'Z')};for(unsigned i=0;i<10;++i)payloads.push_back("queued-"+std::to_string(i));
        const bool encrypted=mode==1||mode==3;std::vector<std::string> expected;std::vector<std::uint32_t> chunks;unsigned i=0;
        for(const auto& payload:payloads){auto result=answer(payload,encrypted);if(encrypted&&i==2)result+=native_message_test::framed(std::string("event\0tail",10),true);expected.push_back(result);auto remaining=result.size();do{const auto n=std::min<std::size_t>(65536,remaining);chunks.push_back(static_cast<std::uint32_t>(n));remaining-=n;}while(remaining);l.dispatch(payload,owned,mode,encrypted&&i==2);++i;}
        check(l.f.queue.count_e8==13&&l.f.queue.map_size_d8==16&&l.containers.map_api.copies.size()==4&&l.sends==1,"actual033d70 source bridge feeds13 messages and two actual deque growths");
        l.drain();check(l.received==expected&&l.lengths==chunks&&l.finishes==13,"full XML/hex/event embedded-NUL FIFO and independent64KiB chunk oracle after actual033d70 and N1-N8/send/finish");
        check(l.native.api.constructs==13&&l.native.api.destroys==13&&l.native.api.dispatches==13&&l.core.effects.commands.size()==13,"same typed normal core covers all messages with controlled032d00/XML lower provider");
        check(!l.core.strings.reads&&!l.core.strings.replaces&&!l.core.strings.constructs&&!l.f.m.api.processes,"historical N5 semantic033d70 and response-constructor adapter are not selected");
        check(l.native.api.parses==((mode==2||mode==3)?26U:13U)&&l.native.api.decrypts==(encrypted?13U:0U)&&l.native.api.locks==unsigned(encrypted)&&l.native.api.unlocks==unsigned(encrypted),"direct retry/autodetection and event lock counts; no double semantic processing");
        check(!l.native.api.nul_grows&&!l.native.api.append_grows,"N9 controlled growth substitutes bypassed by new final012350/007e30 source bridge");
        check(l.f.m.request.live_blocks()==0&&l.f.m.response.live_blocks()==0&&!l.native.api.queue.count_20,"request/native output/event sources freed after separate queued COPY and actual send finish");
    }
    for(unsigned mode:{0U,1U,2U,3U}){Loop l;l.dispatch("deferred",true,mode,false,false);check(!l.f.queue.count_e8&&l.ownership.held.size()==1&&l.f.m.request.live_blocks()==0&&l.f.m.response.live_blocks()==1,"owned task retains native result after caller request/local destruction");auto* task=l.ownership.held.front();l.ownership.held.clear();task->call_slot_00(1);l.drain();check(l.received==std::vector<std::string>{answer("deferred",mode==1||mode==3)}&&l.f.m.response.live_blocks()==0,"deferred native output moves to queue COPY then is freed");}
    {Loop l;l.native.seed(hex(request_xml));l.core.encrypted=1;l.native.api.decrypt=[](auto&){throw native_message_test::Failure{88};return false;};try{s::dispatch_message_028d30(0x12000,l.core.request(),l.message);throw std::runtime_error("expected callback failure");}catch(const native_message_test::Failure& e){check(e.code==88&&l.core.input()==request_xml&&!l.effects.operation&&l.f.m.request.live_blocks()==1&&l.f.m.base.owner.strong_08==1,"outer028d30 callback failure preserves actual decoded input and performs no invented post-call cleanup");}}
}
}
int main(){try{direct_closed();integration();std::cout<<"PASS: "<<checks<<" socket append growth integration checks; actual012350/007e30 final source bridges in033d70 and N1-N9 queue/send/finish; lower XML/cipher/log/allocator providers and native ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
