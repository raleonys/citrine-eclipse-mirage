// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/document_clear.hpp"
#include "tests/support/socket/native_document_test_fixture.hpp"

namespace clear_test {
namespace s=laureondo::socket;
using document_test::State;using document_test::Failure;
using response_test::check;using response_test::checks;
struct Graph {
    State& state;
    std::array<std::array<std::uint8_t,0x80>,16> node_bytes{};
    std::array<std::array<std::uint8_t,0x80>,4> table_bytes{},tracking_bytes{};
    std::array<std::array<std::uint8_t,0x20>,4> pool_bytes{};
    std::array<std::array<std::uint8_t,0x40>,4> pooltable_bytes{};
    unsigned installs=0;
    explicit Graph(State& s):state(s){reset();}
    static constexpr std::uintptr_t node(unsigned i=0){return 0x710000000ULL+i*0x100;}
    static constexpr std::uintptr_t table(unsigned i=0){return 0x720000000ULL+i*0x100;}
    static constexpr std::uintptr_t pool(unsigned i=0){return 0x730000000ULL+i*0x100;}
    static constexpr std::uintptr_t pooltable(unsigned i=0){return 0x740000000ULL+i*0x100;}
    static constexpr std::uintptr_t tracking(unsigned i=0){return 0x750000000ULL+i*0x100;}
    void refresh(){state.refresh();std::size_t slot=450;for(unsigned i=0;i<node_bytes.size();++i)state.regions[slot++]={node(i),node_bytes[i]};for(unsigned i=0;i<4;++i){state.regions[slot++]={table(i),table_bytes[i]};state.regions[slot++]={pool(i),pool_bytes[i]};state.regions[slot++]={pooltable(i),pooltable_bytes[i]};state.regions[slot++]={tracking(i),tracking_bytes[i]};}}
    std::uint64_t get(std::uintptr_t p,unsigned width=8){auto bytes=state.memory.view(p,width);std::uint64_t value=0;for(unsigned i=0;i<width;++i)value|=std::uint64_t(bytes[i])<<(i*8);return value;}
    void set(std::uintptr_t p,std::uint64_t value,unsigned width=8){auto bytes=state.memory.view(p,width);for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));}
    void reset(){for(auto& b:node_bytes)b.fill(0);for(auto& b:table_bytes)b.fill(0);for(auto& b:tracking_bytes)b.fill(0);for(auto& b:pool_bytes)b.fill(0);for(auto& b:pooltable_bytes)b.fill(0);refresh();for(unsigned i=0;i<node_bytes.size();++i){set(node(i),table());set(node(i)+8,State::base);set(node(i)+0x60,pool());}for(unsigned i=0;i<4;++i){set(table(i)+0x38,0xf001+i*0x10);set(table(i)+0x78,0xf002+i*0x10);set(pool(i),pooltable(i));set(pooltable(i)+0x18,0xe001+i*0x10);set(pooltable(i)+0x20,0xe002+i*0x10);}}
    // Controlled one-root shadow for the compatibility parser. This is not
    // recovered native DOM allocation or complete tree materialization.
    void install(){reset();set(node()+0x10,State::base);state.put(0x38,node());state.put(0x40,node());state.put(0x104,0,4);++installs;}
};
struct Nodes:s::NativeNodeClearEffects {
    struct Call{std::string kind;std::uintptr_t target=0,first=0,second=0;std::uint32_t flags=0;};
    Graph& graph;std::vector<Call> calls;std::vector<std::string> trace;std::function<void(std::string)> hook;std::function<void()> after_destroy;
    std::uint64_t probe_result=1;bool mutate=true;unsigned probes=0,destroys=0,releases=0,parents=0,deletes=0,tracks=0;
    explicit Nodes(Graph& g):graph(g){}
    void event(std::string name){trace.push_back(name);if(hook)hook(name);}
    void drop(std::uintptr_t node){const auto doc=graph.get(node+8);const auto count=graph.get(doc+0x104,4);if(!count)return;check(count<=16,"controlled lower helper tracking extent");const auto entries=graph.get(doc+0xa8);for(std::uint64_t i=0;i<count;++i)if(graph.get(entries+i*8)==node){graph.set(entries+i*8,graph.get(entries+(count-1)*8));graph.set(doc+0x104,count-1,4);break;}graph.set(node+0x10,0);}
    std::uint64_t node_probe(std::uintptr_t target,std::uintptr_t node)override{++probes;calls.push_back({"probe",target,node});event("probe");return probe_result;}
    void node_destroy(std::uintptr_t target,std::uintptr_t node,std::uint32_t flags)override{++destroys;calls.push_back({"destroy",target,node,0,flags});event("destroy");if(after_destroy)after_destroy();}
    void pool_release(std::uintptr_t target,std::uintptr_t pool,std::uintptr_t node)override{++releases;calls.push_back({"release",target,pool,node});event("release");}
    void pool_track(std::uintptr_t target,std::uintptr_t pool)override{++tracks;calls.push_back({"track",target,pool});event("track");}
    void remove_parent_003030(std::uintptr_t parent,std::uintptr_t node)override{++parents;calls.push_back({"parent",0,parent,node});event("parent");if(mutate)drop(node);if(after_destroy)after_destroy();}
    void delete_node_003240(std::uintptr_t node)override{++deletes;calls.push_back({"delete",0,node});event("delete");if(mutate)drop(node);if(after_destroy)after_destroy();}
};
struct Docs:s::DocumentClearEffects {
    State& state;Nodes& nodes;document_test::Effects lower;
    Docs(State& s,Nodes& n,native_message_test::Fixture* native=nullptr):DocumentClearEffects(s.memory,n),state(s),nodes(n),lower(s,native){nodes.after_destroy=[this](){lower.reset_parsed();};}
    void set_error_005820(std::uintptr_t p,std::int32_t a,std::uint32_t b,std::uintptr_t c)override{lower.set_error_005820(p,a,b,c);}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{const auto p=lower.allocate_04b100(n);nodes.graph.refresh();return p;}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{lower.copy_078d30(to,from,n);nodes.graph.refresh();}
    void parse_0041b0(std::uintptr_t p)override{lower.parse_0041b0(p);const auto* parsed=lower.native?lower.native->api.document.get():lower.parsed.get();if(parsed&&parsed->first_element())nodes.graph.install();}
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
    s::NativeMessageDocument document;State state{document};Graph graph{state};Nodes nodes{graph};Docs docs{state,nodes};s::NativeDocumentContext document_context{state.memory,docs};s::NativeDocumentClearContext context{state.memory,nodes,docs};
    void construct(){(void)s::construct_native_document_001830(State::base,1,0,document_context);}
    void clear(){docs.clear_002e60(State::base);}
    void delete_nodes(std::uintptr_t owner=State::base){docs.delete_nodes_003110(owner);}
};
} // namespace clear_test
