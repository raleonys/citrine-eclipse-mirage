#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/platform/crypt_wrapper.hpp"
#include <cstdint>
#include <cstring>
using namespace preloader_l;
int main(){
  uint8_t k[16]{}; char reply[96]{}; size_t n=0;
  const uint8_t ab[]={'a','b'};
  const uint8_t fixed[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  // HF15 supersedes HF14 behavioral seed policy only for exact one-byte version 3. The <=2
  // fixed-key branch remains behavioral-compatible and is retained here.
  if(codec::make_challenge_reply(ab,2,"2",1,k,reply,sizeof(reply),&n)!=codec::SessionStatus::ok||std::memcmp(k,fixed,16)!=0) return 1;
  static_assert(platform::curio_crypt_wrapper_rva==0x1264u,"wrapper");
  static_assert(platform::curio_crypt_iat_rva==0x5000u,"iat");
  static_assert(platform::curio_crypt_suffix_rva==0x52C0u,"suffix");
  static_assert(platform::curio_crypt_companion_rva==0x52D0u,"companion");
  static_assert(platform::curio_crypt_original_slot_rva==0x72D0u,"slot");
  return 0;
}
