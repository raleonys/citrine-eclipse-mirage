// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/composed_read_test_fixture.hpp"
#include "laureondo/net/transport_io_completion_dispatch.hpp"
#include <deque>
using namespace read_test;
namespace {
struct Fixture;
Fixture* running=nullptr;
struct Table:s::IoHandlerVtable {
    bool empty;explicit Table(bool e):IoHandlerVtable(e?0x777:0x123),empty(e){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override{}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{to.payload_00=empty?std::array<std::uintptr_t,2>{}:from.payload_00;to.state_18=empty?0:from.state_18;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{return 0x5678;}
};
struct Types:s::IoEnvironmentTypeApi {bool equal=true;std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)override{return equal?0:1;}};
std::uintptr_t query(){return 0x1000;}
void factory(s::IoHandlerEnvironment&,std::uintptr_t,s::IoEnvironmentConstructorScratch&);
void prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&);
void borrowed(s::IoHandlerEnvironment&,s::BorrowedIoHandler&);
void owned(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&);
struct Node {
    std::array<std::byte,1025> bytes{};s::OperationStorage storage;
    std::unique_ptr<s::AsyncReceiveOperation> operation;s::DispatcherOperationBinding packet;
    std::unique_ptr<s::ReceiveHandlerTask> task;
    explicit Node(std::uintptr_t identity):storage{identity,bytes}{}
};
struct Memory:s::HandlerDispatchMemory {
    Fixture& f;std::array<std::unique_ptr<Node>,64> nodes;unsigned allocated=0,freed=0;
    explicit Memory(Fixture& f):f(f){}
    Node& at(s::OperationStorage&);Node& at(OVERLAPPED*);
    s::OperationStorage& allocate_04b0bc(std::uint64_t)override;
    void free_04b788(s::OperationStorage&)override;
    s::PostedOperation& project_posted(s::OperationStorage&)override{r::unexpected();}
    s::ReceiveHandlerTask& project_receive_task(s::OperationStorage&,s::HandlerTaskContext&)override;
    s::SendHandlerTask& project_send_task(s::OperationStorage&,s::HandlerTaskContext&)override{r::unexpected();}
};
struct Packet {OVERLAPPED* identity=nullptr;s::Error error;std::uint32_t bytes=0;bool key2=false;};
struct Dispatcher:r::Completion {
    Fixture& f;explicit Dispatcher(Fixture& f):f(f){}
    std::int32_t post(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override;
};
struct NoSend:s::AsyncSendApi {s::AsyncSendResult send(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override{r::unexpected();}};
struct NoFinish:s::WriteFinishApi {
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{r::unexpected();}[[noreturn]]void invalid_parameter_05c974()override{r::unexpected();}
    void next_write_02a500(std::uintptr_t)override{r::unexpected();}s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{r::unexpected();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{r::unexpected();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{r::unexpected();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{r::unexpected();}void flush_00e1e0(s::WriteFinishStream&)override{r::unexpected();}
};
struct Reader:s::ComposedReadEffects {
    using ComposedReadEffects::ComposedReadEffects;
    s::AsyncSendOperation& project_async_send(s::OperationStorage&)override{r::unexpected();}
    s::Category misc_category_0285b0()override{return {0x3333};}
};
struct ReceiveEffects:s::AsyncReceiveEffectsAdapter {
    Memory& memory;
    ReceiveEffects(Memory& memory,s::IoHandlerSentinels sentinels,s::IoEnvironmentConstructorContext& constructor,std::span<s::IoEnvironmentSourceBinding> sources)
        :AsyncReceiveEffectsAdapter(sentinels,constructor,sources),memory(memory){}
    s::AsyncReceiveOperation& project_async_receive(s::OperationStorage&)override;
};
struct Routing:s::IoDispatcherEffects {
    using IoDispatcherEffects::IoDispatcherEffects;
    void complete_unknown(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{r::unexpected();}
    std::uintptr_t capture_tls_slot()override{r::unexpected();}std::int32_t thread_epoch(std::uintptr_t)override{r::unexpected();}
    void guard_header_04b838(std::int32_t&)override{r::unexpected();}void register_atexit_04b770(std::uintptr_t)override{r::unexpected();}void guard_footer_04b7cc(std::int32_t&)override{r::unexpected();}
    std::uint32_t read_stop_34(s::IocpSchedulerEntry&)override{r::unexpected();}
    std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>&,const std::array<std::uintptr_t,2>&)override{r::unexpected();}
    [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{r::unexpected();}
};
int WSAAPI sdk_receive(SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);
int WSAAPI sdk_error();
struct Fixture {
    b::Fixture buffer;r::Gate gate;CRITICAL_SECTION cs{};r::Entry entry{gate,cs};r::Worker worker;r::Errors errors;std::uint32_t index=17;
    std::array<s::OperationStorageBinding,64> storage{};std::array<s::IoCompletionBinding,64> io{};
    Memory memory{*this};Table table{false},empty{true};s::IoHandlerExecutor executor{&prepare};Types types;
    s::IoHandlerEnvironment source{{11,12},&table,0,0x456,&executor};
    std::array<s::IoEnvironmentTypeBinding,1> queries{{{0x456,&query}}};
    std::array<s::IoEnvironmentFactoryBinding,1> factories{{{&executor,&factory}}};
    s::IoEnvironmentConstructorContext constructor{types,queries,factories};
    std::array<s::IoEnvironmentSourceBinding,1> sources{{{0x123a8,&source}}};
    s::HandlerSchedule schedule{&owned,nullptr};std::array<s::HandlerScheduleBinding,1> schedules{{{0x456,&schedule}}};
    Dispatcher dispatcher{*this};NoSend send;NoFinish finish;
    Reader effects{environment(),worker,memory,schedules,send,dispatcher,errors,{},constructor,sources,finish,{},{},{}};
    ReceiveEffects receive_effects{memory,environment().sentinels,constructor,sources};
    s::NativeAsyncReceiveApi sdk{{&sdk_receive,&sdk_error}};
    s::AsyncReceiveContext async{environment(),worker,memory,sdk,dispatcher,errors,receive_effects};
    s::AsyncReceiveService service{&entry};r::Ref owner,weak;s::ReceiveSocketImplementation implementation{77,0x10,0xabc,&weak};
    std::array<s::ReceiveStringBinding,1> strings{{{0x5000,&buffer.value}}};
    std::array<s::ReceiveStreamBinding,1> streams{{{0x12340,&service,&implementation}}};
    read_test::Api frame;s::ComposedReadContext read{buffer.context,async,effects,frame,strings,streams};
    Routing routing{{},io,environment(),worker,memory,errors,effects};
    std::deque<Packet> packets;std::vector<std::string> chunks;std::vector<std::uint32_t> requests;std::vector<int> sdk_order;
    std::size_t chunk=0;int result=-1,last=997;bool early=false,fail_post=false;unsigned borrowed_calls=0,owned_calls=0;
    Fixture(){running=this;entry.completion.port=0xabcdef;effects.bind_read_context(read);frame.finish=[&]{check(owner.disposes==0,"frame callback runs before final strong release");};}
    s::IoCompletionEnvironment environment(){return {{index,storage},{&empty,0x846e8,&executor,0x777}};}
    void start(std::string initial,std::uint64_t maximum,std::uint8_t delimiter){
        buffer.seed(std::max<std::uint64_t>(15,initial.size()),initial.size());
        auto data=buffer.memory.view(s::receive_buffer_address(buffer.value),initial.size());std::copy(initial.begin(),initial.end(),data.begin());
        s::ReceiveHandler handler;handler.words_60_78={0x12340,0x5000,UINT64_MAX,maximum};handler.byte_80=delimiter;handler.words_88_98={0,0,0xbeef};handler.object_a0=0xbeef;handler.control_a8=&owner;
        s::Error error{0,{0x2222}};effects.receive_body_025c40(handler,error,0,1);
        check(!handler.control_a8,"actual initial receive moved strong ownership");
    }
    void drain(){
        for(unsigned steps=0;steps<32&&frame.finishes==0;++steps){
            Packet packet;
            if(!packets.empty()){packet=packets.front();packets.pop_front();}
            else if(auto* operation=entry.completion.head){
                entry.completion.head=operation->next;if(!entry.completion.head)entry.completion.tail=nullptr;
                packet.identity=&operation->overlapped;packet.key2=true;
            }else throw std::runtime_error("read stalled without completion");
            auto& node=memory.at(packet.identity);auto* operation=&node.operation->io.operation;
            if(packet.key2){packet.error=node.packet.packet_error;packet.bytes=node.packet.transferred_14;}
            routing.complete(&entry,*operation,packet.error,packet.bytes);
        }
        check(frame.finishes==1&&packets.empty()&&!entry.completion.head,"exactly one completed frame without pending packets");
    }
};
Node& Memory::at(s::OperationStorage& p){for(auto& n:nodes)if(n&&&n->storage==&p)return *n;r::unexpected();}
Node& Memory::at(OVERLAPPED* identity){for(auto& n:nodes)if(n&&n->operation&&&n->operation->io.operation.overlapped==identity)return *n;r::unexpected();}
s::OperationStorage& Memory::allocate_04b0bc(std::uint64_t count){
    check(count<=1025,"bounded receive/task allocation");++allocated;
    for(std::size_t i=0;i<nodes.size();++i)if(!nodes[i]){nodes[i]=std::make_unique<Node>(0x100000+i*0x1000);f.storage[i]={nodes[i]->storage.identity,&nodes[i]->storage};return nodes[i]->storage;}
    r::unexpected();
}
void Memory::free_04b788(s::OperationStorage& p){
    ++freed;for(std::size_t i=0;i<nodes.size();++i)if(nodes[i]&&&nodes[i]->storage==&p){f.io[i]={};f.storage[i]={};nodes[i].reset();return;}r::unexpected();
}
s::ReceiveHandlerTask& Memory::project_receive_task(s::OperationStorage& p,s::HandlerTaskContext& context){auto& n=at(p);n.task=std::make_unique<s::ReceiveHandlerTask>(p,context);return *n.task;}
s::AsyncReceiveOperation& ReceiveEffects::project_async_receive(s::OperationStorage& p){
    auto& n=memory.at(p);n.operation=std::make_unique<s::AsyncReceiveOperation>(p);n.packet={&n.operation->io.operation,{},0,&n.operation->io.operation.overlapped};n.operation->packet=&n.packet;
    for(std::size_t i=0;i<memory.nodes.size();++i)if(memory.nodes[i].get()==&n)memory.f.io[i]={&n.operation->io.operation,&n.operation->io,nullptr};return *n.operation;
}
std::int32_t Dispatcher::post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* identity){
    check(port==0xabcdef&&bytes==0&&key==2,"actual028b50/028c00 uses key2");++posts;
    if(f.fail_post)return 0;f.packets.push_back({identity,{},0,true});return 1;
}
void factory(s::IoHandlerEnvironment& e,std::uintptr_t query_value,s::IoEnvironmentConstructorScratch&){
    check(query_value==0x5678,"constructor factory query");auto& f=*running;e={{21,22},&f.table,1,0x456,&f.executor};
}
void prepare(s::IoHandlerEnvironment& e,std::uintptr_t query_value,s::IoPrepareScratch&){
    check(query_value==0x5678,"completion cached prepare query");auto& f=*running;e={{31,32},&f.table,0,0x456,&f.executor};
}
void borrowed(s::IoHandlerEnvironment&,s::BorrowedIoHandler& h){++running->borrowed_calls;h.invoke(running->effects);}
void owned(s::IoHandlerEnvironment&,s::OwnedHandlerTask*& slot){++running->owned_calls;auto* task=slot;slot=nullptr;task->call_slot_00(1);}
int WSAAPI sdk_receive(SOCKET socket,LPWSABUF buffer,DWORD count,LPDWORD bytes,LPDWORD flags,LPWSAOVERLAPPED overlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE completion){
    auto& f=*running;f.sdk_order.push_back(1);check(socket==77&&count==1&&*bytes==0&&*flags==0&&!completion,"actual SDK receive arguments");f.requests.push_back(buffer->len);
    if(f.chunk>=f.chunks.size())throw std::runtime_error("unexpected extra receive");const auto payload=f.chunks[f.chunk++];
    check(payload.size()<=buffer->len,"scripted chunk fits actual request");auto target=f.buffer.memory.view(reinterpret_cast<std::uintptr_t>(buffer->buf),payload.size());std::copy(payload.begin(),payload.end(),target.begin());*bytes=static_cast<DWORD>(payload.size());*flags=0x1234;
    if(f.result==0||f.last==997){
        if(f.early){auto& node=f.memory.at(overlapped);node.packet.packet_error={0,{0x2222}};node.packet.transferred_14=*bytes;node.operation->io.operation.queued=1;}
        else f.packets.push_back({overlapped,{0,{0x2222}},*bytes,false});
    }
    return f.result;
}
int WSAAPI sdk_error(){running->sdk_order.push_back(2);return running->last;}
void scenarios(){
    struct Case {std::string initial;std::vector<std::string> chunks;std::uint64_t maximum;std::uint8_t delimiter;int result,last,error;std::uint64_t matched;std::string final;};
    const Case cases[]={
        {"ready\ntrail",{},4096,10,-1,997,0,6,"ready\ntrail"},
        {"ab",{"c","d\nTAIL"},4096,10,-1,997,0,5,"abcd\nTAIL"},
        {"",{""},4096,10,0,1234,2,0,""},
        {"abcdef",{},6,10,-1,997,3,0,"abcdef"},
        {"",{"xx"},4096,10,-1,64,10054,0,"xx"},
        {std::string("p\0",2),{},4096,0,-1,997,0,2,std::string("p\0",2)},
        {"a",{std::string("b\0TAIL",6)},4096,0,-1,997,0,3,std::string("ab\0TAIL",7)},
        {"",{"a"},1,10,-1,997,3,0,"a"},
        {"",{},0,10,-1,997,3,0,""}
    };
    for(unsigned mode:{0U,1U,2U})for(bool early:{false,true})for(bool fallback:{false,true})for(const auto& c:cases){
        Fixture f;f.types.equal=mode==0;f.schedule.borrowed_slot_18=mode==1?&borrowed:nullptr;f.early=early;f.fail_post=fallback;f.result=c.result;f.last=c.last;f.chunks=c.chunks;
        f.start(c.initial,c.maximum,c.delimiter);f.drain();
        check(f.frame.error.value==c.error&&f.frame.matched==c.matched&&f.buffer.text()==c.final,"full source read-until scenario result and retained trailing data");
        check(f.owner.disposes==1&&f.owner.deletes==1&&f.weak.weak_0c==1&&f.memory.allocated==f.memory.freed,"strong/weak and actual IO/task allocation lifetimes balance across rearm");
        check(f.chunk==c.chunks.size()&&f.sdk_order.size()==f.chunk*2,"no extra SDK reads after match/error/limit");
        for(std::size_t i=0;i<f.sdk_order.size();i+=2)check(f.sdk_order[i]==1&&f.sdk_order[i+1]==2,"each receive has immediate unconditional last-error query");
        if(c.initial=="ab")check(f.requests==std::vector<std::uint32_t>{512,524},"second receive reuses actual grown spare capacity");
        if(mode==0)check(f.borrowed_calls==0&&f.owned_calls==0,"inline executor route");
        if(mode==1)check(f.borrowed_calls>0&&f.owned_calls==0,"actual022c60/022640 borrowed read body route");
        if(mode==2)check(f.owned_calls>0&&!f.borrowed_calls,"actual022c60/022670 owned read body route");
    }
}
}
int main(){try{scenarios();std::cout<<"PASS: "<<checks+b::checks+r::checks<<" composed read integration checks; recovered025c40 closes actual completion/borrowed/owned scheduling through010040/008350/021d10/023dd0/injected SDK/IO route/027710/021a40; split/embedded-NUL/trailing/EOF/limit/error, early key2/fallback and real IO/task lifetimes;025480 remains required\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
