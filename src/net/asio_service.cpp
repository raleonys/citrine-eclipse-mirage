// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/asio_service.hpp"
#include "citrine_eclipse_mirage/net/listener_policy.hpp"
#include "citrine_eclipse_mirage/net/handler_policy.hpp"
#include <asio.hpp>
#include <algorithm>
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace citrine_eclipse_mirage::net {
namespace {
using namespace server;
Error translated(const asio::error_code& e) {
    auto category=ErrorCategory::system;
    if(e.category()==asio::error::get_misc_category())category=ErrorCategory::asio_misc;
    else if(e.category()!=asio::error::get_system_category())category=ErrorCategory::other;
    return {e.value(),category};
}
[[noreturn]] void socket_error(Error e,std::string_view operation) {
    throw std::runtime_error(std::string(operation)+": socket error "+std::to_string(e.value));
}
// A failed Asio close discards its implementation handle. Only would-block
// explicitly guarantees retained native ownership; other errors quarantine
// the observation and must never cause a blind close of a recycled value.
struct NativeClose {
    enum class State {empty,retained,uncertain};
    SOCKET socket=INVALID_SOCKET;
    State state=State::empty;
    int error=0;
    void failed(SOCKET value,int code) noexcept {
        socket=value;error=code;
        state=code==WSAEWOULDBLOCK?State::retained:State::uncertain;
    }
    bool empty()const noexcept{return state==State::empty;}
    asio::error_code retry() noexcept {
        if(empty())return {};
        if(state==State::uncertain)return {error,asio::error::get_system_category()};
        // Restore the application's ordinary non-lingering close policy, so a
        // previous Asio would-block fallback cannot make the retry block.
        const linger option{0,0};
        if(::setsockopt(socket,SOL_SOCKET,SO_LINGER,reinterpret_cast<const char*>(&option),sizeof option)==SOCKET_ERROR) {
            error=WSAGetLastError();
            if(error==WSAENOTSOCK||error==WSANOTINITIALISED)state=State::uncertain;
            return {error,asio::error::get_system_category()};
        }
        if(::closesocket(socket)==0){*this={};return {};}
        failed(socket,WSAGetLastError());
        return {error,asio::error::get_system_category()};
    }
};
template<class Base>class RetainedSocket:public Base {
    void* context_;
    void(*retain_)(void*,NativeClose)noexcept;
    NativeClose pending_;
public:
    RetainedSocket(asio::io_context& io,void* context,decltype(retain_) retain)
        :Base(io),context_(context),retain_(retain){}
    RetainedSocket(Base&& value,void* context,decltype(retain_) retain)noexcept
        :Base(std::move(value)),context_(context),retain_(retain){}
    RetainedSocket& operator=(RetainedSocket&& other) {
        asio::error_code error;close(error);
        if(error||!retired())throw asio::system_error(error,"socket move cleanup incomplete");
        Base::operator=(std::move(other));pending_=std::exchange(other.pending_,{});
        return *this;
    }
    ~RetainedSocket(){asio::error_code error;close(error);if(!pending_.empty())retain_(context_,pending_);}
    void close(asio::error_code& error) {
        if(!pending_.empty()){error=pending_.retry();return;}
        if(!Base::is_open()){error.clear();return;}
        const auto native=Base::native_handle();Base::close(error);
        if(error)pending_.failed(native,error.value());
    }
    bool retired()const noexcept{return !Base::is_open()&&pending_.empty();}
};
using OwnedSocket=RetainedSocket<asio::ip::tcp::socket>;
using OwnedAcceptor=RetainedSocket<asio::ip::tcp::acceptor>;
SessionPolicy validated(SessionPolicy policy){policy.validate();return policy;}
}
struct AsioService::Impl final:ServerBackend {
#ifdef CITRINE_ECLIPSE_MIRAGE_TEST_ASIO_THREAD
    using OwnedThread=CITRINE_ECLIPSE_MIRAGE_TEST_ASIO_THREAD;
#else
    using OwnedThread=std::thread;
#endif
    struct Client;
    HandlerFactory& factory;
    execution::Execution& execution;
    std::uint16_t& port;
    std::uint8_t& ready;
    const SessionPolicy policy;
    // Reserve before native acquisition. At most two accept callbacks and two
    // bind candidates can join the bounded published client/listener owners.
    std::vector<NativeClose> deferred_closes;
    std::atomic<bool> stopping{false}; // Outlives IO/socket members and their retirement callbacks.
    asio::io_context io;
    OwnedAcceptor acceptor{io,this,&Impl::retain_native};
    std::unique_ptr<OwnedAcceptor> acceptor_v6;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> guard;
    OwnedThread runner;
    std::promise<void> started;
    std::mutex jobs_mutex;
    std::condition_variable jobs_changed;
    std::deque<std::function<void()>> jobs;
    std::array<OwnedThread,handler_policy::workers> workers;
    bool jobs_stopping=false;
    std::vector<std::shared_ptr<Client>> clients; // Retained through native/IO retirement; Client only borrows Impl.
    std::size_t active_sessions=0;
    std::array<bool,2> accept_pending{};
    bool start_signalled=false;
    bool used=false;
    bool accept_enabled=true; // IO-thread only; control thread posts activation.

    Impl(HandlerFactory& f,execution::Execution& e,std::uint16_t& p,std::uint8_t& r,SessionPolicy limits,bool defer_accept=false)
        :factory(f),execution(e),port(p),ready(r),policy(validated(limits)),accept_enabled(!defer_accept){
            deferred_closes.reserve(policy.max_sessions+8);
        }
    static void retain_native(void* context,NativeClose pending)noexcept {
        auto& self=*static_cast<Impl*>(context);
        self.stopping.store(true,std::memory_order_release);
        // Capacity is reserved before any socket can be acquired. Admission
        // stops at the first retained transient, so this bound cannot grow.
        if(self.deferred_closes.size()==self.deferred_closes.capacity())std::terminate();
        self.deferred_closes.push_back(pending);
    }
    ~Impl(){stop();}
    void start() {
        if(used)throw std::logic_error("AsioService start is single-use");
        used=true;guard.emplace(asio::make_work_guard(io));
        auto result=started.get_future();
        start_workers();
        try {runner=OwnedThread([this] {
            try {server_start_029ea0(*this,port,ready,execution);}
            catch(...) {
                stopping.store(true,std::memory_order_release);
                auto error=std::current_exception();
                if(!start_signalled){start_signalled=true;started.set_exception(error);}
                factory.error("io_context",error);
            }
            // Once dispatch stops, this thread can retire IO-owned sockets
            // even when a worker could not allocate an emergency post.
            if(stopping.load(std::memory_order_acquire))retire_io();
        });}catch(...){stop_workers();guard.reset();throw;}
        try {result.get();}catch(...){stop();throw;}
    }
    void stop();
    void resume_accepts();
    void retire_io() noexcept;
    void failed_dispatch(std::exception_ptr error) noexcept {
        // No allocation or socket access from a handler worker. The bounded
        // IO run interval observes this flag without another queued operation.
        stopping.store(true,std::memory_order_release);
        factory.error("cleanup dispatch",error);
    }
    void start_workers() {
        try {
            for(auto& worker:workers)worker=OwnedThread([this] {
                for(;;) {
                    std::function<void()> fn;
                    {
                        std::unique_lock lock(jobs_mutex);
                        jobs_changed.wait(lock,[this]{return jobs_stopping||!jobs.empty();});
                        if(jobs_stopping)return;
                        fn=std::move(jobs.front());jobs.pop_front();
                    }
                    try{fn();}catch(...){factory.error("handler",std::current_exception());}
                }
            });
        }catch(...){stop_workers();throw;}
    }
    void stop_workers() {
        {
            std::lock_guard lock(jobs_mutex);
            jobs_stopping=true;jobs.clear();
        }
        jobs_changed.notify_all();
        for(auto& worker:workers)if(worker.joinable())worker.join();
    }
    void job(std::function<void()> fn) {
        {
            std::lock_guard lock(jobs_mutex);
            if(stopping||jobs_stopping)return;
            if(jobs.size()>=handler_policy::queued_jobs)
                throw std::runtime_error("Handler queue is full");
            jobs.push_back(std::move(fn));
        }
        jobs_changed.notify_one();
    }
    void initialize_context_0241b0() override {io.restart();}
    ParsedAddress parse_address_028690(std::uint16_t family,std::string_view literal) override {
        ParsedAddress result;asio::error_code ec;
        if(family==23) {
            auto address=asio::ip::make_address_v6(std::string(literal),ec);
            if(!ec){result.bytes=address.to_bytes();result.scope_id=address.scope_id();}
        }else {
            auto address=asio::ip::make_address_v4(std::string(literal),ec);
            if(!ec){auto bytes=address.to_bytes();std::copy(bytes.begin(),bytes.end(),result.bytes.begin());}
        }
        result.error=translated(ec);result.conversion_result=ec?0:1;return result;
    }
    void construct_acceptor_020e20(const Endpoint& e,bool reuse,int backlog) override {
        asio::ip::address address;
        if(e.family==23)address=asio::ip::address_v6(e.address,e.scope_id);
        else {asio::ip::address_v4::bytes_type bytes;std::copy_n(e.address.begin(),4,bytes.begin());address=asio::ip::address_v4(bytes);}
        asio::error_code error;
        for(std::uint32_t port=listener_policy::first_port;port<=listener_policy::last_port;++port) {
            if(stopping)throw std::runtime_error("Listener cleanup incomplete");
            asio::ip::tcp::endpoint endpoint(address,static_cast<std::uint16_t>(port));
            OwnedAcceptor candidate(io,this,&Impl::retain_native);
            candidate.open(endpoint.protocol());
            auto exclusive=[&](asio::ip::tcp::acceptor& listener) {
#if defined(_WIN32)
            // Windows SO_REUSEADDR can share an already listening endpoint.
            // Each selected port must belong to exactly one service instance.
            (void)reuse;
            const BOOL exclusive=TRUE;
            if(::setsockopt(listener.native_handle(),SOL_SOCKET,SO_EXCLUSIVEADDRUSE,
                    reinterpret_cast<const char*>(&exclusive),sizeof exclusive)==SOCKET_ERROR)
                throw asio::system_error(::WSAGetLastError(),asio::error::get_system_category(),
                    "LSX listener exclusive address");
#else
            listener.set_option(asio::socket_base::reuse_address(reuse));
#endif
            };
            exclusive(candidate);
            candidate.bind(endpoint,error);
            if(!error)candidate.listen(std::min(backlog,static_cast<int>(policy.max_sessions)),error);
            std::unique_ptr<OwnedAcceptor> companion;
            if(!error&&e.family==2) {
                companion=std::make_unique<OwnedAcceptor>(io,this,&Impl::retain_native);
                companion->open(asio::ip::tcp::v6(),error);
                if(!error) {
                    companion->set_option(asio::ip::v6_only(true),error);
                    if(!error){exclusive(*companion);companion->bind({asio::ip::address_v6::loopback(),static_cast<std::uint16_t>(port)},error);}
                    if(!error)companion->listen(std::min(backlog,static_cast<int>(policy.max_sessions)),error);
                }
                if(error==asio::error::address_family_not_supported||
                   (error.category()==asio::error::get_system_category()&&error.value()==WSAEADDRNOTAVAIL)) {
                    companion.reset();error.clear(); // IPv6 disabled on this host.
                }
            }
            if(stopping)throw std::runtime_error("Listener rollback cleanup incomplete");
            if(!error){acceptor=std::move(candidate);acceptor_v6=std::move(companion);return;}
            // Windows may report an exclusive/reserved port as access denied.
            // Both occupied-port forms advance within the same bounded range.
            if(error!=asio::error::address_in_use&&error!=asio::error::access_denied)
                throw asio::system_error(error,"LSX listener bind/listen");
        }
        throw asio::system_error(error,"No available LSX listener port in 3226-3236");
    }
    void initialize_pending_socket_0211b0() override {}
    LocalEndpoint local_endpoint() override {
        asio::error_code ec;auto endpoint=acceptor.local_endpoint(ec);
        auto p=endpoint.port();return {translated(ec),static_cast<std::uint16_t>((p>>8)|(p<<8)),28};
    }
    bool set_environment_variable(std::string_view key,std::string_view value) override {
        return SetEnvironmentVariableA(std::string(key).c_str(),std::string(value).c_str())!=0;
    }
    void async_accept_021b00() override;
    void queue_accept(asio::ip::tcp::acceptor&,std::size_t);
    Error run_context_0298b0() override {
        // A stop must not depend on allocating a post, or on IOCP stop-packet
        // allocation/wakeup succeeding under resource pressure.
        while(!stopping.load(std::memory_order_acquire)) {
            io.run_for(std::chrono::milliseconds(100));
            if(io.stopped())break;
        }
        stopping.store(true,std::memory_order_release);
        return {};
    }
    void destroy_pending_socket_024db0() override {}
    void destroy_acceptor_024db0() override {retire_io();}
    void destroy_context() override {} // RAII destroys the actual io_context after join.
    [[noreturn]] void raise_error(Error e,std::string_view operation) override {socket_error(e,operation);}
};
struct AsioService::Impl::Client final:Transport,Scheduling,std::enable_shared_from_this<Client> {
    Impl& owner;
    OwnedSocket socket;
    std::unique_ptr<Handler> handler;
    Session session;
    asio::steady_timer timer{owner.io};
    SessionDeadlines deadlines{owner.policy};
    std::optional<SessionDeadlines::Time> armed_deadline;
    std::uint64_t timer_generation=0;
    bool counted=false;
    std::size_t pending_reads=0,pending_writes=0,pending_timers=0;
    std::atomic<bool> closed{false};
    Client(Impl& o,asio::ip::tcp::socket s):owner(o),socket(std::move(s),&o,&Impl::retain_native),
        handler(o.factory.create()),session(*this,*this,checked_handler()){}
    Handler& checked_handler(){if(!handler)throw std::invalid_argument("null session handler");return *handler;}
    bool retired()const noexcept {
        return closed&&socket.retired()&&!pending_reads&&!pending_writes&&!pending_timers;
    }
    void retire_count()noexcept {
        if(counted&&retired()){
            counted=false;--owner.active_sessions;
            try{owner.async_accept_021b00();}catch(...){owner.failed_dispatch(std::current_exception());}
        }
    }
    struct CompletionRetirement {
        Client& client;std::size_t& pending;
        ~CompletionRetirement(){--pending;client.retire_count();}
    };
    void close() {
        if(!closed.exchange(true)){
            deadlines.clear();++timer_generation;armed_deadline.reset();session.cancel_output();
        }
        try{timer.cancel();}catch(...){owner.factory.error("timer cancellation",std::current_exception());}
        asio::error_code ec;if(socket.is_open())socket.cancel(ec);socket.close(ec);
        if(ec){owner.stopping.store(true,std::memory_order_release);log_error("socket cleanup",translated(ec));}
        retire_count();
    }
    void refresh_deadline() {
        const auto next=deadlines.next();
        if(next==armed_deadline)return;
        armed_deadline=next;const auto generation=++timer_generation;
        timer.cancel();
        if(!next)return;
        timer.expires_at(*next);
        auto self=shared_from_this();
        ++pending_timers;
        try{timer.async_wait([self,generation](asio::error_code error) {
            CompletionRetirement retirement{*self,self->pending_timers};
            if(error||self->closed||generation!=self->timer_generation)return;
            self->armed_deadline.reset();
            if(self->deadlines.expired(SessionDeadlines::Clock::now())) {
                self->owner.factory.error("session timeout",std::make_exception_ptr(std::runtime_error("LSX socket deadline expired")));
                self->close();
            } else try{self->refresh_deadline();}catch(...){self->close();}
        });}catch(...){--pending_timers;throw;}
    }
    WriteResult synchronous_send(std::span<const char>) override {
        throw std::logic_error("Native greetings require asynchronous transport");
    }
    void async_receive(std::size_t count,std::function<void(Error,std::string)> cb) override {
        auto data=std::make_shared<std::string>(count,'\0');auto self=shared_from_this();
        ++pending_reads;
        try{socket.async_read_some(asio::buffer(*data),[self,data,cb=std::move(cb)](auto ec,std::size_t n) mutable {
            CompletionRetirement retirement{*self,self->pending_reads};
            if(self->owner.stopping||self->closed)return;
            data->resize(n);
            try {
                const auto now=SessionDeadlines::Clock::now();
                if(self->deadlines.expired(now)){self->close();return;}
                self->deadlines.receive_progress(now,n);self->refresh_deadline();
                cb(translated(ec),std::move(*data));
            }catch(...){self->owner.factory.error("receive",std::current_exception());self->close();}
            if(ec)self->close();
        });}catch(...){--pending_reads;throw;}
    }
    void async_send(std::span<const char> bytes,Completion cb) override {
        deadlines.begin_send(SessionDeadlines::Clock::now());refresh_deadline();
        auto self=shared_from_this();
        ++pending_writes;
        try{socket.async_write_some(asio::buffer(bytes.data(),bytes.size()),[self,cb=std::move(cb)](auto ec,std::size_t n) mutable {
            CompletionRetirement retirement{*self,self->pending_writes};
            if(self->owner.stopping||self->closed)return;
            try {
                const auto now=SessionDeadlines::Clock::now();
                if(self->deadlines.expired(now)){self->close();return;}
                self->deadlines.send_progress(now,n);self->refresh_deadline();cb(translated(ec),n);
            }catch(...){self->owner.factory.error("send",std::current_exception());self->close();}
            if(ec)self->close();
        });}catch(...){--pending_writes;throw;}
    }
    void post_io(std::function<void()> fn) override {
        auto self=shared_from_this();asio::post(owner.io,[self,fn=std::move(fn)] {
            if(!self->owner.stopping&&!self->closed)try{fn();}catch(...){self->owner.factory.error("completion",std::current_exception());self->close();}
        });
    }
    void launch_detached(std::function<void()> fn) override {
        auto self=shared_from_this();owner.job([self,fn=std::move(fn)] {
            if(!self->owner.stopping&&!self->closed)try{fn();}catch(...) {
                const auto failure=std::current_exception();
                try{self->post_io([self]{self->close();});}
                catch(...){self->owner.failed_dispatch(std::current_exception());}
                std::rethrow_exception(failure);
            }
        });
    }
    bool ordered_handlers() const override {return true;}
    bool async_greeting() const override {return true;}
    std::size_t max_frame_bytes() const override{return owner.policy.max_frame_bytes;}
    std::size_t max_response_bytes() const override{return owner.policy.max_response_bytes;}
    void receiving(bool buffered) override {deadlines.begin_receive(SessionDeadlines::Clock::now(),buffered);refresh_deadline();}
    void frame_received() override {deadlines.end_receive();refresh_deadline();}
    void write_finished() override {deadlines.end_send();refresh_deadline();}
    void fail_session(Error) override {close();}
    void dispatch_write(std::function<void()> fn) override {post_io(std::move(fn));}
    void log_error(std::string_view op,Error e) override {
        try{socket_error(e,op);}catch(...){owner.factory.error(op,std::current_exception());}
    }
    [[noreturn]] void raise_write_error(Error e) override {socket_error(e,"greeting");}
};
void AsioService::Impl::async_accept_021b00() {
    if(stopping||active_sessions>=policy.max_sessions)return;
    if(!accept_enabled) {
        // Application startup waits only for a bound listener; early sockets
        // remain in the OS backlog until provider/profile readiness is published.
        if(!start_signalled){start_signalled=true;started.set_value();}
        return;
    }
    // Retain the original immediate mode ordering: failed accept submission
    // must fail startup, rather than signalling success before the operation.
    queue_accept(acceptor,0);
    if(acceptor_v6)queue_accept(*acceptor_v6,1);
    if(!start_signalled){start_signalled=true;started.set_value();}
}
void AsioService::Impl::queue_accept(asio::ip::tcp::acceptor& listener,std::size_t index) {
    if(stopping||accept_pending[index]||active_sessions>=policy.max_sessions)return;
    accept_pending[index]=true;
    // async_accept owns its pending socket until completion (no placeholder socket).
    try {listener.async_accept([this,index](asio::error_code ec,asio::ip::tcp::socket socket) {
        accept_pending[index]=false;
        OwnedSocket incoming(std::move(socket),this,&Impl::retain_native);
        if(stopping)return;
        try {
            if(!ec) {
                // Both families may have accepted the final available slot.
                if(active_sessions>=policy.max_sessions)return;
                auto client=std::make_shared<Client>(*this,std::move(incoming));
                clients.erase(std::remove_if(clients.begin(),clients.end(),[](auto& p){return p->retired();}),clients.end());
                clients.push_back(client);
                client->counted=true;++active_sessions;
                try{accept_completion_025720({},&client->session,*this,*client);}
                catch(...){client->close();throw;}
                return;
            }
            try{socket_error(translated(ec),"accept");}catch(...){factory.error("accept",std::current_exception());}
        }catch(...){factory.error("accepted session",std::current_exception());}
        async_accept_021b00();
    });}catch(...){accept_pending[index]=false;throw;}
}
void AsioService::Impl::resume_accepts() {
    if(!used||stopping.load(std::memory_order_acquire))
        throw std::logic_error("Cannot resume accept after service stop");
    asio::post(io,[this] {
        if(stopping.load(std::memory_order_acquire))return;
        accept_enabled=true;
        try{async_accept_021b00();}catch(...){failed_dispatch(std::current_exception());}
    });
}
void AsioService::Impl::retire_io() noexcept {
    try {
        asio::error_code ec;acceptor.cancel(ec);acceptor.close(ec);
        if(acceptor_v6){acceptor_v6->cancel(ec);acceptor_v6->close(ec);}
    }catch(...){factory.error("listener cleanup",std::current_exception());}
    for(auto& c:clients) {
        try{c->close();}catch(...){factory.error("client cleanup",std::current_exception());}
    }
    for(auto& pending:deferred_closes)(void)pending.retry();
    deferred_closes.erase(std::remove_if(deferred_closes.begin(),deferred_closes.end(),[](auto& p){return p.empty();}),deferred_closes.end());
}
void AsioService::Impl::stop() {
    if(!used)return;
    if(runner.joinable()&&std::this_thread::get_id()==runner.get_id())std::terminate();
    stopping.store(true,std::memory_order_release);
    // The runner retires sockets before exit, while active handlers still
    // retain their Client. Control-thread cleanup starts only after that join.
    if(runner.joinable())runner.join();
    retire_io();
    stop_workers();
    guard.reset();
    // If run exited on an adapter error, finish cancellation completions here.
    io.restart();io.run_for(std::chrono::milliseconds(100));
    if(!io.stopped()||!acceptor.retired()||(acceptor_v6&&!acceptor_v6->retired())||
       accept_pending[0]||accept_pending[1]||!deferred_closes.empty()||
       std::any_of(clients.begin(),clients.end(),[](auto& c){return !c->retired();}))
        throw std::runtime_error("Native socket or IO cleanup incomplete");
    clients.clear();
}
AsioService::AsioService(HandlerFactory& f,execution::Execution& e,std::uint16_t& p,std::uint8_t& r,SessionPolicy policy,bool defer_accept)
    :impl_(std::make_unique<Impl>(f,e,p,r,policy,defer_accept)){}
AsioService::~AsioService()=default;
void AsioService::start(){impl_->start();}
void AsioService::resume_accepts(){impl_->resume_accepts();}
void AsioService::stop(){impl_->stop();}
}
