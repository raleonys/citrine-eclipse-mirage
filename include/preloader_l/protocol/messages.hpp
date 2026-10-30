#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace preloader_l::messages_recovered {
using Key = std::array<std::uint8_t,16>;
struct Registration { std::string_view name; std::uint32_t rva; };
std::span<const Registration> registrations_032d00();

class XmlNode {
public:
    virtual ~XmlNode() = default;
    virtual std::string_view name() const = 0;
    virtual const XmlNode* first_element() const = 0;
    virtual const XmlNode* next_element() const = 0;
    virtual std::optional<std::string_view> attribute(std::string_view name) const = 0;
};
class XmlDocument {
public:
    virtual ~XmlDocument() = default;
    virtual const XmlNode* first_element() const = 0;
    virtual std::string_view error_name() const = 0;
};
class XmlPrinter {
public:
    virtual ~XmlPrinter() = default;
    virtual void open(std::string_view) = 0;
    virtual void attribute(std::string_view name,std::string_view value) = 0;
    virtual void close() = 0;
    // Original XMLPrinter BufferSize includes its terminating NUL. Preserve it.
    virtual std::string buffer_including_terminator() const = 0;
};
struct ParseResult {
    std::unique_ptr<XmlDocument> document;
    int returned_code;
    bool has_error; // distinct from the parser return code in 033e2b
};
class XmlBackend {
public:
    virtual ~XmlBackend() = default;
    virtual ParseResult parse_c_string(std::string_view) = 0;
    virtual std::unique_ptr<XmlPrinter> printer() = 0;
};
class Cipher {
public:
    virtual ~Cipher() = default;
    virtual void construct_0423d0_in_place() = 0;
    virtual void set_fixed_key_084c98() = 0;
    virtual void set_key_042780(const Key&) = 0;
    virtual bool decrypt_042480(std::string&) = 0;
    virtual bool encrypt_042610(std::string&) = 0;
};
class CryptoFactory {
public:
    virtual ~CryptoFactory() = default;
    virtual std::unique_ptr<Cipher> create_0423d0() = 0;
};
class Random {
public:
    virtual ~Random() = default;
    // Time-seeded 16 random bytes; actual PRNG algorithm remains mandatory.
    virtual Key challenge_bytes_034cf0() = 0;
    // Original derives 16 bytes from the first two response characters and PRNG.
    virtual Key key_from_response_034c30(std::string_view) = 0;
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual int atoi_05ee08(std::string_view) = 0;
    virtual bool registered_handler(std::uint32_t rva,const XmlNode&,XmlPrinter&) = 0;
    [[noreturn]] virtual void invalid_message_02bd30_04d228() = 0;
    [[noreturn]] virtual void thread_error_04a648(int code) = 0;
    virtual void log_received(std::string_view) = 0;
    virtual void log_response(std::string_view) = 0;
    virtual void log_event(std::string_view) = 0;
    virtual void log_bad_xml(std::string_view error,std::string_view input) = 0;
    virtual void log_decryption_failure(std::string_view input) = 0;
};
class EventQueue {
public:
    virtual ~EventQueue() = default;
    virtual bool nonempty_relaxed() const = 0;
    virtual int lock_04a6ac() = 0;
    virtual std::int32_t& lock_count_4c() = 0;
    virtual bool nonempty_locked() const = 0;
    virtual std::string copy_front_locked() const = 0;
    virtual void pop_front_locked() = 0;
    virtual void unlock_04a6b4() = 0;
    virtual std::uint64_t reserve_front_locked();
    virtual bool finish_reserved(std::uint64_t,bool) noexcept {return false;}
};
// Owns a queue reservation, never a borrowed ciphertext buffer. Completion is
// nonblocking: the native queue atomically records commit/retry and unlinks a
// committed front on the next locked queue operation.
class EventDelivery final {
public:
    ~EventDelivery(){finish(false);}
    void bind(EventQueue& queue,std::uint64_t token) noexcept {queue_=&queue;token_=token;}
    bool finish(bool success) noexcept;
    bool pending() const noexcept {return token_!=0;}
    EventDelivery()=default;
    EventDelivery(const EventDelivery&)=delete;
    EventDelivery& operator=(const EventDelivery&)=delete;
private:
    EventQueue* queue_=nullptr;
    std::uint64_t token_=0;
};
struct State {
    std::uint8_t& encrypted_0c4;
    const std::uint8_t& show_lsx_0a3ef0;
    Cipher& cipher;
    EventQueue& events;
    EventDelivery* delivery=nullptr; // Null retains the pure source dequeue path.
};

void hex_encode_03d190(std::string&);
void hex_decode_03ce30(std::string&);
// Separate from factory creation: original reconstructs this cipher in place.
Cipher& reset_02bc70(Cipher&,std::uint8_t& encrypted_0c4,std::uint8_t& unknown_0c5);
std::string initial_message_02d540(XmlBackend&,Random&,std::string_view version);
bool challenge_response_02d7e0(const XmlNode&,XmlPrinter&,Cipher&,CryptoFactory&,Random&,Effects&);
std::string dispatch_032d00(const XmlDocument&,Cipher&,XmlBackend&,CryptoFactory&,Random&,Effects&);
// Mutates input in place, including retries; no socket or transport operations.
std::string process_message_033d70(State, std::string& input,XmlBackend&,CryptoFactory&,Random&,Effects&);
} // namespace preloader_l::messages_recovered
