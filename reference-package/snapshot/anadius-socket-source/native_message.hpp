#pragma once
#include "request_growth.hpp"
#include "response_hex.hpp"
#include "response_containers.hpp"
#include <atomic>

namespace anadius::socket_recovered {
// Explicit caller-owned projection of the native automatic document/string
// locals. Fresh per invocation, nonaliasing and mapped in strings.memory;
// no shared scratch, reentrancy, MSVC object layout or EH implementation implied.
struct NativeMessageDocument {int error_6c=0;};
struct NativeMessageLocals {NativeMessageDocument document;ReceiveBufferString response,temporary;};
// One shared count cell: the native pre-lock peek is relaxed and cannot wait
// on the event mutex. Map/payload access and count mutations still require the
// actual queue lock. Copies are value snapshots, never another queue owner.
class NativeMessageCount final {
public:
    NativeMessageCount(std::uint64_t value=0)noexcept:value_(value){}
    NativeMessageCount(const NativeMessageCount& other)noexcept:value_(other.load()){}
    NativeMessageCount& operator=(const NativeMessageCount& other)noexcept{return *this=other.load();}
    NativeMessageCount& operator=(std::uint64_t value)noexcept{value_.store(value,std::memory_order_relaxed);return *this;}
    std::uint64_t load()const noexcept{return value_.load(std::memory_order_relaxed);}
    operator std::uint64_t()const noexcept{return load();}
    std::uint64_t operator++()noexcept{return value_.fetch_add(1,std::memory_order_relaxed)+1;}
    std::uint64_t operator--()noexcept{return value_.fetch_sub(1,std::memory_order_relaxed)-1;}
private:
    std::atomic<std::uint64_t> value_;
};
static_assert(std::atomic<std::uint64_t>::is_always_lock_free,"Native event count peek must not hide a library mutex");
struct NativeMessageEventQueue {
    std::span<ReceiveBufferString*> map_08;std::uint64_t map_size_10=0,offset_18=0;NativeMessageCount count_20=0;
};
struct NativeMessageMutex {std::int32_t lock_count_4c=0;};
enum class NativeMessageLog {received,response,event,bad_xml,decryption_failure};
class NativeMessageApi {
public:
    virtual ~NativeMessageApi()=default;
    virtual void construct_document_001830(NativeMessageDocument&,bool,std::uint32_t)=0;
    virtual int parse_document_004280(NativeMessageDocument&,std::uintptr_t,std::uint64_t)=0;
    virtual void destroy_document_001e00(NativeMessageDocument&)=0;
    virtual bool decrypt_042480(std::uintptr_t handler,ReceiveBufferString&,std::uint32_t)=0;
    virtual bool encrypt_042610(std::uintptr_t handler,ReceiveBufferString&,std::uint32_t)=0;
    virtual ReceiveBufferString& dispatch_032d00(ReceiveBufferString&,std::uintptr_t handler,NativeMessageDocument&)=0;
    // Defined log groups stand for006880/0088e0/007990 operations; no original
    // stream, callback-intermediate log ABI or native log helpers recovered.
    virtual void log(NativeMessageLog,NativeMessageDocument&,ReceiveBufferString&)=0;
    // Mandatory lower helpers with the exact defined native arguments. They
    // are not replaced by a claim that010040 is called by native033d70.
    virtual void grow_nul_012350(ReceiveBufferString&,std::uint64_t extra,std::uint64_t hint,std::uint8_t)=0;
    virtual void grow_append_007e30(ReceiveBufferString&,std::uint64_t extra,std::uint64_t hint,std::uintptr_t source,std::uint64_t count)=0;
    virtual NativeMessageEventQueue& event_queue_02c400()=0;
    virtual NativeMessageMutex& event_mutex_02c620()=0;
    virtual int lock_04a6ac(NativeMessageMutex&)=0;
    virtual void unlock_04a6b4(NativeMessageMutex&)=0;
    [[noreturn]]virtual void thread_error_04a648(int)=0;
};
struct NativeMessageBinding {std::uintptr_t handler_f0=0;std::uint8_t* encrypted_c4=nullptr;};
struct NativeMessageContext {
    std::span<NativeMessageBinding> handlers;const std::uint8_t& show_lsx_a3ef0;
    ReceiveBufferContext& strings;RequestMutationContext& mutation;
    NativeMessageLocals& locals;NativeMessageApi& api;
};
ReceiveBufferString& process_native_message_033d70(std::uintptr_t handler_f0,
    ReceiveBufferString& response,ReceiveBufferString& request,NativeMessageContext&);
// Selects the typed normal source in028d30 instead of the historical N5
// deferred-writeback semantic adapter. Lower native providers/ABI/EH stay open.
class RecoveredNativeMessageApi:public RecoveredResponseDispatchApi {
public:
    RecoveredNativeMessageApi(ResponseDispatchContext& responses,NativeMessageContext& context)
        :RecoveredResponseDispatchApi(responses),context_(context){}
    void process_message_033d70(std::uintptr_t,ReceiveBufferString&,ReceiveBufferString&) final;
private:NativeMessageContext& context_;
};
} // namespace anadius::socket_recovered
