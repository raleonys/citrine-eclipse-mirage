// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/parse_entry.hpp"
#include "tests/support/socket/node_base_test_fixture.hpp"

namespace parse_entry_test {
namespace s=laureondo::socket;
using node_base_test::check;using node_base_test::checks;using node_base_test::word;using node_base_test::put;using node_base_test::Failure;
using node_base_test::unsupported;using node_base_test::failure;
struct CoreDocs:s::NativeDocumentEffects {
    struct Error{std::uintptr_t document;std::int32_t code;std::uint32_t line;std::uintptr_t detail;};
    std::vector<Error> errors;std::function<void(const Error&)> error_hook;unsigned other_calls=0;
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected document effect in parse-entry core test");}
    void set_error_005820(std::uintptr_t p,std::int32_t c,std::uint32_t l,std::uintptr_t d)override{errors.push_back({p,c,l,d});if(error_hook)error_hook(errors.back());}
    void clear_002e60(std::uintptr_t)override{unexpected();}
    std::uintptr_t allocate_04b100(std::uint64_t)override{unexpected();}
    void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)override{unexpected();}
    void parse_0041b0(std::uintptr_t)override{unexpected();}
    void delete_nodes_003110(std::uintptr_t)override{unexpected();}
    void clear_pool_002d70(std::uintptr_t)override{unexpected();}
    void clear_pool_002e10(std::uintptr_t)override{unexpected();}
    void clear_pool_002dc0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001c30(std::uintptr_t)override{unexpected();}
    void destroy_pool_001cb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001bb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001d30(std::uintptr_t)override{unexpected();}
    void free_04b0f8(std::uintptr_t)override{unexpected();}
    void destroy_node_base_001ea0(std::uintptr_t)override{unexpected();}
};
struct Lower:s::NativeParseEntryEffects {
    struct Virtual{std::uintptr_t target,document,cursor,zero,line;};
    std::vector<std::uint32_t> classified;std::vector<Virtual> virtuals;
    std::function<std::int32_t(std::uint32_t)> classifier;std::function<void(const Virtual&)> virtual_hook;
    std::int32_t classify_051fc0(std::uint32_t b)override{classified.push_back(b);return classifier?classifier(b):0;}
    void parse_virtual(std::uintptr_t t,std::uintptr_t d,std::uintptr_t c,std::uintptr_t z,std::uintptr_t l)override{virtuals.push_back({t,d,c,z,l});if(virtual_hook)virtual_hook(virtuals.back());}
};
struct Fixture {
    static constexpr std::uintptr_t base=0x390000,input_address=0x590000,table_address=0x790000,other_table=0x7a0000;
    static constexpr std::uintptr_t target=0xf101,other_target=0xf202;
    std::array<std::uint8_t,0x308> document{};std::vector<std::uint8_t> input;std::array<std::uint8_t,0x88> table{},alternate{};
    std::vector<s::ReceiveBufferRegion> regions,extras;std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};CoreDocs docs;Lower lower;s::NativeParseEntryContext context{memory,docs,lower};
    explicit Fixture(std::vector<std::uint8_t> bytes={'x',0}):input(std::move(bytes)){
        document.fill(0xa5);table.fill(0x5a);alternate.fill(0x7b);put(document,0,table_address);put(document,0x98,input_address);put(table,0x80,target);put(alternate,0x80,other_target);refresh();
    }
    void add(std::uintptr_t address,std::span<std::uint8_t> b){if(!omitted||omitted->first>=address+b.size()||omitted->first+omitted->second<=address){regions.push_back({address,b});return;}const auto start=std::max(address,omitted->first),end=std::min(address+b.size(),omitted->first+omitted->second);if(start>address)regions.push_back({address,b.first(start-address)});if(end<address+b.size())regions.push_back({end,b.subspan(end-address)});}
    void refresh(){regions.clear();add(base,document);add(input_address,input);add(table_address,table);add(other_table,alternate);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    void parse(std::uintptr_t doc=base){s::parse_document_entry_0041b0(doc,context);}
    void expect_virtual(std::uintptr_t cursor,std::uintptr_t selected=target,std::uintptr_t doc=base){check(lower.virtuals.size()==1&&lower.virtuals[0].target==selected&&lower.virtuals[0].document==doc&&lower.virtuals[0].cursor==cursor&&!lower.virtuals[0].zero&&lower.virtuals[0].line==doc+0xa0&&docs.errors.empty()&&!docs.other_calls,"exact controlled virtual target identity/register arguments; no callable image pointer or native slot80 claim");}
    void expect_error(){check(docs.errors.size()==1&&docs.errors[0].document==base&&docs.errors[0].code==13&&!docs.errors[0].line&&!docs.errors[0].detail&&lower.virtuals.empty()&&!docs.other_calls,"empty entry tail calls mandatory error provider with13,0,0 and no virtual parse");}
};
inline std::int32_t controlled_space(std::uint32_t b){return b==' '||b=='\n'||b=='\r'||b=='\t';}
inline void prefix(std::span<std::uint8_t> bytes,bool bom=false){put(bytes,0xa0,1,4);put(bytes,0x30,1,4);bytes[0x68]=static_cast<std::uint8_t>(bom);}
} // namespace parse_entry_test
