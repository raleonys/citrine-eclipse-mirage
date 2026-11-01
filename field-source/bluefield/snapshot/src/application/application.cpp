// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/application.hpp"
#include "src/application/providers.hpp"
#include "src/application/events.hpp"
#include "src/application/avatars.hpp"
#include "src/paths/module_relative.hpp"
#include "src/configuration/apply.hpp"
#include "src/configuration/defaults.hpp"
#include "src/protocol/router.hpp"
#include "src/xml/reader.hpp"
#include "src/sessions/handler.hpp"
#include "src/crypto/crypto.hpp"
#include "src/files/license_files.hpp"
#include "src/runtime/startup.hpp"
#include "src/profiles/online_session.hpp"
#include "src/licensing/key_store.hpp"
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <utility>
#include <type_traits>
#ifdef _WIN32
#include <windows.h>
#endif
namespace preloader_l {
namespace {
class ResponsePolicy final:public presence::NoResponse {
 [[noreturn]] void throw_no_response_02bcd0_04d228() override{throw preloader_l::NoResponse{};}
};
#ifdef _WIN32
class Console final:public startup::NativeConsolePlatform {
 bool acquired_=false;
public:
 bool attach_console(std::uint32_t id) override{bool okay=startup::NativeConsolePlatform::attach_console(id);acquired_|=okay;return okay;}
 bool allocate_console() override{bool okay=startup::NativeConsolePlatform::allocate_console();acquired_|=okay;return okay;}
 std::uintptr_t stream_052238(unsigned n) override{if(n>2)throw std::invalid_argument("console stream");return reinterpret_cast<std::uintptr_t>(n==0?stdin:n==1?stdout:stderr);}
 int reopen_05f024(std::uintptr_t output,const char* file,const char* mode,std::uintptr_t input) override{if(output!=input)throw std::invalid_argument("console stream alias");FILE* result=nullptr;return freopen_s(&result,file,mode,reinterpret_cast<FILE*>(input));}
 ~Console(){if(acquired_)FreeConsole();}
};
#endif
class MessageEffects final:public messages::Effects {
 dispatch::Router& router_;Environment& env_;
public:
 MessageEffects(dispatch::Router& r,Environment& e):router_(r),env_(e){}
 int atoi_05ee08(std::string_view s) override{return std::atoi(std::string(s).c_str());}
 bool registered_handler(uint32_t rva,const messages::XmlNode& n,messages::XmlPrinter& p) override{return router_.registered_handler(rva,n,p);}
 [[noreturn]] void invalid_message_02bd30_04d228() override{throw SessionFailure("invalid LSX request");}
 [[noreturn]] void thread_error_04a648(int n) override{throw SessionFailure("message lock error "+std::to_string(n));}
 void log_received(std::string_view) override{env_.log(LogEvent::network_receive);}
 void log_response(std::string_view) override{env_.log(LogEvent::network_send);}
 void log_event(std::string_view) override{env_.log(LogEvent::network_event);}
 void log_bad_xml(std::string_view,std::string_view) override{env_.log(LogEvent::network_parse);}
 void log_decryption_failure(std::string_view) override{env_.log(LogEvent::network_crypto);}
};

}
struct Application::Impl {
 Environment& env;configuration::State state=configuration::initial_state();std::recursive_mutex state_mutex;std::mutex provider_mutex;
 std::uint8_t online=0,show_lsx=0,show_console=0,no_console=0;
 OnlineSession online_session;
 AvatarStore avatars{env.paths};achievement_storage::Paths achievement_paths;files::NativeFactory files{env.paths};
 xml_reader::Backend xml;achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
 timestamps::HostGetTime timestamp_parser;StorageEffects storage_effects{timestamp_parser,env.paths};
 achievement_storage::HostCrtNumbers numbers;
 achievement_storage::Storage storage{achievement_paths,xml,numbers,storage_effects,files};
 ConfigurationEffects configuration_effects;
 ContentEffects content_effects;
 InformationEffects information_effects;
 LicenseEffects license_effects;
 files::NativeTextEnvironment license_text;files::LicenseFiles license_files;
 handlers::Profile profile;
 std::string auth_code;std::int32_t attempted_files=0,saved_avatar=0;std::uint8_t license_blocked=0;
#ifdef _WIN32
 Console console;
#endif
 OwnedMutex achievements_mutex;Events events;ResponsePolicy response_policy;
 std::unique_ptr<licensing::Handlers> licensing_handlers;std::unique_ptr<dispatch::Router> router;
 bool initialized=false,ready=false,online_initialized=false;
 explicit Impl(Environment& e):env(e),online_session(state,e,online),configuration_effects(state,e,storage),content_effects(state,achievement_paths,clock,formatter,storage_effects,files,e,online_session,avatars),information_effects(e,clock,formatter),license_effects(e,clock,formatter,online_session),license_files(license_effects,files,license_text){}
 void apply(const std::filesystem::path& file,bool override_pass){
  // Absence is an optional overlay, not a startup prerequisite. Existing but
  // unreadable or malformed files remain errors; do not mask an I/O failure.
  std::error_code error;const auto status=std::filesystem::status(file,error);
  const bool absent=status.type()==std::filesystem::file_type::not_found;
  if(absent){
   env.log(override_pass?LogEvent::config_override_absent:LogEvent::config_base_absent);
  }
  if(error&&!absent)throw std::filesystem::filesystem_error("configuration status",file,error);
  if(!absent&&!std::filesystem::is_regular_file(status))throw std::invalid_argument("configuration path is not a regular file");
  if(!absent)env.log(override_pass?LogEvent::config_override_present:LogEvent::config_base_present);
  auto bytes=absent?std::string{}:read_file(model_path(file),true);
  parser::State parsed;(void)parser::parse(bytes,parsed);
  const auto* selected=parsed.root.find("Config2");
  auto config=selected?config_hash::selected_copy(*selected):config_hash::Node{};
  if(!absent&&!selected)env.log(LogEvent::phase);
  // SURGE/HF19 exposes this policy; the adopted Anadius application does not.
  // Surface the mismatch without inventing a mapping to authenticated online
  // state, FakeAuth or PretendConnected.
  if(const auto* emulator=config.find("Emulator");emulator&&emulator->find("Online"))
   env.log(LogEvent::config_unsupported);
  config_application::apply_config2(state,config,override_pass,configuration_effects);
  if(!absent)env.log(LogEvent::phase);
  if(!absent)env.log(LogEvent::phase);
 }
 void initialize(){
  if(initialized)throw std::logic_error("application already initialized");
  state.executable_path=model_path(env.executable);
#ifdef _WIN32
  startup::NativeInputPlatform input;
  startup::StateReferences startup_state{state.language_from_arguments,state.registry_spoof,show_lsx,show_console,no_console,state.game_attributes};
  startup::setup_environment_019910(startup_state,input);
  startup::setup_console_038760(startup_state,console);
#endif
  class Pass final:public configuration::ConfigurationPass {
   Impl& app_;
  public:
   explicit Pass(Impl& app):app_(app){}
   void load_and_apply(configuration::State&,bool override_pass,configuration::Dependencies&) override{
    if(override_pass)app_.apply(app_.env.paths.override_config(),true);
    else app_.apply(app_.env.configuration_file,false);
   }
  } passes(*this);
  configuration::apply_configuration_layers(state,configuration_effects,passes);
  profile={state.username,state.persona_id,state.user_id,state.country,state.currency,state.subscriber_level,state.is_subscriber,state.is_under_age,state.is_steam_subscriber};
  licensing_handlers=std::make_unique<licensing::Handlers>(licensing::StateReferences{auth_code,online,state.fake_auth,state.denuvo_token,state.dll_hash,state.exe_hash,state.content_id,state.denuvo_dll,state.executable_path,state.license_directory,state.game_attributes,attempted_files,license_blocked},license_files);
  router=std::make_unique<dispatch::Router>(handlers::StateReferences{state.game_attributes,state.emulator_attributes,profile,online,state.pretend_connected},achievement_runtime::bind_state(state,achievement_paths,online,saved_avatar,achievements_mutex,events),dispatch::Dependencies{xml,information_effects,events,response_policy,content_effects,*licensing_handlers});
  configuration::publish_configuration_environment(state,configuration_effects);
  initialized=true;ready=true;env.log(LogEvent::phase);
 }
 void initialize_online(){
  std::lock_guard provider_lock(provider_mutex);
  {std::lock_guard lock(state_mutex);if(!initialized||online_initialized||!ready)throw std::logic_error("online stage lifecycle");}
  auto result=online_session.initialize();
  std::lock_guard lock(state_mutex);
  // All potentially allocating profile construction precedes publication.
  handlers::Profile next{result.username,result.persona_id,result.user_id,state.country,state.currency,state.subscriber_level,state.is_subscriber,state.is_under_age,state.is_steam_subscriber};
  static_assert(std::is_nothrow_move_assignable_v<handlers::Profile>);
  state.username.swap(result.username);state.persona_id.swap(result.persona_id);state.user_id.swap(result.user_id);
  profile=std::move(next);online=result.online;online_initialized=true;
  // Existing explicit avatar selection remains authoritative.
  env.log(online?LogEvent::profile_online:LogEvent::profile_local);
 }
 void shutdown(){std::lock_guard provider_lock(provider_mutex);std::lock_guard lock(state_mutex);ready=false;online_session.close();}
};
struct Connection::Impl {
 Application::Impl& app;std::mutex mutex;std::uint8_t encrypted=0,unknown=0;
 crypto::Cipher cipher;crypto::CryptoFactory crypto_factory;crypto::WindowsTimeSource time;crypto::Random random{time};
 MessageEffects effects;sessions::MessageHandler handler;
 explicit Impl(Application::Impl& a):app(a),effects(*a.router,a.env),handler({encrypted,unknown,a.show_lsx,a.state.sdk,cipher,a.events},a.xml,crypto_factory,random,effects) {handler.reset_02bc70();}
};
Application::Application(Environment& e):impl_(std::make_unique<Impl>(e)){}
Application::~Application(){impl_->shutdown();}
void Application::initialize(){impl_->initialize();}
void Application::initialize_online(){impl_->initialize_online();}
void Application::shutdown(){impl_->shutdown();}
configuration::State& Application::configuration(){return impl_->state;}
std::unique_ptr<Connection> Application::connect(){std::lock_guard lock(impl_->state_mutex);if(!impl_->ready)throw std::logic_error("application not ready");return std::make_unique<Connection>(*this);}
Connection::Connection(Application& app):impl_(std::make_unique<Impl>(*app.impl_)){}
Connection::~Connection()=default;
std::string Connection::greeting(){std::scoped_lock lock(impl_->mutex,impl_->app.state_mutex);if(!impl_->app.ready)throw SessionFailure("application stopped");return impl_->handler.initial_message_02d540();}
std::string Connection::process(std::string_view message){
 std::scoped_lock lock(impl_->mutex,impl_->app.state_mutex);
 if(!impl_->app.ready)throw SessionFailure("application stopped");
 try{return impl_->handler.process_message_033d70(message);}catch(const NoResponse&){return {};}catch(const PersistenceRecoveryRequired&){impl_->app.ready=false;throw;}
}
}
