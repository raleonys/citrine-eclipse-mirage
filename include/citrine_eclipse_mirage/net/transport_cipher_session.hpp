// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_prefix_cipher.hpp"

namespace citrine_eclipse_mirage::socket {
class SessionCipher;
SessionCipher& construct_session_cipher_0423d0(SessionCipher&);
void set_session_key_iv_042760(SessionCipher&,const crypto::Key&,const crypto::Key&);
void set_session_key_042780(SessionCipher&,const crypto::Key&);
SessionCipher& reset_session_cipher_02bc70(SessionCipher&);
// One owned AES schedule +one live mode/IV and c4/c5 flag pair. Binding native()
// and the semantic Cipher interface to this object prevents the old key-state
// split. This is an owned C++ projection, not a native MSVC object/loader ABI.
class SessionCipher final:public messages::Cipher {
public:
    SessionCipher();
    SessionCipher(const SessionCipher&)=delete;SessionCipher& operator=(const SessionCipher&)=delete;
    SessionCipher(SessionCipher&&)=delete;SessionCipher& operator=(SessionCipher&&)=delete;
    std::uint8_t encrypted_c4=0,unknown_c5=0;
    PrefixCipherState& native() noexcept{return native_;}
    const PrefixCipherState& native()const noexcept{return native_;}
    void construct_0423d0_in_place() final;
    void set_fixed_key_084c98() final;
    void set_key_042780(const crypto::Key&) final;
    void set_key_and_iv_042760(const crypto::Key&,const crypto::Key&);
    bool decrypt_042480(std::string&) final;
    bool encrypt_042610(std::string&) final;
private:
    crypto::Cipher schedule_;PrefixCipherState native_{0,schedule_,{}};
    bool transform_local(std::string&,bool encrypt);
    friend SessionCipher& construct_session_cipher_0423d0(SessionCipher&);
    friend void set_session_key_iv_042760(SessionCipher&,const crypto::Key&,const crypto::Key&);
    friend void set_session_key_042780(SessionCipher&,const crypto::Key&);
};
// Source compatibility factory for the existing semantic ChallengeResponse
// lower body. Fresh state per temporary; not the native allocation/constructor ABI.
class SessionCipherFactory final:public messages::CryptoFactory {
public:std::unique_ptr<messages::Cipher> create_0423d0() final;
};
} // namespace citrine_eclipse_mirage::socket
