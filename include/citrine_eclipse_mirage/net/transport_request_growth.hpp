// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_request_mutation.hpp"

namespace citrine_eclipse_mirage::socket {
ReceiveBufferString& grow_replace_request_02b940(ReceiveBufferString&,std::uint64_t extra,
    std::uint64_t unused_hint,std::uint64_t offset,std::uint64_t erase_count,
    std::uintptr_t source,std::uint64_t insert_count,ReceiveBufferContext&);
// Closes N6's growth hook with the normal02b940 source. Allocation/free/byte
// effects, terminal providers and coherent prefix/header memory stay mandatory.
class RequestGrowthApi:public RequestMutationApi {
public:
    explicit RequestGrowthApi(ReceiveBufferContext& context):context_(context){}
    ReceiveBufferString& grow_replace_02b940(ReceiveBufferString&,std::uint64_t,
        std::uint64_t,std::uint64_t,std::uint64_t,std::uintptr_t,std::uint64_t) final;
private:ReceiveBufferContext& context_;
};
} // namespace citrine_eclipse_mirage::socket
