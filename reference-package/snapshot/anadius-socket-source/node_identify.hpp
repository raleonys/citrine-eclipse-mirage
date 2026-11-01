#pragma once
#include "node_parse.hpp"
#include <array>

namespace anadius::socket_recovered {
struct NativeNodeIdentifyIdentities {
    std::array<std::uintptr_t,5> token_slots{
        0x1800a10a8ULL,0x1800a10b0ULL,0x1800a10b8ULL,0x1800a10c0ULL,0x1800a10c8ULL};
};
// Separate constructor bodies remain mandatory. Their image addresses label
// observed calls; they are never cast to original-image host function pointers.
class NativeNodeIdentifyEffects {
public:
    virtual ~NativeNodeIdentifyEffects()=default;
    virtual std::uintptr_t construct_001610(std::uintptr_t document,std::uintptr_t pool)=0;
    virtual std::uintptr_t construct_001590(std::uintptr_t document,std::uintptr_t pool)=0;
    virtual std::uintptr_t construct_001720(std::uintptr_t document,std::uintptr_t pool)=0;
    virtual std::uintptr_t construct_0017b0(std::uintptr_t document,std::uintptr_t pool)=0;
    virtual std::uintptr_t construct_001690(std::uintptr_t document,std::uintptr_t pool)=0;
};
struct NativeNodeIdentifyContext {
    ReceiveBufferMemory& memory;NativeParseEntryEffects& characters;
    NativeNodeParseEffects& parser;NativeNodeIdentifyEffects& constructors;
    NativeNodeIdentifyIdentities identities{};
};
std::uintptr_t identify_node_0038b0(std::uintptr_t document,std::uintptr_t input,
    std::uintptr_t out_node,NativeNodeIdentifyContext&);

class RecoveredNodeIdentifyEffects:public RecoveredNodeParseEffects,
    public NativeNodeIdentifyEffects {
public:
    RecoveredNodeIdentifyEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments,NativeNodeParseIdentities parse_ids={},
        NativeNodeIdentifyIdentities identify_ids={})
        :RecoveredNodeParseEffects(memory,nodes,arguments,parse_ids),identify_{memory,*this,*this,*this,identify_ids}{}
    std::uintptr_t identify_0038b0(std::uintptr_t,std::uintptr_t,std::uintptr_t) final;
private:NativeNodeIdentifyContext identify_;
};
} // namespace anadius::socket_recovered
