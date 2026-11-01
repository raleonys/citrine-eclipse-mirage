#include "greenfield/protocol/owned_ingress.hpp"
#include "greenfield/core/resource_owners.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

using preloader_l::net::IngressStatus;
namespace {
struct CleanupProbe { greenfield::protocol::OwnedIngress* frame{}; unsigned calls{}; };
bool fail_once_close(void* context, std::uintptr_t token) noexcept {
    auto& probe=*static_cast<CleanupProbe*>(context);
    if (token == 0 || !probe.frame) return false;
    if (++probe.calls == 1) return false;
    probe.frame->shutdown();
    return true;
}
}
int main() {
    constexpr char first[]="<LSX><Request id=\"1\"/></LSX>";
    constexpr char second[]="<LSX><Event/></LSX>";
    greenfield::protocol::OwnedIngress owner;
    constexpr auto first_len=sizeof(first)-1;
    constexpr auto second_len=sizeof(second)-1;
    const auto* fb=reinterpret_cast<const std::uint8_t*>(first);
    assert(owner.feed(fb,5)==IngressStatus::need_more);
    assert(owner.feed(fb+5,first_len-5)==IngressStatus::frame_ready);
    std::size_t needed=0;
    char out[256]{};
    assert(owner.pop(out,3,&needed)==IngressStatus::capacity && needed==first_len);
    assert(owner.pending_bytes()==first_len); // capacity rejection cannot consume
    assert(owner.pop(out,sizeof(out),&needed)==IngressStatus::ok);
    assert(needed==first_len && std::memcmp(out,first,needed)==0);
    assert(owner.pending_bytes()==0);
    assert(owner.pop(out,sizeof(out),&needed)==IngressStatus::need_more);
    // coalesced frames: copy/consume first; second remains queued and unmodified
    std::array<std::uint8_t,256> joined{};
    std::memcpy(joined.data(),first,first_len);
    std::memcpy(joined.data()+first_len,second,second_len);
    assert(owner.feed(joined.data(),first_len+second_len)==IngressStatus::frame_ready);
    assert(owner.pop(out,sizeof(out),&needed)==IngressStatus::ok && needed==first_len);
    assert(std::memcmp(out,first,needed)==0);
    std::memset(out,0,sizeof(out));
    assert(owner.pop(out,sizeof(out),&needed)==IngressStatus::ok && needed==second_len);
    assert(std::memcmp(out,second,needed)==0);
    assert(owner.pending_bytes()==0);
    // ResourceOwners fail-once rollback: never drop cleanup ticket on failure.
    greenfield::core::ResourceOwners resources;
    CleanupProbe probe{&owner,0};
    assert(resources.adopt(0x202u,fail_once_close,&probe));
    assert(resources.drain()==greenfield::core::ResourceOwners::Drain::retry_required);
    assert(owner.active() && resources.remaining()==1 && probe.calls==1);
    assert(resources.drain()==greenfield::core::ResourceOwners::Drain::complete);
    assert(!owner.active() && resources.remaining()==0 && probe.calls==2);
    assert(owner.feed(fb,first_len)==IngressStatus::invalid_argument);
    assert(owner.pop(out,sizeof(out),&needed)==IngressStatus::invalid_argument);
    // Actual adopted codec round-trip through its owned output buffers.
    std::array<std::uint8_t,16> key{};
    assert(preloader_l::codec::derive_session_key(0,key.data())==preloader_l::codec::SessionStatus::ok);
    constexpr char payload[]="<LSX><Response/></LSX>";
    char encoded[256]{};std::size_t wire=0;
    assert(preloader_l::codec::encode_response(key.data(),key.size(),reinterpret_cast<const std::uint8_t*>(payload),sizeof(payload)-1,encoded,sizeof(encoded),&wire)==preloader_l::codec::MessageCodecStatus::ok);
    assert(wire>sizeof(payload)-1 && encoded[wire-1]=='\0');
    std::uint8_t plain[256]{};std::size_t plain_len=0;
    assert(preloader_l::codec::decode_payload(key.data(),key.size(),encoded,wire,plain,sizeof(plain),&plain_len)==preloader_l::codec::MessageCodecStatus::ok);
    assert(plain_len>=sizeof(payload)-1 && std::memcmp(plain,payload,sizeof(payload)-1)==0);
    // Invalid ciphertext is rejected, not passed through into a frame.
    assert(preloader_l::codec::decode_payload(key.data(),key.size(),"GG",2,plain,sizeof(plain),&plain_len)==preloader_l::codec::MessageCodecStatus::invalid_hex);
    std::cout << "GF02 source-linked owner/frame/codec tests PASS\n";
}
