#pragma once
#include "preloader_l/net/transport_request_mutation.hpp"

namespace preloader_l::socket_recovered {
ReceiveBufferString& grow_replace_request_02b940(ReceiveBufferString&,std::uint64_t extra,
    std::uint64_t unused_hint,std::uint64_t offset,std::uint64_t erase_count,
    std::uintptr_t source,std::uint64_t insert_count,ReceiveBufferContext&);
// Closes N6's growth hook with the normal02b940 source. Allocation/free/byte
// effects, terminal providers and coherent prefix/header memory stay mandatory.
class RecoveredRequestGrowthApi:public RequestMutationApi {
public:
    explicit RecoveredRequestGrowthApi(ReceiveBufferContext& context):context_(context){}
    ReceiveBufferString& grow_replace_02b940(ReceiveBufferString&,std::uint64_t,
        std::uint64_t,std::uint64_t,std::uint64_t,std::uintptr_t,std::uint64_t) final;
private:ReceiveBufferContext& context_;
};
} // namespace preloader_l::socket_recovered
