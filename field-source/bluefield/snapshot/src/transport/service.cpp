// SPDX-License-Identifier: GPL-3.0-only
#include "src/transport/service.hpp"
#include "src/application/providers.hpp"
#include "src/transport/read_window.hpp"
#include "src/runtime/startup_steps.hpp"
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/thread_pool.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/post.hpp>
#include <asio/write.hpp>
#if defined(_WIN32) && (!defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP))
#error Windows production transport requires the real Asio IOCP backend.
#endif
#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <stdexcept>
namespace preloader_l {
struct Service::Impl {
 using Tcp=asio::ip::tcp;
 struct Session;
 using Guard=asio::executor_work_guard<asio::io_context::executor_type>;
 Environment& env;
 Application app; // Outlives io handlers, sockets, sessions and worker tasks.
 std::unique_ptr<asio::io_context> io;
 std::unique_ptr<Tcp::acceptor> listener;
 std::unique_ptr<asio::thread_pool> handlers;
 std::unique_ptr<Guard> work;
 std::thread io_thread,provider_thread,extra_thread;
 std::vector<std::shared_ptr<Session>> clients; // IO thread only; host after joins.
 std::mutex lifecycle_mutex,ready_mutex;
 StartupSteps steps;
 std::exception_ptr startup_failure;
 std::atomic<bool> ready{false},stopping{false};
 bool prepared=false,started=false,activated=false;
 std::uint16_t bound_port=0;

