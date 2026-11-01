#include "native.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-socket-source/append_growth.hpp"
#include "../anadius-socket-source/prefix_cipher.hpp"
#include "../anadius-dispatch-source/router.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <limits>
#include <type_traits>

namespace anadius::native_profile {
struct Logic::Invocation {
    Logic& logic;s::SessionMemoryDomain& parent;sp::Strings& owned;
    s::ReceiveBufferString greeting_output,greeting_request;
    s::ReceiveBufferString& output;s::ReceiveBufferString& request;
    std::function<void()> checkpoint;
    s::NativeMessageLocals locals;
    std::unique_ptr<m::XmlDocument> document;
    // Opaque objects returned to semantic lowers are owned here, not by proxy
    // stack temporaries. Unwinding cannot destroy them after an unknown prefix.
    // Selected host policy: retire them at known-normal invocation completion.
    std::vector<std::unique_ptr<m::XmlDocument>> documents;
    std::vector<std::unique_ptr<m::XmlPrinter>> printers;
    std::vector<std::unique_ptr<m::Cipher>> ciphers;
    s::NativeMessageEventQueue* events=nullptr;
    std::vector<s::ReceiveBufferRegion> regions;s::ReceiveBufferMemory memory;
    void add(s::ReceiveBufferString& value){
        const auto address=reinterpret_cast<std::uintptr_t>(value.storage_00.data());
        constexpr std::size_t count=16;
        unsigned complete=0;
        for(const auto& r:regions){
            if(address>=r.address&&address-r.address<=r.bytes.size()&&count<=r.bytes.size()-(address-r.address)){++complete;continue;}
            if((address>=r.address&&address-r.address<r.bytes.size())||(r.address>=address&&r.address-address<count))
                throw s::UnsupportedInput("Partially overlapping native inline string mapping");
        }
        if(complete>1)throw s::UnsupportedInput("Ambiguous native inline string mapping");
        if(!complete)regions.push_back({address,value.storage_00});
    }
    void refresh(){
        const auto upstream=parent.buffer_context().memory.regions;
        regions.assign(upstream.begin(),upstream.end());
        add(output);add(request);add(locals.response);add(locals.temporary);
        if(events){
            if(events->map_08.size()>1048576)throw s::UnsupportedInput("Unbounded native event projection");
            for(auto* value:events->map_08)if(value)add(*value);
        }
        memory.regions=regions;
    }
    void admit(){if(logic.poisoned_||logic.retiring_)throw std::logic_error("Native message source admission was poisoned");checkpoint();refresh();}
    void track(){owned.track(output);owned.track(request);}
    template<class F>std::invoke_result_t<F> lower(F&& fn,bool control=false,bool unknown=false){
        try{
            admit();
            if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(fn)();admit();}
            else{decltype(auto) result=std::forward<F>(fn)();admit();return result;}
        }catch(const m::NoResponseRequiredError&){if(!control){logic.poisoned_=true;throw;}try{admit();}catch(...){logic.poisoned_=true;throw;}throw;}
        catch(const m::UnknownMessageError&){if(!unknown){logic.poisoned_=true;throw;}try{admit();}catch(...){logic.poisoned_=true;throw;}throw;}catch(...){logic.poisoned_=true;throw;}
    }
    struct Document final:m::XmlDocument {
        Invocation& v;m::XmlDocument& raw;
        struct Node final:m::XmlNode,dispatch_recovered::GuardedNodeAccess {
            Document& doc;const m::XmlNode& raw;
            Node(Document& d,const m::XmlNode& r):doc(d),raw(r){}
            std::string_view name()const override{return doc.v.lower([&]{return raw.name();});}
            const m::XmlNode* first_element()const override{return doc.wrap(doc.v.lower([&]{return raw.first_element();}));}
            const m::XmlNode* next_element()const override{return doc.wrap(doc.v.lower([&]{return raw.next_element();}));}
            std::optional<std::string_view> attribute(std::string_view name)const override{return doc.v.lower([&]{return raw.attribute(name);});}
            std::vector<std::pair<std::string,std::string>> attributes_with(dispatch_recovered::NodeAccess& access)const override{return doc.v.lower([&]{return access.attributes_in_order(raw);});}
            const char* text_with(dispatch_recovered::NodeAccess& access)const override{return doc.v.lower([&]{return access.text_003860(raw);});}
        };
        mutable std::vector<std::unique_ptr<Node>> nodes;
        Document(Invocation& p,m::XmlDocument& r):v(p),raw(r){}
        const m::XmlNode* wrap(const m::XmlNode* node)const{
            try{
                if(!node)return nullptr;for(const auto& n:nodes)if(&n->raw==node)return n.get();
                nodes.push_back(std::make_unique<Node>(const_cast<Document&>(*this),*node));return nodes.back().get();
            }catch(...){v.logic.poisoned_=true;throw;}
        }
        const m::XmlNode* first_element()const override{return wrap(v.lower([&]{return raw.first_element();}));}
        std::string_view error_name()const override{return v.lower([&]{return raw.error_name();});}
    };
    struct Printer final:m::XmlPrinter,dispatch_recovered::GuardedPrinterAccess {
        Invocation& v;m::XmlPrinter& raw;Printer(Invocation& p,m::XmlPrinter& r):v(p),raw(r){}
        void open(std::string_view name)override{v.lower([&]{raw.open(name);});}
        void attribute(std::string_view name,std::string_view value)override{v.lower([&]{raw.attribute(name,value);});}
        void close()override{v.lower([&]{raw.close();});}
        std::string buffer_including_terminator()const override{return v.lower([&]{return raw.buffer_including_terminator();});}
        xml_recovered::MemoryPrinter& concrete(){auto* result=dynamic_cast<xml_recovered::MemoryPrinter*>(&raw);if(!result)throw s::UnsupportedInput("Guarded Router serialization requires recovered memory printer backing");return *result;}
        void open_element(std::string_view name,bool compact)override{v.lower([&]{concrete().open_element(name,compact);});}
        void close_element(bool compact)override{v.lower([&]{concrete().close_element(compact);});}
        void push_text(std::string_view text,bool cdata)override{v.lower([&]{concrete().push_text(text,cdata);});}
    };
    struct Xml final:m::XmlBackend {
        Invocation& v;explicit Xml(Invocation& p):v(p){}
        m::ParseResult parse_c_string(std::string_view text)override{return v.lower([&]{
            v.prepare(v.documents);
            auto result=v.logic.deps_.xml->parse_c_string(text);
            auto* raw=result.document.get();if(raw){v.documents.push_back(std::move(result.document));result.document=std::make_unique<Document>(v,*raw);}return result;
        });}
        std::unique_ptr<m::XmlPrinter> printer()override{return v.lower([&]() -> std::unique_ptr<m::XmlPrinter>{
            v.prepare(v.printers);
            auto result=v.logic.deps_.xml->printer();if(!result)throw s::UnsupportedInput("Missing selected XML printer");
            auto* raw=result.get();v.printers.push_back(std::move(result));return std::make_unique<Printer>(v,*raw);
        });}
    } xml{*this};
    struct Cipher final:m::Cipher {
        Invocation& v;m::Cipher& raw;Cipher(Invocation& p,m::Cipher& r):v(p),raw(r){}
        void construct_0423d0_in_place()override{v.lower([&]{raw.construct_0423d0_in_place();});}
        void set_fixed_key_084c98()override{v.lower([&]{raw.set_fixed_key_084c98();});}
        void set_key_042780(const m::Key& key)override{v.lower([&]{raw.set_key_042780(key);});}
        bool decrypt_042480(std::string& text)override{return v.lower([&]{return raw.decrypt_042480(text);});}
        bool encrypt_042610(std::string& text)override{return v.lower([&]{return raw.encrypt_042610(text);});}
    } session_cipher{*this,logic.session_.owner.cipher};
    struct Factory final:m::CryptoFactory {
        Invocation& v;explicit Factory(Invocation& p):v(p){}
        std::unique_ptr<m::Cipher> create_0423d0()override{return v.lower([&]() -> std::unique_ptr<m::Cipher>{
            v.prepare(v.ciphers);
            auto result=v.logic.deps_.factory->create_0423d0();if(!result)throw s::UnsupportedInput("Missing selected temporary cipher");
            auto* raw=result.get();v.ciphers.push_back(std::move(result));return std::make_unique<Cipher>(v,*raw);
        });}
    } factory{*this};
    struct Random final:m::Random {
        Invocation& v;explicit Random(Invocation& p):v(p){}
        m::Key challenge_bytes_034cf0()override{return v.lower([&]{return v.logic.deps_.random->challenge_bytes_034cf0();});}
        m::Key key_from_response_034c30(std::string_view text)override{return v.lower([&]{return v.logic.deps_.random->key_from_response_034c30(text);});}
    } random{*this};
    struct Effects final:m::Effects {
        Invocation& v;explicit Effects(Invocation& p):v(p){}
        int atoi_05ee08(std::string_view text)override{return v.lower([&]{return v.logic.deps_.effects->atoi_05ee08(text);});}
        bool registered_handler(std::uint32_t rva,const m::XmlNode& node,m::XmlPrinter& printer)override{return v.lower([&]{return v.logic.deps_.effects->registered_handler(rva,node,printer);},true);}
        [[noreturn]]void invalid_message_02bd30_04d228()override{v.lower([&]{v.logic.deps_.effects->invalid_message_02bd30_04d228();},false,true);std::terminate();}
        [[noreturn]]void thread_error_04a648(int code)override{v.admit();v.logic.poisoned_=true;v.logic.deps_.effects->thread_error_04a648(code);std::terminate();}
        void log_received(std::string_view text)override{v.lower([&]{v.logic.deps_.effects->log_received(text);});}
        void log_response(std::string_view text)override{v.lower([&]{v.logic.deps_.effects->log_response(text);});}
        void log_event(std::string_view text)override{v.lower([&]{v.logic.deps_.effects->log_event(text);});}
        void log_bad_xml(std::string_view error,std::string_view text)override{v.lower([&]{v.logic.deps_.effects->log_bad_xml(error,text);});}
        void log_decryption_failure(std::string_view text)override{v.lower([&]{v.logic.deps_.effects->log_decryption_failure(text);});}
    } effects{*this};
    template<class T>void prepare(std::vector<std::unique_ptr<T>>& values){
        if(values.size()==values.max_size())throw std::length_error("Native opaque owner inventory exhausted");
        values.reserve(values.size()+1); // before opaque return; adoption cannot allocate
    }
    template<class T>void retire(std::vector<std::unique_ptr<T>>& values){
        for(auto& value:values)if(value)lower([&]{value.reset();});values.clear();
    }
    void retire_objects(){document.reset();retire(ciphers);retire(printers);retire(documents);}
    struct Bytes final:s::ReceiveBufferApi {
        Invocation& v;explicit Bytes(Invocation& p):v(p){}
        std::uintptr_t allocate_04b0bc(std::uint64_t n)override{
            v.admit();const auto result=v.parent.buffer_context().api.allocate_04b0bc(n);v.admit();return result;
        }
        void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{
            v.admit();if(!n)return;auto target=v.memory.view(to,n),source=v.memory.view(from,n);std::memmove(target.data(),source.data(),target.size());
        }
        void fill_bytes(std::uintptr_t to,std::uint8_t byte,std::uint64_t n)override{
            v.admit();if(!n)return;auto target=v.memory.view(to,n);std::fill(target.begin(),target.end(),byte);
        }
        void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{v.admit();v.owned.free_sized(p,n);v.admit();}
        [[noreturn]]void length_error_00ce80()override{v.admit();v.logic.poisoned_=true;v.parent.buffer_context().api.length_error_00ce80();std::terminate();}
        [[noreturn]]void allocation_overflow_00cd30()override{v.admit();v.logic.poisoned_=true;v.parent.buffer_context().api.allocation_overflow_00cd30();std::terminate();}
        [[noreturn]]void invalid_parameter_05c974()override{v.admit();v.logic.poisoned_=true;v.parent.buffer_context().api.invalid_parameter_05c974();std::terminate();}
    } bytes{*this};
    s::ReceiveBufferContext strings{memory,bytes};
    struct Mutation final:s::RequestMutationApi {
        Invocation& v;explicit Mutation(Invocation& p):v(p){}
        [[noreturn]]void out_of_range_00cec0()override{v.admit();throw std::out_of_range("Native request replacement offset");}
        s::ReceiveBufferString& grow_replace_02b940(s::ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uint64_t offset,std::uint64_t erase,std::uintptr_t source,std::uint64_t insert)override{
            auto& result=s::grow_replace_request_02b940(value,extra,hint,offset,erase,source,insert,v.strings);v.owned.track(result);return result;
        }
    } mutation_api{*this};
    s::RequestMutationContext mutation{strings,mutation_api};
    s::RecoveredPrefixCipherBlocks blocks{memory};s::PrefixCipherContext cipher{strings,blocks};
    std::string read(const s::ReceiveBufferString& value){return owned.read(value);}
    void assign(s::ReceiveBufferString& value,std::string_view text){owned.assign(value,text);refresh();}
    void document_identity(s::NativeMessageDocument& d){if(&d!=&locals.document)throw s::UnsupportedInput("Foreign native document projection");}
    void handler(std::uintptr_t id){if(id!=logic.session_.owner.identity()+0xf0)throw s::UnsupportedInput("Foreign native message handler");}
    struct Api final:s::NativeMessageApi,m::UnknownMessageEffects {
        Invocation& v;explicit Api(Invocation& p):v(p){}
        void construct_document_001830(s::NativeMessageDocument& d,bool mode,std::uint32_t flags)override{v.lower([&]{v.document_identity(d);if(!mode||flags||v.document)throw s::UnsupportedInput("Native document construction contract");d.error_6c=0;});}
        int parse_document_004280(s::NativeMessageDocument& d,std::uintptr_t data,std::uint64_t count)override{return v.lower([&]{
            v.document_identity(d);if(count!=UINT64_MAX)throw s::UnsupportedInput("Native parser requires C string sentinel");
            std::string input;for(std::uint64_t i=0;;++i){const auto byte=v.memory.view(data+i,1)[0];if(!byte)break;input.push_back(static_cast<char>(byte));}
            auto result=v.xml.parse_c_string(input);d.error_6c=result.has_error?1:0;v.document=std::move(result.document);return result.returned_code;
        });}
        void destroy_document_001e00(s::NativeMessageDocument& d)override{v.lower([&]{v.document_identity(d);v.document.reset();});}
        bool decrypt_042480(std::uintptr_t id,s::ReceiveBufferString& value,std::uint32_t offset)override{return v.lower([&]{v.handler(id);const auto result=s::decrypt_prefix_042480(v.logic.session_.owner.cipher.native(),value,offset,v.cipher);v.owned.track(value);return result;});}
        bool encrypt_042610(std::uintptr_t id,s::ReceiveBufferString& value,std::uint32_t offset)override{return v.lower([&]{v.handler(id);const auto result=s::encrypt_prefix_042610(v.logic.session_.owner.cipher.native(),value,offset,v.cipher);v.owned.track(value);return result;});}
        s::ReceiveBufferString& dispatch_032d00(s::ReceiveBufferString& out,std::uintptr_t id,s::NativeMessageDocument& d)override{return v.lower([&]() -> s::ReceiveBufferString&{
            v.handler(id);v.document_identity(d);if(!v.document)throw s::UnsupportedInput("Parsed document provider returned no document");
            const auto text=m::dispatch_032d00(*v.document,v.session_cipher,v.xml,v.factory,v.random,v.effects);
            v.assign(out,text);return out;
        },true,true);}
        void unknown_message_07bd70(const m::UnknownMessageInput& input)override{v.lower([&]{
            auto* diagnostic=dynamic_cast<m::UnknownMessageEffects*>(v.logic.deps_.effects.get());if(!diagnostic)throw s::UnsupportedInput("Unknown-message diagnostic provider required");
            struct Input final:m::UnknownMessageInput {Invocation& v;const m::UnknownMessageInput& raw;Input(Invocation& v,const m::UnknownMessageInput& r):v(v),raw(r){}std::string_view view()const override{return v.lower([&]{return raw.view();});}} guarded(v,input);
            diagnostic->unknown_message_07bd70(guarded);
        });}
        void log(s::NativeMessageLog kind,s::NativeMessageDocument& d,s::ReceiveBufferString& value)override{v.lower([&]{
            v.document_identity(d);v.owned.track(value);const auto text=v.read(value);auto& effects=v.effects;
            switch(kind){
            case s::NativeMessageLog::received:effects.log_received(text);break;
            case s::NativeMessageLog::response:effects.log_response(text);break;
            case s::NativeMessageLog::event:effects.log_event(text);break;
            case s::NativeMessageLog::bad_xml:effects.log_bad_xml(v.document?v.document->error_name():std::string_view{},text);break;
            case s::NativeMessageLog::decryption_failure:effects.log_decryption_failure(text);break;
            }
        });}
        void grow_nul_012350(s::ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uint8_t byte)override{v.lower([&]{s::grow_append_character_012350(value,extra,hint,byte,v.strings);v.owned.track(value);});}
        void grow_append_007e30(s::ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uintptr_t source,std::uint64_t n)override{v.lower([&]{s::grow_append_bytes_007e30(value,extra,hint,source,n,v.strings);v.owned.track(value);});}
        s::NativeMessageEventQueue& event_queue_02c400()override{return v.lower([&]() -> s::NativeMessageEventQueue&{auto& result=v.logic.deps_.events->queue();v.events=&result;return result;});}
        s::NativeMessageMutex& event_mutex_02c620()override{return v.lower([&]() -> s::NativeMessageMutex&{return v.logic.deps_.events->mutex();});}
        int lock_04a6ac(s::NativeMessageMutex& mutex)override{return v.lower([&]{return v.logic.deps_.events->lock(mutex);});}
        void unlock_04a6b4(s::NativeMessageMutex& mutex)override{v.lower([&]{v.logic.deps_.events->unlock(mutex);});}
        [[noreturn]]void thread_error_04a648(int code)override{v.effects.thread_error_04a648(code);std::terminate();}
    } api{*this};
    std::array<s::NativeMessageBinding,1> bindings;
    s::NativeMessageContext context;
    Invocation(Logic& l,s::SessionMemoryDomain& p,sp::Strings& o,s::ReceiveBufferString* out,s::ReceiveBufferString* req,std::function<void()> check)
        :logic(l),parent(p),owned(o),output(out?*out:greeting_output),request(req?*req:greeting_request),checkpoint(std::move(check)),
         bindings{{{l.session_.owner.identity()+0xf0,&l.session_.owner.cipher.encrypted_c4}}},
         context{bindings,l.deps_.show_lsx,strings,mutation,locals,api}{}
};
Logic::Logic(sp::SessionView session,Dependencies deps):session_(session),deps_(std::move(deps)){
    if(!deps_.xml||!deps_.factory||!deps_.random||!deps_.effects||!deps_.events)throw std::invalid_argument("Complete native message domain providers required");
}
Logic::~Logic()noexcept{if(invocation_||poisoned_)std::terminate();retiring_=true;{Dependencies retiring=std::move(deps_);}}
void Logic::validate_owner(s::SessionMemoryDomain& parent)const{
    auto* const control=session_.owner.weak_self.control_08;
    if(!parent.owns_control(control)||!control->strong_08.load()||session_.owner.weak_self.object_00!=session_.owner.identity()||
       &static_cast<s::SessionControl*>(control)->object!=&session_.owner)
        throw std::invalid_argument("Native message owner must belong to the admitted parent");
}
std::string Logic::greeting(){if(retiring_)throw std::logic_error("Native message logic is retiring");if(invocation_){poisoned_=true;throw std::logic_error("Native greeting reentry");}throw s::UnsupportedInput("Native message greeting requires its typed Profile route");}
std::string Logic::greeting_native(s::SessionMemoryDomain& parent,sp::Strings& owned,std::function<void()> checkpoint){
    if(retiring_)throw std::logic_error("Native message logic is retiring");
    if(invocation_){poisoned_=true;throw std::logic_error("Native greeting cannot reenter");}
    if(poisoned_)throw std::logic_error("Native message prefix is retained");
    if(!checkpoint||owned.domain_identity()!=&parent)throw std::invalid_argument("Matching native string domain and source admission required");
    checkpoint();(void)parent.buffer_context();validate_owner(parent);
    invocation_=std::make_unique<Invocation>(*this,parent,owned,nullptr,nullptr,std::move(checkpoint));
    try{auto& call=*invocation_;call.admit();const auto text=m::initial_message_02d540(call.xml,call.random,deps_.version);
        call.admit();call.retire_objects();call.admit();invocation_.reset();return text;
    }catch(...){poisoned_=true;throw;}
}
std::string Logic::process(std::string&){if(!retiring_&&invocation_)poisoned_=true;throw s::UnsupportedInput("Native message logic requires its typed Profile route");}
bool Logic::retained_prefix()const noexcept{return invocation_!=nullptr||poisoned_;}
bool Logic::matches(sp::SessionView view)const noexcept{return &view.owner==&session_.owner&&view.token==session_.token;}
void Logic::process_native(s::SessionMemoryDomain& parent,sp::Strings& owned,s::ReceiveBufferString& output,s::ReceiveBufferString& request,std::function<void()> checkpoint){
    if(retiring_)throw std::logic_error("Native message logic is retiring");
    if(invocation_){poisoned_=true;throw std::logic_error("Native message invocation cannot reenter");}
    if(poisoned_)throw std::logic_error("Native message invocation cannot resume");
    if(!checkpoint||&output==&request||owned.domain_identity()!=&parent)throw std::invalid_argument("Distinct native message prefixes, matching string domain and source admission required");
    checkpoint();(void)parent.buffer_context();validate_owner(parent);
    invocation_=std::make_unique<Invocation>(*this,parent,owned,&output,&request,std::move(checkpoint));
    try{
        auto& call=*invocation_;call.admit();
        s::process_native_message_033d70(session_.owner.identity()+0xf0,output,request,call.context);
        call.admit();call.track();call.retire_objects();call.admit();invocation_.reset();
    }catch(...){poisoned_=true;throw;}
}
} // namespace anadius::native_profile
