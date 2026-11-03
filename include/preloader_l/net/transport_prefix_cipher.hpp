// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_append_growth.hpp"
#include "preloader_l/codec/crypto.hpp"

namespace preloader_l::socket {
// Typed state projection, not a native MSVC byte layout. Integrator owns the
// live mode/IV and supplies the coherent existing AES key-schedule source.
struct PrefixCipherState {
    std::uint32_t mode_00=0;const crypto::Cipher& schedule;crypto::Key iv_b4{};
};
class PrefixCipherBlocks {
public:
    virtual ~PrefixCipherBlocks()=default;
    virtual void decrypt_ecb_045620(PrefixCipherState&,std::uintptr_t)=0;
    virtual void encrypt_ecb_045630(PrefixCipherState&,std::uintptr_t)=0;
    virtual void decrypt_cbc_0454e0(PrefixCipherState&,std::uintptr_t,std::uint64_t)=0;
    virtual void encrypt_cbc_045580(PrefixCipherState&,std::uintptr_t,std::uint64_t)=0;
};
class PrefixCipherBlocksAdapter:public PrefixCipherBlocks {
public:
    explicit PrefixCipherBlocksAdapter(ReceiveBufferMemory& memory):memory_(memory){}
    void decrypt_ecb_045620(PrefixCipherState&,std::uintptr_t) final;
    void encrypt_ecb_045630(PrefixCipherState&,std::uintptr_t) final;
    void decrypt_cbc_0454e0(PrefixCipherState&,std::uintptr_t,std::uint64_t) final;
    void encrypt_cbc_045580(PrefixCipherState&,std::uintptr_t,std::uint64_t) final;
private:ReceiveBufferMemory& memory_;
};
struct PrefixCipherContext {ReceiveBufferContext& strings;PrefixCipherBlocks& blocks;};
// Native return's AL only. Raw64 offset arithmetic, live prefix and native
// mutation ordering retained; no std::string staging/offset prevalidation.
bool decrypt_prefix_042480(PrefixCipherState&,ReceiveBufferString&,std::uint64_t offset,PrefixCipherContext&);
bool encrypt_prefix_042610(PrefixCipherState&,ReceiveBufferString&,std::uint64_t offset,PrefixCipherContext&);
struct PrefixCipherBinding {std::uintptr_t handler_f0=0;PrefixCipherState* state=nullptr;};
class MessageCipherApi:public MessageAppendApi {
public:
    MessageCipherApi(PrefixCipherContext& context,std::span<PrefixCipherBinding> bindings)
        :MessageAppendApi(context.strings),context_(context),bindings_(bindings){}
    // Historical NativeMessageApi's third uint32 is used only as offset0 by
    //033d70. Direct bodies above preserve the full native uint64 offset domain.
    bool decrypt_042480(std::uintptr_t,ReceiveBufferString&,std::uint32_t offset) final;
    bool encrypt_042610(std::uintptr_t,ReceiveBufferString&,std::uint32_t offset) final;
private:PrefixCipherContext& context_;std::span<PrefixCipherBinding> bindings_;
};
} // namespace preloader_l::socket
