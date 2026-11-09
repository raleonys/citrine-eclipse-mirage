// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_native_dispatch.hpp"

namespace citrine_eclipse_mirage::socket {
// Selected image identities are stored as values only. No image vtable is
// called or materialized. The constructor touches through+0x307; this is not
// a claim about the complete object size or MSVC class ABI.
struct NativeDocumentIdentities {
    std::uintptr_t document=0x18007fec8ULL;
    std::uintptr_t pool120=0x18007fe08ULL,pool80=0x18007fe38ULL;
    std::uintptr_t pool112=0x18007fe68ULL,pool104=0x18007fe98ULL;
};
class NativeDocumentEffects {
public:
    virtual ~NativeDocumentEffects()=default;
    virtual void clear_002e60(std::uintptr_t)=0;
    virtual void set_error_005820(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t)=0;
    virtual std::uintptr_t allocate_04b100(std::uint64_t)=0;
    virtual void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)=0;
    virtual void parse_0041b0(std::uintptr_t)=0;
    virtual void delete_nodes_003110(std::uintptr_t)=0;
    virtual void clear_pool_002d70(std::uintptr_t)=0;
    virtual void clear_pool_002e10(std::uintptr_t)=0;
    virtual void clear_pool_002dc0(std::uintptr_t)=0;
    virtual void destroy_pool_001c30(std::uintptr_t)=0;
    virtual void destroy_pool_001cb0(std::uintptr_t)=0;
    virtual void destroy_pool_001bb0(std::uintptr_t)=0;
    virtual void destroy_pool_001d30(std::uintptr_t)=0;
    // Only RCX is defined at001e5f/001e74; do not invent a sized-free argument.
    virtual void free_04b0f8(std::uintptr_t)=0;
    virtual void destroy_node_base_001ea0(std::uintptr_t)=0;
};
struct NativeDocumentContext {
    ReceiveBufferMemory& memory;NativeDocumentEffects& effects;
    NativeDocumentIdentities identities{};
};
std::uintptr_t construct_native_document_001830(std::uintptr_t,std::uint8_t,std::uint32_t,NativeDocumentContext&);
std::int32_t parse_native_document_004280(std::uintptr_t,std::uintptr_t,std::uint64_t,NativeDocumentContext&);
void destroy_native_document_001e00(std::uintptr_t,NativeDocumentContext&);
struct NativeDocumentBinding {NativeMessageDocument* projection=nullptr;std::uintptr_t address=0;};
// NativeMessageDocument::error_6c is the actual mapped four-byte cell at+6c.
// No copied status/writeback adapter. Other document bytes are caller-owned.
class MessageDocumentApi:public MessageDispatchApiAdapter {
public:
    MessageDocumentApi(PrefixCipherContext& cipher,std::span<PrefixCipherBinding> ciphers,
        NativeDispatchContext& dispatch,NativeDocumentContext& documents,std::span<NativeDocumentBinding> bindings)
        :MessageDispatchApiAdapter(cipher,ciphers,dispatch),documents_(documents),bindings_(bindings){}
    void construct_document_001830(NativeMessageDocument&,bool,std::uint32_t) final;
    int parse_document_004280(NativeMessageDocument&,std::uintptr_t,std::uint64_t) final;
    void destroy_document_001e00(NativeMessageDocument&) final;
private:
    std::uintptr_t resolve(NativeMessageDocument&);
    NativeDocumentContext& documents_;std::span<NativeDocumentBinding> bindings_;
};
} // namespace citrine_eclipse_mirage::socket
