#pragma once
#include "node_delete.hpp"
#include "document_clear_test_fixture.hpp"

namespace delete_test {
namespace s=anadius::socket_recovered;
using clear_test::Graph;using document_test::State;using document_test::Failure;
using response_test::check;using response_test::checks;
struct Nodes:s::RecoveredNodeDeleteEffects {
    clear_test::Nodes lower;Graph& graph;
    std::function<void(std::string)>& hook=lower.hook;std::function<void()>& after_destroy=lower.after_destroy;
    std::uint64_t& probe_result=lower.probe_result;
    unsigned& probes=lower.probes;unsigned& destroys=lower.destroys;unsigned& releases=lower.releases;
    unsigned& tracks=lower.tracks;unsigned& parents=lower.parents;unsigned& deletes=lower.deletes;
    std::vector<clear_test::Nodes::Call>& calls=lower.calls;std::vector<std::string>& trace=lower.trace;
    explicit Nodes(Graph& g):RecoveredNodeDeleteEffects(g.state.memory),lower(g),graph(g){}
    std::uint64_t node_probe(std::uintptr_t target,std::uintptr_t node)override{return lower.node_probe(target,node);}
    void node_destroy(std::uintptr_t target,std::uintptr_t node,std::uint32_t flags)override{lower.node_destroy(target,node,flags);}
    void pool_release(std::uintptr_t target,std::uintptr_t pool,std::uintptr_t node)override{lower.pool_release(target,pool,node);}
    void pool_track(std::uintptr_t target,std::uintptr_t pool)override{lower.pool_track(target,pool);}
};
struct Docs:s::RecoveredDocumentClearEffects {
    State& state;Nodes& nodes;document_test::Effects lower;
    // Controlled compatibility-parser projections:0 linked root,1 tracked
    // unattached root,2 tracked root attached to a separate mapped parent.
    // This is not recovered native DOM allocation/tracking materialization.
    unsigned projection_mode=1;
    Docs(State& s,Nodes& n,native_message_test::Fixture* native=nullptr):RecoveredDocumentClearEffects(s.memory,n),state(s),nodes(n),lower(s,native){nodes.after_destroy=[this](){lower.reset_parsed();};}
    void set_error_005820(std::uintptr_t p,std::int32_t a,std::uint32_t b,std::uintptr_t c)override{lower.set_error_005820(p,a,b,c);}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{const auto p=lower.allocate_04b100(n);nodes.graph.refresh();return p;}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{lower.copy_078d30(to,from,n);nodes.graph.refresh();}
    void parse_0041b0(std::uintptr_t p)override{
        lower.parse_0041b0(p);const auto* parsed=lower.native?lower.native->api.document.get():lower.parsed.get();
        if(!parsed||!parsed->first_element())return;
        auto& graph=nodes.graph;graph.install();nodes.probe_result=projection_mode?0:1;
        if(!projection_mode)return;
        state.put(0x38,0);state.put(0x40,0);state.put(0x104,1,4);state.put(0xa8,Graph::tracking());
        graph.set(Graph::tracking(),Graph::node());graph.set(Graph::node()+0x10,0);
        if(projection_mode==2){const auto parent=Graph::node(15);graph.set(parent+0x38,Graph::node());graph.set(parent+0x40,Graph::node());graph.set(Graph::node()+0x10,parent);}
    }
    void clear_pool_002d70(std::uintptr_t p)override{lower.clear_pool_002d70(p);}
    void clear_pool_002e10(std::uintptr_t p)override{lower.clear_pool_002e10(p);}
    void clear_pool_002dc0(std::uintptr_t p)override{lower.clear_pool_002dc0(p);}
    void destroy_pool_001c30(std::uintptr_t p)override{lower.destroy_pool_001c30(p);}
    void destroy_pool_001cb0(std::uintptr_t p)override{lower.destroy_pool_001cb0(p);}
    void destroy_pool_001bb0(std::uintptr_t p)override{lower.destroy_pool_001bb0(p);}
    void destroy_pool_001d30(std::uintptr_t p)override{lower.destroy_pool_001d30(p);}
    void free_04b0f8(std::uintptr_t p)override{lower.free_04b0f8(p);nodes.graph.refresh();}
    void destroy_node_base_001ea0(std::uintptr_t p)override{lower.destroy_node_base_001ea0(p);lower.reset_parsed();}
};
struct Fixture {
    s::NativeMessageDocument document;State state{document};Graph graph{state};Nodes nodes{graph};Docs docs{state,nodes};
    s::NativeDocumentContext document_context{state.memory,docs};s::NativeNodeDeleteContext context{state.memory,nodes};s::NativeDocumentClearContext clear_context{state.memory,nodes,docs};
    void construct(){(void)s::construct_native_document_001830(State::base,1,0,document_context);}
    void unlink(std::uintptr_t parent,std::uintptr_t node){nodes.remove_parent_003030(parent,node);}
    void erase(std::uintptr_t node){nodes.delete_node_003240(node);}
    void clear(){docs.clear_002e60(State::base);}
};
} // namespace delete_test
