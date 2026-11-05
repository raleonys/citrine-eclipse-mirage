#include "factory.hpp"
#include "../anadius-license-source/key_processor.hpp"
#include "../anadius-path-source/paths.hpp"
#include "../anadius-config-application-source/application.hpp"
#include "../anadius-config-pipeline-source/pipeline.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::configuration_factory {
namespace core=bootstrap_providers::configuration_routes;
namespace platform=bootstrap_policies::configuration_routes;
namespace {
class ScalarDocuments final:public LegacyDocumentReader {
    std::unique_ptr<file_recovered::Factory> files_;
public:
    explicit ScalarDocuments(std::unique_ptr<file_recovered::Factory> files):files_(std::move(files)){
        if(!files_)throw std::invalid_argument("Owned source document file provider required");
    }
    configuration::Node read(const configuration::Path& path)override{
        auto selected=config_application::read_config2(path,*files_);
        configuration::Node document;
        document.children.emplace("Config2",config_hash::to_configuration(selected));
        document.iteration_order.emplace_back("Config2");
        config_pipeline::require_scalar_application(document);
        return document;
    }
};
// Adopt separate public by-value parameters BEFORE any allocation or rejecting
// validation. Parameter destruction order cannot keep Inputs::lifetime alive
// through borrowed license/document deleters. This aggregate has a fixed order.
struct PendingConfiguration {
    Inputs inputs;
    achievement_storage::Paths paths;
    std::unique_ptr<LicenseEffects> license;
    std::unique_ptr<LegacyDocumentReader> documents;
};
static_assert(std::is_nothrow_move_constructible_v<Inputs>);
static_assert(std::is_nothrow_move_constructible_v<achievement_storage::Paths>);
std::unique_ptr<configuration::Dependencies> build(PendingConfiguration&& pending,bool native_key){
    // make_unique only binds forwarding references before facade allocation;
    // pending still owns everything if that allocation fails.
    if(native_key)return std::make_unique<Configuration>(std::move(pending.inputs),std::move(pending.paths),std::move(pending.documents));
    return std::make_unique<Configuration>(std::move(pending.inputs),std::move(pending.paths),std::move(pending.license),std::move(pending.documents));
}
}
std::unique_ptr<LegacyDocumentReader> make_scalar_documents(std::unique_ptr<file_recovered::Factory> files){
    return std::make_unique<ScalarDocuments>(std::move(files));
}
struct Configuration::Impl {
    bool retiring=false;
    bool recovered_key=false;
    configuration::State* live_state=nullptr;
    file_recovered::Factory* key_files=nullptr;
    status_recovered::Stats* key_stats=nullptr;
    Inputs inputs;
    std::unique_ptr<config_storage::Persistence> persistence;
    // Narrow effects retire first, while Core/stream/filesystem and persistence
    // are still alive. Their captured state must obey the header contract.
    std::unique_ptr<LicenseEffects> license;
    std::unique_ptr<LegacyDocumentReader> documents;
    Impl(PendingConfiguration&& pending,bool native_key)
        :recovered_key(native_key),inputs(std::move(pending.inputs)),license(std::move(pending.license)),documents(std::move(pending.documents)){
        const auto& a=inputs.crt;const auto& p=inputs.platform;const auto& r=inputs.persistence;
        if(!inputs.stream||!inputs.filesystem||!inputs.encoding||!inputs.errors||(!recovered_key&&!license)||!documents||
           !a.space||!a.lower||!a.environment||!p.module_filename||!p.environment||!p.last_error||!p.set_last_error||
           !r.files||!r.xml||!r.numbers||!r.parser||!r.stats||!r.encoding||!r.errors)
            throw std::invalid_argument("Owned configuration resources, selected license policy, legacy reader and complete APIs required");
        // These objects retain their addresses when their unique owners move
        // into Persistence; the engine remains owned until provider retirement.
        key_files=inputs.persistence.files.get();key_stats=inputs.persistence.stats.get();
        persistence=std::make_unique<config_storage::Persistence>(inputs.filesystem,std::move(pending.paths),std::move(inputs.persistence));
    }
};
Configuration::Configuration(Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LicenseEffects> license,std::unique_ptr<LegacyDocumentReader> documents)
    :impl_(nullptr){
    PendingConfiguration pending{std::move(inputs),std::move(paths),std::move(license),std::move(documents)};
    impl_=std::make_unique<Impl>(std::move(pending),false);
}
Configuration::Configuration(Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LegacyDocumentReader> documents):impl_(nullptr){
    PendingConfiguration pending{std::move(inputs),std::move(paths),nullptr,std::move(documents)};
    impl_=std::make_unique<Impl>(std::move(pending),true);
}
Configuration::Impl& Configuration::open()const{
    if(!impl_||impl_->retiring)throw std::logic_error("Configuration factory is unattached or retiring");
    return *impl_;
}
Configuration::~Configuration(){
    if(!impl_)return;
    auto& v=*impl_;v.retiring=true;
    // Keep the guarded facade attached through every opaque destructor. These
    // callbacks may probe it, but cannot restart source work during retirement.
    v.documents.reset();v.license.reset();v.persistence.reset();
    {Inputs retiring=std::move(v.inputs);}
}
const achievement_storage::Paths& Configuration::paths()const{return open().persistence->paths();}
bool Configuration::accepts_observed_empty_paths()const{
    auto& v=open();return !v.live_state&&v.persistence->accepts_observed_empty_paths();
}
void Configuration::bind_configuration_state(configuration::State& state){
    auto& v=open();
    if(v.live_state)throw std::logic_error("Configuration state is already bound");
    v.live_state=&state;
}
void Configuration::bind_achievement_paths(achievement_storage::Paths& paths){
    open().persistence->bind_achievement_paths(paths);
}
configuration::Path Configuration::executable_path_03c6d0(){return platform::executable_path(open().inputs.platform);}
configuration::Path Configuration::parent_path_019820(const configuration::Path& p){(void)open();return path_recovered::parent_path_019820(p);}
configuration::Path Configuration::join_016b00(const configuration::Path& p,const configuration::Path& q){(void)open();return path_recovered::join_016b00(p,q);}
configuration::Path Configuration::widen_0145d0(std::string_view s){return platform::widen_utf8(s,*open().inputs.encoding,*open().inputs.errors);}
configuration::Path Configuration::filesystem_widen_010540(std::string_view s){return encoding_recovered::filesystem_widen_006d70(s,*open().inputs.encoding,*open().inputs.errors);}
std::string Configuration::narrow_path(const configuration::Path& p){return platform::narrow_path(p,*open().inputs.encoding,*open().inputs.errors);}
configuration::Path Configuration::resolve_path_0192d0(std::string_view s,const configuration::Path& p,bool relative){return platform::resolve_path(s,p,relative,*open().inputs.encoding,*open().inputs.errors);}
configuration::Path Configuration::program_data_03c920(){return platform::program_data(open().inputs.platform);}
configuration::Path Configuration::local_app_data_03c6f0(){return platform::local_app_data(open().inputs.platform);}
configuration::Node Configuration::load_document(const configuration::Path& p){return open().documents->read(p);}
void Configuration::log_config_path(const configuration::Path& p){config_logging::log_config_path(*open().inputs.stream,p,*open().inputs.encoding,*open().inputs.errors);}
bool Configuration::crt_is_space(unsigned char c){return core::is_space(open().inputs.crt,c);}
int Configuration::crt_tolower(int c){return core::checked_tolower(open().inputs.crt,c);}
void Configuration::apply_license_key_02a830(std::string_view s){
    auto& v=open();
    if(!v.recovered_key){v.license->apply(s);return;}
    if(!v.live_state)throw std::logic_error("Recovered license processing requires the live Runtime configuration state");
    const auto& state=*v.live_state;
    license_recovered::apply_license_key_02a830(std::string(s),
        {state.license_directory,state.content_id,state.user_id},
        {*v.inputs.encoding,*v.inputs.errors,*v.key_stats,*v.inputs.filesystem,*v.key_files});
}
void Configuration::create_directories_018e00(const configuration::Path& p){open().inputs.filesystem->create_directories(p);}
void Configuration::write_embedded_png_03c020(const configuration::Path& p,configuration::EmbeddedPng i){open().persistence->write_embedded_png_03c020(p,i);}
std::vector<configuration::Path> Configuration::directory_entries(const configuration::Path& p){return open().inputs.filesystem->directory_entries(p);}
void Configuration::visit_directory_entries(const configuration::Path& p,configuration::DirectoryVisitor visitor){open().inputs.filesystem->visit_directory_entries(p,visitor);}
configuration::Path Configuration::filename(const configuration::Path& p){(void)open();return path_recovered::filename_00e120(p);}
void Configuration::remove_04948c(const configuration::Path& p){open().inputs.filesystem->remove(p);}
void Configuration::update_achievement_storage_01eb00(configuration::State& s){open().persistence->update_achievement_storage_01eb00(s);}
bool Configuration::set_environment_variable(std::string_view k,std::string_view v){return core::set_environment(open().inputs.crt,k,v);}
std::unique_ptr<configuration::Dependencies> make_configuration(Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LicenseEffects> license,std::unique_ptr<LegacyDocumentReader> documents){
    PendingConfiguration pending{std::move(inputs),std::move(paths),std::move(license),std::move(documents)};
    return build(std::move(pending),false);
}
std::unique_ptr<configuration::Dependencies> make_configuration(Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LegacyDocumentReader> documents){
    PendingConfiguration pending{std::move(inputs),std::move(paths),nullptr,std::move(documents)};
    return build(std::move(pending),true);
}
void install_configuration(bootstrap_backend::Resources& resources,Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LicenseEffects> license,std::unique_ptr<LegacyDocumentReader> documents){
    PendingConfiguration pending{std::move(inputs),std::move(paths),std::move(license),std::move(documents)};
    if(resources.configuration)throw std::invalid_argument("Configuration Resources slot must be empty");
    auto built=build(std::move(pending),false);
    resources.configuration=std::move(built);
}
void install_configuration(bootstrap_backend::Resources& resources,Inputs inputs,achievement_storage::Paths paths,
    std::unique_ptr<LegacyDocumentReader> documents){
    PendingConfiguration pending{std::move(inputs),std::move(paths),nullptr,std::move(documents)};
    if(resources.configuration)throw std::invalid_argument("Configuration Resources slot must be empty");
    auto built=build(std::move(pending),true);
    resources.configuration=std::move(built);
}
static_assert(!std::is_abstract_v<Configuration>);
static_assert(!std::is_copy_constructible_v<Configuration>&&!std::is_move_constructible_v<Configuration>);
} // namespace anadius::configuration_factory
