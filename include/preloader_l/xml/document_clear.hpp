// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/xml/native_document.hpp"

namespace preloader_l::socket {
// Raw virtual target values are projected through mandatory effects. Nothing
// here casts a shipped vtable address to a callable host function pointer.
class NativeNodeClearEffects {
public:
    virtual ~NativeNodeClearEffects()=default;
    virtual std::uint64_t node_probe(std::uintptr_t target,std::uintptr_t node)=0;
    virtual void node_destroy(std::uintptr_t target,std::uintptr_t node,std::uint32_t flags)=0;
    virtual void pool_release(std::uintptr_t target,std::uintptr_t pool,std::uintptr_t node)=0;
    virtual void pool_track(std::uintptr_t target,std::uintptr_t pool)=0;
    virtual void remove_parent_003030(std::uintptr_t parent,std::uintptr_t node)=0;
    virtual void delete_node_003240(std::uintptr_t node)=0;
};
struct NativeDocumentClearContext {
    ReceiveBufferMemory& memory;NativeNodeClearEffects& nodes;NativeDocumentEffects& documents;
};
void delete_native_nodes_003110(std::uintptr_t owner,NativeDocumentClearContext&);
void clear_native_document_002e60(std::uintptr_t document,NativeDocumentClearContext&);
class DocumentClearEffects:public NativeDocumentEffects {
public:
    DocumentClearEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :clear_{memory,nodes,*this}{}
    DocumentClearEffects(const DocumentClearEffects&)=delete;
    DocumentClearEffects& operator=(const DocumentClearEffects&)=delete;
    DocumentClearEffects(DocumentClearEffects&&)=delete;
    DocumentClearEffects& operator=(DocumentClearEffects&&)=delete;
    void clear_002e60(std::uintptr_t) final;
    void delete_nodes_003110(std::uintptr_t) final;
private:NativeDocumentClearContext clear_;
};
} // namespace preloader_l::socket
