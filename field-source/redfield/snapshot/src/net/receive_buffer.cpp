// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/net/receive_buffer.hpp"

namespace preloader_l { namespace net {

void clear_receive_buffer(ReceiveBuffer* buffer) noexcept
{
    if (!buffer) return;
    buffer->size = 0;
}

ReceiveBufferStatus append_receive_bytes(ReceiveBuffer* buffer,
                                         const uint8_t* input,
                                         size_t length) noexcept
{
    if (!buffer || (!input && length != 0)) return ReceiveBufferStatus::invalid_argument;
    if (buffer->size > receive_buffer_capacity || length > receive_buffer_capacity - buffer->size)
        return ReceiveBufferStatus::capacity;
    for (size_t i = 0; i < length; ++i) buffer->bytes[buffer->size + i] = input[i];
    buffer->size += length;
    return ReceiveBufferStatus::ok;
}

ReceiveBufferStatus consume_receive_bytes(ReceiveBuffer* buffer,
                                          size_t length) noexcept
{
    if (!buffer) return ReceiveBufferStatus::invalid_argument;
    if (length > buffer->size) return ReceiveBufferStatus::invalid_argument;
    const size_t remain = buffer->size - length;
    for (size_t i = 0; i < remain; ++i) buffer->bytes[i] = buffer->bytes[length + i];
    buffer->size = remain;
    return ReceiveBufferStatus::ok;
}

size_t receive_buffer_available(const ReceiveBuffer& buffer) noexcept
{
    if (buffer.size >= receive_buffer_capacity) return 0;
    return receive_buffer_capacity - buffer.size;
}

} }
