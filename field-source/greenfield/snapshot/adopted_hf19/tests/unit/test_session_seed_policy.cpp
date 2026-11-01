#include "preloader_l/codec/session_crypto.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
using namespace preloader_l::codec;

int main() {
    uint8_t key[16]{};
    char reply[128]{};
    size_t written=0;
    const uint8_t challenge[]={'a'};
    const uint8_t expected_consumer_v3[16]={0x4e,0x54,0x1f,0x16,0x5f,0x6a,0x47,0xa9,0xc5,0x18,0xd6,0x66,0x8a,0x14,0x45,0x37};
    const uint8_t expected_hf14_incoming_seed[16]={0xb0,0xb7,0x4a,0xc9,0x14,0x8c,0x10,0xef,0x7b,0x0a,0xb9,0xca,0xaf,0x68,0xf4,0x58};

    // Proven CollegeFB27/Madden27 live path: exact wire version="3".
    if(make_challenge_reply(challenge,1,"3",1,key,reply,sizeof(reply),&written)!=SessionStatus::ok) return 1;
    if(written!=32 || std::memcmp(reply,"fe123486508b0eb503a8468388ef5812",32)!=0) return 2;
    if(std::memcmp(key,expected_consumer_v3,16)!=0) return 3;
    if(std::memcmp(key,expected_hf14_incoming_seed,16)==0) return 4;

    // Remediation boundary: missing version retains HF14 behavioral policy.
    if(make_challenge_reply(challenge,1,nullptr,0,key,reply,sizeof(reply),&written)!=SessionStatus::ok ||
       std::memcmp(key,expected_hf14_incoming_seed,16)!=0) return 5;

    // Non-exact textual forms that atoi() interprets as 3 also retain HF14 policy.
    if(make_challenge_reply(challenge,1,"  +3junk",8,key,reply,sizeof(reply),&written)!=SessionStatus::ok ||
       std::memcmp(key,expected_hf14_incoming_seed,16)!=0) return 6;

    if(make_challenge_reply(challenge,1,"4",1,key,reply,sizeof(reply),&written)!=SessionStatus::ok ||
       std::memcmp(key,expected_hf14_incoming_seed,16)!=0) return 7;

    const uint8_t fixed[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    if(make_challenge_reply(challenge,1,"2",1,key,reply,sizeof(reply),&written)!=SessionStatus::ok ||
       std::memcmp(key,fixed,16)!=0) return 8;
    return 0;
}
