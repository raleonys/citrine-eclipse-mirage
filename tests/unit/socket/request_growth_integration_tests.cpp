// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/request_growth_test_fixture.hpp"
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
struct App:s::CompatibleMessageCoreApi {
    message_test::Fixture& m;App(s::ResponseDispatchContext& c,s::MessageCoreContext& core,message_test::Fixture& m):CompatibleMessageCoreApi(c,core),m(m){}
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{m.api.destroy_empty_message_00aa50(value);}
    void cleanup_exit_waits_04a47c()override{r::unexpected();}
};
struct Loop {
    core_test::Fixture core;container_test::ContainerFixture& containers=core.containers;owned_test::OwnedFixture& ownership=containers.ownership;response_test::Fixture& f=containers.f;r::Fixture io;Send send{*this};Effects effects{*this};Finish finish{*this};
    s::AsyncSendService service{&io.entry};s::SendSocketImplementation implementation{77,0,0,nullptr};
    std::array<s::SendStreamBinding,1> streams{{{0x12010,&service,&implementation,0x12078}}};
    s::ComposedWriteContext write{io.environment,io.worker,io.memory,send,io.completion,io.errors,effects,streams};
    s::QueuedWriteOwner owner{0x12000,&f.m.base.owner,&f.queue,&streams[0]};
    std::array<s::QueuedWriteOwnerBinding,1> owners{{{0x12000,&owner}}};
    s::WriteFinishContext finished{effects,finish,f.owners,{},containers.bytes.headers};
    App app{containers.response,core.context,f.m};s::MessageDispatchContext message{f.m.frame,app,f.m.owners,f.m.copies,f.m.closures};
    growth_test::RequestBytes request_bytes{containers};s::ReceiveBufferContext growth_buffer{containers.bytes.memory,request_bytes};growth_test::Api mutations{growth_buffer};s::RequestMutationContext mutation{growth_buffer,mutations};
    std::vector<std::string> received;std::string current;std::vector<std::uint32_t> lengths;
    unsigned sends=0,completions=0,finishes=0;bool finish_pending=false;
    Loop(){f.queued.owners=owners;f.queued.write=&write;f.invoke_borrowed=true;f.m.base.owner.strong_08=1;f.m.base.projection.object_00=0x12000;
        f.m.source.executor_28=&f.first_executor;f.m.copies[2]={&f.live,&message_copy};
        f.m.copy_hook=[&](auto& to,auto&,unsigned n){if(n==1)to.operations_10=&f.live;};
    }
    void dispatch(std::string payload,bool owned,bool encoded=false,bool alias=false,bool large=false,bool invoke=true){
        response_test::active=&f;active_owned=&ownership;message_test::running=&f.m;f.moves=f.queries=f.m.copy_calls=0;
        active_container=&containers;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed_containers;f.take_owned=true;f.invoke_owned=invoke;
        core.effects.answer=payload;core.encrypted=0;const auto input=request_xml+std::string("\0tail",5);core.seed(encoded?hex(input):input);
        if(encoded)s::decode_request_hex_03ce30(core.request(),containers.strings);
        const auto prior=s::receive_buffer_address(core.request());const auto allocations=f.m.request.api.allocations.size();
        std::string addition;std::uintptr_t source=0;
        if(alias){const auto at=input.find(" id=");const auto end=input.find('"',at+5);addition=input.substr(at,end-at+1);source=prior+at;}
        else{addition=" pad=\""+std::string(large?150000:128,'K')+"\"";source=0x9900003;request_bytes.borrowed={source,std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(addition.data()),addition.size())};request_bytes.refresh();}
        auto expected=input;expected.insert(4,addition);
        s::replace_request_034730(core.request(),4,0,source,addition.size(),mutation);
        check(core.input()==expected&&s::receive_buffer_address(core.request())!=prior&&f.m.request.api.allocations.size()==allocations+1,"actual034730 final bridge invokes02b940 and reallocates external/self input before semantic callbacks");
        request_bytes.borrowed={};request_bytes.refresh();
        core.effects.handling=[&,expected](){check(core.input()==expected,"new grown prefix and complete binary request visible to semantic handler callback");};
        s::dispatch_message_028d30(0x12000,f.m.request.value,message);
        check(f.m.request.value.size_10==0,"actual028d30 outer cleanup consumes request in same allocation pool");
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
void integration(){
    for(bool owned:{false,true})for(unsigned mode:{0U,1U,2U}){Loop l;std::vector<std::string> payloads={"alpha","",std::string(150000,'Z')};for(unsigned i=0;i<10;++i)payloads.push_back("queued-"+std::to_string(i));
        std::vector<std::string> expected;std::vector<std::uint32_t> chunks;unsigned i=0;
        for(const auto& payload:payloads){expected.push_back(answer(payload,false));auto remaining=expected.back().size();do{const auto n=std::min<std::size_t>(65536,remaining);chunks.push_back(static_cast<std::uint32_t>(n));remaining-=n;}while(remaining);l.dispatch(payload,owned,mode==1,mode==2,mode!=2&&i==2);++i;}
        check(l.f.queue.count_e8==13&&l.f.queue.map_size_d8==16&&l.containers.map_api.copies.size()==4&&l.sends==1,"native request growth feeds13 messages through actual two queue map growths");
        l.drain();check(l.received==expected&&l.lengths==chunks&&l.finishes==13,"full XML/NUL FIFO and independent64KiB chunk oracle after native request growth and N1-N6/send/finish");
        check(l.core.effects.commands.size()==13&&l.f.m.api.processes==0&&l.core.strings.constructs==13&&!l.core.strings.replaces,"permanent source bridge producer with no deferred request writeback");
        check(l.f.m.request.api.allocations.size()==payloads.size()&&l.f.m.request.api.frees.size()==payloads.size()*2&&l.f.m.request.live_blocks()==0,"seeded original plus allocated grown request both freed through source growth and actual caller cleanup");
    }
    {Loop l;l.dispatch("deferred",true,false,false,true,false);check(!l.f.queue.count_e8&&l.ownership.held.size()==1&&l.f.m.request.live_blocks()==0,"large grown input destroyed while deferred response owns independent source");auto* task=l.ownership.held.front();l.ownership.held.clear();task->call_slot_00(1);l.drain();check(l.received==std::vector<std::string>{answer("deferred",false)},"deferred response survives original and grown request destruction");}
    {Loop l;l.core.seed(request_xml);std::string addition=" flag=\"growth\"";const auto source=0x9900003;l.request_bytes.borrowed={source,std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(addition.data()),addition.size())};l.request_bytes.refresh();auto expected=request_xml;expected.insert(4,addition);s::replace_request_034730(l.core.request(),4,0,source,addition.size(),l.mutation);l.request_bytes.borrowed={};l.request_bytes.refresh();l.core.effects.handling=[](){throw growth_test::Failure{88};};try{s::dispatch_message_028d30(0x12000,l.core.request(),l.message);r::unexpected();}catch(const growth_test::Failure& e){check(e.code==88&&l.core.input()==expected&&!l.core.strings.constructs&&!l.effects.operation&&l.f.m.base.owner.strong_08==1,"grown prefix survives later handler exception; no native033d70 full orchestration claim");}}
}
}
int main(){try{integration();std::cout<<"PASS: "<<checks<<" socket request growth integration checks; actual034730/02b940 provider closure, external/self source and aligned growth before controlled caller and N1-N6/send/finish; native033d70 orchestration remains open\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
