// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/node_insertion.hpp"
#include <array>

namespace laureondo::socket {
// Ten native local output bytes and a signed DWORD count are projected as
// caller-supplied effects, not a native stack frame or decoder implementation.
class NativeNodeStringEffects {
public:
    virtual ~NativeNodeStringEffects()=default;
    virtual std::uintptr_t decode_0033c0(std::uintptr_t input,
        std::span<std::uint8_t,10> output,std::int32_t& count)=0;
    virtual void copy_string_078d30(std::uintptr_t output,
        std::span<const std::uint8_t,10> scratch,std::uint64_t count)=0;
};
struct NativeNodeStringIdentities {std::uintptr_t entity_table=0x18007f760ULL;};
struct NativeNodeStringContext {
    ReceiveBufferMemory& memory;NativeParseEntryEffects& characters;
    NativeNodeParseEffects& parser;NativeNodeStringEffects& lower;
    NativeNodeStringIdentities identities{};
};
std::uintptr_t materialize_node_string_0035e0(std::uintptr_t pair,NativeNodeStringContext&);

class NodeStringEffects:public NodeInsertionEffects,public NativeNodeStringEffects {
public:
    NodeStringEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments,PoolAllocationDispatch& allocator,
        NativeNodeParseIdentities parse_ids={},NativeNodeIdentifyIdentities identify_ids={},
        NativeNodeConstructorIdentities constructor_ids={},NativeNodeStringIdentities string_ids={})
        :NodeInsertionEffects(memory,nodes,arguments,allocator,parse_ids,identify_ids,constructor_ids),
         string_{memory,*this,*this,*this,string_ids}{}
    std::uintptr_t materialize_0035e0(std::uintptr_t) final;
private:NativeNodeStringContext string_;
};
} // namespace laureondo::socket
