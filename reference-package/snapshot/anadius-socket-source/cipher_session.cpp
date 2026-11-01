#include "cipher_session.hpp"
#include "../anadius-crypto-source/constants.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
SessionCipher& construct_session_cipher_0423d0(SessionCipher& value){
    value.native_.mode_00=0;
    value.schedule_.construct_0423d0_in_place(); // existing zero176 schedule source
    value.native_.iv_b4.fill(0); // remaining16 native context bytes; flags preserved
    return value;
}
void set_session_key_iv_042760(SessionCipher& value,const crypto_recovered::Key& key,const crypto_recovered::Key& iv){
    value.schedule_.set_key_042780(key); // existing045ce0 source, no new expansion claim
    value.native_.iv_b4=iv; // native045650 loads IV after expansion
    value.native_.mode_00=1;
}
void set_session_key_042780(SessionCipher& value,const crypto_recovered::Key& key){
    value.schedule_.set_key_042780(key);value.native_.mode_00=2; // preserve live IV
}
SessionCipher& reset_session_cipher_02bc70(SessionCipher& value){
    construct_session_cipher_0423d0(value);value.encrypted_c4=0;value.unknown_c5=0;return value;
}
SessionCipher::SessionCipher(){construct_session_cipher_0423d0(*this);}
void SessionCipher::construct_0423d0_in_place(){construct_session_cipher_0423d0(*this);}
void SessionCipher::set_fixed_key_084c98(){set_session_key_042780(*this,crypto_recovered::constants::fixed_key);}
void SessionCipher::set_key_042780(const crypto_recovered::Key& key){set_session_key_042780(*this,key);}
void SessionCipher::set_key_and_iv_042760(const crypto_recovered::Key& key,const crypto_recovered::Key& iv){set_session_key_iv_042760(*this,key,iv);}
namespace {
// Local semantic strings borrow a reserved host buffer. No recovered native
// allocation/free ABI is claimed. Real request/response prefixes use N11's
// caller context directly and do not enter this local-string projection.
struct BorrowedApi:BoundedReceiveBufferApi {
    using BoundedReceiveBufferApi::BoundedReceiveBufferApi;
    std::uintptr_t allocate_04b0bc(std::uint64_t)override{throw UnsupportedInput("Local cipher reserve is insufficient");}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{throw UnsupportedInput("Local cipher storage is borrowed");}
    [[noreturn]]void length_error_00ce80()override{throw UnsupportedInput("Local cipher length leaves borrowed domain");}
    [[noreturn]]void allocation_overflow_00cd30()override{throw UnsupportedInput("Local cipher allocation leaves borrowed domain");}
    [[noreturn]]void invalid_parameter_05c974()override{throw UnsupportedInput("Local cipher pointer leaves borrowed domain");}
};
}
bool SessionCipher::transform_local(std::string& value,bool encrypt){
    if(!native_.mode_00)return false;
    const auto size=value.size();
    if(encrypt&&size>value.max_size()-16)throw std::length_error("Local cipher host reserve overflow");
    value.resize(encrypt?size+16:std::max<std::size_t>(16,size));
    ReceiveBufferString prefix;prefix.size_10=size;prefix.capacity_18=value.size();
    const auto data=reinterpret_cast<std::uintptr_t>(value.data());
    for(unsigned i=0;i<8;++i)prefix.storage_00[i]=static_cast<std::uint8_t>(data>>(i*8));
    std::array<ReceiveBufferRegion,1> regions{{{data,{reinterpret_cast<std::uint8_t*>(value.data()),value.size()+1}}}};
    ReceiveBufferMemory memory{regions};BorrowedApi api{memory};ReceiveBufferContext strings{memory,api};
    RecoveredPrefixCipherBlocks blocks{memory};PrefixCipherContext context{strings,blocks};
    const auto sync_length=[&](){if(prefix.size_10>value.size())throw UnsupportedInput("Local cipher result leaves host reserve");value.resize(static_cast<std::size_t>(prefix.size_10));};
    try{const auto result=encrypt?encrypt_prefix_042610(native_,prefix,0,context):decrypt_prefix_042480(native_,prefix,0,context);sync_length();return result;}
    catch(...){sync_length();throw;} // length projection only; never rollback payload/IV
}
bool SessionCipher::decrypt_042480(std::string& value){return transform_local(value,false);}
bool SessionCipher::encrypt_042610(std::string& value){return transform_local(value,true);}
std::unique_ptr<messages_recovered::Cipher> SessionCipherFactory::create_0423d0(){return std::make_unique<SessionCipher>();}
} // namespace anadius::socket_recovered
