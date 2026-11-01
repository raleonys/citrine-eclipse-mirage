// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/cipher_session_test_fixture.hpp"
using namespace session_cipher_test;
namespace {
void control(){
    for(unsigned seed=0;seed<64;++seed){s::SessionCipher value;c::Key key{},iv{};for(unsigned i=0;i<16;++i){key[i]=static_cast<std::uint8_t>(seed*7+i*19);iv[i]=static_cast<std::uint8_t>(seed+i*3);}
        check(value.native().mode_00==0&&value.native().iv_b4==c::Key{}&&std::all_of(value.native().schedule.round_keys().begin(),value.native().schedule.round_keys().end(),[](auto b){return !b;}),"owned session constructor clears mode/schedule/IV");
        value.encrypted_c4=0xa5;value.unknown_c5=0x5a;value.set_key_and_iv_042760(key,iv);c::Cipher expected;expected.set_key_and_iv_042760(key,iv);
        check(value.native().mode_00==1&&value.native().iv_b4==iv&&value.native().schedule.round_keys()==expected.round_keys()&&value.encrypted_c4==0xa5&&value.unknown_c5==0x5a,"CBC key/IV installed coherently, flags preserved");
        auto local=std::string("before\0key",10),oracle=local;check(value.encrypt_042610(local)&&expected.encrypt_042610(oracle)&&local==oracle&&value.native().iv_b4==expected.iv(),"semantic local cipher uses actual typed prefix AES and same live IV");
        const auto retained=value.native().iv_b4;std::reverse(key.begin(),key.end());value.set_key_042780(key);expected.set_key_042780(key);
        check(value.native().mode_00==2&&value.native().iv_b4==retained&&value.native().schedule.round_keys()==expected.round_keys(),"ECB key update preserves typed IV and replaces entire176-byte schedule");
        value.construct_0423d0_in_place();check(!value.native().mode_00&&value.native().iv_b4==c::Key{}&&std::all_of(value.native().schedule.round_keys().begin(),value.native().schedule.round_keys().end(),[](auto b){return !b;})&&value.encrypted_c4==0xa5&&value.unknown_c5==0x5a,"0423d0 reset clears context but preserves c4/c5 flags");
        value.set_fixed_key_084c98();check(&s::reset_session_cipher_02bc70(value)==&value&&!value.encrypted_c4&&!value.unknown_c5&&!value.native().mode_00&&value.native().iv_b4==c::Key{},"02bc70 returns same receiver and clears flags after context reconstruction");
    }
    {s::SessionCipher one,two;one.set_fixed_key_084c98();check(!two.native().mode_00&&&one.native().schedule!=&two.native().schedule,"separate owned sessions never share schedule object");auto value=cipher_test::unhex("00112233445566778899aabbccddeeff");check(one.encrypt_042610(value)&&value.substr(0,16)==cipher_test::unhex("69c4e0d86a7b0430d8cdb78070b4c55a"),"fixed key still matches retained known-answer AES block");}
}
void local_projection(){
    for(bool cbc:{false,true})for(unsigned length=0;length<80;++length){s::SessionCipher enc,dec;c::Cipher oracle;if(cbc){enc.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);dec.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);oracle.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);}else{enc.set_key_042780(cipher_test::test_key);dec.set_key_042780(cipher_test::test_key);oracle.set_key_042780(cipher_test::test_key);}
        std::string input;for(unsigned i=0;i<length;++i)input.push_back(static_cast<char>((i*31+length)&255));auto value=input,expected=input;check(enc.encrypt_042610(value)&&oracle.encrypt_042610(expected)&&value==expected,"borrowed local projection full binary ciphertext and padding oracle");check(dec.decrypt_042480(value)&&value==input&&enc.native().iv_b4==dec.native().iv_b4,"local projection commits actual size/bytes/IV on successful decrypt");
    }
    {s::SessionCipher value;std::string text="unchanged";const auto cap=text.capacity();check(!value.encrypt_042610(text)&&!value.decrypt_042480(text)&&text=="unchanged"&&text.capacity()==cap,"unkeyed semantic adapter does not touch/reserve input");value.native().mode_00=3;check(!value.encrypt_042610(text)&&text.size()==16,"unknown nonzero mode commits padding before false through local adapter");}
    {s::SessionCipher value;value.set_key_042780(cipher_test::test_key);const auto plaintext=std::string(16,'Q');auto encrypted=cipher_test::raw_encrypt(plaintext);check(!value.decrypt_042480(encrypted)&&encrypted==plaintext,"failed local padding validation retains actual plaintext mutation");std::string empty;rejects<s::UnsupportedInput>([&]{value.decrypt_042480(empty);});check(empty.empty(),"bounded empty pre-base failure synchronizes local length without native rollback/IV changes");}
    {s::SessionCipherFactory factory;auto one=factory.create_0423d0(),two=factory.create_0423d0();one->set_fixed_key_084c98();std::string a="factory",b=a;check(one->encrypt_042610(a)&&!two->encrypt_042610(b)&&b=="factory","permanent semantic factory yields independent unkeyed temporary cipher state");}
}
void shared_native(){
    for(bool cbc:{false,true}){native_message_test::Fixture f;s::SessionCipher session;if(cbc)session.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);else session.set_key_042780(cipher_test::test_key);s::PrefixCipherBlocks blocks{f.strings.memory};s::PrefixCipherContext context{f.strings,blocks};
        const auto input=std::string("native\0request",14);c::Cipher oracle;if(cbc)oracle.set_key_and_iv_042760(cipher_test::test_key,cipher_test::test_iv);else oracle.set_key_042780(cipher_test::test_key);auto expected=input;oracle.encrypt_042610(expected);
        f.seed(input);check(s::encrypt_prefix_042610(session.native(),f.core.request(),0,context)&&f.input()==expected,"native prefix consumes same session schedule as semantic control setter");
        std::string next="semantic next";auto next_expected=next;oracle.encrypt_042610(next_expected);check(session.encrypt_042610(next)&&next==next_expected&&session.native().iv_b4==(cbc?oracle.iv():c::Key{}),"native prefix then semantic local share one uninterrupted CBC/ECB state");
    }
}
}
int main(){try{control();local_projection();shared_native();std::cout<<"PASS: "<<checks<<" socket cipher session checks; actual0423d0/042760/042780/02bc70 control projections, owned schedule/mode/IV/flags, semantic local and native prefix state coherence, reset and factory isolation\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
