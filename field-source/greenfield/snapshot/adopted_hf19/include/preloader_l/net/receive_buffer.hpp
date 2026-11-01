#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace net {

constexpr size_t receive_buffer_capacity = 8192;

enum class ReceiveBufferStatus : uint8_t {
    ok = 0,
    invalid_argument,
    capacity,
};

struct ReceiveBuffer {
    uint8_t bytes[receive_buffer_capacity];
    size_t size;
};

void clear_receive_buffer(ReceiveBuffer* buffer) noexcept;
ReceiveBufferStatus append_receive_bytes(ReceiveBuffer* buffer,
                                         const uint8_t* input,
                                         size_t length) noexcept;
ReceiveBufferStatus consume_receive_bytes(ReceiveBuffer* buffer,
                                          size_t length) noexcept;
size_t receive_buffer_available(const ReceiveBuffer& buffer) noexcept;

} }
