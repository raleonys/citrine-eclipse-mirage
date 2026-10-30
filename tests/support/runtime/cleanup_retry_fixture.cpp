#include "preloader_l/runtime/application.hpp"
#include "preloader_l/platform/hook_unload_guard.hpp"
#include <atomic>
#include <stdexcept>
namespace {std::atomic<int> created{},attempts{},destroyed{};std::atomic<std::uint32_t> caller{};}
extern "C" __declspec(dllexport) std::uint32_t FixtureCaller(){return caller;}
extern "C" __declspec(dllexport) int FixtureCreated(){return created;}
extern "C" __declspec(dllexport) int FixtureAttempts(){return attempts;}
extern "C" __declspec(dllexport) int FixtureDestroyed(){return destroyed;}
namespace preloader_l {
void set_shutdown_caller(std::uint32_t id) noexcept {caller=id;} // Test-only scan boundary.
namespace application {
struct Application::Impl {};
Application::Application(Options):impl_(std::make_unique<Impl>()){++created;}
Application::~Application(){++destroyed;}
void Application::start(){} // Fixture deliberately has no sockets/hooks.
void Application::stop(){if(++attempts<3)throw std::runtime_error("Injected cleanup failure");}
const configuration::State& Application::configuration()const {static const configuration::State state{};return state;}
}
}