 explicit Impl(Environment& e):env(e),app(e){}
 void retire(Session* session);
 void accept_next();
 struct Session:std::enable_shared_from_this<Session> {
  Impl& owner;
  Tcp::socket socket;
  std::unique_ptr<Connection> connection;
  std::array<char,65536> buffer{};
  std::string pending;
  std::deque<std::string> requests,output;
  bool computing=false,greeting_pending=false; // IO executor only.
  bool closed=false; // IO thread, or host only after all threads have joined.
  std::atomic<bool> failed{false};
  Session(Impl& o,Tcp::socket accepted):owner(o),socket(std::move(accepted)),connection(o.app.connect()){}
  void finish() noexcept {
   if(closed)return;
   closed=true;
   asio::error_code ignored;socket.close(ignored);
   owner.retire(this);
  }
  void start(){compute({},true);}
  void queue_response(std::string response){
   output.push_back(std::move(response));
   if(output.size()==1)write();
  }
  void write(){
   if(closed||owner.stopping.load()||failed.load()){finish();return;}
   auto self=shared_from_this();
   asio::async_write(socket,asio::buffer(output.front()),[self](asio::error_code error,std::size_t count){
    try{
     if(self->closed)return;
     if(error||count!=self->output.front().size()||self->owner.stopping.load()||self->failed.load()){self->finish();return;}
     self->output.pop_front();
     // Preserve greeting-before-read order with cancellable asynchronous IO.
     if(self->greeting_pending){self->greeting_pending=false;self->consume();}
     if(!self->output.empty())self->write();
    }catch(...){self->owner.env.log(LogEvent::dispatch_failed);self->finish();}
   });
  }
  void compute_next(){
   if(computing||requests.empty()||closed||owner.stopping.load()||failed.load())return;
   compute(std::move(requests.front()),false);
   requests.pop_front();computing=true;
  }
  void consume(){
   if(closed||owner.stopping.load()||failed.load()){finish();return;}
   for(;;){
    const auto delimiter=pending.find('\0');
    if(delimiter==std::string::npos){read();return;}
    std::string message=pending.substr(0,delimiter);
    if(!message.empty()){requests.push_back(std::move(message));compute_next();}
    pending.erase(0,delimiter+1);
   }
  }
  void fail_compute(){
   failed.store(true,std::memory_order_release);
   owner.env.log(LogEvent::failure);
   try{auto self=shared_from_this();asio::post(*owner.io,[self]{self->finish();});}
   catch(...){
    // No socket access from a worker now that receive/write may be pending.
    // If cleanup cannot be posted, stop dispatch and let joined host cleanup
    // close the sockets; never leave a falsely ready stranded listener.
    {std::lock_guard lock(owner.ready_mutex);owner.stopping.store(true);owner.ready.store(false);}
    owner.io->stop();owner.steps.cancel();owner.env.log(LogEvent::dispatch_failed);
   }
  }
  void compute(std::string message,bool greeting){
    auto self=shared_from_this();
    // Ordered per-session computation retains cipher/request ownership while
    // receive admission and queued response writes progress independently.
    asio::post(*owner.handlers,[self,message=std::move(message),greeting]{
     try{
      if(self->owner.stopping.load())return;
      auto response=greeting?self->connection->greeting():self->connection->process(message);
      asio::post(*self->owner.io,[self,response=std::move(response),greeting]() mutable {
       try{
        if(self->closed||self->owner.stopping.load()||self->failed.load()){self->finish();return;}
        if(greeting){
         if(response.empty())self->consume();
         else{self->greeting_pending=true;self->queue_response(std::move(response));}
        }else{
         self->computing=false;
         if(!response.empty())self->queue_response(std::move(response));
         self->compute_next();
        }
       }catch(...){self->owner.env.log(LogEvent::failure);self->finish();}
      });
     }catch(const PersistenceRecoveryRequired&){
      {std::lock_guard lock(self->owner.ready_mutex);self->owner.stopping.store(true);self->owner.ready.store(false);}
      self->owner.io->stop();self->owner.steps.cancel();self->owner.env.log(LogEvent::failure);
     }catch(...){
      self->fail_compute();
     }
    });
  }
  void read(){
   if(closed||owner.stopping.load()||failed.load()){finish();return;}
   const auto count=server::receive_request_size(pending.size(),pending.capacity(),pending.max_size());
   if(!count){owner.env.log(LogEvent::phase);finish();return;}
   auto self=shared_from_this();
   socket.async_read_some(asio::buffer(buffer.data(),std::min(count,buffer.size())),
    [self](asio::error_code error,std::size_t count){
     try{
      // Retained source ordering: commit delivered bytes before testing the
      // completion error, but do not dispatch a frame from an error completion.
      if(count)self->pending.append(self->buffer.data(),count);
      if(error||count==0||self->closed||self->owner.stopping.load()||self->failed.load()){self->finish();return;}
      self->consume();
     }catch(...){self->owner.env.log(LogEvent::failure);self->finish();}
    });
  }
 };
 void prepare(){
  std::lock_guard lifecycle(lifecycle_mutex);
  if(prepared||stopping.load())throw std::logic_error("service prepare lifecycle");
  app.initialize();
  io=std::make_unique<asio::io_context>();
  listener=std::make_unique<Tcp::acceptor>(*io);
  listener->open(Tcp::v4());listener->set_option(Tcp::acceptor::reuse_address(true));
  listener->bind(Tcp::endpoint(asio::ip::address_v4::loopback(),app.configuration().server_port));
  listener->listen(asio::socket_base::max_listen_connections);
  bound_port=listener->local_endpoint().port();app.configuration().server_port=bound_port;
  publish_environment("EALsxPort",std::to_string(bound_port));
  prepared=true;steps.finish(StartupSteps::Step::server);
  // Constructors may allocate: intentionally not noexcept. Partial activation
  // uses the ordinary stop path and cannot escape a background thread entry.
  handlers=std::make_unique<asio::thread_pool>(2);
  work=std::make_unique<Guard>(io->get_executor());
  accept_next();
  io_thread=std::thread([this]{
   try{io->run();}
   catch(...){
    // Dispatch can throw before our accept callback gets a chance to rearm.
    // Continuing run() could otherwise leave a false-ready stranded listener.
    {std::lock_guard lock(ready_mutex);stopping.store(true);ready.store(false);}
    asio::error_code ignored;listener->close(ignored);io->stop();
    steps.cancel();env.log(LogEvent::dispatch_failed);
   }
   if(stopping.load()){asio::error_code ignored;listener->close(ignored);}
  });
  started=true;env.log(LogEvent::listener_started);
 }
 void request_stop(){
  {std::lock_guard lock(ready_mutex);stopping.store(true,std::memory_order_release);ready.store(false);}
  steps.cancel();
 }
 void fail_startup(){
  {std::lock_guard lock(ready_mutex);if(!startup_failure)startup_failure=std::current_exception();}
  request_stop();env.log(LogEvent::failure);
 }
 void activate_async(std::function<void(bool)> extra_loader){
  std::lock_guard lifecycle(lifecycle_mutex);
  if(!prepared||!started||activated||stopping.load())throw std::logic_error("service activation lifecycle");
  activated=true;
  provider_thread=std::thread([this]{
   try{
    if(!steps.wait(1))return;
    app.initialize_online();
    steps.finish(StartupSteps::Step::provider);
   }catch(...){fail_startup();}
  });
  // Source true phase is synchronous with bootstrap; no wait/third flag.
  if(extra_loader)extra_loader(true);
  extra_thread=std::thread([this,load=std::move(extra_loader)]{
   try{
    if(!steps.wait(2))return;
    if(load)load(false);
    // Publish readiness before waking successful all-step waiters. Failure
    // and stop use the same mutex, so they cannot leave false-ready state.
    std::lock_guard lock(ready_mutex);
    if(stopping.load())return;
    ready.store(true,std::memory_order_release);
    steps.finish(StartupSteps::Step::extra_worker);
   }catch(...){fail_startup();}
  });
 }
 void wait_startup(){
  if(steps.wait(-1))return;
  std::exception_ptr failure;
  {std::lock_guard lock(ready_mutex);failure=startup_failure;}
  if(failure)std::rethrow_exception(failure);
  throw SessionFailure("initialization stopped");
 }
 void stop(){
  std::lock_guard lifecycle(lifecycle_mutex);
  request_stop();
  if(io)io->stop();
  if(io_thread.joinable())io_thread.join();
  // No IO callbacks now execute. Worker tasks may post completion ownership,
  // so the io_context remains alive until the pool has completely drained.
  if(handlers){handlers->stop();handlers->join();handlers.reset();}
  asio::error_code ignored;
  if(listener)listener->close(ignored);
  for(auto& client:clients){client->closed=true;client->socket.close(ignored);}
  clients.clear();
  // No request worker can use the provider now. Close sockets before joins
  // so provider/extra workers waiting for local IO can observe cancellation.
  if(provider_thread.joinable())provider_thread.join();
  if(extra_thread.joinable())extra_thread.join();
  work.reset();listener.reset();
  // Destroy undelivered/cancelled completion closures before Application.
  io.reset();app.shutdown();
  if(started)env.log(LogEvent::stopped);started=false;
 }
 ~Impl(){stop();}
};
void Service::Impl::retire(Session* session){
 clients.erase(std::remove_if(clients.begin(),clients.end(),[&](const auto& p){return p.get()==session;}),clients.end());
}
void Service::Impl::accept_next(){
 if(stopping.load())return;
 listener->async_accept([this](asio::error_code error,Tcp::socket socket){
  if(error&&!stopping.load()){
   env.log(LogEvent::phase);
   // Source accept_completion_025720 logs the completion error and rearms
   // accept. Keep terminal dispatch/rearm exceptions on their existing paths.
  }
  try{
   clients.erase(std::remove_if(clients.begin(),clients.end(),[](const auto& p){return p->failed.load(std::memory_order_acquire);}),clients.end());
   if(!error&&!stopping.load()){
    auto client=std::make_shared<Session>(*this,std::move(socket));
    clients.push_back(client);
    try{client->start();}catch(...){client->finish();throw;}
   }
  }catch(...){env.log(LogEvent::failure);}
  if(!stopping.load()){
   try{accept_next();}
   catch(...){
    // A failed accept rearm is a visible service failure, never success or an
    // unbounded allocation retry. Existing module ownership remains with host.
    env.log(LogEvent::accept_rearm_failed);
    asio::error_code ignored;listener->close(ignored);
    {std::lock_guard lock(ready_mutex);stopping.store(true);ready.store(false);}
    steps.cancel();
   }
  }
 });
}
Service::Service(Environment& e):impl_(std::make_unique<Impl>(e)){}
Service::~Service()=default;
void Service::start(){try{impl_->prepare();activate();}catch(...){impl_->stop();throw;}}
void Service::prepare(){try{impl_->prepare();}catch(...){impl_->request_stop();throw;}}
void Service::activate(){activate_async({});impl_->wait_startup();}
void Service::activate_async(std::function<void(bool)> load){try{impl_->activate_async(std::move(load));}catch(...){impl_->request_stop();throw;}}
bool Service::wait_steps(std::int32_t count){return impl_->steps.wait(count);}
void Service::wait_startup(){impl_->wait_startup();}
void Service::request_stop(){impl_->request_stop();}
void Service::wait_ready_or_stopping(){(void)impl_->steps.wait(-1);}
void Service::stop(){impl_->stop();}
bool Service::is_ready() const noexcept{return impl_->ready.load(std::memory_order_acquire);}
std::uint16_t Service::port() const{return impl_->bound_port;}
Application& Service::application(){return impl_->app;}
}
