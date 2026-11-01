#pragma once
#include "../anadius-bootstrap-policies-source/config_platform.hpp"
#include "../anadius-config-storage-source/persistence.hpp"
#include "../anadius-config-logging-source/logging.hpp"
#include "../anadius-config-source/state_binding.hpp"
#include "../anadius-achievement-storage-source/path_binding.hpp"

namespace anadius::configuration_factory {
// Custom license effects remain supported by the legacy constructor. The
// native-key constructor below selects the recovered02a830 source instead.
// Captures must own their backing or be covered by Inputs::lifetime. No legacy
// collection-order decoder or original MSVC ABI is fabricated here.
class LicenseEffects {
public:
    virtual ~LicenseEffects()=default;
    virtual void apply(std::string_view)=0;
};
class LegacyDocumentReader {
public:
    virtual ~LegacyDocumentReader()=default;
    virtual configuration::Node read(const configuration::Path&)=0;
};
struct Inputs {
    // Optional backing owner for supplied SDK/stream/effect captures; declared
    // first so it retires last. Otherwise supplied callbacks must be permanent
    // or own their captures. No implicit process-global profile/path is read.
    std::shared_ptr<void> lifetime;
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<config_storage::Filesystem> filesystem=std::make_shared<config_storage::Filesystem>();
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
    bootstrap_providers::ConfigApi crt=bootstrap_providers::native_config_api();
    bootstrap_policies::ConfigPlatformApi platform=bootstrap_policies::native_config_platform_api();
    config_storage::PersistenceResources persistence;
};
// Concrete ownership/composition of all 21 required methods plus streaming
// visitation. Select recovered license processing or the explicit custom
// license effect; the legacy document reader remains caller supplied. No giant lower
// Dependencies, unreachable-method stub, callback publication or I/O in ctor.
// Configuration calls are serialized; construction may occur on a different
// thread from source startup. Retire after application/worker/callback joins.
// Compiler/SDK source policy, not original MSVC class/EH/loader equivalence.
// The additional StateBinding/PathBinding bases change this class's HOST layout;
// allocating/constructing/deleting/crosscasting consumers must be rebuilt.
// Dependencies and bootstrap Resources retain their existing layouts.
class Configuration final:public configuration::Dependencies,public configuration::StateBinding,
                          public achievement_storage::PathBinding {
public:
    Configuration(Inputs,achievement_storage::Paths,
        std::unique_ptr<LicenseEffects>,std::unique_ptr<LegacyDocumentReader>);
    // Recovered02a830 route, using existing owned persistence files/stats and
    // this provider's path/encoding resources. Requires passive state binding
    // before apply_license_key; SourceBackend performs it automatically.
    Configuration(Inputs,achievement_storage::Paths,std::unique_ptr<LegacyDocumentReader>);
    ~Configuration()override;
    Configuration(const Configuration&)=delete;
    Configuration& operator=(const Configuration&)=delete;
    Configuration(Configuration&&)=delete;
    Configuration& operator=(Configuration&&)=delete;
    const achievement_storage::Paths& paths()const;
    bool accepts_observed_empty_paths()const;
    void bind_configuration_state(configuration::State&)override;
    void bind_achievement_paths(achievement_storage::Paths&)override;
    configuration::Path executable_path_03c6d0()override;
    configuration::Path parent_path_019820(const configuration::Path&)override;
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override;
    configuration::Path widen_0145d0(std::string_view)override;
    configuration::Path filesystem_widen_010540(std::string_view)override;
    std::string narrow_path(const configuration::Path&)override;
    configuration::Path resolve_path_0192d0(std::string_view,const configuration::Path&,bool)override;
    configuration::Path program_data_03c920()override;
    configuration::Path local_app_data_03c6f0()override;
    configuration::Node load_document(const configuration::Path&)override;
    void log_config_path(const configuration::Path&)override;
    bool crt_is_space(unsigned char)override;
    int crt_tolower(int)override;
    void apply_license_key_02a830(std::string_view)override;
    void create_directories_018e00(const configuration::Path&)override;
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng)override;
    std::vector<configuration::Path> directory_entries(const configuration::Path&)override;
    void visit_directory_entries(const configuration::Path&,configuration::DirectoryVisitor)override;
    configuration::Path filename(const configuration::Path&)override;
    void remove_04948c(const configuration::Path&)override;
    void update_achievement_storage_01eb00(configuration::State&)override;
    bool set_environment_variable(std::string_view,std::string_view)override;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& open()const;
};
std::unique_ptr<configuration::Dependencies> make_configuration(Inputs,achievement_storage::Paths,
    std::unique_ptr<LicenseEffects>,std::unique_ptr<LegacyDocumentReader>);
std::unique_ptr<configuration::Dependencies> make_configuration(Inputs,achievement_storage::Paths,
    std::unique_ptr<LegacyDocumentReader>);
// Install only into an empty configuration slot; other Resources stay caller
// supplied. Construction must finish before the slot is committed.
void install_configuration(bootstrap_backend::Resources&,Inputs,achievement_storage::Paths,
    std::unique_ptr<LicenseEffects>,std::unique_ptr<LegacyDocumentReader>);
void install_configuration(bootstrap_backend::Resources&,Inputs,achievement_storage::Paths,
    std::unique_ptr<LegacyDocumentReader>);
} // namespace anadius::configuration_factory
