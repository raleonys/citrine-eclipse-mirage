#pragma once
#include "preloader_l/net/transport_native_message.hpp"

namespace preloader_l::socket_recovered {
// Both helpers always allocate. Extra controls new size/capacity independently
// of the single012350 character or007e30 count. No implicit fill of the gap.
ReceiveBufferString& grow_append_character_012350(ReceiveBufferString&,std::uint64_t extra,
    std::uint64_t unused_hint,std::uint8_t character,ReceiveBufferContext&);
ReceiveBufferString& grow_append_bytes_007e30(ReceiveBufferString&,std::uint64_t extra,
    std::uint64_t unused_hint,std::uintptr_t source,std::uint64_t count,ReceiveBufferContext&);
class RecoveredMessageAppendApi:public NativeMessageApi {
public:
    explicit RecoveredMessageAppendApi(ReceiveBufferContext& strings):strings_(strings){}
    void grow_nul_012350(ReceiveBufferString&,std::uint64_t,std::uint64_t,std::uint8_t) final;
    void grow_append_007e30(ReceiveBufferString&,std::uint64_t,std::uint64_t,std::uintptr_t,std::uint64_t) final;
private:ReceiveBufferContext& strings_;
};
} // namespace preloader_l::socket_recovered
