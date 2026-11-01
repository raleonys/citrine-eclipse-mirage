#pragma once
#include "cipher_session.hpp"
#include "prefix_cipher_test_fixture.hpp"
#include "../anadius-crypto-source/constants.hpp"

namespace session_cipher_test {
namespace s=anadius::socket_recovered;namespace c=anadius::crypto_recovered;namespace m=anadius::messages_recovered;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Time:c::TimeSource{std::int64_t unix_seconds_05eeb8()override{throw std::runtime_error("time must not be queried by response-key derivation");}};
struct Effects:core_test::Effects{int atoi_05ee08(std::string_view value)override{return std::stoi(std::string(value));}};
struct Api:cipher_test::Api {
    native_message_test::Api& source;s::SessionCipher& session;s::SessionCipherFactory factory;Time time;c::Random random{time};Effects effects;
    Api(s::PrefixCipherContext& context,std::span<s::PrefixCipherBinding> bindings,native_message_test::Api& source,s::SessionCipher& session):cipher_test::Api(context,bindings,source),source(source),session(session){}
    s::ReceiveBufferString& dispatch_032d00(s::ReceiveBufferString& out,std::uintptr_t id,s::NativeMessageDocument& d)override{
        check(id==0x120f0&&&d==&source.locals.document&&&out==&source.locals.temporary,"session dispatch uses native temp/handler/document");++source.dispatches;source.event("dispatch");
        const auto result=m::dispatch_032d00(*source.document,session,source.core.xml,factory,random,effects);source.assign(out,result);return out;
    }
};
inline std::string challenge_xml(std::string_view key,std::string_view version){return "<LSX><Request id=\"bridge\" recipient=\"fixture\"><ChallengeResponse key=\""+std::string(key)+"\""+(version.empty()?std::string{}:" version=\""+std::string(version)+"\"")+"/></Request></LSX>";}
inline std::string challenge_answer(std::string_view response){return "<LSX>\n    <Response id=\"bridge\" sender=\"fixture\">\n        <ChallengeAccepted response=\""+std::string(response)+"\"/>\n    </Response>\n</LSX>\n";}
inline std::string response_hex(std::string key){c::Cipher fixed;fixed.set_fixed_key_084c98();check(fixed.encrypt_042610(key),"independent challenge reference fixed encryption");return core_test::hex(key);}
inline c::Key selected_key(std::string_view response,std::string_view version){if(!version.empty()&&std::stoi(std::string(version))<=2)return anadius::crypto_recovered::constants::fixed_key;Time time;c::Random random{time};return random.key_from_response_034c30(response);}
} // namespace session_cipher_test
