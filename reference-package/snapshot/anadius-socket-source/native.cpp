#include "native.hpp"
#include <cstring>
#include <limits>
#include <memory>
#include <list>
#include <mutex>

namespace anadius::socket_recovered {
std::string misc_message_020a80(std::int32_t code){
    // Retained pseudo ev_f52c22ab6d0482e58f7ca56d36cbfec5a1c0913b03e9f41689c3a3276162843d,
    // asm ev_604360fb740f3377313684b0bfdc1e461a65ae4fedb7995928d16dbc8f4448c0.
    switch(code){
    case 1:return "Already open";
    case 2:return "End of file";
    case 3:return "Element not found";
    case 4:return "The descriptor does not fit into the select call's fd_set";
    default:return "asio.misc error";
    }
}
const std::error_category& misc_error_category(){
    class Misc final:public std::error_category {
    public:
        const char* name()const noexcept override{return "asio.misc";}
        std::string message(int code)const override{return misc_message_020a80(code);}
    };
    static const Misc category;return category;
}
std::string system_message_020950(std::int32_t code,SystemMessageState& state,SystemMessageImports imports,
    const std::function<void()>& checkpoint){
    if(!imports.format_message||!imports.local_free)throw std::invalid_argument("Complete system-message SDK imports required");
    if(state.buffer||state.entered||state.release_entered)throw std::logic_error("System-message SDK prefix is already retained");
    const auto check=[&]{
        if(checkpoint){
            const auto saved=::GetLastError();struct Error {DWORD value;~Error(){::SetLastError(value);}} error{saved};
            checkpoint(); // metadata only; imported SDK error effects stay live
        }
    };
    state.entered=true;check();
    auto count=imports.format_message(0x1300,nullptr,static_cast<DWORD>(code),0x400,
        reinterpret_cast<LPSTR>(&state.buffer),0,nullptr);
    auto* const buffer=state.buffer;check();
    bool use_message=count!=0;
    if(use_message&&buffer[count-1]=='\n'){buffer[--count]=0;use_message=count!=0;}
    if(use_message&&buffer[count-1]=='\r'){buffer[--count]=0;use_message=count!=0;}
    // Native scans C strlen after the counted trim. A leading/embedded NUL
    // can shorten success without selecting the fallback.
    std::string result=use_message?std::string(buffer):std::string("asio.system error");
    check();state.release_entered=true;
    (void)imports.local_free(reinterpret_cast<HLOCAL>(buffer)); // including NULL; return ignored
    check();state={};return result;
}
namespace {
struct MessageMetadataError {DWORD value=::GetLastError();~MessageMetadataError(){::SetLastError(value);}};
class NativeSystemCategory final:public std::error_category {
    struct Attempt {SystemMessageState state;};
    const SystemMessageImports imports_;
    mutable std::mutex gate_;
    // Each call owns an independent stable row. Failed SDK prefixes remain
    // passive here; another call neither retries nor retires that row.
    mutable std::list<Attempt> attempts_;
public:
    explicit NativeSystemCategory(SystemMessageImports imports):imports_(imports){}
    bool matches(SystemMessageImports imports)const noexcept{return imports_.format_message==imports.format_message&&imports_.local_free==imports.local_free;}
    const char* name()const noexcept override{return "asio.system";}
    std::string message(int code)const override{
        std::list<Attempt> prepared;
        {
            MessageMetadataError error;
            prepared.emplace_back(); // allocation/new-handler outside gate
        }
        const auto attempt=prepared.begin();
        {
            MessageMetadataError error;std::lock_guard lock(gate_);
            attempts_.splice(attempts_.end(),prepared,attempt);
        }
        // No mutex or shared busy flag spans SDK/string effects. A throw leaves
        // this call's row retained; there is no guessed native EH buffer free.
        auto result=system_message_020950(code,attempt->state,imports_);
        {
            MessageMetadataError error;
            {std::lock_guard lock(gate_);prepared.splice(prepared.end(),attempts_,attempt);}
            prepared.clear(); // retirement outside gate preserves the SDK error
        }
        return result;
    }
};
struct SystemCategoryPool {
    std::mutex gate;
    std::list<std::unique_ptr<NativeSystemCategory>> rows;
};
SystemCategoryPool& system_category_pool(){static SystemCategoryPool pool;return pool;}
}
const std::error_category& system_error_category_028620(SystemMessageImports imports){
    MessageMetadataError error;
    if(!imports.format_message||!imports.local_free)throw std::invalid_argument("Complete system-category SDK imports required");
    auto& pool=system_category_pool();
    {std::lock_guard lock(pool.gate);for(const auto& row:pool.rows)if(row->matches(imports))return *row;}
    // Both the category and its physical registry node are prepared outside the
    // metadata gate. A reentrant/concurrent lookup may publish the same pair.
    std::list<std::unique_ptr<NativeSystemCategory>> prepared;
    prepared.push_back(std::make_unique<NativeSystemCategory>(imports));
    const NativeSystemCategory* selected=nullptr;
    {
        std::lock_guard lock(pool.gate);
        for(const auto& row:pool.rows)if(row->matches(imports)){selected=row.get();break;}
        if(!selected){selected=prepared.front().get();pool.rows.splice(pool.rows.end(),prepared);}
    }
    // Unpublished duplicate candidates also retire outside the metadata gate.
    return *selected;
}
static_assert(sizeof(Socket)==8 && sizeof(SOCKET)==8);
static_assert(sizeof(wchar_t)==2 && sizeof(DWORD)==4);
static_assert(AF_INET==2 && AF_INET6==23 && sizeof(sockaddr_storage)==128);
static_assert(sizeof(sockaddr_in)==16 && offsetof(sockaddr_in,sin_addr)==4);
static_assert(sizeof(sockaddr_in6)==28 && offsetof(sockaddr_in6,sin6_addr)==8 &&
              offsetof(sockaddr_in6,sin6_scope_id)==24);
void NativeApi::set_last_error(std::int32_t value){imports_.set_last_error(value);}
Category NativeApi::system_category_028620(){return categories_.get_028620();}
std::int32_t NativeApi::get_last_error(){return imports_.get_last_error();}
Conversion NativeApi::convert_acp(const char* text,std::int32_t capacity) {
    if(capacity<=0)throw UnsupportedInput("Wide address capacity must be positive");
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[static_cast<std::size_t>(capacity)]);
    const auto result=imports_.convert(0,0,text,-1,buffer.get(),capacity);
    Conversion converted;converted.result=result;
    if(result>0 && result<=capacity && buffer[result-1]==L'\0')
        converted.text.emplace(buffer.get(),static_cast<std::size_t>(result-1));
    return converted;
}
AddressCall NativeApi::string_to_address(Conversion& wide,std::int32_t family,std::int32_t initial_size) {
    if(!wide.text)throw UnsupportedInput("Original address conversion would read undefined wide storage");
    if(initial_size!=128 || (family!=AF_INET && family!=AF_INET6))
        throw UnsupportedInput("Typed address call requires modeled family and128byte storage");
    sockaddr_storage storage{};int size=initial_size;
    AddressCall call;call.result=imports_.parse(wide.text->data(),family,nullptr,
        reinterpret_cast<sockaddr*>(&storage),&size);
    if(call.result!=-1){
        const auto needed=family==AF_INET?sizeof(sockaddr_in):sizeof(sockaddr_in6);
        if(size<static_cast<int>(needed) || size>128)
            throw UnsupportedInput("Successful address API output outside declared initialized range");
        // Copy actual SDK structures from storage; no incompatible C++ alias.
        if(family==AF_INET){sockaddr_in value;std::memcpy(&value,&storage,sizeof(value));
            std::memcpy(call.bytes.data(),&value.sin_addr,4);
        }else{sockaddr_in6 value;std::memcpy(&value,&storage,sizeof(value));
            std::memcpy(call.bytes.data(),&value.sin6_addr,16);call.scope=value.sin6_scope_id;
        }
    }
    return call;
}
SendCall NativeApi::wsa_send(Socket socket,std::span<Buffer> buffers,std::uint32_t flags) {
    if(buffers.size()>std::numeric_limits<DWORD>::max())
        throw UnsupportedInput("WSABUF count exceeds modeled DWORD domain");
    DWORD bytes=0;
    const auto result=imports_.send(static_cast<SOCKET>(socket),buffers.data(),
        static_cast<DWORD>(buffers.size()),&bytes,flags,nullptr,nullptr);
    return {result,bytes};
}
std::int32_t NativeApi::select_write(Socket socket,std::int32_t nfds) {
    fd_set write{};write.fd_count=1;write.fd_array[0]=static_cast<SOCKET>(socket);
    return imports_.select_write(nfds,nullptr,&write,nullptr,nullptr);
}
} // namespace anadius::socket_recovered
