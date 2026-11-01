#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace protocol {

enum class HandshakeStatus : uint8_t {
    ok = 0,
    invalid_argument = 1,
    malformed_request = 2,
    wrong_command = 3,
    missing_id = 4,
    // Historical values 5 and 7 were unreachable missing_response /
    // missing_version states. They are intentionally retired without
    // renumbering the remaining status values.
    missing_key = 6,
    codec_error = 8,
    response_error = 9,
};

struct HandshakeResult {
    uint8_t session_key[16];
    size_t response_length;
};

enum class GreetingStatus : uint8_t { ok=0, invalid_argument, capacity };

// Approved quality promotion: behavioral-derived greeting policy. The seed is Unix
// seconds (low 32 bits); SDK comes from runtime configuration and '.' is normalized
// to ','. The 16 generated MSVC-rand bytes are serialized as lowercase hex.
GreetingStatus build_challenge_greeting(uint32_t unix_seconds_seed, char* output,
                                        size_t capacity, size_t* written) noexcept;

HandshakeStatus process_challenge_request(const char* input, size_t length,
                                          char* response, size_t response_capacity,
                                          HandshakeResult* result) noexcept;

} }
