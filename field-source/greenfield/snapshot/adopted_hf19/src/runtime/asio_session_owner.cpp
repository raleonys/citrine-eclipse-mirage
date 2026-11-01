#include "asio_session_owner.hpp"
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
#include "asio_acceptance_delivery.hpp"
#include "asio_acceptance_resource_fault.hpp"
#include <chrono>
#endif
#include <asio/bind_executor.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/post.hpp>
#include <chrono>
#include <utility>
#if defined(_WIN32) && (!defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP))
#error HF13 shipping Asio session requires the Windows IOCP backend.
#endif

namespace preloader_l { namespace runtime {
namespace wire=preloader_l::protocol::detail;
using preloader_l::protocol::ExchangeStatus;

AsioSessionOwner::AsioSessionOwner(Socket&& socket)
    : socket_(std::move(socket)), strand_(asio::make_strand(socket_.get_executor())),
      deadline_timer_(strand_) {
    preloader_l::protocol::reset_exchange(&exchange_);
}

AsioSessionOwner::~AsioSessionOwner() noexcept {
    // The final reference must not be retired before pending Asio callbacks.
    // Callbacks retain shared ownership until completion (or cancellation).
    asio::error_code ec;
    socket_.close(ec);
    preloader_l::protocol::reset_exchange(&exchange_);
}

bool AsioSessionOwner::start(std::string remote, AsioSessionObserver observer) noexcept {
    try {
        auto self=shared_from_this();
        asio::post(strand_,[self,remote=std::move(remote),observer]() mutable noexcept {
            if (self->started_||self->stopping_||!self->socket_.is_open()) return;
            self->started_=true;
            self->remote_=std::move(remote);
            self->observer_=observer;
            self->send_greeting();
        });
        return true;
    } catch (...) { return false; }
}

bool AsioSessionOwner::request_stop() noexcept {
    try {
        auto self=shared_from_this();
        asio::post(strand_,[self]() noexcept {self->stop(Status::peer_closed);});
        return true;
    } catch (...) { return false; }
}


// The only deadline timer is owned by the same strand as the LSX socket.
// Every reschedule cancels prior waits; an epoch and an outstanding-wait count
// prevent stale callbacks and early service quiescence respectively.
bool AsioSessionOwner::refresh_deadline() noexcept {
    try {
        AsioSessionTimeoutPolicy::Time when{};
        ++deadline_generation_;
        if (!timeout_policy_.next(&when)) {
            (void)deadline_timer_.cancel();
            return true;
        }
        deadline_timer_.expires_at(when);
        const auto generation = deadline_generation_;
        auto self = shared_from_this();
        ++deadline_callbacks_;
        try {
            deadline_timer_.async_wait(asio::bind_executor(strand_,
                [self,generation](const asio::error_code& ec) noexcept {
                    if (self->deadline_callbacks_) --self->deadline_callbacks_;
                    if (!ec && !self->stopping_ && generation == self->deadline_generation_) {
                        const auto cause=self->timeout_policy_.expired(
                            AsioSessionTimeoutPolicy::Clock::now());
                        if (cause == AsioSessionTimeoutPolicy::Expired::receive)
                            self->stop(Status::receive_timeout);
                        else if (cause == AsioSessionTimeoutPolicy::Expired::send)
                            self->stop(Status::send_timeout);
                        else if (!self->refresh_deadline()) self->stop(Status::resource_error);
                    }
                    if (self->stopping_) self->maybe_finish();
                }));
        } catch (...) {--deadline_callbacks_; throw;}
        return true;
    } catch (...) {return false;}
}
void AsioSessionOwner::cancel_deadline() noexcept {
    timeout_policy_.clear();
    ++deadline_generation_;
    try { (void)deadline_timer_.cancel(); }
    catch (...) {terminal_status_=Status::resource_error;}
}
bool AsioSessionOwner::begin_write_deadline() noexcept {
    write_observed_=0u;
    timeout_policy_.begin_send(AsioSessionTimeoutPolicy::Clock::now());
    return refresh_deadline();
}
void AsioSessionOwner::write_progress(size_t transferred) noexcept {
    if (transferred > write_observed_ && write_pending_ && !stopping_) {
        const auto now=AsioSessionTimeoutPolicy::Clock::now();
        if (timeout_policy_.expired(now) == AsioSessionTimeoutPolicy::Expired::send) {
            stop(Status::send_timeout);
            return;
        }
        write_observed_=transferred;
        timeout_policy_.send_progress(now);
        if (!refresh_deadline()) stop(Status::resource_error);
    }
}
void AsioSessionOwner::end_write_deadline() noexcept {
    timeout_policy_.end_send();
    if (!refresh_deadline()) stop(Status::resource_error);
}

void AsioSessionOwner::send_greeting() noexcept {
    if (stopping_) return;
    if (preloader_l::protocol::build_challenge_greeting(
            wire::current_greeting_seed(),greeting_,
            sizeof(greeting_),&greeting_size_) != preloader_l::protocol::GreetingStatus::ok ||
        greeting_size_>=sizeof(greeting_)) {
        stop(Status::handshake_error);return;
    }
    try {
        write_pending_=true;
        if (!begin_write_deadline()) {write_pending_=false;stop(Status::resource_error);return;}
        auto self=shared_from_this();
        asio::async_write(socket_,asio::buffer(greeting_,greeting_size_+1u),
            [self](const asio::error_code& ec,size_t n) noexcept -> size_t {
                self->write_progress(n);
                return asio::transfer_all()(ec,n);
            },
            asio::bind_executor(strand_,[self](const asio::error_code& ec,size_t n) noexcept {
                self->write_pending_=false;
                self->end_write_deadline();
                if (self->stopping_) {self->maybe_finish();return;}
                if (ec||n!=self->greeting_size_+1u) {self->stop(Status::send_error);return;}
                self->proceed();
            }));
    } catch (...) {write_pending_=false;stop(Status::resource_error);}
}

void AsioSessionOwner::proceed() noexcept {
    if (stopping_||read_pending_||write_pending_||transaction_) return;
    wire::WireFrameView frame{};
    if (wire::scan_available_wire_frame(&exchange_,&frame)) {
        timeout_policy_.end_receive();
        if (!refresh_deadline()) {stop(Status::resource_error);return;}
        handle_frame(frame);return;
    }
    if (exchange_.wire_size>=sizeof(exchange_.wire_response)) {
        stop(Status::frame_error);return;
    }
    read_more();
}

void AsioSessionOwner::read_more() noexcept {
    if (stopping_||read_pending_) return;
    const size_t available=sizeof(exchange_.wire_response)-exchange_.wire_size;
    if (!available) {stop(Status::frame_error);return;}
    try {
        timeout_policy_.begin_receive(AsioSessionTimeoutPolicy::Clock::now(),
                                      exchange_.wire_size != 0u);
        read_pending_=true;
        if (!refresh_deadline()) {read_pending_=false;stop(Status::resource_error);return;}
        auto self=shared_from_this();
        socket_.async_read_some(asio::buffer(exchange_.wire_response+exchange_.wire_size,available),
            asio::bind_executor(strand_,[self](const asio::error_code& ec,size_t bytes) noexcept {
                self->read_pending_=false;
                if (self->stopping_) {self->maybe_finish();return;}
                if (ec) {
                    self->stop(ec==asio::error::eof?Status::peer_closed:Status::receive_error);
                    return;
                }
                if (!bytes||bytes>sizeof(self->exchange_.wire_response)-self->exchange_.wire_size) {
                    self->stop(Status::receive_error);return;
                }
                const auto now=AsioSessionTimeoutPolicy::Clock::now();
                if (self->timeout_policy_.expired(now) ==
                    AsioSessionTimeoutPolicy::Expired::receive) {
                    self->stop(Status::receive_timeout);return;
                }
                self->exchange_.wire_size+=bytes;
                self->timeout_policy_.receive_progress(now,true);
                if (!self->refresh_deadline()) {self->stop(Status::resource_error);return;}
                self->proceed();
            }));
    } catch (...) {read_pending_=false;stop(Status::resource_error);}
}

void AsioSessionOwner::handle_frame(Frame frame) noexcept {
    const auto step=wire::classify_received_wire_frame(&exchange_,frame,&malformed_streak_);
    if (step.kind==wire::WireStepKind::terminal) {stop(step.status);return;}
    if (step.kind==wire::WireStepKind::discarded) {proceed();return;}
    if (step.kind==wire::WireStepKind::handshake_reply) {
        try {
            auto self=shared_from_this();
            write_pending_=true;
            if (!begin_write_deadline()) {write_pending_=false;stop(Status::resource_error);return;}
            asio::async_write(socket_,asio::buffer(exchange_.plain_response,step.handshake.response_length+1u),
                [self](const asio::error_code& ec,size_t n) noexcept -> size_t {
                    self->write_progress(n);
                    return asio::transfer_all()(ec,n);
                },
                asio::bind_executor(strand_,[self,frame,pending=step.handshake](const asio::error_code& ec,size_t bytes) noexcept {
                    self->write_pending_=false;
                    self->end_write_deadline();
                    if (self->stopping_) {self->maybe_finish();return;}
                    if (ec||bytes!=pending.response_length+1u) {self->stop(Status::send_error);return;}
                    if (!wire::commit_sent_handshake(&self->exchange_,frame,pending,&self->malformed_streak_)) {
                        self->stop(Status::handshake_error);return;
                    }
                    self->proceed();
                }));
        } catch (...) {write_pending_=false;stop(Status::resource_error);}
        return;
    }
    if (step.kind!=wire::WireStepKind::xml_request) {stop(Status::dispatch_error);return;}
    preloader_l::core::DispatchEnvironment env{remote_.data(),remote_.size()};
    env.local_social_session=&exchange_.local_social_session;
    TxResult result{};
    auto* prepared=wire::prepare_response_transaction(&exchange_,
        reinterpret_cast<const char*>(exchange_.decoded),step.xml_length,&env,&result);
    if (!prepared) {
        const auto status=result.dispatch_status;
        if (status==preloader_l::core::DispatchStatus::incomplete_request ||
            status==preloader_l::core::DispatchStatus::malformed_request) {
            wire::consume_wire_frame(&exchange_,frame.consume_length);
            if (!wire::tolerate_malformed_frame(&malformed_streak_)) {stop(Status::frame_error);return;}
            proceed();return;
        }
        if (status==preloader_l::core::DispatchStatus::unknown_route ||
            status==preloader_l::core::DispatchStatus::unsupported_route ||
            status==preloader_l::core::DispatchStatus::no_response_required) {
            malformed_streak_=0u;
            wire::consume_wire_frame(&exchange_,frame.consume_length);
            proceed();return;
        }
        stop(result.status==Status::ok?Status::dispatch_error:result.status);
        return;
    }
    try {
        pending_frame_=frame;
        transaction_=std::make_unique<TxDriver>(prepared,this,&begin_prepared_write,&completed_prepared);
        if (!transaction_->begin()) {
            // finish() may already have scheduled the completion callback.
            if (!transaction_finished_) stop(Status::send_error);
        }
    } catch (...) {
        wire::destroy_prepared_response_transaction(prepared);
        stop(Status::resource_error);
    }
}

bool AsioSessionOwner::begin_prepared_write(void* ctx,const uint8_t* bytes,size_t length) noexcept {
    return static_cast<AsioSessionOwner*>(ctx)->send_prepared(bytes,length);
}
void AsioSessionOwner::completed_prepared(void* ctx,const TxResult& result) noexcept {
    static_cast<AsioSessionOwner*>(ctx)->prepared_finished(result);
}
bool AsioSessionOwner::send_prepared(const uint8_t* bytes,size_t length) noexcept {
    if (stopping_||write_pending_||!bytes||!length) return false;
    write_pending_=true;
    if (!begin_write_deadline()) {write_pending_=false;return false;}
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (release_acceptance_should_hold_asio_send_windows(this)) {
        if (schedule_stalled_prepared(bytes,length)) return true;
        write_pending_=false;
        end_write_deadline();
        return false;
    }
#endif
    if (launch_prepared_write(bytes,length)) return true;
    write_pending_=false;
    end_write_deadline();
    return false;
}
bool AsioSessionOwner::launch_prepared_write(const uint8_t* bytes,size_t length) noexcept {
    try {
        auto self=shared_from_this();
        asio::async_write(socket_,asio::buffer(bytes,length),
            [self](const asio::error_code& ec,size_t n) noexcept -> size_t {
                self->write_progress(n);
                return asio::transfer_all()(ec,n);
            },
            asio::bind_executor(strand_,[self](const asio::error_code& ec,size_t sent) noexcept {
                self->write_pending_=false;
                self->end_write_deadline();
                if (self->transaction_) self->transaction_->completed(!ec && !self->stopping_,sent);
                if (self->stopping_) self->maybe_finish();
            }));
        return true;
    } catch (...) {return false;}
}
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
bool AsioSessionOwner::schedule_stalled_prepared(const uint8_t* bytes,size_t length) noexcept {
    try {
        if (!f02_stall_timer_)
            f02_stall_timer_=std::make_unique<asio::steady_timer>(strand_);
        f02_stall_timer_->expires_after(std::chrono::milliseconds(40));
        auto self=shared_from_this();
        f02_stall_timer_->async_wait(asio::bind_executor(strand_,
            [self,bytes,length](const asio::error_code& ec) noexcept {
                // The prepared frame remains owned by the pending transaction.
                // Timer cancellation is a real failed write completion; a
                // stopped session must not commit or silently lose the frame.
                if (self->stopping_ || ec) {
                    self->write_pending_=false;
                    self->end_write_deadline();
                    if (self->transaction_) {
                        // completed() posts deferred driver retirement. Do not
                        // advertise service quiescence before resume_transaction.
                        self->transaction_->completed(false,0u);
                    } else if (self->stopping_) self->maybe_finish();
                    return;
                }
                if (release_acceptance_should_hold_asio_send_windows(self.get())) {
                    if (self->schedule_stalled_prepared(bytes,length)) return;
                } else if (self->launch_prepared_write(bytes,length)) {
                    return;
                }
                self->write_pending_=false;
                self->end_write_deadline();
                if (self->transaction_) self->transaction_->completed(false,0u);
            }));
        return true;
    } catch (...) {return false;}
}
#endif
void AsioSessionOwner::prepared_finished(const TxResult& result) noexcept {
    pending_result_=result;
    transaction_finished_=true;
    // Never destroy the PreparedAsyncTransaction synchronously inside its own
    // finished callback. Defer retirement to the owning strand turn.
    try {
        auto self=shared_from_this();
        asio::post(strand_,[self]() noexcept {self->resume_transaction();});
    } catch (...) {stop(Status::resource_error);}
}
void AsioSessionOwner::resume_transaction() noexcept {
    if (!transaction_finished_) return;
    transaction_finished_=false;
    transaction_.reset();
    if (stopping_) {maybe_finish();return;}
    if (pending_result_.status!=Status::ok||!pending_result_.response_sent) {
        stop(pending_result_.status==Status::ok?Status::dispatch_error:pending_result_.status);return;
    }
    wire::consume_wire_frame(&exchange_,pending_frame_.consume_length);
    malformed_streak_=0u;
    proceed();
}
void AsioSessionOwner::stop(Status status) noexcept {
    if (!stopping_) {
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        if (status == Status::send_timeout)
            release_acceptance_note_send_idle_timeout_windows();
#endif
        stopping_=true;terminal_status_=status;
    }
    cancel_deadline();
    if (transaction_) transaction_->request_stop();
    asio::error_code ec;
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (f02_stall_timer_) {
        // Asio 1.38.2 exposes timer.cancel() without an error_code overload.
        // stop() is noexcept: a cancellation failure cannot escape; the
        // outstanding wait remains an owned pending operation until callback.
        try { (void)f02_stall_timer_->cancel(); }
        catch (...) { terminal_status_=Status::resource_error; }
    }
#endif
    socket_.cancel(ec);
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // Exercise failure BEFORE performing the real close. The same Asio socket
    // remains in the session registry, so shutdown cannot acknowledge unload.
    // Retention is counted once per exact owner, not per retry request.
    if (socket_.is_open() && consume_asio_acceptance_socket_close_failure()) {
        if (!acceptance_socket_close_retained_) {
            acceptance_socket_close_retained_=true;
            note_asio_acceptance_socket_close_retained();
        }
        terminal_status_=Status::resource_error;
        return;
    }
#endif
    ec.clear();
    if (socket_.is_open()) socket_.close(ec);
    // A failure to close retains socket ownership and blocks retirement.
    if (ec || socket_.is_open()) return;
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (acceptance_socket_close_retained_) {
        acceptance_socket_close_retained_=false;
        note_asio_acceptance_socket_close_retired();
    }
#endif
    maybe_finish();
}
void AsioSessionOwner::maybe_finish() noexcept {
    if (!stopping_||completion_notified_||socket_.is_open()||read_pending_||write_pending_||
        (transaction_&&!transaction_->is_finished())||deadline_callbacks_!=0u) return;
    completion_notified_=true;
    if (observer_.finished) observer_.finished(observer_.context,terminal_status_);
}
} }
