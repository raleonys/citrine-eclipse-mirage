#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace codec {

enum class SessionStatus : uint8_t {
    ok = 0,
    invalid_argument,
    capacity,
    overflow,
};

SessionStatus derive_session_key(uint16_t seed, uint8_t output[16]) noexcept;

// Encrypt the incoming ChallengeResponse key with the fixed 00..0F key and
// emit lowercase hex plus NUL. *written excludes the NUL. A present version
// with atoi(version) <= 2 selects the fixed 00..0F session key. Exact one-byte
// version "3" derives the live-consumer-compatible dynamic seed from
// ChallengeAccepted/@response[0:2]. Missing version and other >2 forms retain
// the behavioral-derived incoming ChallengeResponse/@key[0:2] seed policy
// (NUL-filled for a short/empty key), including signed-byte prefix semantics.
SessionStatus make_challenge_reply(const uint8_t* challenge, size_t length,
                                   const char* version, size_t version_length,
                                   uint8_t session_key[16],
                                   char* reply, size_t capacity,
                                   size_t* written) noexcept;

} } // namespace preloader_l::codec
