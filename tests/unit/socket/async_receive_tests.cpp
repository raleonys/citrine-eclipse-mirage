#include "tests/support/socket/async_receive_test_fixture.hpp"
using namespace receive_test;
namespace {
void decisions(){
    // Expected outcomes are explicit WinSock scenarios, not a copy of the body.
    struct Scenario{int result;std::uint32_t error;bool started;std::int32_t status;};
    const Scenario cases[]={{0,64,true,0},{0,1234,true,0},{0,10054,true,0},{-1,997,true,0},
        {2,997,true,0},{-1,64,false,10054},{-1,1234,false,10061},{-1,0,false,0},{2,10038,false,10038}};
    for(const auto& c:cases)for(bool early:{false,true})for(bool failed_post:{false,true}){
        Fixture f;Ref weak,owned;weak.weak_0c=4;f.source.control_a8=&owned;f.implementation.weak_control_18=&weak;
        f.implementation.weak_object_10=0x1234;f.implementation.state_08=0x81;f.entry.work_count_30=UINT32_MAX;
        f.receive.answer={c.result,c.error,0x87654321};f.completion.result=failed_post?0:1;
        const auto original=f.source;s::ReceiveBufferSlice buffer{0x123456780000ULL,0x100000005ULL};
        f.effects.construct=[&](auto&,auto id){
            const auto& op=*f.effects.operation;
            check(id==0x987&&weak.weak_0c==6,"temporary and retained weak increments precede constructor");
            check(f.source.object_a0==0&&f.source.control_a8==nullptr&&op.io.handler.object_a0==0xbeef&&op.io.handler.control_a8==&owned,"owned handler transferred before constructor");
            check(op.io.handler.words_60_78==original.words_60_78&&op.io.handler.byte_80==0x81&&op.io.handler.dword_84==0x87654321&&op.io.handler.words_88_98==original.words_88_98,"all defined receive fields including separate byte and DWORD copied");
            check(op.io.state_38==0x81&&op.weak_object_40==0x1234&&op.io.weak_48==&weak,"receive state and weak identity snapshots");
            check(op.callback_identity_28==0x180027710ULL&&!f.effects.packet.packet_error.value&&!f.effects.packet.transferred_14&&op.io.operation.error.value==-12,"callback packet and cancellation metadata remain distinct");
            check(op.buffer_data_50==buffer.data&&op.io.requested_58==0x100000005ULL,"initial full64 buffer snapshot retained");
            buffer={0x876543210000ULL,0x200000003ULL};
        };
        f.receive.hook=[&]{check(weak.weak_0c==5&&f.entry.work_count_30==0,"temporary weak released and work wraps before API");if(early)f.effects.operation->io.operation.queued=1;};
        f.start(buffer);
        check(f.receive.calls==1&&f.receive.observed.len==3&&reinterpret_cast<std::uintptr_t>(f.receive.observed.buf)==buffer.data&&f.receive.flags==0x12345678,"SDK uses reloaded buffer low32 length and original flags");
        const unsigned posts=c.started?(early?1:0):1;
        check(f.completion.posts==posts&&f.effects.operation->io.operation.queued==1,"immediate or pending handshake versus status publication");
        check(f.errors.categories==(c.started?0U:1U)&&f.effects.packet.packet_error.value==c.status&&f.effects.packet.transferred_14==(c.started?0U:0x87654321U),"receive-only64 mapping and shared1234 mapping");
        check(f.gate.enters==(failed_post?posts:0U)&&f.gate.enters==f.gate.leaves,"fallback gated only when required post fails");
        if(failed_post&&posts)check(f.entry.completion.head==&f.effects.operation->io.operation&&f.entry.completion.tail==f.entry.completion.head&&f.entry.completion.dispatch_required==1,"actual fallback queue receives operation");
        check(f.memory.allocations==std::vector<std::uint64_t>{225}&&f.memory.bytes[224]==std::byte{56}&&f.worker.gets==1&&!f.memory.frees,"allocation uses0xe0 plus marker; null tail does not free submitted operation");
    }
}
void shortcuts_and_reloads(){
    for(bool stream:{false,true})for(bool invalid:{false,true})for(std::uint64_t size:{0ULL,0x100000000ULL,5ULL}){
        Fixture f;f.implementation.state_08=stream?0x10:0;f.implementation.socket_00=invalid?s::invalid_socket:77;
        s::ReceiveBufferSlice b{0x4444,size};f.start(b);
        const bool shortcut=stream&&size==0;
        check(f.receive.calls==(!shortcut&&!invalid?1U:0U),"shortcut tests full64 zero before invalid socket");
        check(f.effects.packet.packet_error.value==(!shortcut&&invalid?10009:0)&&f.completion.posts==(shortcut||invalid?1U:0U),"shortcut status and invalid socket error");
        check(f.entry.work_count_30==1,"one work reference on every path");
    }
    Fixture f;s::ReceiveBufferSlice b{0x4444,5};f.effects.construct=[&](auto&,auto){f.implementation.state_08=0x10;f.implementation.socket_00=s::invalid_socket;b.size=0;};f.start(b);
    check(f.effects.operation->io.state_38==0&&f.effects.operation->io.requested_58==5&&!f.receive.calls&&f.effects.packet.packet_error.value==0,"stored state/buffer separate from reloaded shortcut inputs");
    Fixture cache;cache.worker.tls=&cache.tls;cache.thread.cache_00[0]=cache.memory.storage.identity;cache.memory.bytes[0]=std::byte{56};s::ReceiveBufferSlice c{0,0};cache.start(c);
    check(cache.memory.allocations.empty()&&!cache.thread.cache_00[0]&&cache.memory.bytes[224]==std::byte{56},"receive uses actual two-slot IO storage cache");
}
void lifetime_and_failures(){
    Fixture f;Ref weak;weak.weak_0c=UINT32_MAX;f.implementation.weak_control_18=&weak;s::ReceiveBufferSlice b{0x1111,5};
    weak.erase=[&]{f.implementation.state_08=0x10;b.size=0;};f.start(b);
    check(weak.weak_0c==0&&weak.deletes==1&&!f.receive.calls,"weak counter wraps; last temporary decrement invokes delete before live reload");
    Fixture destroyed;s::ReceiveBufferSlice d{0x1111,5};destroyed.receive.answer={-1,64,5};destroyed.completion.hook=[&]{destroyed.effects.operation.reset();};destroyed.start(d);
    check(!destroyed.effects.operation&&destroyed.completion.posts==1,"successful publication may end actual operation lifetime before return");
    Fixture early;s::ReceiveBufferSlice e{0x1111,5};early.receive.hook=[&]{early.effects.operation->io.operation.queued=1;};early.completion.hook=[&]{early.effects.operation.reset();};early.start(e);
    check(!early.effects.operation&&early.completion.posts==1,"early handshake post may end operation lifetime");
    Fixture failure;Ref owned;failure.source.control_a8=&owned;failure.effects.construct=[](auto&,auto){throw std::runtime_error("constructor");};
    rejects<std::runtime_error>([&]{failure.start(b);});check(!failure.source.control_a8&&failure.effects.operation->io.handler.control_a8==&owned&&!failure.entry.work_count_30&&!failure.receive.calls,"constructor failure prefix has transferred owner; no fabricated unwind claims");
    Fixture missing;missing.service.scheduler_08=nullptr;rejects([&]{missing.start(b);});check(!missing.receive.calls,"missing scheduler rejected before SDK");
}
std::vector<int> sdk_order;int sdk_result=-1,sdk_status=997;DWORD expected_flags=0;OVERLAPPED* expected_overlap=nullptr;
int WSAAPI sdk_receive(SOCKET socket,LPWSABUF buffer,DWORD count,LPDWORD transferred,LPDWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE completion){
    sdk_order.push_back(1);check(socket==77&&buffer->len==5&&reinterpret_cast<std::uintptr_t>(buffer->buf)==0x1234&&count==1,"exact SDK receive arguments");
    check(transferred&&*transferred==0&&flags&&*flags==expected_flags&&ov==expected_overlap&&!completion,"private DWORD outputs and null completion callback");
    *transferred=UINT32_MAX;*flags=0xdeadbeef;return sdk_result;
}
int WSAAPI sdk_error(){sdk_order.push_back(2);return sdk_status;}
void native_adapter(){
    s::NativeAsyncReceiveApi default_construction; // no calls
    for(int result:{0,-1})for(int status:{64,997,1234}){
        sdk_order.clear();sdk_result=result;sdk_status=status;expected_flags=0x87654321;
        s::NativeAsyncReceiveApi api{{&sdk_receive,&sdk_error}};s::Buffer b{5,reinterpret_cast<char*>(0x1234)};OVERLAPPED ov{};expected_overlap=&ov;
        const auto r=api.receive(77,b,expected_flags,ov);
        check(sdk_order==std::vector<int>{1,2}&&r.result==result&&r.last_error==static_cast<std::uint32_t>(status)&&r.transferred==UINT32_MAX,"SDK error is unconditional immediate and retains raw status");
    }
    s::Buffer b{};OVERLAPPED ov{};sdk_order.clear();s::NativeAsyncReceiveApi no_error{{&sdk_receive,nullptr}},no_receive{{nullptr,&sdk_error}};
    rejects([&]{no_error.receive(77,b,0,ov);});rejects([&]{no_receive.receive(77,b,0,ov);});check(sdk_order.empty(),"missing imports rejected without partial SDK call");
}
}
int main(){try{decisions();shortcuts_and_reloads();lifetime_and_failures();native_adapter();std::cout<<"PASS: "<<checks<<" async receive core checks; actual021d10 normal paths, receive-only64 mapping, full64 reloads, ownership/weak/cache/publication, injected exact SDK WSARecv/GetLastError and actual lifetime ends\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
