#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include "../anadius-execution-source/execution.hpp"

namespace anadius::server_recovered {
enum class ErrorCategory { system, asio_misc, other };
struct Error { int value = 0; ErrorCategory category = ErrorCategory::system; };
struct ParsedAddress {
    int conversion_result = 0;
    Error error;
    std::array<std::uint8_t,16> bytes{};
    std::uint32_t scope_id = 0;
};
struct Endpoint {
    std::uint16_t family = 2; // observed Windows AF_INET/AF_INET6 values: 2/23
    std::uint16_t port_host_order = 0;
    std::array<std::uint8_t,16> address{};
    std::uint32_t scope_id = 0;
};
struct LocalEndpoint { Error error; std::uint16_t port_network_order = 0; std::uint32_t returned_size = 28; };

// Mandatory ASIO/kernel boundaries. These declarations do not implement any
// socket/network operation. Addresses, port publication and readiness are below.
struct ServerBackend {
    virtual ~ServerBackend() = default;
    virtual void initialize_context_0241b0() = 0;
    virtual ParsedAddress parse_address_028690(std::uint16_t family, std::string_view literal) = 0;
    virtual void construct_acceptor_020e20(const Endpoint&, bool reuse_address, int backlog) = 0;
    virtual void initialize_pending_socket_0211b0() = 0;
    virtual LocalEndpoint local_endpoint() = 0;
    virtual bool set_environment_variable(std::string_view key, std::string_view value) = 0;
    virtual void async_accept_021b00() = 0;
    virtual Error run_context_0298b0() = 0;
    virtual void destroy_pending_socket_024db0() = 0;
    virtual void destroy_acceptor_024db0() = 0;
    virtual void destroy_context() = 0;
    [[noreturn]] virtual void raise_error(Error, std::string_view observed_context) = 0;
};
void construct_server_024480(ServerBackend&, std::uint16_t& port_0a3c90,
                            std::uint8_t& ready_0a3c3d, execution_recovered::Execution&);
// Normal construction/run/destruction order. Backend owns ASIO context internals
// and their exception cleanup, not the application port-ready decision.
void server_start_029ea0(ServerBackend&, std::uint16_t& port_0a3c90,
                        std::uint8_t& ready_0a3c3d, execution_recovered::Execution&);

struct Handler {
    virtual ~Handler() = default;
    virtual void reset_02bc70() = 0;
    virtual std::string initial_message_02d540() = 0;
    virtual std::string process_message_033d70(std::string_view request) = 0;
};
using Completion = std::function<void(Error,std::size_t)>;
struct WriteResult { Error error; std::size_t transferred = 0; };
struct Transport {
    virtual ~Transport() = default;
    virtual WriteResult synchronous_send(std::span<const char>) = 0;
    // Receive must return <= requested bytes. EOF is an Error, not a successful
    // zero-byte receive. Buffer/operation ownership must outlive completions.
    virtual void async_receive(std::size_t requested, std::function<void(Error,std::string)>) = 0;
    virtual void async_send(std::span<const char>, Completion) = 0;
};
struct Scheduling {
    virtual ~Scheduling() = default;
    // Must queue IO completion (never call inline). Session lifetime is a caller
    // obligation; the original detached job also captures a raw session pointer.
    virtual void post_io(std::function<void()>) = 0;
    // Must implement the observed CRT launch/detach contract or throw on failure.
    // See the separately recovered bootstrap thread helper for its error mapping.
    virtual void launch_detached(std::function<void()>) = 0;
    // Must serialize writes through the socket executor; may dispatch inline.
    virtual void dispatch_write(std::function<void()>) = 0;
    virtual void log_error(std::string_view operation, Error) = 0;
    [[noreturn]] virtual void raise_write_error(Error) = 0;
};

class Session {
public:
    Session(Transport& transport, Scheduling& scheduling, Handler& handler)
        : transport_(transport), scheduling_(scheduling), handler_(handler) {handler_.reset_02bc70();}
    // Accepted-session application portion of025720: second Handler reset,
    // blocking greeting write-all, then start delimiter reads.
    void start_025720();
    void do_read_0280d0();
    void process_message_028d30(std::string message);
    void do_write_028270(std::string message);
    void write_loop_02a500();
    std::string_view buffered_input() const {return input_;}
    std::size_t queued_messages() const {return output_.size();}
    std::size_t written_offset() const {return written_;}
private:
    void frame_completed_025480(Error, std::size_t delimiter_bytes);
    void receive_completed(Error, std::string);
    void send_completed_0261d0(Error, std::size_t);
    void write_completed_025a70(Error);
    void send_next_chunk();
    void log_unless_eof(std::string_view, Error);
    Transport& transport_;
    Scheduling& scheduling_;
    Handler& handler_;
    std::string input_;
    std::deque<std::string> output_;
    std::size_t written_ = 0;
};
// Exact observed ASIO dynamic string growth request, given caller's capacity.
std::size_t receive_request_size(std::size_t size, std::size_t capacity,
                                 std::size_t maximum);
// Allocation/socket-move/shared ownership portion of023000 remains a boundary:
// on success caller must pass a newly constructed accepted Session.
void accept_completion_025720(Error, Session* accepted_session, ServerBackend&, Scheduling&);
}
