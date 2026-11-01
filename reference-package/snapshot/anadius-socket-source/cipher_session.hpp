#pragma once
#include "prefix_cipher.hpp"

namespace anadius::socket_recovered {
class SessionCipher;
SessionCipher& construct_session_cipher_0423d0(SessionCipher&);
void set_session_key_iv_042760(SessionCipher&,const crypto_recovered::Key&,const crypto_recovered::Key&);
void set_session_key_042780(SessionCipher&,const crypto_recovered::Key&);
SessionCipher& reset_session_cipher_02bc70(SessionCipher&);
// One owned AES schedule +one live mode/IV and c4/c5 flag pair. Binding native()
// and the semantic Cipher interface to this object prevents the old key-state
// split. This is an owned C++ projection, not a native MSVC object/loader ABI.
class SessionCipher final:public messages_recovered::Cipher {
public:
    SessionCipher();
    SessionCipher(const SessionCipher&)=delete;SessionCipher& operator=(const SessionCipher&)=delete;
    SessionCipher(SessionCipher&&)=delete;SessionCipher& operator=(SessionCipher&&)=delete;
    std::uint8_t encrypted_c4=0,unknown_c5=0;
    PrefixCipherState& native() noexcept{return native_;}
    const PrefixCipherState& native()const noexcept{return native_;}
    void construct_0423d0_in_place() final;
    void set_fixed_key_084c98() final;
    void set_key_042780(const crypto_recovered::Key&) final;
    void set_key_and_iv_042760(const crypto_recovered::Key&,const crypto_recovered::Key&);
    bool decrypt_042480(std::string&) final;
    bool encrypt_042610(std::string&) final;
private:
    crypto_recovered::Cipher schedule_;PrefixCipherState native_{0,schedule_,{}};
    bool transform_local(std::string&,bool encrypt);
    friend SessionCipher& construct_session_cipher_0423d0(SessionCipher&);
    friend void set_session_key_iv_042760(SessionCipher&,const crypto_recovered::Key&,const crypto_recovered::Key&);
    friend void set_session_key_042780(SessionCipher&,const crypto_recovered::Key&);
    friend void copy_session_cipher_defined_fields(SessionCipher&,const SessionCipher&);
};
// Source compatibility factory for the existing semantic ChallengeResponse
// lower body. Fresh state per temporary; not the native allocation/constructor ABI.
class SessionCipherFactory final:public messages_recovered::CryptoFactory {
public:std::unique_ptr<messages_recovered::Cipher> create_0423d0() final;
};
} // namespace anadius::socket_recovered
