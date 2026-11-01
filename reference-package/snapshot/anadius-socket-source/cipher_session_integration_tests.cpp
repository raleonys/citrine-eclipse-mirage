#include "cipher_session_test_fixture.hpp"
using namespace core_test;
using container_test::active_container;
using container_test::borrowed_containers;
using owned_test::active_owned;
namespace {
struct Loop;
void message_copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& in){out.payload_00=in.payload_00;out.state_18=in.state_18;}
struct Send:s::AsyncSendApi {
    Loop& f;explicit Send(Loop& f):f(f){}
    s::AsyncSendResult send(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override;
};
struct Effects:s::ComposedWriteEffects,s::IoCompletionEffects {
    Loop& f;explicit Effects(Loop& f):f(f){}
    std::unique_ptr<s::AsyncSendOperation> operation;s::DispatcherOperationBinding packet;
    s::AsyncSendOperation& project_async_send(s::OperationStorage&)override;
    void construct_environment_023dd0(s::IoHandlerEnvironment&,std::uintptr_t)override;
    void finish_write_025a70(s::SendHandler&,s::Error&)override;
    s::Category misc_category_0285b0()override{return {0x2222};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{r::unexpected();}
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override;
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{r::unexpected();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{r::unexpected();}
};
struct Finish:s::WriteFinishApi {
    Loop& f;explicit Finish(Loop& f):f(f){}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
    [[noreturn]]void invalid_parameter_05c974()override{r::unexpected();}
    void next_write_02a500(std::uintptr_t)override;
    s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{r::unexpected();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{r::unexpected();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{r::unexpected();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{r::unexpected();}void flush_00e1e0(s::WriteFinishStream&)override{r::unexpected();}
};
struct App:s::RecoveredNativeMessageApi {
    message_test::Fixture& m;App(s::ResponseDispatchContext& c,s::NativeMessageContext& core,message_test::Fixture& m):RecoveredNativeMessageApi(c,core),m(m){}
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{m.api.destroy_empty_message_00aa50(value);}
    void cleanup_exit_waits_04a47c()override{r::unexpected();}
};
struct Loop {
    native_message_test::Fixture native;core_test::Fixture& core=native.core;container_test::ContainerFixture& containers=core.containers;owned_test::OwnedFixture& ownership=containers.ownership;response_test::Fixture& f=containers.f;r::Fixture io;Send send{*this};Effects effects{*this};Finish finish{*this};
    s::AsyncSendService service{&io.entry};s::SendSocketImplementation implementation{77,0,0,nullptr};
    std::array<s::SendStreamBinding,1> streams{{{0x12010,&service,&implementation,0x12078}}};
    s::ComposedWriteContext write{io.environment,io.worker,io.memory,send,io.completion,io.errors,effects,streams};
    s::QueuedWriteOwner owner{0x12000,&f.m.base.owner,&f.queue,&streams[0]};
    std::array<s::QueuedWriteOwnerBinding,1> owners{{{0x12000,&owner}}};
    s::WriteFinishContext finished{effects,finish,f.owners,{},containers.bytes.headers};
    anadius::crypto_recovered::Cipher oracle;s::SessionCipher session;s::PrefixCipherState& cipher=session.native();
    s::RecoveredPrefixCipherBlocks blocks{containers.bytes.memory};s::PrefixCipherContext cipher_context{native.strings,blocks};
    std::array<s::PrefixCipherBinding,1> cipher_bindings{{{0x120f0,&cipher}}};session_cipher_test::Api cipher_api{cipher_context,cipher_bindings,native.api,session};
    std::array<s::NativeMessageBinding,1> session_bindings{{{0x120f0,&session.encrypted_c4}}};
    s::NativeMessageContext selected{session_bindings,core.show,native.strings,native.mutation,native.locals,cipher_api};
    App app{containers.response,selected,f.m};s::MessageDispatchContext message{f.m.frame,app,f.m.owners,f.m.copies,f.m.closures};
    std::vector<std::string> received;std::string current;std::vector<std::uint32_t> lengths;
    unsigned sends=0,completions=0,finishes=0;bool finish_pending=false;
    explicit Loop(unsigned mode=2){if(mode==1){session.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);oracle.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);}else{session.set_key_042780(cipher_test::test_key);oracle.set_key_042780(cipher_test::test_key);}f.queued.owners=owners;f.queued.write=&write;f.invoke_borrowed=true;f.m.base.owner.strong_08=1;f.m.base.projection.object_00=0x12000;
        f.m.source.executor_28=&f.first_executor;f.m.copies[2]={&f.live,&message_copy};
        f.m.copy_hook=[&](auto& to,auto&,unsigned n){if(n==1)to.operations_10=&f.live;};
    }
    std::string dispatch(std::string payload,bool owned,unsigned mode=0,bool event=false,bool invoke=true){
        response_test::active=&f;active_owned=&ownership;message_test::running=&f.m;f.moves=f.queries=f.m.copy_calls=0;
        active_container=&containers;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed_containers;f.take_owned=true;f.invoke_owned=invoke;
        cipher_api.effects.answer=payload;session.encrypted_c4=mode==1;core.show=0;auto input=request_xml+std::string("\0tail",5);
        if(mode==2){input.insert(4," version=\"\"");native.api.codes={7};native.api.errors={1};}
        auto expected_input=input;if(mode==2)expected_input.erase(expected_input.find("version=\"\""),10);
        const bool encrypted=mode==1||mode==3;
        if(encrypted){check(oracle.encrypt_042610(input),"reference peer request encryption");input=hex(input);}
        native.seed(input);const auto event_payload=std::string("event\0tail",10);if(event)native.event(event_payload);
        auto expected=native_message_test::response(payload);
        if(encrypted)check(oracle.encrypt_042610(expected),"reference peer response cipher with same chain after request");
        if(encrypted)expected=hex(expected);expected.push_back(0);
        if(event){auto wire_event=event_payload;check(oracle.encrypt_042610(wire_event),"reference peer event cipher after response in same chain");expected+=hex(wire_event);expected.push_back(0);}
        cipher_api.effects.handling=[&,expected_input](){check(core.input()==expected_input,"handler sees actual decoded/decrypted/erased request prefix");};
        s::dispatch_message_028d30(0x12000,f.m.request.value,message);
        check(f.m.request.value.size_10==0&&f.m.request.live_blocks()==0,"native033d70 feeds actual028d30 cleanup with original live request");
        check(cipher.iv_b4==(cipher.mode_00==1?oracle.iv():anadius::crypto_recovered::Key{}),"server typed IV matches independent peer request/response/event chronology");
        return expected;
    }
    std::string handshake(std::string key,std::string version,bool owned){
        response_test::active=&f;active_owned=&ownership;message_test::running=&f.m;active_container=&containers;f.moves=f.queries=f.m.copy_calls=0;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed_containers;f.take_owned=true;f.invoke_owned=true;
        native.seed(session_cipher_test::challenge_xml(key,version));cipher_api.effects.handling={};
        const auto response=session_cipher_test::response_hex(key);const auto selected_key=session_cipher_test::selected_key(response,version);
        oracle.set_key_042780(selected_key);auto expected=session_cipher_test::challenge_answer(response);expected.push_back(0);
        s::dispatch_message_028d30(0x12000,core.request(),message);
        check(cipher.mode_00==2&&cipher.schedule.round_keys()==oracle.round_keys()&&!session.encrypted_c4,"ChallengeResponse semantic setter updates same owned schedule selected by native prefix API; flag remains plain");
        check(core.request().size_10==0&&!f.m.request.live_blocks(),"handshake request source cleaned by real caller");return expected;
    }
    void drain(){
        for(unsigned steps=0;effects.operation&&steps<32;++steps){
            auto operation=std::move(effects.operation);auto handler=operation->io.handler;operation->io.handler.control_98=nullptr;
            const auto bytes=operation->buffer_48.size;
            // Controlled completion adapter, not native027a20/IOCP/worker.
            // Actual storage release and021a80/0261d0/025a70 are executed.
            s::release_io_storage_026a00(operation->io.storage,0xd0,io.environment.storage,io.worker,io.memory);
            operation.reset();io.entry.work_count_30.fetch_sub(1);
            handler.error={0,{0x2222}};handler.transferred=bytes;++completions;
            s::invoke_send_021a80(handler,effects);
            auto* c=handler.control_98;if(c&&c->strong_08.fetch_sub(1)==1){c->dispose_slot_00();if(c->weak_0c.fetch_sub(1)==1)c->delete_slot_08();}
        }
        check(!effects.operation&&!f.queue.count_e8&&!f.queue.offset_e0&&f.m.base.owner.strong_08==1,"actual send/finish/next loop drains queue and restores initial shared reference");
        check(io.memory.allocations.size()==io.memory.frees&&!io.entry.work_count_30,"actual send storage allocation/release balanced under controlled consumer");
    }
};
s::AsyncSendOperation& Effects::project_async_send(s::OperationStorage& storage){operation=std::make_unique<s::AsyncSendOperation>(storage);packet={&operation->io.operation,{},0,&operation->io.operation.overlapped};operation->packet=&packet;return *operation;}
void Effects::construct_environment_023dd0(s::IoHandlerEnvironment&,std::uintptr_t allocator){check(allocator==0x12078,"real queued-write/send forwards owner+78 allocator identity");}
void Effects::finish_write_025a70(s::SendHandler& handler,s::Error& error){++f.finishes;f.received.push_back(f.current);f.current.clear();s::finish_write_025a70(handler,error,f.finished);}
void Effects::send_body_0261d0(s::SendHandler& handler,s::Error& error,std::uint64_t n,std::uint32_t flag){s::composed_write_0261d0(handler,error,n,flag,f.write);}
void Finish::free_sized_04b0f8(std::uintptr_t data,std::uint64_t n){f.containers.bytes.free_sized_04b0f8(data,n);}
void Finish::next_write_02a500(std::uintptr_t owner){s::start_queued_write_02a500(owner,f.f.queued);}
s::AsyncSendResult Send::send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& overlapped){
    check(socket==77&&!flags&&!overlapped.Internal&&!overlapped.InternalHigh&&!overlapped.Pointer&&!overlapped.hEvent,"actual021fd0 invokes injected AsyncSendApi with initialized OVERLAPPED");
    const auto index=(f.f.queue.map_size_d8-1)&f.f.queue.offset_e0;const auto* head=f.f.queue.map_d0[index];const auto data=head->size_10?s::receive_buffer_address(*head):0;
    const auto address=reinterpret_cast<std::uintptr_t>(buffer.buf);check(address>=data&&address-data<=head->size_10&&buffer.len<=65536&&buffer.len<=head->size_10-(address-data),"actual send pointer/progress and64KiB cap stay within queued source string");
    const auto text=f.containers.bytes.text(*head);f.current+=text.substr(address-data,buffer.len);f.lengths.push_back(buffer.len);++f.sends;return {-1,997,0};
}
void regression(){
    for(unsigned cipher_mode:{1U,2U})for(bool owned:{false,true})for(unsigned mode:{0U,1U,2U,3U}){Loop l(cipher_mode);std::vector<std::string> payloads={"alpha","",std::string(150000,'Z')};for(unsigned i=0;i<10;++i)payloads.push_back("queued-"+std::to_string(i));
        const bool encrypted=mode==1||mode==3;std::vector<std::string> expected;std::vector<std::uint32_t> chunks;unsigned i=0;
        for(const auto& payload:payloads){const auto result=l.dispatch(payload,owned,mode,encrypted&&i==2);expected.push_back(result);auto remaining=result.size();do{const auto n=std::min<std::size_t>(65536,remaining);chunks.push_back(static_cast<std::uint32_t>(n));remaining-=n;}while(remaining);++i;}
        check(l.f.queue.count_e8==13&&l.f.queue.map_size_d8==16&&l.containers.map_api.copies.size()==4&&l.sends==1,"actual prefix cipher/033d70 source bridges feed13 messages and two deque growths");
        l.drain();check(l.received==expected&&l.lengths==chunks&&l.finishes==13,"full real AES ECB/CBC XML/event hex/NUL FIFO and independent64KiB chunk oracle through N1-N10/send/finish");
        check(l.native.api.constructs==13&&l.native.api.destroys==13&&l.native.api.dispatches==13&&l.cipher_api.effects.commands.size()==13,"typed message core and controlled032d00/XML lower provider used once per request");
        check(!l.core.strings.reads&&!l.core.strings.replaces&&!l.core.strings.constructs&&!l.f.m.api.processes&&!l.native.api.decrypts&&!l.native.api.encrypts&&!l.native.api.nul_grows&&!l.native.api.append_grows,"historical deferred adapter/identity cipher/growth substitutes bypassed by final native sources");
        check(l.native.api.parses==((mode==2||mode==3)?26U:13U)&&l.native.api.locks==unsigned(encrypted)&&l.native.api.unlocks==unsigned(encrypted),"native retries/autodetection and event-lock counts remain correct with real cipher");
        check(l.f.m.request.live_blocks()==0&&l.f.m.response.live_blocks()==0&&!l.native.api.queue.count_20,"all request/cipher response/event sources freed after owned queue COPY and send finish");
    }
    for(unsigned cipher_mode:{1U,2U})for(unsigned mode:{0U,1U,2U,3U}){Loop l(cipher_mode);const auto expected=l.dispatch("deferred",true,mode,false,false);check(!l.f.queue.count_e8&&l.ownership.held.size()==1&&l.f.m.request.live_blocks()==0&&l.f.m.response.live_blocks()==1,"owned task retains real cipher response after request/local destruction");auto* task=l.ownership.held.front();l.ownership.held.clear();task->call_slot_00(1);l.drain();check(l.received==std::vector<std::string>{expected}&&l.f.m.response.live_blocks()==0,"deferred real encrypted source survives until queue COPY then is freed");}
    {Loop l;auto input=request_xml;check(l.oracle.encrypt_042610(input),"reference failing request encryption");l.native.seed(hex(input));l.session.encrypted_c4=1;l.native.api.hook=[](std::string name){if(name=="parse")throw native_message_test::Failure{88};};try{s::dispatch_message_028d30(0x12000,l.core.request(),l.message);throw std::runtime_error("expected parser failure");}catch(const native_message_test::Failure& e){check(e.code==88&&l.core.input()==request_xml&&!l.effects.operation&&l.f.m.request.live_blocks()==1&&l.f.m.base.owner.strong_08==1&&!l.native.api.decrypts,"parser exception after actual AES decrypt retains native plaintext prefix; no deferred writeback or invented outer cleanup");}}
}
void transcripts(){
    for(bool owned:{false,true})for(const auto& version:{std::string{},std::string("1"),std::string("2"),std::string("3")}){
        Loop l(1);l.session.encrypted_c4=7;l.session.unknown_c5=9;auto advance=std::string("advance CBC before reset");check(l.session.encrypt_042610(advance),"advance original IV before transcript reset");s::reset_session_cipher_02bc70(l.session);
        check(!l.session.native().mode_00&&!l.session.encrypted_c4&&!l.session.unknown_c5&&l.session.native().iv_b4==anadius::crypto_recovered::Key{},"reset removes previous CBC/key/flags before new handshake");
        std::vector<std::string> expected{l.handshake("challenge-fixture",version,owned)};std::vector<std::string> payloads={"first","",std::string(150000,'K')};for(unsigned i=0;i<10;++i)payloads.push_back("after-handshake-"+std::to_string(i));
        unsigned i=0;for(const auto& payload:payloads){expected.push_back(l.dispatch(payload,owned,i?1:3,i==2));++i;}
        l.drain();check(l.received==expected&&l.finishes==14&&l.cipher_api.effects.commands.size()==13&&l.f.queue.map_size_d8==16,"version-dependent ChallengeResponse key used by13 real native encrypted query/response/event messages through queue/send/finish");
        check(!l.native.api.decrypts&&!l.native.api.encrypts&&!l.core.cipher.decrypts&&!l.core.cipher.encrypts&&!l.f.m.response.live_blocks(),"old identity and split semantic cipher providers unused; owned session source lifetimes balanced");
    }
    for(bool owned:{false,true}){Loop l;std::vector<std::string> expected{l.handshake("first-key","3",owned)};expected.push_back(l.dispatch("before-rotation",owned,3,true));s::reset_session_cipher_02bc70(l.session);expected.push_back(l.handshake("second-key","2",owned));expected.push_back(l.dispatch("after-rotation",owned,3,true));l.drain();check(l.received==expected&&l.finishes==4&&l.session.encrypted_c4==1,"reset/rekey leaves already queued ciphertext intact while later native response uses new key");}
    {Loop l(1);const auto iv=l.cipher.iv_b4;const auto keys=l.cipher.schedule.round_keys();l.native.seed("<LSX><Request id=\"bridge\" recipient=\"fixture\"><ChallengeResponse version=\"3\"/></Request></LSX>");try{s::dispatch_message_028d30(0x12000,l.core.request(),l.message);throw std::runtime_error("expected invalid handshake");}catch(const core_test::Failure& e){check(e.code==66&&l.cipher.mode_00==1&&l.cipher.iv_b4==iv&&l.cipher.schedule.round_keys()==keys&&!l.effects.operation&&!l.native.api.destroys,"missing challenge key rejects before changing owned key/IV; no invented native EH cleanup");}}
}

}
int main(){try{regression();transcripts();std::cout<<"PASS: "<<checks<<" socket cipher session integration checks; owned init/key/IV/reset and permanent semantic cipher/factory adapters, ChallengeResponse to real native cipher/append and N1-N11 queue/send/finish; XML/native object/allocator/ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}


