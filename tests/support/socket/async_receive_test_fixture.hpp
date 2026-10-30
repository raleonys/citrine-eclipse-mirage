#pragma once
#include "preloader_l/net/transport_async_receive.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace receive_test {
namespace s=preloader_l::socket_recovered;
inline unsigned checks=0;
inline void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
[[noreturn]]inline void unexpected(){throw std::logic_error("Unexpected synthetic receive dependency");}
template<class E=s::UnsupportedInput,class F>void rejects(F action){bool caught=false;try{action();}catch(const E&){caught=true;}check(caught,"required failure propagates");}
struct Gate:s::SocketGate {unsigned enters=0,leaves=0;void enter()override{++enters;}void leave()override{++leaves;}};
struct Entry:s::IocpSchedulerEntry {using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unexpected();}};
struct Ref:s::IoReference {
    unsigned disposes=0,deletes=0;std::function<void()> dispose,erase;
    void dispose_slot_00()override{++disposes;if(dispose)dispose();}
    void delete_slot_08()override{++deletes;if(erase)erase();}
};
struct Worker:s::WorkerRunApi {
    unsigned gets=0;void* tls=nullptr;
    void* tls_get(std::uint32_t n)override{check(n==17,"live receive TLS index");++gets;return tls;}
    std::int32_t tls_set(std::uint32_t,void*)override{unexpected();}
    std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{unexpected();}
    std::uint32_t get_last_error()override{unexpected();}
};
struct Errors:s::Api {
    unsigned categories=0;std::function<void()> hook;
    s::Category system_category_028620()override{++categories;if(hook)hook();return {0x2222};}
    void set_last_error(std::int32_t)override{unexpected();}std::int32_t get_last_error()override{unexpected();}
    s::Conversion convert_acp(const char*,std::int32_t)override{unexpected();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unexpected();}
    s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unexpected();}
    std::int32_t select_write(s::Socket,std::int32_t)override{unexpected();}
};
struct Completion:s::DispatcherApi {
    unsigned posts=0;std::int32_t result=1;std::function<void()> hook;
    std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED*)override{
        check(port==0xabcdef&&bytes==0&&key==2,"receive publication uses actual key2/zero bytes contract");++posts;if(hook)hook();return result;
    }
    void set_last_error(std::uint32_t)override{unexpected();}std::uint32_t get_last_error()override{unexpected();}
    std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t)override{unexpected();}
    std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unexpected();}
};
struct Memory:s::PostedMemoryEffects {
    std::array<std::byte,1025> bytes{};s::OperationStorage storage{0x10001,bytes};
    std::vector<std::uint64_t> allocations;unsigned frees=0;std::function<void()> free;
    s::OperationStorage& allocate_04b0bc(std::uint64_t n)override{allocations.push_back(n);bytes.fill(std::byte{0xa5});return storage;}
    void free_04b788(s::OperationStorage& p)override{check(&p==&storage,"free same receive allocation");++frees;if(free)free();}
    s::PostedOperation& project_posted(s::OperationStorage&)override{unexpected();}
};
struct Receive:s::AsyncReceiveApi {
    unsigned calls=0;s::AsyncReceiveResult answer{-1,997,0};s::Buffer observed{};std::uint32_t flags=0;
    std::function<void()> hook;
    s::AsyncReceiveResult receive(s::Socket socket,s::Buffer& b,std::uint32_t f,OVERLAPPED& ov)override{
        check(socket==77,"live receive socket");check(!ov.Internal&&!ov.InternalHigh&&!ov.Pointer&&!ov.hEvent,"SDK receive OVERLAPPED zeroed");
        ++calls;observed=b;flags=f;if(hook)hook();return answer;
    }
};
struct Effects:s::AsyncReceiveEffects {
    std::unique_ptr<s::AsyncReceiveOperation> operation;s::DispatcherOperationBinding packet;unsigned constructs=0;
    std::function<void(s::IoHandlerEnvironment&,std::uintptr_t)> construct;
    s::AsyncReceiveOperation& project_async_receive(s::OperationStorage& storage)override{
        operation=std::make_unique<s::AsyncReceiveOperation>(storage);
        packet={&operation->io.operation,{-77,{0xdead}},99,&operation->io.operation.overlapped};operation->packet=&packet;
        operation->io.operation.error={-12,{0xfeed}};return *operation;
    }
    void construct_environment_023dd0(s::IoHandlerEnvironment& e,std::uintptr_t id)override{++constructs;if(construct)construct(e,id);}
};
struct Fixture {
    Gate gate;CRITICAL_SECTION cs{};Entry entry{gate,cs};Worker worker;Memory memory;Receive receive;Completion completion;Errors errors;Effects effects;
    std::uint32_t index=17;std::array<s::OperationStorageBinding,1> storage{{{memory.storage.identity,&memory.storage}}};
    s::IoCompletionEnvironment environment{{index,storage},{}};
    s::AsyncReceiveService service{&entry};s::ReceiveSocketImplementation implementation{77,0,0,nullptr};
    s::AsyncReceiveContext context{environment,worker,memory,receive,completion,errors,effects};
    s::ReceiveHandler source;s::WorkerThreadState thread{};s::WorkerTlsContext tls{nullptr,&thread,nullptr};
    Fixture(){entry.completion.port=0xabcdef;source.words_60_78={0x1111,0x2222,0x3333,0x4444};source.byte_80=0x81;source.dword_84=0x87654321;source.words_88_98={0x8888,0x9999,0xaaaa};source.object_a0=0xbeef;}
    void start(s::ReceiveBufferSlice& buffer,std::uint32_t flags=0x12345678){s::start_async_receive_021d10(service,implementation,buffer,flags,source,0x987,context);}
};
} // namespace receive_test
