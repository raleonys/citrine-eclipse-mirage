// SPDX-License-Identifier: GPL-3.0-only
// Test-only literal packet Session projection; no production include/link.
#pragma once
#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
namespace preloader_l::test_packet {
enum class ErrorCategory { system, asio_misc, other };
struct Error { int value = 0; ErrorCategory category = ErrorCategory::system; };
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
std::size_t receive_request_size(std::size_t,std::size_t,std::size_t);
std::size_t receive_request_size(std::size_t size, std::size_t capacity, std::size_t maximum) {
    if (size >= maximum) return 0;
    const auto spare = std::min(capacity, maximum) - size;
    return std::min({maximum-size, std::size_t{0x10000}, std::max(spare,std::size_t{0x200})});
}
void Session::log_unless_eof(std::string_view operation, Error error) {
    if (error.value != 0 && !(error.category == ErrorCategory::asio_misc && error.value == 2))
        scheduling_.log_error(operation,error);
}
void Session::start_025720() {
    handler_.reset_02bc70();
    const auto initial = handler_.initial_message_02d540();
    std::size_t sent = 0;
    while (sent < initial.size()) {
        const auto count = std::min(initial.size()-sent,std::size_t{0x10000});
        const auto result = transport_.synchronous_send({initial.data()+sent,count});
        sent += result.transferred;
        if (result.error.value != 0) scheduling_.raise_write_error(result.error);
        // Original blocking loop does not synthesize a failure on zero progress.
    }
    do_read_0280d0();
}
void Session::do_read_0280d0() {
    const auto delimiter = input_.find('\0');
    if (delimiter != std::string::npos) {
        const auto transferred = delimiter+1;
        scheduling_.post_io([this,transferred]{frame_completed_025480({},transferred);});
        return;
    }
    const auto requested = receive_request_size(input_.size(),input_.capacity(),input_.max_size());
    if (requested == 0) {
        scheduling_.post_io([this]{frame_completed_025480({3,ErrorCategory::asio_misc},0);});
        return;
    }
    transport_.async_receive(requested,[this](Error error,std::string bytes){receive_completed(error,std::move(bytes));});
}
void Session::receive_completed(Error error, std::string bytes) {
    input_.append(bytes);
    if (error.value != 0) {frame_completed_025480(error,0);return;}
    do_read_0280d0();
}
void Session::frame_completed_025480(Error error, std::size_t transferred) {
    if (error.value != 0) {log_unless_eof("read",error);return;}
    // Match unsigned param_3-1 then min with input size, including zero count.
    auto message = input_.substr(0,std::min(transferred-1,input_.size()));
    scheduling_.launch_detached([this,message=std::move(message)]() mutable {
        process_message_028d30(std::move(message));
    });
    // Launch/detach failures throw before consuming input or rearming reads.
    input_.erase(0,std::min(transferred,input_.size()));
    do_read_0280d0();
}
void Session::process_message_028d30(std::string message) {
    if (message.empty()) return;
    auto response = handler_.process_message_033d70(message);
    scheduling_.dispatch_write([this,response=std::move(response)]() mutable {do_write_028270(std::move(response));});
}
void Session::do_write_028270(std::string message) {
    output_.push_back(std::move(message));
    if (output_.size() == 1) write_loop_02a500();
}
void Session::write_loop_02a500() {
    if (output_.empty()) return;
    written_ = 0;
    send_next_chunk();
}
void Session::send_next_chunk() {
    const auto& message = output_.front();
    const auto offset = std::min(written_,message.size());
    transport_.async_send({message.data()+offset,std::min(message.size()-offset,std::size_t{0x10000})},
                         [this](Error error,std::size_t transferred){send_completed_0261d0(error,transferred);});
}
void Session::send_completed_0261d0(Error error, std::size_t transferred) {
    written_ += transferred;
    if (error.value != 0 || transferred == 0 || written_ >= output_.front().size()) {
        write_completed_025a70(error);
    } else send_next_chunk();
}
void Session::write_completed_025a70(Error error) {
    if (error.value != 0) {log_unless_eof("write",error);return;}
    output_.pop_front();
    write_loop_02a500();
}
}
