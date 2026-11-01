#pragma once
#include "node_base.hpp"

namespace anadius::socket_recovered {
// Required lower effects. No locale/classifier implementation or native parse
// virtual target is supplied by this stage. The target is an identity only;
// it must never be cast to an original-image host function pointer.
class NativeParseEntryEffects {
public:
    virtual ~NativeParseEntryEffects()=default;
    virtual std::int32_t classify_051fc0(std::uint32_t byte)=0;
    // The void entry projection discards the tail target's return value. These
    // are observed register arguments, not a recovered native callee ABI.
    virtual void parse_virtual(std::uintptr_t target,std::uintptr_t document,
        std::uintptr_t cursor,std::uintptr_t zero_argument,std::uintptr_t line_counter)=0;
};
struct NativeParseEntryContext {
    ReceiveBufferMemory& memory;NativeDocumentEffects& documents;NativeParseEntryEffects& lower;
};
void parse_document_entry_0041b0(std::uintptr_t document,NativeParseEntryContext&);

class RecoveredParseEntryEffects:public RecoveredNodeBaseEffects,public NativeParseEntryEffects {
public:
    RecoveredParseEntryEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :RecoveredNodeBaseEffects(memory,nodes),parse_entry_{memory,*this,*this}{}
    void parse_0041b0(std::uintptr_t) final;
private:NativeParseEntryContext parse_entry_;
};
} // namespace anadius::socket_recovered
