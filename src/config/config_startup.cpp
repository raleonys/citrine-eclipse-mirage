// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/startup.hpp"

namespace preloader_l::config_startup {
void FilePass::load_and_apply(configuration::State& state,bool override_pass,configuration::Dependencies& deps) {
    config_application::load_and_apply(state,override_pass,deps,files_);
}
void initialize_configuration(configuration::State& state,configuration::Dependencies& deps,file::Factory& files) {
    FilePass passes(files);
    configuration::initialize_configuration(state,deps,passes);
}
BootstrapConfiguration::BootstrapConfiguration(configuration::State& state,configuration::Dependencies& deps,
    file::Factory& files,ConsoleFlags flags,startup::InputPlatform& input,
    startup::ConsolePlatform& console)
    :state_(state),dependencies_(deps),files_(files),flags_(flags),input_(input),console_(console) {}
startup::StateReferences BootstrapConfiguration::references() const {
    return {state_.language_from_arguments,state_.registry_spoof,
        flags_.show_lsx,flags_.show_console,flags_.no_console,state_.game_attributes};
}
void BootstrapConfiguration::setup_environment_019910() {
    startup::setup_environment_019910(references(),input_);
}
void BootstrapConfiguration::setup_console_038760() {
    startup::setup_console_038760(references(),console_);
}
void BootstrapConfiguration::load_configuration_01c870() {
    config_startup::initialize_configuration(state_,dependencies_,files_);
}
} // namespace preloader_l::config_startup
