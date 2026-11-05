#pragma once
#include "factory.hpp"
#include "../anadius-configuration-factory-source/factory.hpp"

namespace anadius::resources_factory {
// Passive selection of the existing concrete native-key configuration and
// compiler-console providers. Legacy document policy remains explicitly owned;
// actual Runtime FilePass uses recovered parser/hash/file code instead.
struct ConfigurationInputs {
    ConfigurationInputs()=default;
    ConfigurationInputs(const ConfigurationInputs&)=delete;ConfigurationInputs& operator=(const ConfigurationInputs&)=delete;
    ConfigurationInputs(ConfigurationInputs&&)=delete;ConfigurationInputs& operator=(ConfigurationInputs&&)=delete;
    std::shared_ptr<shared_log::Stream> stream;
    configuration_factory::Inputs configuration;
    // Destroy the narrow reader before configuration/backing on failed input
    // construction as well as after ownership is transferred to Configuration.
    std::unique_ptr<configuration_factory::LegacyDocumentReader> documents;
    bootstrap_providers::ConsoleApi console=bootstrap_providers::native_console_api();
    std::array<FILE*,3> streams=bootstrap_providers::compiler_streams();
private:
    bool consumed_=false;
    friend void install_configuration_console(bootstrap_backend::Resources&,ConfigurationInputs&);
};
// Controller before publication. Empty configuration/console slots only;
// validate complete settings before consuming either owning input. Builds both
// providers before committing. Selected Paths start empty and remain unexposed
// until actual Backend PathBinding. Construction has no I/O/UI/stream mutation.
// Compiler-console FILE policy is not original MSVC FILE/object/CRT ABI.
void install_configuration_console(bootstrap_backend::Resources&,ConfigurationInputs&);
// Concrete image startup selection using native configuration/persistence/SDK,
// compiler console/FILE identities and the real scalar legacy reader. The
// Runtime FilePass still runs full recovered file/parser/reused hash-copy code.
// Empty target slots and the image's managed stream are required. Construction
// performs no configuration read, console setup or application publication.
void install_native_configuration_console(bootstrap_backend::Resources&,
    std::shared_ptr<shared_log::Stream>);
}
