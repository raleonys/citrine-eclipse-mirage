#include "asio_service_owner.hpp"
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
// Native-only observer API. Do not include release_server_io_internal.hpp here:
// it declares private WinSock/Win32 symbols that conflict with Asio's SDK
// declarations within this translation unit. These are declarations only;
// the counters remain defined and owned by release_server_lifecycle.cpp.
namespace preloader_l { namespace runtime { namespace release_server_windows_detail {
void note_release_acceptance_accepted() noexcept;
void note_release_acceptance_started() noexcept;
void note_release_acceptance_completed() noexcept;
void note_release_acceptance_peer_nonempty() noexcept;
void note_release_acceptance_reaped() noexcept;
void note_release_acceptance_asio_owner_opened() noexcept;
void note_release_acceptance_asio_owner_retired() noexcept;
} } }
#endif
#include <asio/post.hpp>
#include <algorithm>
#include <utility>
#if defined(_WIN32) && (!defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP))
#error HF13 Windows Asio service requires IOCP; reactor fallback is forbidden.
#endif
namespace preloader_l { namespace runtime {
AsioServiceOwner::AsioServiceOwner(asio::io_context& io)
    : service_strand_(asio::make_strand(io)),
      listener_(std::make_shared<AsioListenerOwner>(io)) {}
AsioServiceOwner::~AsioServiceOwner() noexcept = default;

bool AsioServiceOwner::bind_loopback(uint16_t port,asio::error_code& ec) noexcept {
    if (started_) { ec=asio::error::already_open;return false; }
    return listener_->bind_loopback(port,ec);
}
uint16_t AsioServiceOwner::bound_port(asio::error_code& ec) const noexcept {
    return listener_->bound_port(ec);
}
bool AsioServiceOwner::abort_before_start(asio::error_code& ec) noexcept {
    if (started_ || self_hold_) { ec=asio::error::already_started; return false; }
    return listener_->abort_before_start(ec);
}
bool AsioServiceOwner::start(AsioServiceObserver observer) noexcept {
    if (started_||!observer.quiesced||!observer.failed) return false;
    try {
        auto self=shared_from_this();
        observer_=observer;
        // The callback ABI uses raw context pointers. Retain this bridge until
        // listener + all accepted sessions have fully completed their callbacks.
        self_hold_=self;
        started_=true;
        asio::post(service_strand_,[self]() noexcept {
            if (!self->listener_->start({self.get(), &accepted,
                                          &listener_quiesced, &listener_failed})) {
                self->fail(asio::error::no_memory);
                self->apply_stop();
            }
        });
        return true;
    } catch (...) {
        started_=false;
        observer_={};
        self_hold_.reset();
        return false;
    }
}
bool AsioServiceOwner::request_stop() noexcept {
    if (!started_) return false;
    try {
        auto self=shared_from_this();
        asio::post(service_strand_,[self]() noexcept {self->apply_stop();});
        return true;
    } catch (...) {return false;}
}
void AsioServiceOwner::accepted(void* ctx,asio::ip::tcp::socket&& socket,
                                const asio::ip::tcp::endpoint& remote) noexcept {
    auto* raw=static_cast<AsioServiceOwner*>(ctx);
    try {
        auto self=raw->shared_from_this();
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        // Counts a socket accepted by IOCP, not a fabricated WinSock worker.
        release_server_windows_detail::note_release_acceptance_accepted();
#endif
        auto ip=remote.address().to_string();
        asio::post(self->service_strand_,[self,socket=std::move(socket),ip=std::move(ip)]() mutable noexcept {
            self->add_session(std::move(socket),std::move(ip));
        });
    } catch (...) {
        asio::error_code ignored;
        socket.close(ignored);
        // Accepted sockets have been counted by the listener; release admission
        // even when the bridge cannot allocate a session or queue the callback.
        (void)raw->listener_->session_finished();
        (void)raw->request_stop();
    }
}
void AsioServiceOwner::listener_quiesced(void* ctx) noexcept {
    auto* raw=static_cast<AsioServiceOwner*>(ctx);
    try {
        auto self=raw->shared_from_this();
        asio::post(self->service_strand_,[self]() noexcept {
            self->listener_drained_=true;
            self->finish();
        });
    } catch (...) { /* cannot safely report quiescence if posting fails */ }
}
void AsioServiceOwner::listener_failed(void* ctx,const asio::error_code& ec) noexcept {
    auto* raw=static_cast<AsioServiceOwner*>(ctx);
    try {
        auto self=raw->shared_from_this();
        asio::post(self->service_strand_,[self,ec]() noexcept {
            self->fail(ec);
            self->apply_stop();
        });
    } catch (...) { /* terminal failure remains unacknowledged, not success */ }
}
void AsioServiceOwner::session_finished(void* ctx,
                                        preloader_l::protocol::ExchangeStatus status) noexcept {
    auto* entry=static_cast<SessionEntry*>(ctx);
    auto* raw=entry->owner;
    try {
        auto self=raw->shared_from_this();
        asio::post(self->service_strand_,[self,entry,status]() noexcept {
            self->retire_session(entry,status);
        });
    } catch (...) { /* retain the session in registry; no false quiescence */ }
}
void AsioServiceOwner::add_session(asio::ip::tcp::socket socket,std::string remote) noexcept {
    if (stopping_) {
        asio::error_code ignored;
        socket.close(ignored);
        if (!listener_->session_finished()) fail(asio::error::no_memory);
        return;
    }
    try {
        auto entry=std::make_unique<SessionEntry>();
        entry->owner=this;
        entry->session=std::make_shared<AsioSessionOwner>(std::move(socket));
        SessionEntry* record=entry.get();
        sessions_.push_back(std::move(entry));
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        release_server_windows_detail::note_release_acceptance_asio_owner_opened();
        if (!remote.empty())
            release_server_windows_detail::note_release_acceptance_peer_nonempty();
#endif
        if (observer_.session_accepted &&
            !observer_.session_accepted(observer_.context)) {
            retire_session(record,preloader_l::protocol::ExchangeStatus::resource_error);
            fail(asio::error::operation_aborted);
            apply_stop();
            return;
        }
        if (!record->session->start(std::move(remote),{record,&session_finished})) {
            // start() failed to schedule any callback: explicitly retire the
            // record, preventing a permanently stuck listener count.
            retire_session(record,preloader_l::protocol::ExchangeStatus::resource_error);
            fail(asio::error::no_memory);
            apply_stop();
        }
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        else {
            record->started=true;
            release_server_windows_detail::note_release_acceptance_started();
        }
#endif
    } catch (...) {
        asio::error_code ignored;
        socket.close(ignored);
        if (!listener_->session_finished()) fail(asio::error::no_memory);
        fail(asio::error::no_memory);
        apply_stop();
    }
}
void AsioServiceOwner::retire_session(
    SessionEntry* record,preloader_l::protocol::ExchangeStatus status) noexcept {
    const auto it=std::find_if(sessions_.begin(),sessions_.end(),
        [record](const std::unique_ptr<SessionEntry>& e){return e.get()==record;});
    if (it==sessions_.end()) return;
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // Completion is observed only after a real session start. Reaped means
    // registry entry retired; no legacy jthread is claimed to exist.
    if ((*it)->started)
        release_server_windows_detail::note_release_acceptance_completed();
    release_server_windows_detail::note_release_acceptance_asio_owner_retired();
    release_server_windows_detail::note_release_acceptance_reaped();
#endif
    sessions_.erase(it);
    if (observer_.session_finished) observer_.session_finished(observer_.context,status);
    if (!listener_->session_finished()) fail(asio::error::no_memory);
    finish();
}
void AsioServiceOwner::apply_stop() noexcept {
    if (completed_) return;
    stopping_=true;
    for (auto& item:sessions_) {
        if (item->session && !item->session->request_stop()) fail(asio::error::no_memory);
    }
    if (!listener_->request_stop()) fail(asio::error::no_memory);
    finish();
}
void AsioServiceOwner::fail(const asio::error_code& ec) noexcept {
    if (failure_notified_) return;
    failure_notified_=true;
    if (observer_.failed) observer_.failed(observer_.context,ec);
}
void AsioServiceOwner::finish() noexcept {
    if (!stopping_||!listener_drained_||!sessions_.empty()||completed_) return;
    completed_=true;
    if (observer_.quiesced) observer_.quiesced(observer_.context);
    self_hold_.reset();
}
} }
