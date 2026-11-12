// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/config/application.hpp"
#include "citrine_eclipse_mirage/runtime/bootstrap.hpp"
#include "citrine_eclipse_mirage/runtime/startup.hpp"

namespace citrine_eclipse_mirage::config_startup {
class FilePass final : public configuration::ConfigurationPass {
public:
    explicit FilePass(file::Factory& files):files_(files){}
    void load_and_apply(configuration::State&,bool,configuration::Dependencies&) override;
private:
    file::Factory& files_;
};
void initialize_configuration(configuration::State&,configuration::Dependencies&,file::Factory&);

struct ConsoleFlags {std::uint8_t& show_lsx;std::uint8_t& show_console;std::uint8_t& no_console;};
// Partial abstract bridge: bind actual argument/console/configuration source to
// Bootstrap::initialize. Other BootstrapDependencies methods remain mandatory.
// All borrowed objects must outlive the bridge; no native global defaults exist.
class BootstrapConfiguration : public system_services::BootstrapDependencies {
public:
    BootstrapConfiguration(configuration::State&,configuration::Dependencies&,
        file::Factory&,ConsoleFlags,startup::InputPlatform&,
        startup::ConsolePlatform&);
    void setup_environment_019910() final;
    void setup_console_038760() final;
    void load_configuration_01c870() final;
private:
    startup::StateReferences references() const;
    configuration::State& state_;
    configuration::Dependencies& dependencies_;
    file::Factory& files_;
    ConsoleFlags flags_;
    startup::InputPlatform& input_;
    startup::ConsolePlatform& console_;
};
} // namespace citrine_eclipse_mirage::config_startup
