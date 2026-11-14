// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/parse_entry.hpp"

namespace laureondo::socket {
// An explicitly supplied invocation identity for the native variadic tail.
// This is neither a host va_list nor proof of a native caller-stack ABI.
struct NativeErrorArgumentTail {std::uintptr_t opaque_cursor=0;};
struct NativeDocumentErrorIdentities {
    std::uintptr_t error_names=0x1800a1010ULL;
    std::uintptr_t header_format=0x180080068ULL;
    std::uintptr_t separator_format=0x180080094ULL;
};
class NativeDocumentErrorEffects {
public:
    virtual ~NativeDocumentErrorEffects()=default;
    virtual void format_header_0060a0(std::uintptr_t destination,std::uint64_t capacity,
        std::uintptr_t format,std::uintptr_t name,std::int32_t error_id,
        std::uint32_t hex_id,std::uint32_t line)=0;
    virtual void format_separator_0060a0(std::uintptr_t destination,
        std::uint64_t capacity,std::uintptr_t format)=0;
    virtual void format_detail_006110(std::uintptr_t destination,
        std::uint64_t capacity,std::uintptr_t format,std::uintptr_t opaque_arguments)=0;
};
struct NativeDocumentErrorContext {
    ReceiveBufferMemory& memory;NativeDocumentEffects& documents;
    NativeDocumentErrorEffects& lower;NativeDocumentErrorIdentities identities{};
};
void set_document_error_005820(std::uintptr_t document,std::int32_t error_id,
    std::uint32_t line,std::uintptr_t detail_format,NativeErrorArgumentTail arguments,
    NativeDocumentErrorContext&);

// The invoker must supply the argument-tail context for each call. Formatting,
// including a nonzero detail format, remains a mandatory lower effect. This
// source bridge does not capture or synthesize a native caller's varargs.
class DocumentErrorEffects:public ParseEntryEffects,
    public NativeDocumentErrorEffects {
public:
    DocumentErrorEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments)
        :ParseEntryEffects(memory,nodes),error_{memory,*this,*this},arguments_(arguments){}
    void set_error_005820(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t) final;
private:
    NativeDocumentErrorContext error_;const NativeErrorArgumentTail& arguments_;
};
} // namespace laureondo::socket
