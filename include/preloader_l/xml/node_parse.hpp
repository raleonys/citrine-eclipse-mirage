#pragma once
#include "preloader_l/xml/document_error.hpp"

namespace preloader_l::socket_recovered {
// Caller-owned mapped invocation workspace. It represents native local slots,
// not a recovered host stack frame or a native allocation/lifetime protocol.
struct NativeNodeParseScratch {std::uintptr_t out_node,local_pair;};
struct NativeNodeParseError {
    std::uintptr_t document;std::int32_t code;std::uint32_t line;
    std::uintptr_t format;bool has_pointer;std::uintptr_t pointer;
};
struct NativeNodeParseIdentities {
    std::uintptr_t node_parse_target=0x180004970ULL;
    std::uintptr_t declaration_format=0x180080000ULL;
    std::uintptr_t element_format=0x180080018ULL;
    std::uintptr_t depth_format=0x180080098ULL;
};
class NativeNodeParseEffects {
public:
    virtual ~NativeNodeParseEffects()=default;
    virtual std::uintptr_t identify_0038b0(std::uintptr_t document,
        std::uintptr_t cursor,std::uintptr_t out_node)=0;
    virtual std::uintptr_t parse_child_virtual(std::uintptr_t target,std::uintptr_t node,
        std::uintptr_t cursor,std::uintptr_t closing_pair,std::uintptr_t line_counter)=0;
    virtual std::uintptr_t materialize_0035e0(std::uintptr_t pair)=0;
    virtual std::int32_t compare_05c550(std::uintptr_t left,std::uintptr_t right,
        std::uint64_t limit)=0;
    virtual void prepare_003aa0(std::uintptr_t owner,std::uintptr_t child)=0;
    virtual void free_04b0f8(std::uintptr_t pointer)=0;
    // Explicit pointer-vararg transport. Implementations must establish the
    // corresponding invocation context before using the N24 source helper.
    virtual void report_error(const NativeNodeParseError&)=0;
};
struct NativeNodeParseContext {
    ReceiveBufferMemory& memory;NativeNodeClearEffects& nodes;
    NativeNodeParseEffects& lower;NativeNodeParseIdentities identities{};
};
std::uintptr_t parse_node_004970(std::uintptr_t owner,std::uintptr_t cursor,
    std::uintptr_t closing_pair,std::uintptr_t line_counter,NativeNodeParseScratch,
    NativeNodeParseContext&);

// Selected source dispatch only. Other targets, invocation workspace and
// parser primitives remain mandatory; the generic virtual engine is open.
class RecoveredNodeParseEffects:public RecoveredDocumentErrorEffects,
    public NativeNodeParseEffects {
public:
    RecoveredNodeParseEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments,NativeNodeParseIdentities identities={})
        :RecoveredDocumentErrorEffects(memory,nodes,arguments),parse_{memory,nodes,*this,identities}{}
    void parse_virtual(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t) final;
    std::uintptr_t parse_child_virtual(std::uintptr_t,std::uintptr_t,std::uintptr_t,
        std::uintptr_t,std::uintptr_t) final;
    virtual NativeNodeParseScratch acquire_parse_scratch(std::uintptr_t node,
        std::uintptr_t cursor,std::uintptr_t closing_pair,std::uintptr_t line_counter)=0;
    virtual std::uintptr_t parse_other_virtual(std::uintptr_t target,std::uintptr_t node,
        std::uintptr_t cursor,std::uintptr_t closing_pair,std::uintptr_t line_counter)=0;
private:NativeNodeParseContext parse_;
};
} // namespace preloader_l::socket_recovered
