#pragma once
#include "native_dispatch.hpp"
#include "cipher_session_test_fixture.hpp"

namespace dispatch_test {
namespace s=anadius::socket_recovered;namespace m=anadius::messages_recovered;
using response_test::check;using response_test::checks;
struct Bytes:s::ReceiveBufferApi {
    native_message_test::Fixture& native;s::NativeDispatchLocals& locals;std::function<void()> freeing;
    Bytes(native_message_test::Fixture& n,s::NativeDispatchLocals& l):native(n),locals(l){refresh();}
    void refresh(){native.bytes.refresh();native.core.containers.bytes.regions[251]={reinterpret_cast<std::uintptr_t>(locals.scratch.storage_00.data()),locals.scratch.storage_00};}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{const auto p=native.bytes.allocate_04b0bc(n);refresh();return p;}
    void copy_bytes(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{native.bytes.copy_bytes(a,b,n);}
    void fill_bytes(std::uintptr_t a,std::uint8_t b,std::uint64_t n)override{native.bytes.fill_bytes(a,b,n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{if(freeing)freeing();native.bytes.free_sized_04b0f8(p,n);refresh();}
    [[noreturn]]void length_error_00ce80()override{native.bytes.length_error_00ce80();}
    [[noreturn]]void allocation_overflow_00cd30()override{native.bytes.allocation_overflow_00cd30();}
    [[noreturn]]void invalid_parameter_05c974()override{native.bytes.invalid_parameter_05c974();}
};
struct Printer:m::XmlPrinter {
    std::unique_ptr<m::XmlPrinter> lower;std::function<void(std::string)> event;
    explicit Printer(std::unique_ptr<m::XmlPrinter> p,std::function<void(std::string)> e):lower(std::move(p)),event(std::move(e)){}
    void open(std::string_view n)override{event("open-"+std::string(n));lower->open(n);}
    void attribute(std::string_view n,std::string_view v)override{event("attribute-"+std::string(n));lower->attribute(n,v);}
    void close()override{event("close");lower->close();}
    std::string buffer_including_terminator()const override{return lower->buffer_including_terminator();}
};
// Controlled lower providers. These models do not claim native DOM/printer,
// TLS/RB-tree, string constructor/reserve/append/destructor or exception ABI.
struct Effects:s::NativeDispatchEffects {
    native_message_test::Fixture& native;s::ReceiveBufferContext& strings;s::NativeDispatchLocals& locals;s::SessionCipher& session;
    session_cipher_test::Effects effects;s::SessionCipherFactory factory;session_cipher_test::Time time;anadius::crypto_recovered::Random random{time};
    std::array<s::NativeDispatchEntry,39> entries{};s::NativeDispatchEntry sentinel{},other_end{};s::NativeDispatchEntry* end=&sentinel;
    std::unique_ptr<Printer> printer;std::vector<std::string> trace;std::function<void(std::string)> hook;
    std::string data,override_data;std::optional<std::int32_t> override_size;std::uint64_t reserved=0,appended=0;
    unsigned prepared=0,constructed=0,destroyed=0,outputs=0,handled=0,challenges=0;bool symbolic=false;
    Effects(native_message_test::Fixture& n,s::ReceiveBufferContext& c,s::NativeDispatchLocals& l,s::SessionCipher& session):native(n),strings(c),locals(l),session(session){}
    void event(std::string name){trace.push_back(name);if(hook)hook(name);}
    std::string text(const s::ReceiveBufferString& v){const auto bytes=strings.memory.view(s::receive_buffer_address(v),v.size_10);return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};}
    void assign(s::ReceiveBufferString& v,std::string_view bytes){s::resize_receive_buffer_010040(v,bytes.size(),0,strings);if(!bytes.empty()){auto to=strings.memory.view(s::receive_buffer_address(v),bytes.size());std::copy(bytes.begin(),bytes.end(),to.begin());}}
    void prepare_registry(std::span<const m::Registration> registrations)override{++prepared;event("registry");check(registrations.size()==entries.size(),"exact39 registration table");for(std::size_t i=0;i<entries.size();++i)entries[i].handler_rva=registrations[i].rva;}
    s::NativeDispatchEntry* find_exact(s::ReceiveBufferString& name)override{event("find");const auto key=text(name);auto table=m::registrations_032d00();for(std::size_t i=0;i<table.size();++i)if(table[i].name==key)return &entries[i];return end;}
    s::NativeDispatchEntry* registry_end()override{event("end");return end;}
    const m::XmlDocument& document(s::NativeMessageDocument& d)override{check(&d==&native.locals.document&&native.api.document!=nullptr,"same live parsed document");event("document");return *native.api.document;}
    void construct_name_006b30(s::ReceiveBufferString& name,std::string_view value)override{event("name");check(!name.size_10&&!name.capacity_18&&std::all_of(name.storage_00.begin(),name.storage_00.end(),[](auto x){return x==0;}),"name constructor receives zero prefix");name.capacity_18=15;assign(name,value);}
    m::XmlPrinter& construct_printer_0019b0(std::uintptr_t file,std::uint8_t compact,std::int32_t depth)override{check(!file&&!compact&&!depth,"printer arguments0/0/0");++constructed;event("printer");printer=std::make_unique<Printer>(native.core.xml.printer(),[this](auto name){event(std::move(name));});return *printer;}
    bool registered_handler(std::uint32_t rva,std::uintptr_t id,const m::XmlNode& command,m::XmlPrinter& p)override{check(id==0x120f0,"cached handler identity");++handled;event("handler");return effects.registered_handler(rva,command,p);}
    bool challenge_02d7e0(std::uintptr_t id,const m::XmlNode& command,m::XmlPrinter& p)override{check(id==0x120f0,"challenge handler identity");++challenges;event("challenge");return m::challenge_response_02d7e0(command,p,session,factory,random,effects);}
    std::int32_t printer_size(m::XmlPrinter& p)override{event("size");return override_size.value_or(static_cast<std::int32_t>(p.buffer_including_terminator().size()));}
    std::span<const std::uint8_t> printer_data(m::XmlPrinter& p)override{event("data");data=override_data.empty()?p.buffer_including_terminator():override_data;return {reinterpret_cast<const std::uint8_t*>(data.data()),data.size()};}
    void reserve_02b300(s::ReceiveBufferString& v,std::uint64_t n)override{reserved=n;event("reserve");check(!v.size_10&&v.capacity_18==15&&std::all_of(v.storage_00.begin(),v.storage_00.end(),[](auto x){return x==0;}),"scratch prefix reset before reserve");if(!symbolic){assign(v,std::string(n,'\0'));s::resize_receive_buffer_010040(v,0,0,strings);}}
    void append_018b30(s::ReceiveBufferString& v,std::span<const std::uint8_t> bytes,std::uint64_t n)override{appended=n;event("append");if(n>bytes.size())throw s::UnsupportedInput("controlled append extent");assign(v,{reinterpret_cast<const char*>(bytes.data()),static_cast<std::size_t>(n)});}
    s::ReceiveBufferString& construct_output_009fd0(s::ReceiveBufferString& out,std::span<const std::uint8_t> bytes)override{event("output");out.storage_00.fill(0);out.size_10=out.capacity_18=0;const auto nul=std::find(bytes.begin(),bytes.end(),0);if(nul==bytes.end())throw s::UnsupportedInput("controlled C string extent");out.capacity_18=15;assign(out,{reinterpret_cast<const char*>(bytes.data()),static_cast<std::size_t>(nul-bytes.begin())});++outputs;return out;}
    void free_scratch_00b520(s::ReceiveBufferString& v,std::uintptr_t data,std::uint64_t capacity)override{event("free-scratch");check(&v==&locals.scratch&&data==s::receive_buffer_address(v)&&capacity==v.capacity_18,"cached scratch destructor arguments");auto raw=data,n=capacity+1;if(n>=4096){raw=0;auto h=strings.memory.view(data-8,8);for(unsigned i=0;i<8;++i)raw|=std::uintptr_t(h[i])<<(i*8);n+=39;}strings.api.free_sized_04b0f8(raw,n);}
    void destroy_printer_001f60(m::XmlPrinter& p)override{check(&p==printer.get(),"same printer destructor receiver");event("destroy-printer");++destroyed;printer.reset();}
    [[noreturn]]void invalid_message_02bd30_04d228()override{event("invalid");throw core_test::Failure{66};}
};
struct Api:s::RecoveredMessageDispatchApi {
    s::NativeMessageApi& next;session_cipher_test::Effects& effects;
    Api(s::PrefixCipherContext& c,std::span<s::PrefixCipherBinding> b,s::NativeDispatchContext& d,s::NativeMessageApi& n,session_cipher_test::Effects& e):RecoveredMessageDispatchApi(c,b,d),next(n),effects(e){}
    void construct_document_001830(s::NativeMessageDocument& d,bool a,std::uint32_t b)override{next.construct_document_001830(d,a,b);}
    int parse_document_004280(s::NativeMessageDocument& d,std::uintptr_t p,std::uint64_t n)override{return next.parse_document_004280(d,p,n);}
    void destroy_document_001e00(s::NativeMessageDocument& d)override{next.destroy_document_001e00(d);}
    void log(s::NativeMessageLog k,s::NativeMessageDocument& d,s::ReceiveBufferString& v)override{next.log(k,d,v);}
    s::NativeMessageEventQueue& event_queue_02c400()override{return next.event_queue_02c400();}
    s::NativeMessageMutex& event_mutex_02c620()override{return next.event_mutex_02c620();}
    int lock_04a6ac(s::NativeMessageMutex& m)override{return next.lock_04a6ac(m);}
    void unlock_04a6b4(s::NativeMessageMutex& m)override{next.unlock_04a6b4(m);}
    [[noreturn]]void thread_error_04a648(int n)override{next.thread_error_04a648(n);}
};
struct Fixture {
    native_message_test::Fixture native;s::NativeDispatchLocals locals;Bytes bytes{native,locals};s::ReceiveBufferContext strings{native.core.containers.bytes.memory,bytes};s::SessionCipher session;
    Effects effects{native,strings,locals,session};s::NativeDispatchContext context{strings,locals,effects};
    Fixture(){bytes.freeing=[this](){effects.event("free");};}
    void parse(std::string_view xml){native.api.document=native.core.xml.parse_c_string(xml).document;bytes.refresh();}
    s::ReceiveBufferString& run(){return s::dispatch_native_032d00(native.locals.temporary,0x120f0,native.locals.document,context);}
    std::string output(){return effects.text(native.locals.temporary);}
};
inline std::string request(std::string_view name,std::string_view attrs="id=\"bridge\" recipient=\"fixture\""){return "<LSX><Request "+std::string(attrs)+"><"+std::string(name)+"/></Request></LSX>";}
} // namespace dispatch_test
