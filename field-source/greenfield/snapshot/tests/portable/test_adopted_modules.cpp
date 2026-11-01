#include "preloader_l/codec/hex_codec.hpp"
#include "preloader_l/hash/fnv_hash.hpp"
#include "preloader_l/core/byte_semantics.hpp"
#include "preloader_l/core/checked_math.hpp"
#include "preloader_l/net/port_mapping.hpp"
#include "preloader_l/net/winsock_version_policy.hpp"
#include "runtime/asio_session_timeout_policy.hpp"
#include "config/runtime_configuration_command_line.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

namespace {
using preloader_l::codec::HexStatus;
uint64_t ref_fnv64(const uint8_t* p, std::size_t n) {
  uint64_t h=preloader_l::hash::fnv1a64_offset_basis;
  for(std::size_t i=0;i<n;++i) { const int64_t x=static_cast<int64_t>(p[i])-((p[i]>=128)?256:0); h=(h^static_cast<uint64_t>(x))*preloader_l::hash::fnv1a64_prime; }
  return h;
}
uint32_t ref_fnv32(const uint8_t* p, std::size_t n, uint32_t h) {
  for(std::size_t i=0;i<n;++i) { const int32_t x=static_cast<int32_t>(p[i])-((p[i]>=128)?256:0); h=(h^static_cast<uint32_t>(x))*preloader_l::hash::fnv1a32_prime; }
  return h;
}
struct Probe { int attempts{}; int success_at{}; uint16_t captured[16]{}; };
bool port_probe(uint16_t port,void* raw) noexcept {
  auto& p=*static_cast<Probe*>(raw);
  p.captured[p.attempts++]=port;
  return p.success_at==p.attempts;
}
}
int main() {
  using namespace preloader_l;
  std::size_t result=999;
  assert(core::checked_add_size(4,7,result) && result==11);
  assert(!core::checked_add_size(std::numeric_limits<std::size_t>::max(),1,result) && result==0);
  assert(core::checked_mul_size(6,7,result) && result==42);
  assert(!core::checked_mul_size(std::numeric_limits<std::size_t>::max(),2,result) && result==0);
  assert(core::checked_mul_size(0,std::numeric_limits<std::size_t>::max(),result) && result==0);
  assert(core::sign_extend_byte_u64(0xff)==UINT64_MAX);
  assert(core::sign_extend_byte_u32(0x80)==UINT32_C(0xffffff80));
  for (int v=0;v<256;++v) {
    const uint8_t byte=static_cast<uint8_t>(v);
    const uint8_t input[1]={byte};
    char hex[2]={'X','X'};
    assert(codec::hex_encode_lower(input,1,hex,2,&result)==HexStatus::ok && result==2);
    assert(codec::hex_encode_lower(input,1,hex,1,&result)==HexStatus::capacity && result==2);
    uint8_t decoded=0;
    assert(codec::hex_decode_strict(hex,2,&decoded,1,&result)==HexStatus::ok && decoded==byte && result==1);
    assert(hash::fnv1a64_signed_bytes(input,1)==ref_fnv64(input,1));
    assert(hash::fnv1a32_signed_bytes(input,1,12345u)==ref_fnv32(input,1,12345u));
  }
  const char invalid[]={'0','0','G','G'};
  uint8_t unchanged[2]={0xab,0xcd};
  assert(codec::hex_decode_strict(invalid,4,unchanged,2,&result)==HexStatus::invalid_digit);
  assert(unchanged[0]==0xab && unchanged[1]==0xcd);
  assert(codec::hex_decode_strict("0",1,unchanged,2,&result)==HexStatus::invalid_length);
  assert(codec::hex_encode_lower(nullptr,1,nullptr,0,&result)==HexStatus::invalid_argument);
  assert(codec::hex_decode_strict(nullptr,2,nullptr,0,&result)==HexStatus::invalid_argument);
  assert(codec::hex_encode_lower(nullptr,0,nullptr,0,&result)==HexStatus::ok && result==0);
  assert(codec::hex_decode_strict(nullptr,0,nullptr,0,&result)==HexStatus::ok && result==0);
  const uint8_t sample[]={0x00,0x80,0xff,0x41};
  assert(hash::fnv1a64_signed_bytes(sample,4)==ref_fnv64(sample,4));
  assert(hash::fnv1a32_signed_bytes(sample,4,0u)==ref_fnv32(sample,4,0u));
  assert(net::select_port(nullptr,nullptr)==0);
  Probe one;one.success_at=1;
  assert(net::select_port(port_probe,&one)==3226 && one.attempts==1);
  Probe last;last.success_at=11;
  assert(net::select_port(port_probe,&last)==3236 && last.attempts==11);
  Probe none;
  assert(net::select_port(port_probe,&none)==3227 && none.attempts==11);
  for(int i=0;i<11;++i) assert(none.captured[i]==3226+(i==0?0:i));
  assert(net::map_requested_port(3216,4011)==4011);
  assert(net::map_requested_port(5001,4011)==5001);
  assert(net::winsock_negotiated_version_supported(0x0202));
  assert(!net::winsock_negotiated_version_supported(0x0101));
  using Timeout=runtime::AsioSessionTimeoutPolicy;
  const auto t0=Timeout::Time{};
  Timeout time;
  Timeout::Time next;
  assert(!time.next(&next) && time.expired(t0)==Timeout::Expired::none);
  time.begin_receive(t0,false);
  assert(time.next(&next) && next==t0+std::chrono::seconds(30));
  time.receive_progress(t0+std::chrono::seconds(5),true);
  assert(time.next(&next) && next==t0+std::chrono::seconds(15));
  time.receive_progress(t0+std::chrono::seconds(14),true);
  assert(time.next(&next) && next==t0+std::chrono::seconds(15));
  assert(time.expired(t0+std::chrono::seconds(15))==Timeout::Expired::receive);
  time.clear();
  time.begin_send(t0);
  assert(time.next(&next) && next==t0+std::chrono::seconds(5));
  time.send_progress(t0+std::chrono::seconds(4));
  assert(time.expired(t0+std::chrono::seconds(8))==Timeout::Expired::none);
  assert(time.expired(t0+std::chrono::seconds(9))==Timeout::Expired::send);
  using namespace config::runtime_command_line_detail;
  const char* cursor=" -emuGameLang \"en_US\" -showLSX ";
  CommandToken token{};
  assert(next_command_token(&cursor,&token)==CommandTokenStatus::token && token_equals(token,"-emuGameLang"));
  assert(next_command_token(&cursor,&token)==CommandTokenStatus::token && token_equals(token,"en_US"));
  assert(next_command_token(&cursor,&token)==CommandTokenStatus::token && token_equals(token,"-showLSX"));
  assert(next_command_token(&cursor,&token)==CommandTokenStatus::end);
  const char* malformed="\"unterminated";
  assert(next_command_token(&malformed,&token)==CommandTokenStatus::malformed);
  std::cout<<"GF01_DIRECT_HF19_ADOPTION_TEST_PASS\n";
}
