// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/receive_buffer.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace net {

enum class IngressStatus : uint8_t {
    ok = 0,
    frame_ready,
    need_more,
    invalid_argument,
    capacity,
    invalid_frame,
};

struct IngressState {
    ReceiveBuffer receive;
    uint64_t bytes_fed;
    uint32_t feed_events;
    uint32_t frames_ready;
    uint32_t frames_consumed;
    uint32_t errors;
};

void reset_ingress(IngressState* state) noexcept;
IngressStatus feed_ingress(IngressState* state, const uint8_t* input,
                           size_t length, preloader_l::xml::TextSpan* frame) noexcept;
IngressStatus peek_ingress_frame(IngressState* state,
                                 preloader_l::xml::TextSpan* frame) noexcept;
IngressStatus consume_ingress_frame(IngressState* state, size_t frame_length) noexcept;

} }
