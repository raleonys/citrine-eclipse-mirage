#include "greenfield/protocol/owned_ingress.hpp"
#include <cstring>

namespace greenfield::protocol {
using preloader_l::net::IngressStatus;
OwnedIngress::OwnedIngress() noexcept { preloader_l::net::reset_ingress(&state_); }
IngressStatus OwnedIngress::feed(const std::uint8_t* data, std::size_t length) noexcept {
    if (!active_) return IngressStatus::invalid_argument;
    preloader_l::xml::TextSpan ignored;
    return preloader_l::net::feed_ingress(&state_,data,length,&ignored);
}
IngressStatus OwnedIngress::pop(char* data, std::size_t capacity, std::size_t* written) noexcept {
    if (written) *written=0;
    if (!active_ || !written || (!data && capacity != 0)) return IngressStatus::invalid_argument;
    preloader_l::xml::TextSpan borrowed;
    const auto status=preloader_l::net::peek_ingress_frame(&state_,&borrowed);
    if (status != IngressStatus::frame_ready) return status;
    *written=borrowed.size;
    if (!data || capacity < borrowed.size) return IngressStatus::capacity;
    std::memcpy(data,borrowed.data,borrowed.size);
    return preloader_l::net::consume_ingress_frame(&state_,borrowed.size);
}
void OwnedIngress::shutdown() noexcept {
    preloader_l::net::reset_ingress(&state_);
    active_=false;
}
}
