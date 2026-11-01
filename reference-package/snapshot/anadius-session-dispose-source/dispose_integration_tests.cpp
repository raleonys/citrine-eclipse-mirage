#include "dispose.hpp"
#include "../anadius-socket-source/async_receive_test_fixture.hpp"
#include "../anadius-socket-source/receive_buffer_test_fixture.hpp"
#include "../anadius-socket-source/message_dispatch.hpp"
#include "../anadius-handlers-source/handlers.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <algorithm>
#include <exception>
#include <optional>

namespace {
namespace s=anadius::socket_recovered;namespace m=anadius::messages_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
[[noreturn]]void unused(){throw std::logic_error("Unselected owner integration dependency");}
void weak_release(s::IoReference* value){if(value&&value->weak_0c.fetch_sub(1)==1)value->delete_slot_08();}
void strong_release(s::IoReference* value){if(value&&value->strong_08.fetch_sub(1)==1){value->dispose_slot_00();weak_release(value);}}
struct Cancel:s::Control {
    unsigned disposed=0,deleted=0;Cancel(){strong=1;weak=1;resource=0x9876;}
    void destroy_resource()override{++disposed;}void delete_control()override{++deleted;}
};
struct EmptyControl:s::ControlFactory {s::Control* allocate_04b0bc(std::uint32_t)override{return nullptr;}};
struct Gate:s::SocketGate {bool held=false;void enter()override{if(held)unused();held=true;}void leave()override{if(!held)unused();held=false;}};
struct Life:s::LifecycleEffects {
    s::SocketService& service;std::vector<s::Socket> closed;unsigned cleanups=0;
    std::function<void(s::SocketObject&)> before_close;
    explicit Life(s::SocketService& v):service(v){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService& selected,s::SocketObject& object)override{
        if(&selected!=&service)unused();
        if(before_close)before_close(object);
        if(object.state.socket!=s::invalid_socket){closed.push_back(object.state.socket);object.state.socket=s::invalid_socket;}
        EmptyControl empty;s::replace_owner_0237f0(object.cancellation,0,empty);
    }
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{++cleanups;}
};
struct Observations {
    unsigned disposed=0,deleted=0,proxy_deleted=0,input_freed=0,blocks_freed=0,maps_freed=0,queued_heap_freed=0;
    bool proxy_null_first=false,input_live_at_free=false,socket_after_input=false,final_table=false;
    std::vector<std::string> order;
};
// Allocation provenance only: cleanup algorithms are the production026880,
// 026570,017420 and026290 bound by the final RecoveredSessionControl.
struct Free:s::SessionFreeEffects {
    buffer_test::Fixture& memory;Observations& seen;s::RecoveredSessionControl* live=nullptr;s::DequeProxy* proxy=nullptr;
    s::WriteFinishString** map=nullptr;std::uint64_t map_size=0;std::vector<s::WriteFinishString*> blocks;
    Free(buffer_test::Fixture& m,Observations& o):memory(m),seen(o){}
    void free_sized_04b0f8(std::uintptr_t address,std::uint64_t bytes)override{
        if(live&&address==reinterpret_cast<std::uintptr_t>(static_cast<s::SessionControl*>(live))){
            if(bytes!=0x1c8||live->strong_08!=0||live->weak_0c!=0)unused();
            seen.final_table=live->table_00==0x180084750ULL;seen.order.push_back("control");++seen.deleted;
            auto* old=live;live=nullptr;delete old;return;
        }
        if(proxy&&address==reinterpret_cast<std::uintptr_t>(proxy)){
            if(bytes!=16||!live)unused();auto& owner=live->object;
            seen.proxy_null_first=owner.output.proxy_c8==nullptr&&owner.output.fields.map_d0.empty()&&!owner.output.fields.map_size_d8;
            seen.order.push_back("proxy");++seen.disposed;++seen.proxy_deleted;delete proxy;proxy=nullptr;return;
        }
        for(auto& block:blocks)if(block&&address==reinterpret_cast<std::uintptr_t>(block)){
            if(bytes!=32)unused();seen.order.push_back("block");++seen.blocks_freed;delete block;block=nullptr;return;
        }
        if(map&&address==reinterpret_cast<std::uintptr_t>(map)){
            if(bytes!=map_size*8)unused();seen.order.push_back("map");++seen.maps_freed;delete[] map;map=nullptr;return;
        }
        if(!live)unused();auto& input=live->object.input;
        if(input.capacity_18>15&&address==s::receive_buffer_address(input)){
            seen.input_live_at_free=input.size_10==0&&input.capacity_18>15&&live->object.output.proxy_c8==nullptr;
            seen.order.push_back("input");++seen.input_freed;
        }else{seen.order.push_back("queued-heap");++seen.queued_heap_freed;}
        memory.api.free_sized_04b0f8(address,bytes);
    }
    [[noreturn]]void invalid_parameter_05c974()override{unused();}
};
struct Alloc:s::SessionAllocator {
    s::SessionDisposalContext& context;Free& free;unsigned controls=0,proxies=0;
    Alloc(s::SessionDisposalContext& c,Free& f):context(c),free(f){}
    s::SessionControl* allocate_control_04b0bc(std::uint64_t n)override{if(n!=0x1c8||free.live)unused();++controls;free.live=new s::RecoveredSessionControl(context);return free.live;}
    s::DequeProxy* allocate_proxy_04b0bc(std::uint64_t n)override{if(n!=16||free.proxy)unused();++proxies;free.proxy=new s::DequeProxy;return free.proxy;}
};
struct Factory {
    Gate gate;s::SocketService service{gate};Life life{service};Cancel cancel;buffer_test::Fixture bytes;Observations seen;
    Free free{bytes,seen};s::SessionDisposalContext disposal{bytes.memory,free,life};Alloc alloc{disposal,free};
    s::ConstructionScratch scratch;s::AcceptorState pending_state;s::SocketObject pending{pending_state};s::SessionPair pair;
    unsigned moves=0;s::ConstructionContext context{alloc,{this,move},life,scratch};
    static void move(void* p,s::ExecutorFields& destination,s::ExecutorFields& source,std::uintptr_t table){
        auto& f=*static_cast<Factory*>(p);if(table!=0x180084928ULL||f.gate.held)unused();++f.moves;
        destination.context=source.context;destination.zero_74=source.zero_74;destination.self_reference_80=true;
    }
    Factory(){
        s::initialize_0211b0(pending,{0x1234},life);pending_state.socket=77;pending_state.socket_state=0x10;
        pending.cancellation={0x9876,&cancel};s::construct_session_owner_023000(pair,pending,context);
        bytes.regions[0]={reinterpret_cast<std::uintptr_t>(pair.object->input.storage_00.data()),pair.object->input.storage_00};
        life.before_close=[&](s::SocketObject& object){
            if(free.live&&&object==&free.live->object.socket){auto& owner=free.live->object;
                seen.socket_after_input=seen.proxy_deleted==1&&seen.input_freed==1&&owner.input.size_10==0&&owner.input.capacity_18==15&&owner.input.storage_00[0]==0&&owner.weak_self.control_08==free.live&&free.live->weak_0c>=2;
                seen.order.push_back("socket");
            }
        };
    }
    void release_external(){auto* control=pair.control;pair={};strong_release(control);}
    void destroy_pending(){s::destroy_024db0(pending,life);}
};
struct Table:s::IoHandlerVtable {
    unsigned moves=0,destroys=0;Table():IoHandlerVtable(0x180017a30ULL){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;to.payload_00=from.payload_00;to.state_18=from.state_18;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}
};
struct Terminal:s::FrameReadTerminal {[[noreturn]]void bad_weak_ptr_026860()override{unused();}};
struct Category:s::WriteFinishCategory {Category(){identity_08=0x3333;}void message_slot_10(s::WriteFinishString&,std::int32_t)override{unused();}};
struct Loop;
struct Completion:s::IoCompletionEffects {
    Loop& loop;explicit Completion(Loop& l):loop(l){}
    s::Category misc_category_0285b0()override{return {0x3333};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override;
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}
};
struct FrameApi:s::FrameReadApi {
    Loop& loop;explicit FrameApi(Loop& l):loop(l){}std::unique_ptr<s::FrameMessageClosure> closure;unsigned detached=0,freed=0;
    void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override;
    s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override{if(n!=48||closure)unused();closure=std::make_unique<s::FrameMessageClosure>();return closure.get();}
    std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t a,std::uint32_t b,std::uintptr_t entry,s::FrameMessageClosure* p,std::uint32_t flags,std::uint32_t& id)override{
        if(a||b||flags||entry!=0x180021770ULL||p!=closure.get())unused();id=57;return 99;
    }
    std::int32_t detach_04a458(const s::FrameThreadHandle& h)override{if(h.id!=57||h.handle!=99)unused();++detached;return 0;}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
    [[noreturn]]void invalid_parameter_05c974()override{unused();}
    void next_write_02a500(std::uintptr_t)override{unused();}
    s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}
    void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}
};
struct ReadApi:s::RecoveredFrameReadApi {
    using RecoveredFrameReadApi::RecoveredFrameReadApi;
    void construct_length_error_024390(s::ReadLengthError&,std::uintptr_t)override{unused();}
    [[noreturn]]void throw_length_error_023930(s::ReadLengthError&)override{unused();}
};
struct Loop {
    Factory factory;receive_test::Fixture io;Table table;Terminal terminal;Category category;Completion completion{*this};FrameApi api{*this};
    s::SessionOwner& owner=*factory.pair.object;s::IoReference* control=factory.pair.control;std::uintptr_t identity=owner.identity();
    std::array<s::FrameReadOwnerBinding,1> owners{{{identity,&owner.weak_self}}};
    std::array<s::WriteFinishCategoryBinding,1> categories{{{{0x3333},&category}}};
    s::FrameReadContext frame{factory.bytes.context,completion,api,owners,categories,{},terminal,{0x1111},nullptr};ReadApi read_api{frame};
    std::array<s::ReceiveStringBinding,1> strings{{{identity+0xa8,&owner.input}}};
    std::array<s::ReceiveStreamBinding,1> streams{{{identity+0x10,&io.service,&io.implementation}}};
    s::ComposedReadContext read{factory.bytes.context,io.context,completion,read_api,strings,streams};unsigned callbacks=0;
    Loop(){
        frame.read=&read;io.implementation={owner.socket_state.socket,0x10,identity,control};
        io.environment.sentinels={&table,0,nullptr,table.destroy_identity};
        io.effects.construct=[&](auto& env,auto allocator){if(allocator!=identity+0x78)unused();env.operations_10=&table;env.state_18=0;};
    }
    void start(){s::start_frame_read_0280d0(identity,frame);}
    void complete(std::string payload,bool eof=false){
        auto operation=std::move(io.effects.operation);if(!operation||payload.size()>operation->io.requested_58)unused();
        auto destination=factory.bytes.memory.view(operation->buffer_data_50,payload.size());std::copy(payload.begin(),payload.end(),destination.begin());
        ++callbacks;s::complete_receive_027710(1,operation->io,{0,{0x2222}},payload.size(),io.environment,io.worker,io.memory,io.errors,completion);
        io.entry.work_count_30.fetch_sub(1);
        if(eof&&io.effects.operation)unused();
    }
    std::string text(const s::ReceiveBufferString& value){
        if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
        auto bytes=factory.bytes.memory.view(s::receive_buffer_address(value),value.size_10);return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
    }
    void copy(s::ReceiveBufferString& out,std::string_view value){
        out={};out.size_10=value.size();
        if(value.size()<=15){std::copy(value.begin(),value.end(),out.storage_00.begin());return;}
        out.capacity_18=value.size();const auto address=factory.bytes.api.allocate_04b0bc(value.size()+1);buffer_test::word(out.storage_00,address);
        auto bytes=factory.bytes.memory.view(address,value.size()+1);std::copy(value.begin(),value.end(),bytes.begin());bytes[value.size()]=0;
    }
    void free_string(s::ReceiveBufferString& value){if(value.capacity_18>15)api.free_sized_04b0f8(s::receive_buffer_address(value),value.capacity_18+1);value={};}
};
void Completion::receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t n,std::uint32_t f){s::composed_receive_025c40(h,e,n,f,loop.read);}
void FrameApi::construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t address,std::uint64_t count){auto bytes=loop.factory.bytes.memory.view(address,count);loop.copy(out,{reinterpret_cast<const char*>(bytes.data()),bytes.size()});}
void FrameApi::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){
    if(closure&&p==reinterpret_cast<std::uintptr_t>(closure.get())){if(n!=48||closure->message_00.size_10)unused();closure.reset();++freed;return;}
    loop.factory.bytes.api.free_sized_04b0f8(p,n);
}
void factory_read_eof(){
    Loop loop;auto& f=loop.factory;auto* const control=loop.control;
    require(f.moves==2&&f.scratch.complete&&!f.scratch.temporary_live&&f.service.head==&loop.owner.socket&&loop.owner.socket.next==&f.pending,"Factory performs both moves and destroys temporary before published owner is consumed");
    require(control->strong_08==1&&control->weak_0c==2&&loop.owner.weak_self.control_08==control&&loop.owner.weak_self.input_a8==&loop.owner.input,"Factory output is one strong plus self/implicit weak, with exact owned input projection");
    require(reinterpret_cast<void*>(control)!=reinterpret_cast<void*>(loop.owner.socket.cancellation.control)&&loop.owner.socket.cancellation.control==&f.cancel&&f.cancel.strong==1,"Factory session IoReference and transferred cancellation control are distinct ownership domains");
    require(loop.owner.input.size_10==0&&loop.owner.input.capacity_18==15&&loop.owner.output.fields.map_d0.empty()&&loop.owner.output.proxy_c8->owner_00==&loop.owner.output&&!loop.owner.cipher.encrypted_c4&&!loop.owner.cipher.unknown_c5&&!loop.owner.cipher.native().mode_00,"Real factory initializes native input/deque/proxy and first cipher reset");
    control->weak_0c.fetch_add(1);loop.start();
    require(loop.io.effects.operation&&loop.io.effects.operation->io.handler.control_a8==control&&loop.io.effects.operation->io.weak_48==control&&loop.io.effects.operation->weak_object_40==loop.identity,"Actual0280d0/025c40/021d10 publishes factory strong and weak pairs into actual receive operation");
    require(control->strong_08==2&&control->weak_0c==4&&loop.io.receive.calls==1&&loop.owner.input.capacity_18>=512,"Actual receive prepares factory input heap and retains strong handler plus weak operation");
    f.release_external();require(control->strong_08==1&&!f.seen.disposed&&f.life.closed.empty(),"Dropping external factory strong leaves pending native IO as sole object owner");
    loop.complete("",true);
    require(f.seen.disposed==1&&!f.seen.deleted&&control->strong_08==0&&control->weak_0c==1&&!loop.io.effects.operation,"Actual027710 EOF drains pending strong/weak, disposes object and preserves explicit observer weak only");
    require(f.life.closed==std::vector<s::Socket>{77}&&f.cancel.disposed==1&&f.cancel.deleted==1&&f.seen.proxy_deleted==1&&f.seen.input_freed==1&&f.bytes.live_blocks()==0,"Selected disposal really closes socket, releases cancellation, frees input heap and deletes proxy exactly once");
    require(f.service.head==&f.pending&&!f.pending.next&&!loop.io.entry.work_count_30&&loop.io.memory.frees==1,"EOF leaves only invalid pending node and no outstanding operation/storage/work");
    require(f.seen.proxy_null_first&&f.seen.input_live_at_free&&f.seen.socket_after_input&&loop.owner.weak_self.control_08==control&&loop.owner.weak_self.object_00==loop.identity,"Actual026570 nulls proxy before free, resets input before socket callbacks, and preserves selfweak slots while decrementing owned weak");
    require(f.seen.order==std::vector<std::string>({"proxy","input","socket"}),"Actual empty-deque disposal order reaches socket only after proxy and live input frees");
    weak_release(control);require(f.seen.deleted==1,"Final observer weak ends actual heap control/object allocation once");
    require(f.seen.final_table&&!f.free.live&&f.seen.order.back()=="control","Actual017420 virtual slot10 flags1 dispatch reaches026290 table restore and exact1c8 provenance delete");
    f.destroy_pending();require(!f.service.head&&f.life.closed.size()==1,"Explicit pending destruction empties service without another live socket close");
}
struct Events:m::EventQueue {
    bool nonempty_relaxed()const override{return false;}int lock_04a6ac()override{unused();}std::int32_t& lock_count_4c()override{unused();}bool nonempty_locked()const override{unused();}std::string copy_front_locked()const override{unused();}void pop_front_locked()override{unused();}void unlock_04a6b4()override{unused();}
};
struct Random:m::Random {m::Key challenge_bytes_034cf0()override{return {};}m::Key key_from_response_034c30(std::string_view)override{return {};}};
struct XmlFacade:anadius::handlers_recovered::Xml {
    m::XmlPrinter& printer;explicit XmlFacade(m::XmlPrinter& p):printer(p){}
    const char* attribute_002cc0(anadius::handlers_recovered::Request,const char*)override{unused();}
    void open_004080(const char* name,bool)override{printer.open(name);}void attribute_005560(const char* key,const char* value)override{printer.attribute(key,value);}void close_virtual_48(bool)override{printer.close();}
};
struct MessageEffects:m::Effects {
    unsigned commands=0;int atoi_05ee08(std::string_view)override{unused();}
    bool registered_handler(std::uint32_t,const m::XmlNode&,m::XmlPrinter& printer)override{++commands;XmlFacade xml(printer);return anadius::handlers_recovered::get_config_02e5f0(xml);}
    [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{}void log_response(std::string_view)override{}void log_event(std::string_view)override{unused();}void log_bad_xml(std::string_view,std::string_view)override{unused();}void log_decryption_failure(std::string_view)override{unused();}
};
struct MessageApi:s::MessageDispatchApi {
    Loop& loop;anadius::xml_reader_bridge::Backend xml;s::SessionCipherFactory crypto;Random random;Events events;MessageEffects effects;
    std::optional<s::MessageWriteTask> job;unsigned processed=0,cleanups=0;std::uint8_t show=0;
    explicit MessageApi(Loop& l):loop(l){}
    void process_message_033d70(std::uintptr_t handler,s::ReceiveBufferString& response,s::ReceiveBufferString& request)override{
        if(handler!=loop.identity+0xf0||loop.control->strong_08==0)unused();++processed;
        auto input=loop.text(request);auto output=m::process_message_033d70({loop.owner.cipher.encrypted_c4,show,loop.owner.cipher,events},input,xml,crypto,random,effects);
        loop.free_string(request);loop.copy(request,input);loop.copy(response,output);
    }
    // Explicit controlled lower scheduling adapter, not recovered021420/IOCP.
    void dispatch_response_021420(s::IoHandlerEnvironment&,s::MessageWriteTask& task,std::uint64_t a,std::uint64_t b)override{
        if(a||b||job||task.control_30!=loop.control||task.object_28!=loop.identity||task.function_00!=0x180028270ULL)unused();job=task;task.response_08={};task.object_28=0;task.control_30=nullptr;
    }
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{loop.free_string(value);}
    void cleanup_exit_waits_04a47c()override{++cleanups;}
};
void copy_environment(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from){to.payload_00=from.payload_00;to.state_18=from.state_18;}
void seed_quiescent_output(Loop& loop){
    auto& allocation=loop.factory.free;auto& fields=loop.owner.output.fields;
    allocation.map_size=8;allocation.map=new s::WriteFinishString*[8]{};
    for(auto index:{3,6,7}){auto* block=new s::WriteFinishString;allocation.blocks.push_back(block);allocation.map[index]=block;}
    loop.copy(*allocation.map[6],std::string(96,'q'));loop.copy(*allocation.map[7],"last");
    fields={std::span<s::WriteFinishString*>(allocation.map,8),8,6,2};
}
void factory_frame_message_response(bool encrypted){
    Loop loop;auto& f=loop.factory;auto* const control=loop.control;control->weak_0c.fetch_add(1);
    s::IoHandlerEnvironment environment{{1,2},&loop.table,0,0x456,nullptr};s::MessageDispatchOwner owner{&loop.owner.weak_self,&environment};
    std::array<s::MessageDispatchOwnerBinding,1> owners{{{loop.identity,&owner}}};std::array<s::MessageEnvironmentCopyBinding,1> copies{{{&loop.table,copy_environment}}};
    MessageApi api{loop};s::MessageDispatchContext message{loop.frame,api,owners,copies,{}};
    // First reset belongs to actual023000. Explicit accepted-start reset is
    // second; no semantic Session constructor introduces another reset.
    s::reset_session_cipher_02bc70(loop.owner.cipher);loop.start();
    const std::string request="<LSX><Request id=\"owner\" recipient=\"test\"><GetConfig/></Request></LSX>";
    auto wire=request;s::SessionCipher client;
    if(encrypted){
        client.set_fixed_key_084c98();loop.owner.cipher.set_fixed_key_084c98();loop.owner.cipher.encrypted_c4=1;
        require(client.encrypt_042610(wire),"Independent client encrypts owned request for factory SessionCipher");m::hex_encode_03d190(wire);
    }
    loop.complete(wire+std::string(1,'\0'));
    require(loop.api.closure&&loop.api.closure->owner_20==loop.identity&&loop.api.detached==1&&loop.io.effects.operation&&control->strong_08==2,"Actual025480 publishes raw factory owner closure and rearms actual native receive before message execution");
    f.release_external();require(control->strong_08==1&&!f.seen.disposed,"Pending read explicitly keeps raw detached job owner alive before033d70 strong acquisition");
    require(s::message_thread_entry_021770(*loop.api.closure,message)==0,"Actual021770 runs actual028d30 against factory owner and frees its frame closure");
    require(!loop.api.closure&&loop.api.freed==1&&api.processed==1&&api.effects.commands==1&&api.cleanups==1&&api.job&&api.job->control_30==control&&control->strong_08==2,"Actual message core/GetConfig response transfers acquired factory strong into controlled response job");
    auto response=loop.text(api.job->response_08);
    require(!response.empty()&&response.back()==0,"Response job preserves real message core NUL terminator");
    if(encrypted){
        response.pop_back();m::hex_decode_03ce30(response);
        require(client.decrypt_042480(response)&&loop.owner.cipher.encrypted_c4==1&&loop.owner.cipher.native().mode_00==2,"Factory SessionCipher decrypts request and encrypts response, independently client-decrypted after native dispatch");response.push_back(0);
    }
    require(response.find("GetConfigResponse")!=std::string::npos&&response.find("id=\"owner\"")!=std::string::npos&&!response.empty()&&response.back()==0,"Real XML parser/MemoryPrinter and recovered GetConfig body produce exact correlated NUL response");
    loop.complete("",true);
    require(control->strong_08==1&&!f.seen.disposed&&f.life.closed.empty(),"After receive EOF only response job owns factory object; socket stays open until response drain");
    loop.free_string(api.job->response_08);auto* residual=api.job->control_30;api.job.reset();
    // Own synthetic retained deque allocations after all external message/read
    // borrowers drain. No new native write publication or thread claim.
    seed_quiescent_output(loop);strong_release(residual);
    require(f.seen.disposed==1&&!f.seen.deleted&&f.seen.proxy_deleted==1&&f.seen.input_freed==1&&f.bytes.live_blocks()==0&&f.life.closed==std::vector<s::Socket>{77},"Controlled response drain releases actual object resources after all read/message borrowers end");
    require(f.seen.blocks_freed==3&&f.seen.maps_freed==1&&f.seen.queued_heap_freed==1&&!loop.owner.output.fields.count_e8&&!loop.owner.output.fields.offset_e0&&loop.owner.output.fields.map_d0.empty(),"Actual026880 frees active deque heap, all retained32-byte blocks including unused slot, and native-sized pointer map");
    require(f.seen.proxy_null_first&&f.seen.input_live_at_free&&f.seen.socket_after_input&&loop.owner.weak_self.control_08==control&&loop.owner.weak_self.object_00==loop.identity&&control->weak_0c==1,"Actual026570 preserves selfweak fields while dropping self and implicit weak after ordered resource cleanup");
    require(f.seen.order==std::vector<std::string>({"queued-heap","block","block","block","map","proxy","input","socket"}),"Actual nonempty output clear precedes proxy free, input free/reset and socket lifecycle callbacks");
    weak_release(control);f.destroy_pending();
    require(f.seen.deleted==1&&!f.service.head&&!loop.io.entry.work_count_30&&f.cancel.disposed==1&&f.cancel.deleted==1,"Final factory weak and pending cleanup leave no IO work, list node or live cancellation resource");
    require(f.seen.final_table&&!f.free.live&&f.seen.order.back()=="control","Final observer weak triggers actual017420/026290 exact control free after complete resource drain");
}
}
int main(){try{factory_read_eof();factory_frame_message_response(false);factory_frame_message_response(true);std::cout<<"PASS: "<<checks<<" session-disposal integration checks; actual023000/026880/026570/017420/026290 with actual native read/frame/message, real XML/GetConfig/cipher and controlled response scheduler\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected disposal integration exception\n";return 1;}}
