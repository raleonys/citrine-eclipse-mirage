#pragma once
#include "../anadius-messages-source/messages.hpp"
#include <array>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace anadius::crypto_recovered {
using Key=messages_recovered::Key;
// Outside the source's defined/supported memory domain; not an original error ABI.
class OutOfEvidenceInput:public std::invalid_argument {public:using std::invalid_argument::invalid_argument;};
class Cipher final:public messages_recovered::Cipher {
public:
    Cipher();
    void construct_0423d0_in_place() override;
    void set_fixed_key_084c98() override;
    void set_key_042780(const Key&) override;
    void set_key_and_iv_042760(const Key&,const Key&);
    bool decrypt_042480(std::string&) override;
    bool encrypt_042610(std::string&) override;
    bool decrypt_with_offset(std::string&,std::size_t offset);
    bool encrypt_with_offset(std::string&,std::size_t offset);
    void decrypt_block_0458d0(std::span<std::uint8_t,16>) const;
    void encrypt_block_045670(std::span<std::uint8_t,16>) const;
    unsigned mode()const noexcept{return mode_;}
    const Key& iv()const noexcept{return iv_;}
    const std::array<std::uint8_t,176>& round_keys()const noexcept{return round_keys_;}
private:
    unsigned mode_{};
    std::array<std::uint8_t,176> round_keys_{};
    Key iv_{};
    void expand_045ce0(const Key&);
};
class CryptoFactory final:public messages_recovered::CryptoFactory {
public:std::unique_ptr<messages_recovered::Cipher> create_0423d0() override;
};
class TimeSource {public:virtual ~TimeSource()=default;virtual std::int64_t unix_seconds_05eeb8()=0;};
// Calls only the Windows system-time API when explicitly used by an integrator.
class WindowsTimeSource final:public TimeSource {public:std::int64_t unix_seconds_05eeb8() override;};
std::int64_t seconds_from_filetime_05eeb8(std::uint64_t ticks);
class Lcg {
public:
    explicit Lcg(std::uint32_t seed):state_(seed){}
    void seed_045060(std::uint32_t seed)noexcept{state_=seed;}
    std::uint32_t next_045070()noexcept;
    std::uint32_t state()const noexcept{return state_;}
private:std::uint32_t state_;
};
class Random final:public messages_recovered::Random {
public:
    explicit Random(TimeSource& source):time_(source){}
    // Adapter returns the 16 raw bytes; messages.cpp performs 03d190 once.
    Key challenge_bytes_034cf0() override;
    Key key_from_response_034c30(std::string_view) override;
    // Complete helper result, including the original 03d190 hex conversion.
    std::string challenge_hex_034cf0();
private:TimeSource& time_;
};
}
