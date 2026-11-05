#include "native.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-socket-source/append_growth.hpp"
#include "../anadius-socket-source/prefix_cipher.hpp"
#include "../anadius-dispatch-source/router.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <list>
#include <limits>
#include <mutex>
#include <type_traits>

namespace anadius::native_profile {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
template<class Error=std::logic_error>[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* message){lock.unlock();throw Error(message);}
bool complete(const Dependencies& d){return d.xml&&d.factory&&d.random&&d.effects&&d.events;}
}
struct Logic::State {
    enum class Phase {Preparing,Running,Retiring,Failed};
    struct Admission {
        s::SessionMemoryDomain* parent;const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        s::ReceiveBufferString* output; s::ReceiveBufferString* request;Admission* next=nullptr;
        Admission(s::SessionMemoryDomain& p,s::ReceiveBufferString* out,s::ReceiveBufferString* req):parent(&p),output(out),request(req){}
    };
    mutable std::mutex gate;
    s::SessionMemoryDomain* parent=nullptr;s::OperationPolicy policy=s::OperationPolicy::Serialized;
    bool poisoned=false,retiring=false;Admission* admissions=nullptr;
    std::list<std::unique_ptr<Invocation>> calls;
    void remove(Admission& admission)noexcept{
        auto** at=&admissions;while(*at&&*at!=&admission)at=&(*at)->next;
        if(!*at)std::terminate();*at=admission.next;
    }
};
struct Logic::Invocation {
    Logic& logic;s::SessionMemoryDomain& parent;sp::Strings& owned;
    s::ReceiveBufferString greeting_output,greeting_request;
    s::ReceiveBufferString& output;s::ReceiveBufferString& request;
    Dependencies deps;
    std::function<void()> checkpoint;
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    State::Phase phase=State::Phase::Preparing;
    s::NativeMessageLocals locals;
    std::unique_ptr<m::XmlDocument> document;
    // Each opaque result is adopted before any proxy allocation/postcheck.
    // Unknown prefixes remain owned by this invocation, never a sibling.
    std::vector<std::unique_ptr<m::XmlDocument>> documents;
    std::vector<std::unique_ptr<m::XmlPrinter>> printers;
    std::vector<std::unique_ptr<m::Cipher>> ciphers;
    std::array<s::ReceiveBufferRegion,4> regions{};
    struct Composite final:s::ReceiveBufferResolver {
        Invocation& v;explicit Composite(Invocation& call):v(call){}
        s::MemoryLookup lookup_bytes(std::uintptr_t address,std::uint64_t count,std::span<std::uint8_t>& out)override{
            PreserveError error;v.validate();
            std::span<std::uint8_t> local;bool found=false,partial=false;
            for(const auto& region:v.regions){
                if(address>=region.address){
                    const auto offset=address-region.address;
                    if(offset<=region.bytes.size()&&count<=region.bytes.size()-offset){
                        if(found)return s::MemoryLookup::Ambiguous;
                        found=true;local=region.bytes.subspan(static_cast<std::size_t>(offset),static_cast<std::size_t>(count));
                    }else if(count&&offset<region.bytes.size())partial=true;
                }else if(count&&region.address-address<count)partial=true;
            }
            if(partial)return s::MemoryLookup::Ambiguous;
            std::span<std::uint8_t> upstream;
            const auto match=v.parent.buffer_context().memory.lookup_bytes(address,count,upstream);
            if(match==s::MemoryLookup::Ambiguous)return match;
            if(found&&match==s::MemoryLookup::Found){
                // A supplied prefix may already be an actual Parent inline
                // region. Count the identical physical window once.
                if(local.data()!=upstream.data()||local.size()!=upstream.size())return s::MemoryLookup::Ambiguous;
                out=upstream;return s::MemoryLookup::Found;
            }
            if(match==s::MemoryLookup::Found){out=upstream;return match;}
            if(found){out=local;return s::MemoryLookup::Found;}
            return s::MemoryLookup::Miss;
        }
    } composite{*this};
    s::ReceiveBufferMemory memory{{},&composite};
    void refresh()noexcept{
        // Only these invocation-owned/supplied inline prefixes. Heap windows
        // resolve on demand through the exact Parent; no union catalog copy.
        regions={{{reinterpret_cast<std::uintptr_t>(output.storage_00.data()),output.storage_00},
                  {reinterpret_cast<std::uintptr_t>(request.storage_00.data()),request.storage_00},
                  {reinterpret_cast<std::uintptr_t>(locals.response.storage_00.data()),locals.response.storage_00},
                  {reinterpret_cast<std::uintptr_t>(locals.temporary.storage_00.data()),locals.temporary.storage_00}}};
    }
    void fail()noexcept{logic.fail(this);}
    void validate(){logic.check(*this);(void)parent.buffer_context();logic.check(*this);}
    void admit(){
        try{validate();checkpoint();validate();}
        catch(...){fail();throw;}
    }
    void track(){owned.track(output);owned.track(request);}
    template<class F>std::invoke_result_t<F> lower(F&& fn,bool control=false,bool unknown=false){
        try{
            admit();
            if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(fn)();admit();}
            else{decltype(auto) result=std::forward<F>(fn)();admit();return result;}
        }catch(const m::NoResponseRequiredError&){if(!control){fail();throw;}try{admit();}catch(...){fail();throw;}throw;}
        catch(const m::UnknownMessageError&){if(!unknown){fail();throw;}try{admit();}catch(...){fail();throw;}throw;}catch(...){fail();throw;}
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
            }catch(...){v.fail();throw;}
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
            auto result=v.deps.xml->parse_c_string(text);
            auto* raw=result.document.get();if(raw){v.documents.push_back(std::move(result.document));result.document=std::make_unique<Document>(v,*raw);}return result;
        });}
        std::unique_ptr<m::XmlPrinter> printer()override{return v.lower([&]() -> std::unique_ptr<m::XmlPrinter>{
            v.prepare(v.printers);
            auto result=v.deps.xml->printer();if(!result)throw s::UnsupportedInput("Missing selected XML printer");
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
            auto result=v.deps.factory->create_0423d0();if(!result)throw s::UnsupportedInput("Missing selected temporary cipher");
            auto* raw=result.get();v.ciphers.push_back(std::move(result));return std::make_unique<Cipher>(v,*raw);
        });}
    } factory{*this};
    struct Random final:m::Random {
        Invocation& v;explicit Random(Invocation& p):v(p){}
        m::Key challenge_bytes_034cf0()override{return v.lower([&]{return v.deps.random->challenge_bytes_034cf0();});}
        m::Key key_from_response_034c30(std::string_view text)override{return v.lower([&]{return v.deps.random->key_from_response_034c30(text);});}
    } random{*this};
    struct Effects final:m::Effects {
        Invocation& v;explicit Effects(Invocation& p):v(p){}
        int atoi_05ee08(std::string_view text)override{return v.lower([&]{return v.deps.effects->atoi_05ee08(text);});}
        bool registered_handler(std::uint32_t rva,const m::XmlNode& node,m::XmlPrinter& printer)override{return v.lower([&]{return v.deps.effects->registered_handler(rva,node,printer);},true);}
        [[noreturn]]void invalid_message_02bd30_04d228()override{v.lower([&]{v.deps.effects->invalid_message_02bd30_04d228();},false,true);std::terminate();}
        [[noreturn]]void thread_error_04a648(int code)override{v.admit();v.fail();v.deps.effects->thread_error_04a648(code);std::terminate();}
        void log_received(std::string_view text)override{v.lower([&]{v.deps.effects->log_received(text);});}
        void log_response(std::string_view text)override{v.lower([&]{v.deps.effects->log_response(text);});}
        void log_event(std::string_view text)override{v.lower([&]{v.deps.effects->log_event(text);});}
        void log_bad_xml(std::string_view error,std::string_view text)override{v.lower([&]{v.deps.effects->log_bad_xml(error,text);});}
        void log_decryption_failure(std::string_view text)override{v.lower([&]{v.deps.effects->log_decryption_failure(text);});}
    } effects{*this};
    template<class T>void prepare(std::vector<std::unique_ptr<T>>& values){
        if(values.size()==values.max_size())throw std::length_error("Native opaque owner inventory exhausted");
        values.reserve(values.size()+1); // before opaque return; adoption cannot allocate
    }
    template<class T>void retire(std::vector<std::unique_ptr<T>>& values){
        for(auto& value:values)if(value)lower([&]{value.reset();});values.clear();
    }
    void retire_objects(){document.reset();retire(ciphers);retire(printers);retire(documents);}
    void retire_sources(){
        // Keep this descriptor published through every callback-bearing target
        // retirement. Stop at the first observed failure; retain the rest.
        {std::function<void()> retiring;retiring.swap(checkpoint);}
        validate();
        deps.events.reset();validate();
        deps.effects.reset();validate();
        deps.random.reset();validate();
        deps.factory.reset();validate();
        deps.xml.reset();validate();
        {std::string retiring;retiring.swap(deps.version);}validate();
        deps.lifetime.reset();validate();
    }
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
        [[noreturn]]void length_error_00ce80()override{v.admit();v.fail();v.parent.buffer_context().api.length_error_00ce80();std::terminate();}
        [[noreturn]]void allocation_overflow_00cd30()override{v.admit();v.fail();v.parent.buffer_context().api.allocation_overflow_00cd30();std::terminate();}
        [[noreturn]]void invalid_parameter_05c974()override{v.admit();v.fail();v.parent.buffer_context().api.invalid_parameter_05c974();std::terminate();}
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
            auto* diagnostic=dynamic_cast<m::UnknownMessageEffects*>(v.deps.effects.get());if(!diagnostic)throw s::UnsupportedInput("Unknown-message diagnostic provider required");
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
        s::NativeMessageEventQueue& event_queue_02c400()override{return v.lower([&]() -> s::NativeMessageEventQueue&{auto& result=v.deps.events->queue();return result;});}
        s::NativeMessageMutex& event_mutex_02c620()override{return v.lower([&]() -> s::NativeMessageMutex&{return v.deps.events->mutex();});}
        int lock_04a6ac(s::NativeMessageMutex& mutex)override{return v.lower([&]{return v.deps.events->lock(mutex);});}
        void unlock_04a6b4(s::NativeMessageMutex& mutex)override{v.lower([&]{v.deps.events->unlock(mutex);});}
        [[noreturn]]void thread_error_04a648(int code)override{v.effects.thread_error_04a648(code);std::terminate();}
    } api{*this};
    std::array<s::NativeMessageBinding,1> bindings;
    s::NativeMessageContext context;
    Invocation(Logic& l,s::SessionMemoryDomain& p,sp::Strings& o,s::ReceiveBufferString* out,s::ReceiveBufferString* req)
        :logic(l),parent(p),owned(o),output(out?*out:greeting_output),request(req?*req:greeting_request),
         bindings{{{l.session_.owner.identity()+0xf0,&l.session_.owner.cipher.encrypted_c4}}},
         context{bindings,deps.show_lsx,strings,mutation,locals,api}{refresh();}
};
Logic::Logic(sp::SessionView session,Dependencies deps):session_(session),deps_(std::move(deps)),state_(std::make_unique<State>()){
    if(!complete(deps_))throw std::invalid_argument("Complete native message domain providers required");
}
Logic::~Logic()noexcept{
    {PreserveError error;std::unique_lock lock(state_->gate);
        if(state_->poisoned||state_->admissions||!state_->calls.empty())std::terminate();
        state_->retiring=true;
    }
    // Dependency callbacks see the live retirement seal, with no active prefix.
    {Dependencies retiring=std::move(deps_);}
}
void Logic::validate_owner(s::SessionMemoryDomain& parent)const{
    auto* const control=session_.owner.weak_self.control_08;
    if(!parent.owns_control(control)||!control->strong_08.load()||session_.owner.weak_self.object_00!=session_.owner.identity()||
       &static_cast<s::SessionControl*>(control)->object!=&session_.owner)
        throw std::invalid_argument("Native message owner must belong to the admitted parent");
}
void Logic::fail(Invocation* call)noexcept{
    PreserveError error;std::unique_lock lock(state_->gate);state_->poisoned=true;
    if(call)call->phase=State::Phase::Failed;
}
void Logic::check(const Invocation& call)const{
    PreserveError error;const auto thread=::GetCurrentThreadId();auto* const fiber=::GetCurrentFiber();
    std::unique_lock lock(state_->gate);bool found=false;
    for(const auto& current:state_->calls)if(current.get()==&call){found=true;break;}
    if(state_->retiring||state_->poisoned||!found||call.phase==State::Phase::Failed||
       call.thread!=thread||call.fiber!=fiber||state_->parent!=&call.parent)
        reject(lock,"Native message exact invocation admission was poisoned or ended");
}
std::string Logic::greeting(){
    PreserveError error;std::unique_lock lock(state_->gate);
    if(state_->retiring)reject(lock,"Native message logic is retiring");
    if(state_->admissions||!state_->calls.empty()){state_->poisoned=true;reject(lock,"Native greeting reentry");}
    lock.unlock();throw s::UnsupportedInput("Native message greeting requires its typed Profile route");
}
std::string Logic::process(std::string&){
    PreserveError error;std::unique_lock lock(state_->gate);
    if(!state_->retiring&&(state_->admissions||!state_->calls.empty()))state_->poisoned=true;
    lock.unlock();throw s::UnsupportedInput("Native message logic requires its typed Profile route");
}
bool Logic::retained_prefix()const noexcept{
    PreserveError error;std::unique_lock lock(state_->gate);
    return state_->poisoned||state_->admissions||!state_->calls.empty();
}
bool Logic::failed_prefix()const noexcept{
    PreserveError error;std::unique_lock lock(state_->gate);
    if(state_->poisoned)return true;
    return std::any_of(state_->calls.begin(),state_->calls.end(),[](const auto& call){return call->phase==State::Phase::Failed;});
}
std::size_t Logic::active_calls()const noexcept{
    PreserveError error;std::unique_lock lock(state_->gate);std::size_t count=0;
    for(auto* pending=state_->admissions;pending;pending=pending->next)++count;
    for(const auto& call:state_->calls)if(call->phase!=State::Phase::Failed)++count;
    return count;
}
bool Logic::matches(sp::SessionView view)const noexcept{return &view.owner==&session_.owner&&view.token==session_.token;}
std::string Logic::greeting_native(s::SessionMemoryDomain& parent,sp::Strings& owned,std::function<void()> checkpoint){
    return invoke(parent,owned,nullptr,nullptr,deps_,checkpoint,true);
}
void Logic::process_native(s::SessionMemoryDomain& parent,sp::Strings& owned,s::ReceiveBufferString& output,s::ReceiveBufferString& request,std::function<void()> checkpoint){
    process_native_call(parent,owned,output,request,deps_,std::move(checkpoint));
}
void Logic::process_native_call(s::SessionMemoryDomain& parent,sp::Strings& owned,s::ReceiveBufferString& output,s::ReceiveBufferString& request,
    const Dependencies& selected,std::function<void()> checkpoint){
    (void)invoke(parent,owned,&output,&request,selected,checkpoint,false);
}
std::string Logic::invoke(s::SessionMemoryDomain& parent,sp::Strings& owned,s::ReceiveBufferString* output,s::ReceiveBufferString* request,
    const Dependencies& selected,std::function<void()>& checkpoint,bool greeting){
    State::Admission admission(parent,output,request);
    const auto aliases=[&](const s::ReceiveBufferString* out,const s::ReceiveBufferString* req){
        return (output&&(output==out||output==req))||(request&&(request==out||request==req));
    };
    {
        PreserveError error;std::unique_lock lock(state_->gate);
        if(state_->retiring)reject(lock,"Native message logic is retiring");
        if(state_->poisoned)reject(lock,"Native message prefix is retained");
        if(state_->parent&&state_->parent!=&parent)reject<std::invalid_argument>(lock,"Native Logic belongs to a different Parent");
        for(auto* pending=state_->admissions;pending;pending=pending->next)
            if(pending->parent!=&parent)reject<std::invalid_argument>(lock,"Native Logic Parent admission is already reserved");
        const auto policy=state_->parent?state_->policy:parent.operation_policy();
        if(policy==s::OperationPolicy::Serialized&&(state_->admissions||!state_->calls.empty())){
            state_->poisoned=true;reject(lock,"Native message invocation cannot reenter");
        }
        for(auto* pending=state_->admissions;pending;pending=pending->next)
            if((pending->thread==admission.thread&&pending->fiber==admission.fiber)||aliases(pending->output,pending->request)){
                state_->poisoned=true;reject(lock,"Native invocation admission cannot reenter or alias an active prefix");
            }
        for(const auto& call:state_->calls)
            if((call->thread==admission.thread&&call->fiber==admission.fiber)||aliases(&call->output,&call->request)){
                state_->poisoned=true;reject(lock,"Native invocation cannot reenter its actor or alias another message");
            }
        if(!checkpoint||owned.domain_identity()!=&parent||(!greeting&&output==request))
            reject<std::invalid_argument>(lock,"Distinct native prefixes, matching string domain and source admission required");
        if(!complete(selected))reject<std::invalid_argument>(lock,"Complete native message call providers required");
        admission.next=state_->admissions;state_->admissions=&admission;
    }
    struct Pending {
        State& state;State::Admission& admission;bool linked=true;
        ~Pending(){PreserveError error;if(linked){std::unique_lock lock(state.gate);state.remove(admission);}}
        void remove_locked(){state.remove(admission);linked=false;}
    } pending{*state_,admission};
    // Before any descriptor/dependency copy or opaque callback, require actual
    // explicit Parent entry and the exact live session/control ownership.
    (void)parent.buffer_context();validate_owner(parent);
    {
        PreserveError error;std::unique_lock lock(state_->gate);
        if(state_->retiring||state_->poisoned)reject(lock,"Native invocation preparation lost source admission");
        if(!state_->parent){state_->parent=&parent;state_->policy=parent.operation_policy();}
        else if(state_->parent!=&parent)reject<std::invalid_argument>(lock,"Native invocation Parent changed during admission");
    }
    std::list<std::unique_ptr<Invocation>> prepared,retired;Invocation* call=nullptr;
    try{
        {
            PreserveError error;
            prepared.emplace_back();prepared.front()=std::make_unique<Invocation>(*this,parent,owned,output,request);call=prepared.front().get();
            std::unique_lock lock(state_->gate);
            state_->calls.splice(state_->calls.end(),prepared);pending.remove_locked();
            if(state_->retiring||state_->poisoned)reject(lock,"Native invocation descriptor preparation was poisoned");
        }
        // The published descriptor retains partial copies and callback targets
        // if compiler allocation/new-handler reentry or a later check fails.
        {PreserveError error;call->checkpoint.swap(checkpoint);call->deps=selected;}
        {
            PreserveError error;std::unique_lock lock(state_->gate);
            if(state_->retiring||state_->poisoned)reject(lock,"Native invocation dependency copy was poisoned");
            call->phase=State::Phase::Running;
        }
        call->admit();std::string result;
        if(greeting)result=m::initial_message_02d540(call->xml,call->random,call->deps.version);
        else s::process_native_message_033d70(session_.owner.identity()+0xf0,*output,*request,call->context);
        call->admit();if(!greeting)call->track();call->retire_objects();call->admit();
        {
            PreserveError error;std::unique_lock lock(state_->gate);
            if(state_->retiring||state_->poisoned||call->phase==State::Phase::Failed)reject(lock,"Native invocation normal retirement was poisoned");
            call->phase=State::Phase::Retiring;
        }
        call->retire_sources();call->validate();
        {
            PreserveError error;std::unique_lock lock(state_->gate);
            if(state_->retiring||state_->poisoned||call->phase!=State::Phase::Retiring)reject(lock,"Native invocation source retirement was poisoned");
            const auto found=std::find_if(state_->calls.begin(),state_->calls.end(),[&](const auto& row){return row.get()==call;});
            if(found==state_->calls.end())std::terminate();retired.splice(retired.end(),state_->calls,found);
        }
        {PreserveError error;retired.clear();}
        return result;
    }catch(...){
        PreserveError error;
        if(call){
            std::unique_lock lock(state_->gate);
            if(!prepared.empty()&&prepared.front())state_->calls.splice(state_->calls.end(),prepared);
            call->phase=State::Phase::Failed;state_->poisoned=true;
        }
        throw; // Actual provider/byte prefix remains owned; no guessed cleanup.
    }
}
} // namespace anadius::native_profile
