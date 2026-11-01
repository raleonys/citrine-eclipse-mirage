#include "receive_buffer_test_fixture.hpp"
#include "async_receive_test_fixture.hpp"
#include <algorithm>
using namespace buffer_test;
namespace r=receive_test;
namespace {
struct Empty:s::IoHandlerVtable {
    Empty():IoHandlerVtable(0x777){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override{}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment&)override{to.payload_00={};to.state_18=0;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{throw std::logic_error("unused empty query");}
};
struct Types:s::IoEnvironmentTypeApi {
    unsigned comparisons=0;
    std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{++comparisons;check(a==0x1008&&b==0x1800a25c8ULL,"actual receive constructor comparator");return 0;}
};
std::uintptr_t query(){return 0x1000;}
struct Constructed:s::RecoveredAsyncReceiveEffects {
    r::Fixture& f;
    Constructed(r::Fixture& f,s::IoHandlerSentinels sentinels,s::IoEnvironmentConstructorContext& ctor,std::span<s::IoEnvironmentSourceBinding> sources)
        :RecoveredAsyncReceiveEffects(sentinels,ctor,sources),f(f){}
    s::AsyncReceiveOperation& project_async_receive(s::OperationStorage& p)override{return f.effects.project_async_receive(p);}
};
struct Callback:s::IoCompletionEffects {
    unsigned calls=0;std::uint64_t bytes=0;std::int32_t error=0;
    s::Category misc_category_0285b0()override{return {0x9999};}
    void receive_body_025c40(s::ReceiveHandler& h,s::Error& copy,std::uint64_t n,std::uint32_t flag)override{
        ++calls;bytes=n;error=copy.value;check(flag==0&&h.transferred==n&&h.error.value==error,"actual021a40 forwards completion to still-required025c40 boundary");
    }
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{throw std::logic_error("unused send");}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{throw std::logic_error("unused send schedule");}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{throw std::logic_error("unused receive schedule");}
};
struct SdkState {
    Fixture& buffer;std::uintptr_t expected_data;std::uint32_t request;std::string payload;
    int result=0,error=0;std::vector<int> order;
};
SdkState* active=nullptr;
int WSAAPI receive(SOCKET socket,LPWSABUF b,DWORD count,LPDWORD bytes,LPDWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE completion){
    auto& state=*active;state.order.push_back(1);
    check(socket==77&&count==1&&b->len==state.request&&reinterpret_cast<std::uintptr_t>(b->buf)==state.expected_data,"grown buffer slice reaches actual injected SDK adapter");
    check(*bytes==0&&*flags==0&&ov&&!completion,"SDK transferred/flags private DWORDs and null callback");
    auto destination=state.buffer.memory.view(state.expected_data,state.request);
    check(std::all_of(destination.begin(),destination.end(),[](auto byte){return byte==0;}),"prepare zero-fill completed before WSARecv");
    std::copy(state.payload.begin(),state.payload.end(),destination.begin());
    *bytes=static_cast<DWORD>(state.payload.size());*flags=0xdeadbeef;return state.result;
}
int WSAAPI last_error(){active->order.push_back(2);return active->error;}
void integration(){
    struct Case{std::string payload;int result,error,expected;};
    const Case cases[]={{"abc\n",0,1234,0},{std::string("a\0b\n",4),-1,997,0},{"",-1,997,2},{"partial",-1,64,10054}};
    for(std::uint64_t capacity:{15ULL,16ULL,4095ULL})for(std::uint64_t initial:{0ULL,7ULL})
    for(std::uint32_t requested:{512U,65536U})for(const auto& c:cases){
        Fixture buffer;buffer.seed(capacity,initial);const auto prefix=buffer.text();
        // Controlled prepare/commit caller. This is not a recovered025c40 body.
        s::resize_receive_buffer_010040(buffer.value,initial+requested,0,buffer.context);
        r::Fixture io;Empty empty;Types types;s::IoHandlerExecutor executor{};
        auto env=io.environment;env.sentinels={&empty,0x846e8,&executor,0x777};
        s::IoHandlerEnvironment source{{},&empty,0,0x456,&executor};
        std::array<s::IoEnvironmentTypeBinding,1> queries{{{0x456,&query}}};std::array<s::IoEnvironmentFactoryBinding,0> factories{};
        s::IoEnvironmentConstructorContext constructor{types,queries,factories};
        std::array<s::IoEnvironmentSourceBinding,1> sources{{{0x987,&source}}};Constructed effects{io,env.sentinels,constructor,sources};
        s::NativeAsyncReceiveApi api{{&receive,&last_error}};
        s::AsyncReceiveContext context{env,io.worker,io.memory,api,io.completion,io.errors,effects};
        s::ReceiveBufferSlice slice{s::receive_buffer_address(buffer.value)+initial,requested};
        SdkState state{buffer,slice.data,requested,c.payload,c.result,c.error,{}};active=&state;
        r::Ref owner,weak;io.source.control_a8=&owner;io.implementation.weak_control_18=&weak;io.implementation.state_08=0x10;
        s::start_async_receive_021d10(io.service,io.implementation,slice,0,io.source,0x987,context);
        check(state.order==std::vector<int>{1,2}&&types.comparisons==1,"real buffer/021d10/SDK/023dd0 source composition");
        check(io.effects.operation->buffer_data_50==slice.data&&io.effects.operation->io.requested_58==requested,"operation retains prepared slice");
        const auto error=c.result!=0&&c.error!=997?io.effects.packet.packet_error:s::Error{0,{0x2222}};
        Callback callback;auto* operation=&io.effects.operation->io;
        io.memory.free=[&]{io.effects.operation.reset();};
        s::complete_receive_027710(1,*operation,error,c.payload.size(),env,io.worker,io.memory,io.errors,callback);
        check(callback.calls==1&&callback.bytes==c.payload.size()&&callback.error==c.expected,"real completion delivers successful, embedded NUL, EOF or mapped failure");
        check(!io.effects.operation&&owner.disposes==1&&owner.deletes==1&&weak.weak_0c==1,"operation lifetime ends and ownership releases after completion");
        s::resize_receive_buffer_010040(buffer.value,initial+c.payload.size(),0,buffer.context);
        check(buffer.text()==prefix+c.payload&&buffer.memory.view(s::receive_buffer_address(buffer.value)+buffer.value.size_10,1)[0]==0,"selected commit shrink preserves received bytes and places NUL");
        check(buffer.value.capacity_18>=initial+requested,"commit retains prepared capacity");
    }
}
}
int main(){try{integration();std::cout<<"PASS: "<<checks+r::checks<<" receive buffer integration checks; actual010040/008350 byte storage with021d10/023dd0/injected SDK/027710/021a40; controlled prepare/commit caller, embedded NUL/EOF/error/owned cleanup;025c40 remains open\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
