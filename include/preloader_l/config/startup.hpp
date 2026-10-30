#pragma once
#include "preloader_l/config/application.hpp"
#include "preloader_l/runtime/bootstrap.hpp"
#include "preloader_l/runtime/startup.hpp"

namespace preloader_l::config_startup {
class FilePass final : public configuration::ConfigurationPass {
public:
    explicit FilePass(file_recovered::Factory& files):files_(files){}
    void load_and_apply(configuration::State&,bool,configuration::Dependencies&) override;
private:
    file_recovered::Factory& files_;
};
void initialize_configuration(configuration::State&,configuration::Dependencies&,file_recovered::Factory&);

struct ConsoleFlags {std::uint8_t& show_lsx;std::uint8_t& show_console;std::uint8_t& no_console;};
// Partial abstract bridge: bind actual argument/console/configuration source to
// Bootstrap::initialize. Other BootstrapDependencies methods remain mandatory.
// All borrowed objects must outlive the bridge; no native global defaults exist.
class BootstrapConfiguration : public recovered::BootstrapDependencies {
public:
    BootstrapConfiguration(configuration::State&,configuration::Dependencies&,
        file_recovered::Factory&,ConsoleFlags,startup_recovered::InputPlatform&,
        startup_recovered::ConsolePlatform&);
    void setup_environment_019910() final;
    void setup_console_038760() final;
    void load_configuration_01c870() final;
private:
    startup_recovered::StateReferences references() const;
    configuration::State& state_;
    configuration::Dependencies& dependencies_;
    file_recovered::Factory& files_;
    ConsoleFlags flags_;
    startup_recovered::InputPlatform& input_;
    startup_recovered::ConsolePlatform& console_;
};
} // namespace preloader_l::config_startup
