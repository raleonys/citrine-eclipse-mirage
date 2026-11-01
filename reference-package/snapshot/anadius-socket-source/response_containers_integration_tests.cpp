#include "response_containers_test_fixture.hpp"
using namespace container_test;
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
struct App:s::RecoveredResponseDispatchApi {
    message_test::Fixture& m;App(s::ResponseDispatchContext& c,message_test::Fixture& m):RecoveredResponseDispatchApi(c),m(m){}
    void process_message_033d70(std::uintptr_t id,s::ReceiveBufferString& out,s::ReceiveBufferString& in)override{m.api.process_message_033d70(id,out,in);}
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{m.api.destroy_empty_message_00aa50(value);}
    void cleanup_exit_waits_04a47c()override{r::unexpected();}
};
struct Loop {
    ContainerFixture containers;owned_test::OwnedFixture& ownership=containers.ownership;response_test::Fixture& f=containers.f;r::Fixture io;Send send{*this};Effects effects{*this};Finish finish{*this};
    s::AsyncSendService service{&io.entry};s::SendSocketImplementation implementation{77,0,0,nullptr};
    std::array<s::SendStreamBinding,1> streams{{{0x12010,&service,&implementation,0x12078}}};
    s::ComposedWriteContext write{io.environment,io.worker,io.memory,send,io.completion,io.errors,effects,streams};
    s::QueuedWriteOwner owner{0x12000,&f.m.base.owner,&f.queue,&streams[0]};
    std::array<s::QueuedWriteOwnerBinding,1> owners{{{0x12000,&owner}}};
    s::WriteFinishContext finished{effects,finish,f.owners,{},containers.bytes.headers};
    App app{containers.response,f.m};s::MessageDispatchContext message{f.m.frame,app,f.m.owners,f.m.copies,f.m.closures};
    std::vector<std::string> received;std::string current;std::vector<std::uint32_t> lengths;
    unsigned sends=0,completions=0,finishes=0;bool finish_pending=false;
    Loop(){f.queued.owners=owners;f.queued.write=&write;f.invoke_borrowed=true;f.m.base.owner.strong_08=1;f.m.base.projection.object_00=0x12000;
        f.m.source.executor_28=&f.first_executor;f.m.copies[2]={&f.live,&message_copy};
        f.m.copy_hook=[&](auto& to,auto&,unsigned n){if(n==1)to.operations_10=&f.live;};
    }
    void dispatch(std::string payload,bool owned,bool take=true,bool invoke=true){
        response_test::active=&f;active_owned=&ownership;message_test::running=&f.m;f.moves=f.queries=f.m.copy_calls=0;
        active_container=&containers;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed_containers;f.take_owned=take;f.invoke_owned=invoke;
        f.m.seed(f.m.response,payload,std::max<std::uint64_t>(15,payload.size()));f.m.seed(f.m.request,"request");containers.bytes.refresh();
        s::dispatch_message_028d30(0x12000,f.m.request.value,message);
        check(f.m.request.value.size_10==0,"actual N1 caller consumes input after actual N2 executor dispatch");
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
void integration(){
    for(bool owned:{false,true}){Loop l;std::vector<std::string> responses={std::string("one\0tail",8),"",std::string(150000,'Z')+std::string("\0",1)};for(unsigned i=0;i<10;++i)responses.push_back("queued-"+std::to_string(i));
        for(const auto& response:responses)l.dispatch(response,owned);
        check(l.f.queue.count_e8==13&&l.f.queue.map_size_d8==16&&l.containers.map_api.copies.size()==4&&l.effects.operation&&l.sends==1,"multiple N1 responses queue behind first pending send; no duplicate first-start");
        l.drain();check(l.received==responses&&l.finishes==13&&l.lengths==std::vector<std::uint32_t>({8,0,65536,65536,18929,8,8,8,8,8,8,8,8,8,8}),"independent13-message full payload oracle includes embedded NUL, zero-length response and150001-byte chunking");
        check(std::find(l.f.trace.begin(),l.f.trace.end(),"string-copy")==l.f.trace.end()&&l.containers.queue_api.allocations==13,"both invoke branches use real009eb0 and dynamic32-byte queue cell provider");
        check(l.f.m.base.owner.strong_08==1&&!l.f.m.base.owner.disposes&&!l.f.m.base.owner.deletes,"actual N1/N2/send ownership balances across reentrant next-write");
    }
    {Loop l;l.dispatch("deferred",true,true,false);check(l.f.queue.count_e8==0&&l.ownership.held.size()==1&&l.f.m.base.owner.strong_08==2&&!l.effects.operation,"owned scheduler may retain response without immediate queue insertion");
        auto* task=l.ownership.held.front();l.ownership.held.clear();task->call_slot_00(1);l.drain();check(l.received==std::vector<std::string>{"deferred"},"later owned consumer invokes actual0219d0/write queue source after closure release");}
    {Loop l;l.dispatch("discarded",true,false,false);check(l.f.queue.count_e8==0&&!l.effects.operation&&l.f.m.base.owner.strong_08==1&&l.ownership.memory.frees.size()==1,"unclaimed owned functor receives flag0 and discards without send");}
}
}
int main(){try{integration();std::cout<<"PASS: "<<checks<<" socket response containers integration checks; actual009eb0/026670 plus N1/N2/N3/queued-send/finish source;13-message FIFO, two map growths; controlled allocation/constructor/completion and injected send\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
