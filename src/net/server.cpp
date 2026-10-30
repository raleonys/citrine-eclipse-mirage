#include "preloader_l/net/server.hpp"
#include <algorithm>
#include <limits>
#include <utility>
#include <stdexcept>

namespace preloader_l::server_recovered {
namespace {
std::uint16_t network_port_to_host(std::uint16_t value) {
    return static_cast<std::uint16_t>((value >> 8) | (value << 8));
}
}
void construct_server_024480(ServerBackend& backend, std::uint16_t& port,
                            std::uint8_t& ready, execution_recovered::Execution& execution) {
    const auto ipv6 = backend.parse_address_028690(23, "127.0.0.1");
    Endpoint endpoint;
    endpoint.port_host_order = port;
    if (ipv6.error.value == 0) {
        endpoint.family = 23;
        if (ipv6.conversion_result > 0) {
            endpoint.address = ipv6.bytes;
            endpoint.scope_id = ipv6.scope_id;
        }
    } else {
        const auto ipv4 = backend.parse_address_028690(2, "127.0.0.1");
        if (ipv4.error.value != 0) backend.raise_error(ipv4.error, "address");
        endpoint.family = 2;
        if (ipv4.conversion_result > 0) std::copy_n(ipv4.bytes.begin(), 4, endpoint.address.begin());
    }
    backend.construct_acceptor_020e20(endpoint, true, std::numeric_limits<std::int32_t>::max());
    backend.initialize_pending_socket_0211b0();
    const auto local = backend.local_endpoint();
    if (local.error.value != 0) backend.raise_error(local.error, "local_endpoint");
    if (local.returned_size > 128) backend.raise_error({0x2726, ErrorCategory::system}, "endpoint_size");
    port = network_port_to_host(local.port_network_order);
    // 02466d..0246e4 writes the actual bound port; BOOL result is ignored.
    backend.set_environment_variable("EALsxPort", std::to_string(port));
    execution.finish_flag_036820(ready);
    backend.async_accept_021b00();
}
void server_start_029ea0(ServerBackend& backend, std::uint16_t& port,
                        std::uint8_t& ready, execution_recovered::Execution& execution) {
    backend.initialize_context_0241b0();
    construct_server_024480(backend, port, ready, execution);
    const auto error = backend.run_context_0298b0();
    if (error.value != 0) backend.raise_error(error, "io_context::run");
    backend.destroy_pending_socket_024db0();
    backend.destroy_acceptor_024db0();
    backend.destroy_context();
}

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
    auto initial = handler_.initial_message_02d540();
    if(scheduling_.async_greeting()) {
        greeting_=true;do_write_028270(std::move(initial));return;
    }
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
        if(transferred>scheduling_.max_frame_bytes())throw std::length_error("LSX frame exceeds limit");
        scheduling_.frame_received();
        scheduling_.post_io([this,transferred]{frame_completed_025480({},transferred);});
        return;
    }
    const auto maximum=std::min(input_.max_size(),scheduling_.max_frame_bytes());
    if(input_.size()>=maximum)throw std::length_error("LSX frame exceeds limit");
    const auto requested = receive_request_size(input_.size(),input_.capacity(),maximum);
    // size < maximum above: every operand in receive_request_size's minimum
    // is positive, so its zero-size/full-buffer branch cannot occur here.
    scheduling_.receiving(!input_.empty());
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
    if(!scheduling_.ordered_handlers())do_read_0280d0();
}
void Session::process_message_028d30(std::string message) {
    if (message.empty()) {
        if(scheduling_.ordered_handlers())scheduling_.post_io([this]{do_read_0280d0();});
        return;
    }
    auto response=std::make_shared<PreparedResponse>(scheduling_.ordered_handlers()
        ?handler_.prepare_response(message):PreparedResponse{handler_.process_message_033d70(message)});
    if(response->bytes.size()>scheduling_.max_response_bytes())throw std::length_error("LSX response exceeds limit");
    scheduling_.dispatch_write([this,response=std::move(response)]() mutable {
        queue_response(std::move(response));
    });
}
void Session::do_write_028270(std::string message) {
    queue_response(std::make_shared<PreparedResponse>(std::move(message)));
}
void Session::queue_response(std::shared_ptr<PreparedResponse> message) {
    if(message->bytes.size()>scheduling_.max_response_bytes())throw std::length_error("LSX response exceeds limit");
    output_.push_back(std::move(message));
    if(output_.size()==1)write_loop_02a500();
}
void Session::cancel_output() noexcept {
    for(auto& response:output_)response->finish(false);
    output_.clear();
}
void Session::write_loop_02a500() {
    if (output_.empty()) return;
    written_ = 0;
    if(output_.front()->bytes.empty()&&scheduling_.ordered_handlers()) {
        scheduling_.post_io([this]{write_completed_025a70({});});return;
    }
    send_next_chunk();
}
void Session::send_next_chunk() {
    auto owner=output_.front();const auto& message=owner->bytes;
    const auto offset=std::min(written_,message.size());
    transport_.async_send({message.data()+offset,std::min(message.size()-offset,std::size_t{0x10000})},
        [this,owner](Error error,std::size_t transferred){send_completed_0261d0(error,transferred);});
}
void Session::send_completed_0261d0(Error error,std::size_t transferred) {
    if(output_.empty())return; // A cancelled operation still owns its buffer.
    if(scheduling_.ordered_handlers()&&
       (transferred==0||transferred>std::min(output_.front()->bytes.size()-written_,std::size_t{0x10000}))) {
        if(!error.value)error={29,ErrorCategory::system};
    }
    written_+=transferred;
    if(error.value!=0||transferred==0||written_>=output_.front()->bytes.size())write_completed_025a70(error);
    else send_next_chunk();
}
void Session::write_completed_025a70(Error error) {
    scheduling_.write_finished();
    if(error.value!=0) {
        log_unless_eof("write",error);
        if(scheduling_.ordered_handlers()){cancel_output();scheduling_.fail_session(error);}
        return;
    }
    if(!output_.front()->finish(true)) {
        cancel_output();scheduling_.fail_session({29,ErrorCategory::system});return;
    }
    output_.pop_front();
    if(!output_.empty()){write_loop_02a500();return;}
    if(greeting_||scheduling_.ordered_handlers()) {
        greeting_=false;do_read_0280d0();
    }
}
void accept_completion_025720(Error error, Session* session, ServerBackend& server, Scheduling& scheduling) {
    if (error.value == 0) session->start_025720();
    else scheduling.log_error("accept",error);
    server.async_accept_021b00();
}
}
