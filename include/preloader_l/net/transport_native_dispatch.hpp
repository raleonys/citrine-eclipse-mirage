// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_cipher_session.hpp"

namespace preloader_l::socket {
struct NativeDispatchEntry {std::uint32_t handler_rva=0;};
// The native stack reuses the command-name string as the discarded response
// copy. Caller provides one nonaliasing live prefix mapped in strings.memory.
struct NativeDispatchLocals {ReceiveBufferString scratch;};
class NativeDispatchEffects {
public:
    virtual ~NativeDispatchEffects()=default;
    // Mandatory native static-init/TLS/RB-tree boundary, before XML validation.
    virtual void prepare_registry(std::span<const messages::Registration>)=0;
    virtual NativeDispatchEntry* find_exact(ReceiveBufferString&)=0;
    virtual NativeDispatchEntry* registry_end()=0;
    virtual const messages::XmlDocument& document(NativeMessageDocument&)=0;
    virtual void construct_name_006b30(ReceiveBufferString&,std::string_view)=0;
    virtual messages::XmlPrinter& construct_printer_0019b0(std::uintptr_t,std::uint8_t,std::int32_t)=0;
    virtual bool registered_handler(std::uint32_t,std::uintptr_t,const messages::XmlNode&,messages::XmlPrinter&)=0;
    virtual bool challenge_02d7e0(std::uintptr_t,const messages::XmlNode&,messages::XmlPrinter&)=0;
    virtual std::int32_t printer_size(messages::XmlPrinter&)=0;
    virtual std::span<const std::uint8_t> printer_data(messages::XmlPrinter&)=0;
    virtual void reserve_02b300(ReceiveBufferString&,std::uint64_t)=0;
    virtual void append_018b30(ReceiveBufferString&,std::span<const std::uint8_t>,std::uint64_t)=0;
    virtual ReceiveBufferString& construct_output_009fd0(ReceiveBufferString&,std::span<const std::uint8_t>)=0;
    virtual void free_scratch_00b520(ReceiveBufferString&,std::uintptr_t,std::uint64_t)=0;
    virtual void destroy_printer_001f60(messages::XmlPrinter&)=0;
    [[noreturn]]virtual void invalid_message_02bd30_04d228()=0;
};
struct NativeDispatchContext {ReceiveBufferContext& strings;NativeDispatchLocals& locals;NativeDispatchEffects& effects;};
// Per-call032d00 normal projection. Registry creation, native DOM/printer/string
// helpers and exception ABI stay explicit mandatory providers. No EH rollback.
ReceiveBufferString& dispatch_native_032d00(ReceiveBufferString&,std::uintptr_t handler,
    NativeMessageDocument&,NativeDispatchContext&);
class MessageDispatchApiAdapter:public MessageCipherApi {
public:
    MessageDispatchApiAdapter(PrefixCipherContext& cipher,std::span<PrefixCipherBinding> bindings,NativeDispatchContext& dispatch)
        :MessageCipherApi(cipher,bindings),dispatch_(dispatch){}
    ReceiveBufferString& dispatch_032d00(ReceiveBufferString&,std::uintptr_t,NativeMessageDocument&) final;
private:NativeDispatchContext& dispatch_;
};
} // namespace preloader_l::socket
