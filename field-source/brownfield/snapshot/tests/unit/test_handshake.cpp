#include "preloader_l/protocol/challenge_handshake.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cstring>

int main(){
 using namespace preloader_l;
 config::clear_runtime_configuration();
 constexpr char cfg[]=R"CFG("Config2" { "Emulator" { "SDK" "13.764.0.6277" } })CFG";
 if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 1;
 char greeting[512]{}; size_t greeting_n=0;
 if(protocol::build_challenge_greeting(UINT32_C(1700000000),greeting,sizeof(greeting),&greeting_n)!=protocol::GreetingStatus::ok) return 2;
 if(!std::strstr(greeting,"<Challenge key=\"2e0387643d564d117b0c192518e9ad28\"")) return 3;
 if(!std::strstr(greeting,"build=\"release\" version=\"13,764,0,6277\"")) return 4;
 if(greeting_n!=std::strlen(greeting)) return 5;
 config::clear_runtime_configuration();
 constexpr char special_sdk[]=R"CFG("Config2" { "Emulator" { "SDK" "1&2<3>" } })CFG";
 if(config::set_runtime_configuration_cfg(special_sdk,sizeof(special_sdk)-1)!=config::RuntimeConfigurationStatus::ok) return 20;
 std::memset(greeting,0,sizeof(greeting)); greeting_n=0;
 if(protocol::build_challenge_greeting(UINT32_C(1700000000),greeting,sizeof(greeting),&greeting_n)!=protocol::GreetingStatus::ok) return 21;
 if(!std::strstr(greeting,"version=\"1&amp;2&lt;3&gt;\"")) return 22;

 config::clear_runtime_configuration();
 std::memset(greeting,0,sizeof(greeting)); greeting_n=0;
 if(protocol::build_challenge_greeting(UINT32_C(1700000000),greeting,sizeof(greeting),&greeting_n)!=protocol::GreetingStatus::ok) return 14;
 if(!std::strstr(greeting,"build=\"release\" version=\"\"")) return 15;
 constexpr char empty_sdk[]=R"CFG("Config2" { "Emulator" { "SDK" "" } })CFG";
 if(config::set_runtime_configuration_cfg(empty_sdk,sizeof(empty_sdk)-1)!=config::RuntimeConfigurationStatus::ok) return 17;
 std::memset(greeting,0,sizeof(greeting)); greeting_n=0;
 if(protocol::build_challenge_greeting(UINT32_C(1700000000),greeting,sizeof(greeting),&greeting_n)!=protocol::GreetingStatus::ok) return 18;
 if(!std::strstr(greeting,"build=\"release\" version=\"\"")) return 19;

 char out[2048]{}; protocol::HandshakeResult r{};
 const uint8_t response_seed_a[16]={0x4e,0x54,0x1f,0x16,0x5f,0x6a,0x47,0xa9,0xc5,0x18,0xd6,0x66,0x8a,0x14,0x45,0x37};
 const uint8_t hf14_dynamic_a[16]={0xb0,0xb7,0x4a,0xc9,0x14,0x8c,0x10,0xef,0x7b,0x0a,0xb9,0xca,0xaf,0x68,0xf4,0x58};
 const uint8_t fixed[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
 const uint8_t response_seed_empty[16]={0xbe,0xb7,0xdf,0x18,0x8a,0x4e,0x66,0xe5,0x49,0xe8,0x59,0x0a,0x18,0xf8,0xee,0x3d};
 auto run=[&](const char* xml,protocol::HandshakeStatus expect,const uint8_t* key)->bool{
   std::memset(out,0,sizeof(out)); r={};
   const auto st=protocol::process_challenge_request(xml,std::strlen(xml),out,sizeof(out),&r);
   if(st!=expect)return false;
   if(st==protocol::HandshakeStatus::ok && key && std::memcmp(r.session_key,key,16)!=0)return false;
   return true;
 };
 if(!run("<LSX><Request id=\"7\"><ChallengeResponse key=\"a\" version=\"3\"/></Request></LSX>",protocol::HandshakeStatus::ok,response_seed_a)) return 6;
 if(!std::strstr(out,"<ChallengeAccepted response=\"fe123486508b0eb503a8468388ef5812\"/>")) return 7;
 if(!run("<LSX><Request id=\"7b\"><ChallengeResponse response=\"ignored\" key=\"a\"/></Request></LSX>",protocol::HandshakeStatus::ok,hf14_dynamic_a)) return 8;
 if(!run("<LSX><Request id=\"7c\"><ChallengeResponse key=\"a\" version=\"2\"/></Request></LSX>",protocol::HandshakeStatus::ok,fixed)) return 9;
 if(!run("<LSX><Request id=\"7d\"><ChallengeResponse key=\"a\" version=\"garbage\"/></Request></LSX>",protocol::HandshakeStatus::ok,fixed)) return 10;
 if(!run("<LSX><Request id=\"7e\"><ChallengeResponse key=\"\" version=\"3\"/></Request></LSX>",protocol::HandshakeStatus::ok,response_seed_empty)) return 11;
 if(!std::strstr(out,"response=\"954f64f2e4e86e9eee82d20216684899\"")) return 12;
 if(!run("<LSX><Request id=\"7f\"><ChallengeResponse version=\"3\"/></Request></LSX>",protocol::HandshakeStatus::missing_key,nullptr)) return 13;
 if(!run("<LSX><Request id=\"7g\"><GetConfig/></Request></LSX>",protocol::HandshakeStatus::wrong_command,nullptr)) return 23;
 const char bad_entity[]="<LSX><Request id=\"7h\"><ChallengeResponse key=\"a&bogus;\" version=\"3\"/></Request></LSX>";
 // Exact pinned upstream TinyXML2 preserves unknown named references as text.
 // A02-B remains OPEN in THIRD_PARTY_KNOWN_VULNERABILITIES: this is NOT strict XML 1.0 validation.
 // Check the observed compatibility-bound behavior against a valid escaped equivalent.
 if(protocol::process_challenge_request(bad_entity,sizeof(bad_entity)-1,out,sizeof(out),&r)!=protocol::HandshakeStatus::ok) return 24;
 const auto unknown_key_result=r;
 const char escaped_equivalent[]="<LSX><Request id=\"7h\"><ChallengeResponse key=\"a&amp;bogus;\" version=\"3\"/></Request></LSX>";
 if(protocol::process_challenge_request(escaped_equivalent,sizeof(escaped_equivalent)-1,out,sizeof(out),&r)!=protocol::HandshakeStatus::ok) return 30;
 if(std::memcmp(unknown_key_result.session_key,r.session_key,16)!=0) return 31;
 // F319-P111-001: malformed raw key bytes must not report success or leave a
 // derived session key in the result.
 const char bad_raw_key[]="<LSX><Request id=\"7i\"><ChallengeResponse key=\"a\x01" "b\" version=\"3\"/></Request></LSX>";
 std::memset(&r,0xA5,sizeof(r));
 if(protocol::process_challenge_request(bad_raw_key,sizeof(bad_raw_key)-1,out,sizeof(out),&r)!=protocol::HandshakeStatus::malformed_request) return 26;
 for(unsigned i=0;i<16u;++i) if(r.session_key[i]!=0u) return 27;
 const char bad_utf8_key[] = {'<','L','S','X','>','<','R','e','q','u','e','s','t',' ','i','d','=','\"','7','j','\"','>','<','C','h','a','l','l','e','n','g','e','R','e','s','p','o','n','s','e',' ','k','e','y','=','\"','a',static_cast<char>(0xC0),static_cast<char>(0xAF),'b','\"',' ','v','e','r','s','i','o','n','=','\"','3','\"','/','>','<','/','R','e','q','u','e','s','t','>','<','/','L','S','X','>'};
 std::memset(&r,0xA5,sizeof(r));
 if(protocol::process_challenge_request(bad_utf8_key,sizeof(bad_utf8_key),out,sizeof(out),&r)!=protocol::HandshakeStatus::malformed_request) return 28;
 for(unsigned i=0;i<16u;++i) if(r.session_key[i]!=0u) return 29;
 const char no_id[]="<LSX><Request><ChallengeResponse key=\"a\" version=\"3\"/></Request></LSX>";
 const auto no_id_st=protocol::process_challenge_request(no_id,sizeof(no_id)-1,out,sizeof(out),&r);
 if(no_id_st!=protocol::HandshakeStatus::malformed_request && no_id_st!=protocol::HandshakeStatus::missing_id) return 25;
 config::clear_runtime_configuration();
 return 0;
}
