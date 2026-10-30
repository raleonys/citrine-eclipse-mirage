#include "preloader_l/net/transport_recycle.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

namespace s=preloader_l::socket_recovered;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Limit final:std::exception {};
struct Failure final:s::RecycleFailureEffects {
    unsigned calls=0;std::uintptr_t token=0;
    [[noreturn]]void length_error_047fc4(std::uintptr_t value)override{++calls;token=value;throw Limit{};}
};
void ring(s::CancelNode& sentinel,const std::vector<s::CancelNode*>& nodes){
    auto* previous=&sentinel;
    for(auto* node:nodes){previous->next=node;node->previous=previous;previous=node;}
    previous->next=&sentinel;sentinel.previous=previous;
}
void verify(s::CancelNode& sentinel,const std::vector<s::CancelNode*>& expected){
    auto* node=sentinel.next;auto* previous=&sentinel;
    for(auto* wanted:expected){check(node==wanted && node->previous==previous,"forward ring order and reverse link");previous=node;node=node->next;}
    check(node==&sentinel && sentinel.previous==previous,"forward traversal stops exactly at sentinel");
    node=sentinel.previous;auto* next=&sentinel;
    for(auto it=expected.rbegin();it!=expected.rend();++it){check(node==*it && node->next==next,"reverse ring order and forward link");next=node;node=node->previous;}
    check(node==&sentinel && sentinel.next==next,"reverse traversal stops exactly at sentinel");
}
struct Graph {
    s::CancelNode active,free,prefix,suffix;
    std::vector<s::CancelNode> nodes,unused;
    std::array<s::CancelBucket,5> buckets{};
    s::CancelMap map{buckets,&active};s::RecycleMap metadata{map,free};
    std::vector<s::CancelNode*> active_order,free_order;
    Graph(unsigned count,unsigned available):nodes(count),unused(available){
        prefix.socket=1;suffix.socket=2;active_order.push_back(&prefix);
        for(unsigned i=0;i<count;++i){nodes[i].socket=0xfedcba9876543200ULL+i*5ULL;while(nodes[i].socket%5!=3)++nodes[i].socket;active_order.push_back(&nodes[i]);}
        active_order.push_back(&suffix);
        for(auto& n:unused)free_order.push_back(&n);
        ring(active,active_order);ring(free,free_order);
        for(auto& b:buckets)b={&active,&active};
        buckets[1]={&prefix,&prefix};buckets[2]={&suffix,&suffix};buckets[3]={&nodes.front(),&nodes.back()};
        metadata.map_size=metadata.active_size=active_order.size();metadata.free_size=available;
    }
};
void removal_matrix(){
    // Independent expected node sequences, rather than a second splice algorithm.
    for(unsigned count=1;count<=8;++count)for(unsigned at=0;at<count;++at)for(unsigned available=0;available<=4;++available){
        Graph g(count,available);Failure failure;auto& node=g.nodes[at];
        s::CancelOperation a,b;a.next=&b;a.queued=83;b.queued=94;node.head=&a;node.tail=&b;
        const auto original_key=node.socket;const auto original_active=g.active_order.size();
        auto expected_active=g.active_order;expected_active.erase(expected_active.begin()+at+1);
        auto expected_free=g.free_order;expected_free.insert(expected_free.begin(),&node);
        s::recycle_0283c0(g.metadata,node,failure);
        check(original_key%5==3 && original_key>0xffffffffULL && node.socket==0,"original full-width key selects bucket before clearing");
        const auto* first=count==1?&g.active:(at==0?&g.nodes[1]:&g.nodes[0]);
        const auto* last=count==1?&g.active:(at+1==count?&g.nodes[count-2]:&g.nodes[count-1]);
        check(g.buckets[3].first==first && g.buckets[3].last==last,"sole/head/tail/middle bucket endpoints");
        check(g.buckets[0].first==&g.active && g.buckets[0].last==&g.active && g.buckets[1].first==&g.prefix && g.buckets[1].last==&g.prefix && g.buckets[2].first==&g.suffix && g.buckets[2].last==&g.suffix,"other bucket endpoints untouched");
        verify(g.active,expected_active);verify(g.free,expected_free);
        check(g.metadata.map_size==original_active-1 && g.metadata.active_size==original_active-1 && g.metadata.free_size==available+1,"one logical count transferred and map size decremented");
        check(node.head==&a && node.tail==&b && a.next==&b && a.queued==83 && b.queued==94,"direct recycler preserves operation FIFO payload");
        check(failure.calls==0,"ordinary recycle never requests length error");
    }
}
void singleton_and_zero(){
    s::CancelNode active,free,node;s::CancelBucket bucket{&node,&node};s::CancelMap map{std::span<s::CancelBucket>(&bucket,1),&active};s::RecycleMap meta(map,free);Failure failure;
    ring(active,{&node});ring(free,{});node.socket=0;meta.map_size=meta.active_size=1;
    s::recycle_0283c0(meta,node,failure);verify(active,{});verify(free,{&node});
    check(bucket.first==&active && bucket.last==&active && meta.map_size==0 && meta.active_size==0 && meta.free_size==1,"sole zero-key node empties active ring");
}
void unsigned_counts(){
    constexpr auto maximum=std::numeric_limits<std::uint64_t>::max();
    for(auto value:{std::uint64_t{0},s::recycle_limit-1,s::recycle_limit+1,maximum}){
        Graph g(1,0);Failure failure;g.metadata.free_size=value;g.metadata.active_size=g.metadata.map_size=0;
        s::recycle_0283c0(g.metadata,g.nodes[0],failure);
        check(g.metadata.free_size==value+1 && g.metadata.active_size==maximum && g.metadata.map_size==maximum,"all counts use unsigned modulo64 including values above equality limit");
        check(failure.calls==0,"above-limit counts are not rejected as greater-or-equal");verify(g.active,{&g.prefix,&g.suffix});verify(g.free,{&g.nodes[0]});
    }
}
void failure_order(){
    for(unsigned count=1;count<=4;++count)for(unsigned at=0;at<count;++at){
        Graph g(count,2);Failure failure;auto& node=g.nodes[at];g.metadata.free_size=s::recycle_limit;
        const auto active=g.metadata.active_size,map_size=g.metadata.map_size;bool caught=false;
        try{s::recycle_0283c0(g.metadata,node,failure);}catch(const Limit&){caught=true;}
        check(caught && failure.calls==1 && failure.token==0x1800846b8ULL,"exact limit invokes required nonreturning047fc4 opaque message identity");
        check(node.socket==0,"key clear survives length failure");
        const auto* first=count==1?&g.active:(at==0?&g.nodes[1]:&g.nodes[0]);
        const auto* last=count==1?&g.active:(at+1==count?&g.nodes[count-2]:&g.nodes[count-1]);
        check(g.buckets[3].first==first && g.buckets[3].last==last,"bucket edits precede length failure for each endpoint case");
        check(g.metadata.free_size==s::recycle_limit && g.metadata.active_size==active && g.metadata.map_size==map_size,"length failure precedes every count mutation");
        verify(g.active,g.active_order);verify(g.free,g.free_order);
    }
}
void binding(){
    Graph a(1,0),b(1,0);Failure failure;std::array<s::RecycleMap*,3> metadata{nullptr,&a.metadata,&b.metadata};s::RecoveredCancellationRecycler recycler(metadata,failure);
    auto projection=b.map;recycler.recycle_0283c0(projection,b.nodes[0]);
    check(b.metadata.map_size==2 && a.metadata.map_size==3,"copied map projection binds shared bucket/sentinel identity and correct counts");verify(b.free,{&b.nodes[0]});
    s::CancelNode stranger;auto different_sentinel=a.map;different_sentinel.sentinel=&stranger;auto different_buckets=a.map;std::array<s::CancelBucket,5> buckets{};different_buckets.buckets=buckets;auto different_count=a.map;different_count.buckets=different_count.buckets.first(4);
    for(auto* wrong:{&different_sentinel,&different_buckets,&different_count}){bool caught=false;try{recycler.recycle_0283c0(*wrong,a.nodes[0]);}catch(const s::UnsupportedInput&){caught=true;}check(caught && a.nodes[0].socket!=0 && a.metadata.map_size==3,"mismatched projection rejected before bucket/key/count mutation");}
    s::CancelMap empty;bool caught=false;try{s::RecycleMap m(empty,stranger);s::recycle_0283c0(m,a.nodes[0],failure);}catch(const s::UnsupportedInput&){caught=true;}check(caught && a.nodes[0].socket!=0,"zero bucket count outside initialized domain is explicit");
}
}
int main(){try{removal_matrix();singleton_and_zero();unsigned_counts();failure_order();binding();std::cout<<"PASS: "<<checks<<" socket recycler checks; retained0283c0 contracts and offline initialized graphs only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
