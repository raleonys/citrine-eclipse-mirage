#include "query.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <windows.h>
#include <cstring>

namespace anadius::online_query {
online_recovered::Procedure query_slot(online_recovered::Object object,Slot slot){
    const auto offset=static_cast<std::size_t>(slot);
    if(slot!=Slot::Status08&&slot!=Slot::Username10&&slot!=Slot::Persona18&&slot!=Slot::UserId20&&slot!=Slot::Avatar28&&slot!=Slot::Auth30)
        throw std::invalid_argument("Unobserved online query slot");
    if(!object)throw std::invalid_argument("Null online query object");
    std::uintptr_t table=0,address=0;
    std::memcpy(&table,reinterpret_cast<const void*>(object),sizeof(table));
    if(!table)throw online_recovered::OutOfEvidenceInput("Null online query vptr");
    std::memcpy(&address,reinterpret_cast<const std::byte*>(table)+offset,sizeof(address));
    if(!address)throw online_recovered::OutOfEvidenceInput("Null online query slot");
    return address;
}
static_assert(sizeof(void*)==8);
} // namespace anadius::online_query

namespace anadius::online_version {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
void require_getter(online_query::Slot slot){
    Error error;
    if(slot!=online_query::Slot::Username10&&slot!=online_query::Slot::Persona18&&slot!=online_query::Slot::UserId20)
        throw std::invalid_argument("Online string query requires a getter slot");
}
}
struct Binding::QueryUse {
    Binding& owner;Procedure address;
    QueryUse(Binding& binding,Object object,online_query::Slot slot):owner(binding){
        Error error;
        owner.begin_object(object); // rejects stale/arbitrary tokens before read
        try{address=online_query::query_slot(object,slot);}
        catch(...){
            // Invalid selected layout: retain code/object without further reads.
            {std::lock_guard lock(owner.gate_);owner.poisoned_=true;owner.object_unknown_=true;}
            owner.end_object(); // only this failed constructor's reservation
            throw;
        }
    }
    ~QueryUse(){owner.end_object();}
};
std::int32_t Binding::status(Object object){
    QueryUse use(*this,object,online_query::Slot::Status08);
    return reinterpret_cast<online_query::StatusFunction>(use.address)(reinterpret_cast<void*>(object));
}
const char* Binding::getter(Object object,online_query::Slot slot){
    require_getter(slot);QueryUse use(*this,object,slot);
    return reinterpret_cast<online_query::GetterFunction>(use.address)(reinterpret_cast<void*>(object));
}
std::string Binding::copy_getter(Object object,online_query::Slot slot){
    require_getter(slot);QueryUse use(*this,object,slot);
    const auto* value=reinterpret_cast<online_query::GetterFunction>(use.address)(reinterpret_cast<void*>(object));
    Error error; // native outgoing error survives string allocation/metadata
    if(!value)throw online_recovered::OutOfEvidenceInput("035bd0 scans a null provider C string");
    return std::string(value); // NUL scan + allocation + copy remain reserved
}
bool Binding::avatar(Object object,const char16_t* path,const char* owner,std::int32_t size){
    {Error error;if(!path||!owner)throw std::invalid_argument("Online avatar requires path and owner C strings");}
    QueryUse use(*this,object,online_query::Slot::Avatar28);
    return reinterpret_cast<online_query::AvatarFunction>(use.address)(reinterpret_cast<void*>(object),path,owner,size)!=0;
}
const char* Binding::auth(Object object,const char* client){
    {Error error;if(!client)throw std::invalid_argument("Online auth requires a client C string");}
    QueryUse use(*this,object,online_query::Slot::Auth30);
    return reinterpret_cast<online_query::AuthFunction>(use.address)(reinterpret_cast<void*>(object),client);
}
std::string Binding::copy_auth(Object object,const char* client){
    {Error error;if(!client)throw std::invalid_argument("Online auth requires a client C string");}
    QueryUse use(*this,object,online_query::Slot::Auth30);
    const auto* value=reinterpret_cast<online_query::AuthFunction>(use.address)(reinterpret_cast<void*>(object),client);
    Error error;
    if(!value)throw online_recovered::OutOfEvidenceInput("02e416 scans a null provider auth C string");
    return std::string(value); // native NUL scan + owned copy while reserved
}
} // namespace anadius::online_version
