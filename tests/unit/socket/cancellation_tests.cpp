// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_cancellation.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

namespace s=preloader_l::socket;
namespace srv=preloader_l::server;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected cancellation dependency");}
struct Gate final:s::SocketGate {
    int depth=0;unsigned enters=0,leaves=0;std::function<void(bool)> hook;
    void enter()override{check(depth==0,"fallback gate entered once");++depth;++enters;if(hook)hook(true);}
    void leave()override{check(depth==1,"fallback gate leaves while held");if(hook)hook(false);--depth;++leaves;}
};
struct Wake final:s::Api {
    std::vector<std::string> events;std::function<void()> send_hook,category_hook;
    s::Socket expected_socket=0xfedcba9876543210ULL;int error=0,status=-1;unsigned sends=0;
    void set_last_error(std::int32_t)override{unused();}
    s::Category system_category_028620()override{events.push_back("category");if(category_hook)category_hook();return {55,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{events.push_back("get");return error;}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffers,std::uint32_t flags)override{
        events.push_back("send");++sends;check(socket==expected_socket,"wake forwards full-width live socket");
        check(buffers.size()==1 && buffers[0].len==1 && buffers[0].buf && buffers[0].buf[0]==0 && flags==0,"wake uses exactly one NUL byte and flags0");
        if(send_hook)send_hook();return {status,0xffffffffU};
    }
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Recycle final:s::CancellationRecycler {
    std::vector<s::CancelNode*> nodes;std::function<void(s::CancelMap&,s::CancelNode&)> hook;
    void recycle_0283c0(s::CancelMap& map,s::CancelNode& node)override{
        check(!node.head && !node.tail,"required recycler sees fully drained operation FIFO");nodes.push_back(&node);
        auto& bucket=map.buckets[node.socket%map.buckets.size()];
        if(bucket.first==&node)bucket.first=node.next;
        if(bucket.last==&node)bucket.last=node.previous;
        if(node.previous)node.previous->next=node.next;
        if(node.next && node.next!=map.sentinel)node.next->previous=node.previous;
        if(bucket.first==map.sentinel)bucket.last=map.sentinel;
        if(hook)hook(map,node);
    }
};
struct Post final:s::CompletionApi {
    std::vector<s::CancelOperation*> attempted;std::function<int(std::uintptr_t,s::CancelOperation&,unsigned)> hook;
    std::int32_t post(std::uintptr_t port,s::CancelOperation& operation)override{
        check(!operation.next && operation.queued==1,"post observes popped, detached operation with queued marker1");
        const auto index=static_cast<unsigned>(attempted.size());attempted.push_back(&operation);
        return hook?hook(port,operation,index):1;
    }
};
void append(s::CancelNode& node,s::CancelOperation& op){if(node.tail)node.tail->next=&op;else node.head=&op;node.tail=&op;}
void setup(s::CancellationState& state,std::array<std::array<s::CancelBucket,7>,4>& buckets,std::array<s::CancelNode,4>& sentinels){
    for(unsigned m=0;m<4;++m){const std::size_t count=std::array<std::size_t,4>{1,3,5,7}[m];state.maps[m]={std::span<s::CancelBucket>(buckets[m].data(),count),&sentinels[m]};for(auto& b:state.maps[m].buckets)b={&sentinels[m],&sentinels[m]};}
}
void matrix(){
    constexpr s::Socket socket=0xabcdef0123456789ULL;
    for(unsigned per_map=0;per_map<=3;++per_map)for(unsigned fail=0;fail<=4*per_map;++fail){
        Gate gate;s::CompletionScheduler scheduler(gate);scheduler.port=123;scheduler.dispatch_required=9;
        s::CancelOperation existing;existing.queued=81;scheduler.head=scheduler.tail=&existing;
        s::CancellationState state;state.scheduler=&scheduler;Wake wake;state.wake.socket=wake.expected_socket;
        std::array<std::array<s::CancelBucket,7>,4> buckets{};std::array<s::CancelNode,4> sentinel{},collision{},nodes{};std::array<std::array<s::CancelOperation,3>,4> ops{};setup(state,buckets,sentinel);
        std::vector<s::CancelOperation*> expected;
        s::Error reason{995,{0x44556677,srv::ErrorCategory::other}};
        for(unsigned m=0;m<4;++m){auto& node=nodes[m];node.socket=socket;node.next=&sentinel[m];node.previous=&collision[m];collision[m].socket=socket+1;collision[m].next=&node;auto& b=state.maps[m].buckets[socket%state.maps[m].buckets.size()];b={&collision[m],&node};for(unsigned j=0;j<per_map;++j){auto& op=ops[m][j];op.queued=40+m*3+j;op.error={-2,{1,srv::ErrorCategory::system}};op.overlapped.Internal=0x7788;append(node,op);expected.push_back(&op);}}
        Recycle recycler;Post post;
        gate.hook=[&](bool enter){check(recycler.nodes.size()==4,"fallback only after all four recycler effects");check(scheduler.dispatch_required==(enter?9:1),"dispatch exchange precedes leave and follows enter");};
        post.hook=[&](std::uintptr_t port,s::CancelOperation& op,unsigned index){check(recycler.nodes.size()==4 && port==123 && &op==expected[index],"post uses map-order/FIFO aggregate and scheduler port");check(op.error.value==995 && op.error.category.name_identity==0x44556677 && op.error.category.logical==srv::ErrorCategory::other,"operation copies full cancellation reason");return index==fail?0:(index%2==0?-1:5);};
        wake.send_hook=[&]{check(gate.depth==0 && post.attempted.size()==(fail<expected.size()?fail+1:expected.size()),"one wake happens after posting or fallback transfer");};
        s::cancel_queue_026aa0(state,socket,reason,recycler,post,wake);
        check(recycler.nodes==std::vector<s::CancelNode*>{&nodes[0],&nodes[1],&nodes[2],&nodes[3]},"all four matching nodes recycled, including empty FIFOs");
        check(wake.events==std::vector<std::string>{"send","category","get"} && wake.sends==1,"any matched node triggers one wake regardless of operation count");
        const bool failed=fail<expected.size();check(post.attempted.size()==(failed?fail+1:expected.size()),"first zero post stops all remaining attempts");
        check(gate.enters==static_cast<unsigned>(failed) && gate.leaves==gate.enters && !gate.depth,"only failed post acquires fallback gate once");
        check(scheduler.dispatch_required==(failed?1:9),"successful/empty aggregate leaves dispatch flag untouched");
        check(scheduler.head==&existing && existing.queued==81,"existing fallback queue head and marker preserved");
        check(scheduler.tail==(failed?expected.back():&existing),"fallback tail is aggregate tail or old tail");
        check(existing.next==(failed?expected[fail]:nullptr),"failed plus all remaining appended after existing fallback tail");
        for(unsigned i=0;i<expected.size();++i){auto* op=expected[i];const unsigned original=40+(i/per_map)*3+i%per_map;check(op->queued==(i<=fail?1:original),"unattempted remaining queued marker stays unchanged");check(op->error.value==995 && op->error.category.name_identity==0x44556677 && op->overlapped.Internal==0x7788,"all drained errors copied; SDK OVERLAPPED payload unchanged");check(op->next==((failed && i>=fail && i+1<expected.size())?expected[i+1]:nullptr),"successful nodes detached and failed remainder keeps FIFO links");}
        for(unsigned m=0;m<4;++m)check(collision[m].next==&sentinel[m] && state.maps[m].buckets[socket%state.maps[m].buckets.size()].last==&collision[m],"required selected recycler owns bucket/list mutation");
    }
}
void ranges_and_empty(){
    Gate gate;s::CompletionScheduler scheduler(gate);s::CancellationState state;state.scheduler=&scheduler;Wake wake;state.wake.socket=wake.expected_socket;Recycle recycler;Post post;s::Error reason;
    std::array<std::array<s::CancelBucket,7>,4> buckets{};std::array<s::CancelNode,4> sentinel{};setup(state,buckets,sentinel);
    state.maps[0].buckets={};s::CancelNode outside,first;outside.socket=17;first.socket=18;first.next=&outside;auto& range=state.maps[1].buckets[17%3];range={&first,&first};
    s::CancelNode excluded;excluded.socket=17;sentinel[2].socket=17;sentinel[2].next=&excluded;state.maps[2].buckets[17%5]={&sentinel[2],&excluded};
    s::cancel_queue_026aa0(state,17,reason,recycler,post,wake);check(recycler.nodes.empty() && post.attempted.empty() && wake.events.empty(),"empty map, initial sentinel, exclusive last.next and absent key produce no effects");
    s::CancelNode one,two;one.socket=two.socket=17;one.next=&two;two.next=&sentinel[3];two.previous=&one;state.maps[3].buckets[17%7]={&one,&two};
    s::cancel_queue_026aa0(state,17,reason,recycler,post,wake);check(recycler.nodes==std::vector<s::CancelNode*>{&one} && post.attempted.empty() && wake.sends==1 && two.socket==17,"first matching empty node recycles/wakes and stops before duplicate key");
    s::CancellationState none;none.scheduler=nullptr;Recycle r2;Post p2;Wake w2;s::cancel_queue_026aa0(none,0,reason,r2,p2,w2);check(w2.events.empty(),"no pending operation requires no completion scheduler");
    s::CancellationState encountered;s::CancelNode start,stop,after;start.socket=18;start.next=&stop;stop.socket=17;stop.next=&after;after.socket=17;s::CancelBucket encountered_bucket{&start,&stop};encountered.maps[0]={std::span<s::CancelBucket>(&encountered_bucket,1),&stop};s::cancel_queue_026aa0(encountered,17,reason,r2,p2,w2);check(r2.nodes.empty() && p2.attempted.empty() && w2.events.empty(),"sentinel reached during key scan is not recycled or reported matched");
}
void live_reads(){
    Gate old_gate,new_gate;s::CompletionScheduler old_scheduler(old_gate),new_scheduler(new_gate);old_scheduler.port=1;new_scheduler.port=100;
    s::CancellationState state;state.scheduler=&old_scheduler;Wake wake;state.wake.socket=wake.expected_socket;std::array<std::array<s::CancelBucket,7>,4> buckets{};std::array<s::CancelNode,4> sentinels{},nodes{};std::array<s::CancelOperation,4> ops{};setup(state,buckets,sentinels);s::Error reason{995,{77,srv::ErrorCategory::system}};
    for(unsigned m=0;m<4;++m){nodes[m].socket=9;nodes[m].next=&sentinels[m];append(nodes[m],ops[m]);state.maps[m].buckets[9%state.maps[m].buckets.size()]={&nodes[m],&nodes[m]};}
    Recycle recycler;recycler.hook=[&](s::CancelMap&,s::CancelNode&){state.scheduler=&new_scheduler;reason.value+=1;reason.category.name_identity+=1;};Post post;
    post.hook=[&](std::uintptr_t port,s::CancelOperation& op,unsigned i){check(port==100+i && &op==&ops[i],"scheduler read after recyclers and its port reread each post");check(op.error.value==995+static_cast<int>(i) && op.error.category.name_identity==77+i,"reason copied per operation before corresponding recycler effect");new_scheduler.port++;state.scheduler=&old_scheduler;if(i==3){state.wake.socket=0x8888777766665555ULL;wake.expected_socket=state.wake.socket;}return 1;};
    s::cancel_queue_026aa0(state,9,reason,recycler,post,wake);check(post.attempted.size()==4 && !old_gate.enters && !new_gate.enters,"cached scheduler identity stays selected while port remains live");
    Gate gate;s::CompletionScheduler fallback(gate);s::CancelOperation old_head,late_tail;fallback.head=fallback.tail=&old_head;fallback.dispatch_required=-5;state.scheduler=&fallback;state.maps={};s::CancelNode node,sentinel;node.socket=11;node.next=&sentinel;s::CancelOperation a,b;a.queued=7;b.queued=8;append(node,a);append(node,b);s::CancelBucket bucket{&node,&node};state.maps[0]={std::span<s::CancelBucket>(&bucket,1),&sentinel};Recycle r;Post p;p.hook=[](std::uintptr_t,s::CancelOperation&,unsigned){return 0;};
    gate.hook=[&](bool enter){if(enter){old_head.next=&late_tail;fallback.tail=&late_tail;}else check(fallback.head==&old_head && old_head.next==&late_tail && late_tail.next==&a && a.next==&b && fallback.tail==&b && fallback.dispatch_required==1,"fallback uses live tail under gate and exchanges flag before leaving");};
    s::cancel_queue_026aa0(state,11,reason,r,p,wake);check(a.queued==1 && b.queued==8 && !b.next,"failed operation marked, transferred unattempted remainder retains marker");
}
void wakes(){for(int status:{-1,0,7})for(int error:{0,64,1234,10035}){Wake wake;wake.status=status;wake.error=error;s::wake_028850({wake.expected_socket},wake);const auto expected=(error==64 || error==1234)?std::vector<std::string>{"send","category","get","category"}:std::vector<std::string>{"send","category","get"};check(wake.events==expected,"wake ignores send result and observes category/get/remap side-effect order");}
    Wake wake;wake.error=7;wake.category_hook=[&]{wake.error=64;};s::wake_028850({wake.expected_socket},wake);check(wake.events==std::vector<std::string>{"send","category","get","category"},"wake samples last-error after mandatory category effect");
}
}
int main(){try{matrix();ranges_and_empty();live_reads();wakes();std::cout<<"PASS: "<<checks<<" socket cancellation checks; finite offline graph and injected effects only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
