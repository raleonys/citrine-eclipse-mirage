#include "crypto.hpp"
#include "constants.hpp"
#include <algorithm>
#include <bit>
#include <limits>
#include <windows.h>

namespace anadius::crypto_recovered {
namespace {
std::uint8_t multiply(std::uint8_t a,std::uint8_t b){
    std::uint8_t result=0;
    for(unsigned i=0;i<8;++i){if(b&1)result^=a;a=static_cast<std::uint8_t>((a<<1)^((a&0x80)?0x1b:0));b>>=1;}
    return result;
}
void add_key(std::span<std::uint8_t,16> s,const std::array<std::uint8_t,176>& keys,unsigned round){
    for(unsigned i=0;i<16;++i)s[i]^=keys[round*16+i];
}
void shift_rows(std::span<std::uint8_t,16> s,bool inverse){
    Key copy{};std::copy(s.begin(),s.end(),copy.begin());
    for(unsigned row=0;row<4;++row)for(unsigned col=0;col<4;++col)
        s[col*4+row]=copy[((col+(inverse?4-row:row))%4)*4+row];
}
void mix_columns(std::span<std::uint8_t,16> s,bool inverse){
    for(unsigned col=0;col<4;++col){
        Key a{};for(unsigned j=0;j<4;++j)a[j]=s[col*4+j];
        constexpr std::uint8_t forward[4]={2,3,1,1},backward[4]={14,11,13,9};
        const auto* coefficients=inverse?backward:forward;
        for(unsigned row=0;row<4;++row){s[col*4+row]=0;for(unsigned j=0;j<4;++j)
            s[col*4+row]^=multiply(a[j],coefficients[(j+4-row)%4]);}
    }
}
std::span<std::uint8_t,16> block_at(std::string& s,std::size_t pos){return std::span<std::uint8_t,16>(reinterpret_cast<std::uint8_t*>(s.data()+pos),16);}
}
Cipher::Cipher(){construct_0423d0_in_place();}
void Cipher::construct_0423d0_in_place(){mode_=0;round_keys_.fill(0);iv_.fill(0);}
void Cipher::expand_045ce0(const Key& key){
    std::copy(key.begin(),key.end(),round_keys_.begin());
    for(unsigned word=4;word<44;++word){
        std::array<std::uint8_t,4> temp{};
        std::copy_n(round_keys_.begin()+(word-1)*4,4,temp.begin());
        if(word%4==0){const auto first=temp[0];temp[0]=constants::sbox[temp[1]]^constants::rcon[word/4];
            temp[1]=constants::sbox[temp[2]];temp[2]=constants::sbox[temp[3]];temp[3]=constants::sbox[first];}
        for(unsigned j=0;j<4;++j)round_keys_[word*4+j]=round_keys_[(word-4)*4+j]^temp[j];
    }
}
void Cipher::set_fixed_key_084c98(){set_key_042780(constants::fixed_key);}
void Cipher::set_key_042780(const Key& key){expand_045ce0(key);mode_=2;}
void Cipher::set_key_and_iv_042760(const Key& key,const Key& iv){expand_045ce0(key);iv_=iv;mode_=1;}
void Cipher::encrypt_block_045670(std::span<std::uint8_t,16> s)const{
    add_key(s,round_keys_,0);
    for(unsigned round=1;round<=10;++round){for(auto& byte:s)byte=constants::sbox[byte];shift_rows(s,false);
        if(round!=10)mix_columns(s,false);add_key(s,round_keys_,round);}
}
void Cipher::decrypt_block_0458d0(std::span<std::uint8_t,16> s)const{
    add_key(s,round_keys_,10);
    for(int round=9;round>=0;--round){shift_rows(s,true);for(auto& byte:s)byte=constants::inverse_sbox[byte];
        add_key(s,round_keys_,static_cast<unsigned>(round));if(round!=0)mix_columns(s,true);}
}
bool Cipher::encrypt_042610(std::string& value){return encrypt_with_offset(value,0);}
bool Cipher::decrypt_042480(std::string& value){return decrypt_with_offset(value,0);}
bool Cipher::encrypt_with_offset(std::string& value,std::size_t offset){
    if(mode_==0)return false;
    if(offset>value.size())throw OutOfEvidenceInput("encrypt offset exceeds string");
    const auto padding=16-((value.size()-offset)&15);
    value.append(padding,static_cast<char>(padding));
    for(auto pos=offset;pos<value.size();pos+=16){auto block=block_at(value,pos);
        if(mode_==1)for(unsigned j=0;j<16;++j)block[j]^=iv_[j];
        encrypt_block_045670(block);
        if(mode_==1)std::copy(block.begin(),block.end(),iv_.begin());
    }
    return true;
}
bool Cipher::decrypt_with_offset(std::string& value,std::size_t offset){
    if(mode_==0)return false;
    if(offset>value.size())throw OutOfEvidenceInput("decrypt offset exceeds string");
    if(((value.size()-offset)&15)!=0)return false;
    if(value.empty())throw OutOfEvidenceInput("original reads before empty string");
    for(auto pos=offset;pos<value.size();pos+=16){auto block=block_at(value,pos);Key original{};
        if(mode_==1)std::copy(block.begin(),block.end(),original.begin());
        decrypt_block_0458d0(block);
        if(mode_==1){for(unsigned j=0;j<16;++j)block[j]^=iv_[j];iv_=original;}
    }
    const auto padding=static_cast<unsigned char>(value.back());
    if(padding<1||padding>16)return false;
    if(padding>value.size())throw OutOfEvidenceInput("original padding pointer before string");
    for(auto pos=value.size()-padding;pos<value.size();++pos)if(static_cast<unsigned char>(value[pos])!=padding)return false;
    value.resize(value.size()-padding);return true;
}
std::unique_ptr<messages_recovered::Cipher> CryptoFactory::create_0423d0(){return std::make_unique<Cipher>();}
std::uint32_t Lcg::next_045070()noexcept{state_=state_*0x343fdu+0x269ec3u;return (state_>>16)&0x7fff;}
std::int64_t seconds_from_filetime_05eeb8(std::uint64_t ticks){
    const auto signed_delta=std::bit_cast<std::int64_t>(ticks-0x19db1ded53e8000ull);
    const auto seconds=signed_delta/10000000;
    return seconds>0x793582affll?-1:seconds;
}
std::int64_t WindowsTimeSource::unix_seconds_05eeb8(){FILETIME time{};GetSystemTimeAsFileTime(&time);
    return seconds_from_filetime_05eeb8((std::uint64_t(time.dwHighDateTime)<<32)|time.dwLowDateTime);}
Key Random::challenge_bytes_034cf0(){
    Lcg generator(static_cast<std::uint32_t>(time_.unix_seconds_05eeb8()));Key bytes{};
    for(auto& byte:bytes)byte=static_cast<std::uint8_t>(generator.next_045070());return bytes;
}
std::string Random::challenge_hex_034cf0(){
    const auto bytes=challenge_bytes_034cf0();std::string result;result.reserve(32);
    constexpr char digits[]="0123456789abcdef";
    for(auto byte:bytes){result.push_back(digits[byte>>4]);result.push_back(digits[byte&15]);}return result;
}
Key Random::key_from_response_034c30(std::string_view response){
    if(response.empty())throw OutOfEvidenceInput("empty response has an unspecified second source byte");
    // MOVSX both bytes, SHL first by8, OR signed second, then 32-bit ADD.
    const auto first=static_cast<std::uint32_t>(static_cast<std::int32_t>(std::bit_cast<std::int8_t>(static_cast<std::uint8_t>(response[0]))));
    // The original reads a std::string: a one-byte response has a NUL at [1].
    const auto second_byte=response.size()==1?std::uint8_t{0}:static_cast<std::uint8_t>(response[1]);
    const auto second=static_cast<std::uint32_t>(static_cast<std::int32_t>(std::bit_cast<std::int8_t>(second_byte)));
    Lcg generator(7);const auto once=generator.next_045070();generator.seed_045060(((first<<8)|second)+once);
    Key result{};for(auto& byte:result)byte=static_cast<std::uint8_t>(generator.next_045070());return result;
}
}
