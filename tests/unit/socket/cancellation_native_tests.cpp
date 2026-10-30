#include "preloader_l/net/transport_cancellation_native.hpp"
#include "preloader_l/net/transport_native.hpp"
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

namespace s=preloader_l::socket_recovered;
namespace srv=preloader_l::server_recovered;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected native cancellation dependency");}
struct Fixture final:s::CategoryProvider {
    std::vector<std::string> trace;std::uintptr_t port=0x123456789abcdef0ULL;s::Socket socket=0xfedcba9876543210ULL;
    s::CancelOperation* expected=nullptr;int post_status=0,last=64,send_status=-1;unsigned posts=0,sends=0,categories=0,gets=0;
    s::Category get_028620()override{trace.push_back("category");++categories;return {0x9911,srv::ErrorCategory::system};}
};
Fixture* active=nullptr;
BOOL WINAPI post(HANDLE port,DWORD bytes,ULONG_PTR key,LPOVERLAPPED overlapped){
    auto& f=*active;f.trace.push_back("post");++f.posts;
    check(reinterpret_cast<std::uintptr_t>(port)==f.port && bytes==0 && key==0,"PostQueuedCompletionStatus exact full-width port/zero bytes/zero key");
    check(f.expected && overlapped==&f.expected->overlapped,"SDK post passes the actual operation OVERLAPPED member");
    return f.post_status;
}
int WSAAPI send(SOCKET socket,LPWSABUF buffer,DWORD count,LPDWORD bytes,DWORD flags,LPWSAOVERLAPPED overlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE completion){
    auto& f=*active;f.trace.push_back("send");++f.sends;
    check(socket==f.socket && buffer && count==1 && buffer->len==1 && buffer->buf && buffer->buf[0]==0 && flags==0,"typed SDK wake sends exactly one NUL byte on full socket");
    check(bytes && *bytes==0 && !overlapped && !completion,"typed wake uses DWORD0 and synchronous null callbacks");*bytes=0xffffffffU;return f.send_status;
}
int WSAAPI get(){active->trace.push_back("get");++active->gets;return active->last;}
struct Gate final:s::SocketGate {int depth=0;std::vector<std::string>* trace=nullptr;std::string prefix;std::function<void(bool)> hook;
    void enter()override{check(!depth,"integration gate is not recursively entered");++depth;if(trace)trace->push_back(prefix+"enter");if(hook)hook(true);}
    void leave()override{check(depth==1,"integration matching gate is held on leave");if(hook)hook(false);if(trace)trace->push_back(prefix+"leave");--depth;}
};
struct Recycler final:s::CancellationRecycler {unsigned calls=0;std::function<void()> hook;
    void recycle_0283c0(s::CancelMap& map,s::CancelNode& node)override{++calls;check(!node.head && !node.tail,"native composition recycler sees drained node");auto& bucket=map.buckets[node.socket%map.buckets.size()];check(bucket.first==&node && bucket.last==&node,"selected recycler fixture handles its exact single-node bucket");bucket={map.sentinel,map.sentinel};if(hook)hook();}
};
void typed(){
    Fixture f;active=&f;s::NativeCompletionApi default_api;check(f.trace.empty(),"default completion adapter constructor invokes no API");
    s::CompletionImports imports;imports.post=post;s::NativeCompletionApi completion(imports);s::CancelOperation operation;f.expected=&operation;operation.overlapped.Internal=0xabc;operation.queued=83;operation.error={995,{9,srv::ErrorCategory::other}};
    for(int status:{0,-1,1,2}){f.post_status=status;f.trace.clear();check(completion.post(f.port,operation)==status,"typed completion preserves all BOOL result bits");check(f.trace==std::vector<std::string>{"post"} && !f.gets && !f.categories,"even failed completion invokes no error/category APIs");check(operation.overlapped.Internal==0xabc && operation.queued==83 && operation.error.value==995,"completion adapter changes no operation data");}
    for(int status:{0,-1,7}){
        f.trace.clear();f.post_status=status;f.posts=f.sends=f.categories=f.gets=0;
        Gate gate;gate.trace=&f.trace;gate.prefix="fallback_";s::CompletionScheduler scheduler(gate);scheduler.port=f.port;s::CancellationState state;state.scheduler=&scheduler;state.wake.socket=f.socket;s::CancelNode node,sentinel;node.socket=21;node.next=&sentinel;node.head=node.tail=&operation;operation.next=nullptr;operation.queued=7;s::CancelBucket bucket{&node,&node};state.maps[0]={std::span<s::CancelBucket>(&bucket,1),&sentinel};Recycler recycler;
        s::Imports native_imports;native_imports.send=send;native_imports.get_last_error=get;s::NativeApi wake(f,native_imports);
        s::cancel_queue_026aa0(state,21,{995,{9,srv::ErrorCategory::other}},recycler,completion,wake);
        const auto expected=status==0?std::vector<std::string>{"post","fallback_enter","fallback_leave","send","category","get","category"}:std::vector<std::string>{"post","send","category","get","category"};
        check(f.trace==expected && f.posts==1 && f.sends==1 && f.gets==1 && f.categories==2,"typed queue composition posts then falls back only for BOOL0, then wakes with error64 remap effect");
        check(scheduler.head==(status==0?&operation:nullptr) && scheduler.tail==scheduler.head && operation.queued==1 && !operation.next,"typed completion queue transfer exact single operation");
    }
    active=nullptr;
}
struct Owner final:s::Control {unsigned destroys=0,deletes=0;Owner(){strong=weak=1;}void destroy_resource()override{++destroys;}void delete_control()override{++deletes;}};
struct Factory final:s::ControlFactory {Owner owner;s::Control* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==24,"real acceptor ownership requests recovered allocation size");return &owner;}};
struct Acceptor final:s::AcceptorApi {
    Fixture& fixture;unsigned closes=0;s::Socket created=13;
    explicit Acceptor(Fixture& fixture):fixture(fixture){}
    s::Socket create_socket(std::int32_t family)override{check(family==2,"real acceptor selects IPv4");fixture.trace.push_back("create");return created;}
    std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{check(socket==created && port==99,"actual acceptor attaches selected service IOCP identity");fixture.trace.push_back("attach");return 23;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t name,std::int32_t value)override{check(socket==created && level==65535 && name==128 && value==0,"real close requests recovered linger option");fixture.trace.push_back("option");return 0;}
    std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t bytes)override{check(socket==created && bytes==16,"real acceptor binds IPv4 length16");fixture.trace.push_back("bind");return 0;}
    std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{check(socket==created && backlog==2147483647,"real acceptor listens with recovered backlog");fixture.trace.push_back("listen");return 0;}
    std::int32_t close_socket(s::Socket socket)override{check(socket==created,"real close retains selected socket");fixture.trace.push_back("close");++closes;return 0;}
    std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Effects final:s::ClosingLifecycleEffects {
    s::SocketService& service;Fixture& f;unsigned cleaned=0;
    Effects(s::SocketService& service,s::CloseContext& context,s::Api& errors,Acceptor& api,s::CancellationEffects& cancel,Fixture& f):ClosingLifecycleEffects(service,context,errors,api,cancel),service(service),f(f){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity identity)override{check(identity.value==42,"real lifecycle resolves selected context");f.trace.push_back("resolve");return service;}
    void executor_cleanup(s::SocketObject& object,std::uintptr_t table)override{check(!object.next && !object.previous && !service.head && object.state.socket==s::invalid_socket && !object.cancellation.control && table==0x180084928ULL,"actual destruction unlinks and resets before executor cleanup");++cleaned;f.trace.push_back("executor");}
};
struct Runtime final:s::LifecycleRuntime {using LifecycleRuntime::LifecycleRuntime;s::Category miscellaneous_category_0285b0()override{unused();}[[noreturn]]void raise_028240(s::Error,std::string_view)override{unused();}};
void integration(){
    for(bool fail:{false,true}){
        Fixture f;active=&f;f.post_status=fail?0:-1;f.last=0;f.send_status=0;
        s::Imports wake_imports;wake_imports.send=send;wake_imports.get_last_error=get;s::NativeApi errors(f,wake_imports);s::CompletionImports completion_imports;completion_imports.post=post;s::NativeCompletionApi completion(completion_imports);
        Gate engine_gate,service_gate,fallback_gate;engine_gate.trace=service_gate.trace=fallback_gate.trace=&f.trace;engine_gate.prefix="engine_";service_gate.prefix="service_";fallback_gate.prefix="fallback_";
        s::CancellationEngine engine(engine_gate);s::CloseContext context;context.engine=&engine;s::CompletionScheduler scheduler(fallback_gate);scheduler.port=f.port;s::CancellationState queue;queue.scheduler=&scheduler;queue.wake.socket=f.socket;
        s::CancelOperation operation;operation.queued=7;f.expected=&operation;s::CancelNode node,sentinel;node.socket=13;node.next=&sentinel;node.head=node.tail=&operation;s::CancelBucket bucket{&node,&node};queue.maps[0]={std::span<s::CancelBucket>(&bucket,1),&sentinel};Recycler recycler;
        recycler.hook=[&]{check(engine_gate.depth==1 && !service_gate.depth && !fallback_gate.depth,"real close invokes queue drain under cached engine gate before service unlink");check(operation.error.value==995 && operation.error.category.name_identity==0x9911,"real outer close supplies995/category to queue operation");f.trace.push_back("recycle");};
        s::QueueCancellationEffects cancellation(engine,queue,recycler,completion,errors);s::SocketService service(service_gate);service.completion_port=99;Acceptor api(f);Effects effects(service,context,errors,api,cancellation,f);s::AcceptorState state;s::SocketObject object(state);Factory factory;Runtime runtime(object,{42},effects,factory);alignas(8)std::array<std::byte,128> address{};address[0]=std::byte{2};
        s::acceptor_020e20(state,address,false,errors,api,runtime);check(service.head==&object && object.cancellation.control==&factory.owner,"actual acceptor constructor and ownership bridge installed");
        state.socket_state=0x1b;f.trace.clear();runtime.destroy_socket_024db0();
        auto expected=std::vector<std::string>{"engine_enter","category","recycle","post"};if(fail){expected.push_back("fallback_enter");expected.push_back("fallback_leave");}const std::vector<std::string> suffix{"send","category","get","engine_leave","option","close","service_enter","service_leave","executor"};expected.insert(expected.end(),suffix.begin(),suffix.end());
        check(f.trace==expected,"actual QueueEffects->026fb0->026e50->024db0 chain retains native normal-order boundaries");
        check(!service.head && state.socket==s::invalid_socket && state.socket_state==0 && !object.cancellation.control && factory.owner.destroys==1 && factory.owner.deletes==1,"actual destruction resets, unlinks and releases shared owner exactly once");
        check(effects.cleaned==1 && api.closes==1 && recycler.calls==1 && f.posts==1 && f.sends==1 && f.gets==1,"composed queue and SDK imports each execute intended effect once");
        check(scheduler.head==(fail?&operation:nullptr) && scheduler.dispatch_required==(fail?1:0) && !engine_gate.depth && !service_gate.depth && !fallback_gate.depth,"fallback ownership and all gate depths valid after full destruction");
        Gate other_gate;s::CancellationEngine other(other_gate);bool caught=false;try{cancellation.cancel_026aa0(other,13,{});}catch(const s::UnsupportedInput&){caught=true;}check(caught && recycler.calls==1,"bound engine mismatch rejected before effects");active=nullptr;
    }
}
}
int main(){try{typed();integration();std::cout<<"PASS: "<<checks<<" socket native cancellation checks; injected SDK and actual source integration only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
