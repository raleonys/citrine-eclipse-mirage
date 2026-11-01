#include "crypto.hpp"
#include "constants.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace anadius::crypto_recovered;
static unsigned checks=0;
void check(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED "<<message<<'\n';std::exit(1);}}
std::string unhex(std::string_view in){std::string out;auto n=[](char c){return c<='9'?c-'0':c-'a'+10;};for(std::size_t i=0;i<in.size();i+=2)out.push_back(static_cast<char>((n(in[i])<<4)|n(in[i+1])));return out;}
Key key(std::string_view in){const auto bytes=unhex(in);Key out{};std::copy_n(reinterpret_cast<const std::uint8_t*>(bytes.data()),16,out.begin());return out;}
void raw_encrypt(Cipher& cipher,std::string& value){for(std::size_t i=0;i<value.size();i+=16)cipher.encrypt_block_045670(std::span<std::uint8_t,16>(reinterpret_cast<std::uint8_t*>(value.data()+i),16));}
struct FakeTime:TimeSource {std::int64_t seconds=0;unsigned calls=0;std::int64_t unix_seconds_05eeb8()override{++calls;return seconds;}};
// Deliberately wide arithmetic and division instead of the production uint32/shift.
std::uint32_t reference_next(std::uint64_t& s){s=(s*214013+2531011)%4294967296ull;return static_cast<std::uint32_t>((s/65536)%32768);}
Key reference_response(unsigned first,unsigned second){std::uint64_t state=7;auto initial=reference_next(state);
    const auto a=first<128?first:first|0xffffff00u,b=second<128?second:second|0xffffff00u;
    state=static_cast<std::uint32_t>(((a<<8)|b)+initial);Key out{};for(auto& byte:out)byte=static_cast<std::uint8_t>(reference_next(state));return out;}
template<class F>void outside(F f){bool caught=false;try{f();}catch(const OutOfEvidenceInput&){caught=true;}check(caught,"unsupported memory-domain input explicit");}
int main(){
    Cipher cipher;std::string unchanged="data";check(cipher.mode()==0,"constructor mode");check(!cipher.encrypt_042610(unchanged)&&unchanged=="data","unkeyed encryption no mutation");check(!cipher.decrypt_042480(unchanged)&&unchanged=="data","unkeyed decrypt no mutation");
    check(std::all_of(cipher.round_keys().begin(),cipher.round_keys().end(),[](auto b){return b==0;}),"constructor zero schedule");
    const auto nist_key=key("2b7e151628aed2a6abf7158809cf4f3c");
    const auto iv=key("000102030405060708090a0b0c0d0e0f");
    const auto plain=unhex("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e5130c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710");
    const auto ecb_expected=unhex("3ad77bb40d7a3660a89ecaf32466ef97f5d3d58503b9699de785895a96fdbaaf43b1cd7f598ece23881b00e3ed0306887b0c785e27e8ad3f8223207104725dd4");
    const auto cbc_expected=unhex("7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b273bed6b8e3c1743b7116e69e222295163ff1caa1681fac09120eca307586e1a7");
    cipher.set_key_042780(nist_key);auto raw=plain;raw_encrypt(cipher,raw);check(raw==ecb_expected,"NIST four-block ECB encryption");
    for(std::size_t pos=0;pos<raw.size();pos+=16)cipher.decrypt_block_0458d0(std::span<std::uint8_t,16>(reinterpret_cast<std::uint8_t*>(raw.data()+pos),16));check(raw==plain,"NIST ECB decrypt");
    cipher.set_key_and_iv_042760(nist_key,iv);auto cbc=plain;check(cipher.encrypt_042610(cbc),"CBC padded encrypt success");check(cbc.substr(0,64)==cbc_expected&&cbc.size()==80,"NIST four-block CBC encryption prefix");check(cipher.iv()!=iv,"CBC changes IV");
    const auto final_iv=cipher.iv();check(std::equal(final_iv.begin(),final_iv.end(),reinterpret_cast<const std::uint8_t*>(cbc.data()+64)),"CBC final IV=last ciphertext");
    Cipher dec;dec.set_key_and_iv_042760(nist_key,iv);check(dec.decrypt_042480(cbc)&&cbc==plain,"CBC padded decryption");check(dec.iv()==final_iv,"decrypt saves original ciphertext as IV");
    // Multi-call CBC must retain its chaining state, including padding blocks.
    Cipher enc_stream,dec_stream;enc_stream.set_key_and_iv_042760(nist_key,iv);dec_stream.set_key_and_iv_042760(nist_key,iv);
    for(auto text:{std::string("first"),std::string(16,'z'),std::string("third\0message",13)}){auto value=text;check(enc_stream.encrypt_042610(value),"stream encrypt");check(dec_stream.decrypt_042480(value)&&value==text,"stream decrypt");check(enc_stream.iv()==dec_stream.iv(),"stream matching chain state");}
    // ECB setter replaces keys/mode but does not overwrite the previous IV.
    auto retained=cipher.iv();cipher.set_key_042780(nist_key);check(cipher.mode()==2&&cipher.iv()==retained,"ECB setter preserves IV");
    cipher.set_fixed_key_084c98();raw=unhex("00112233445566778899aabbccddeeff");raw_encrypt(cipher,raw);check(raw==unhex("69c4e0d86a7b0430d8cdb78070b4c55a"),"incrementing hardcoded key known answer");
    for(unsigned length=0;length<=256;++length)for(unsigned prefix:{0u,1u,65u}){std::string value(prefix,'P');for(unsigned j=0;j<length;++j)value.push_back(static_cast<char>(j));auto original=value;
        check(cipher.encrypt_with_offset(value,prefix),"all block/padding offsets encrypt");check(value.substr(0,prefix)==original.substr(0,prefix),"prefix intact");check((value.size()-prefix)%16==0&&value.size()>original.size(),"padding includes full block");check(cipher.decrypt_with_offset(value,prefix)&&value==original,"all padded lengths roundtrip");}
    for(unsigned pad=0;pad<=255;++pad){if(pad==1)continue;std::string plaintext(16,'Q');plaintext.back()=static_cast<char>(pad);auto value=plaintext;raw_encrypt(cipher,value);check(!cipher.decrypt_042480(value),"malformed pad rejected");check(value==plaintext,"failed padding leaves plaintext mutation");}
    for(unsigned pad=1;pad<=16;++pad){std::string plaintext(16,'Q');std::fill(plaintext.end()-pad,plaintext.end(),static_cast<char>(pad));auto encrypted=plaintext;raw_encrypt(cipher,encrypted);check(cipher.decrypt_042480(encrypted)&&encrypted==plaintext.substr(0,16-pad),"every valid padding value");
        for(unsigned j=0;j+1<pad;++j){auto malformed=plaintext;malformed[16-pad+j]^=1;auto value=malformed;raw_encrypt(cipher,value);check(!cipher.decrypt_042480(value)&&value==malformed,"each wrong padding location");}}
    for(unsigned n=1;n<48;++n)if(n%16){std::string value(n,'x');auto copy=value;check(!cipher.decrypt_042480(value)&&value==copy,"unaligned decrypt no mutation");}
    std::string empty;outside([&]{cipher.decrypt_042480(empty);});outside([&]{cipher.encrypt_with_offset(empty,1);});outside([&]{cipher.decrypt_with_offset(empty,1);});
    // Source only checks full string padding, even when offset selects no blocks.
    std::string unusual="prefix";unusual.push_back(1);check(cipher.decrypt_with_offset(unusual,unusual.size())&&unusual=="prefix","zero encrypted extent retains original padding behavior");
    Cipher cbc_bad;cbc_bad.set_key_and_iv_042760(nist_key,iv);auto invalid=cbc_expected;check(!cbc_bad.decrypt_042480(invalid),"NIST plaintext tail is invalid padding");check(invalid==plain&&cbc_bad.iv()==key("3ff1caa1681fac09120eca307586e1a7"),"failed CBC pad mutates plaintext and IV");
    CryptoFactory factory;auto factory_cipher=factory.create_0423d0();factory_cipher->set_fixed_key_084c98();std::string factory_value="factory";check(factory_cipher->encrypt_042610(factory_value)&&factory_cipher->decrypt_042480(factory_value)&&factory_value=="factory","concrete interface factory");
    FakeTime time;Random random(time);
    for(auto seed:{0ull,1ull,7ull,0x7fffffffull,0xffffffffull,0x100000001ull}){time.seconds=static_cast<std::int64_t>(seed);const auto before=time.calls;auto generated=random.challenge_bytes_034cf0();check(time.calls==before+1,"time read once");std::uint64_t reference=static_cast<std::uint32_t>(seed);Key expected{};for(auto& b:expected)b=static_cast<std::uint8_t>(reference_next(reference));check(generated==expected,"challenge LCG seed width");}
    time.seconds=1;check(random.challenge_hex_034cf0()=="2923be84e16cd6ae529049f1f1bbe9eb","known time1 challenge");
    // Exhaustive first-two-byte signedness and reseeding; tails must not matter.
    for(unsigned first=0;first<256;++first)for(unsigned second=0;second<256;++second){std::string response;response.push_back(static_cast<char>(first));response.push_back(static_cast<char>(second));const auto expected=reference_response(first,second);check(random.key_from_response_034c30(response)==expected,"all first2 signed-byte pairs");response+="ignored-tail";check(random.key_from_response_034c30(response)==expected,"response tail ignored");}
    for(unsigned first=0;first<256;++first){const std::string one_byte(1,static_cast<char>(first));check(random.key_from_response_034c30(one_byte)==reference_response(first,0),"one-byte std::string reads its trailing NUL");}
    outside([&]{random.key_from_response_034c30("");});
    const std::uint64_t epoch=0x19db1ded53e8000ull;check(seconds_from_filetime_05eeb8(epoch)==0,"FILETIME epoch");check(seconds_from_filetime_05eeb8(epoch+19999999)==1,"subsecond truncation");check(seconds_from_filetime_05eeb8(epoch-9999999)==0,"negative truncation toward zero");check(seconds_from_filetime_05eeb8(epoch-10000000)==-1,"negative second");check(seconds_from_filetime_05eeb8(epoch+0x793582affull*10000000)==0x793582affll,"time64 upper bound accepted");check(seconds_from_filetime_05eeb8(epoch+0x793582b00ull*10000000)==-1,"time64 upper bound rejected");
    cipher.construct_0423d0_in_place();check(cipher.mode()==0&&cipher.iv()==Key{}&&std::all_of(cipher.round_keys().begin(),cipher.round_keys().end(),[](auto b){return b==0;}),"reset clears mode schedule IV");
    for(unsigned i=0;i<256;++i)check(constants::inverse_sbox[constants::sbox[i]]==i,"raw tables are inverses");
    std::cout<<"PASS: "<<checks<<" deterministic crypto checks (NIST vectors, padding, mutation, state, PRNG/time); target DLL/EXE not executed\n";
}
