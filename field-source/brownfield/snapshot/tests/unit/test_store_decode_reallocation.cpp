#include "../../src/config/store_internal.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>

int main() {
    using namespace preloader_l::config::store_entitlement_internal;

    StoreState b64{};
    static const char encoded[]="QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=";
    const size_t encoded_len=sizeof(encoded)-1u;
    b64.arena=static_cast<uint8_t*>(transient_alloc(encoded_len));
    assert(b64.arena);
    b64.arena_capacity=encoded_len;
    b64.arena_used=encoded_len;
    std::memcpy(b64.arena,encoded,encoded_len);
    Span decoded{};
    assert(decode_quartets(b64,Span{0u,encoded_len,true},&decoded)==ParseCode::ok);
    static const char expected[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    assert(decoded.len==sizeof(expected)-1u);
    assert(std::memcmp(b64.arena+decoded.off,expected,decoded.len)==0);
    assert(b64.arena_capacity>encoded_len); // forced growth occurred before borrowing input
    reset_store(b64);

    StoreState hex{};
    static const char hex_text[]="000102030405060708090A0B0C0D0E0F";
    const size_t hex_len=sizeof(hex_text)-1u;
    hex.arena=static_cast<uint8_t*>(transient_alloc(hex_len));
    assert(hex.arena);
    hex.arena_capacity=hex_len;
    hex.arena_used=hex_len;
    std::memcpy(hex.arena,hex_text,hex_len);
    Span bytes{};
    assert(decode_hex(hex,Span{0u,hex_len,true},&bytes)==ParseCode::ok);
    assert(bytes.len==16u);
    for(size_t i=0;i<16u;++i) assert(hex.arena[bytes.off+i]==static_cast<uint8_t>(i));
    assert(hex.arena_capacity>hex_len);
    reset_store(hex);
    return 0;
}
