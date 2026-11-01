#include "persistence.hpp"
#include "embedded_png_data.hpp"
#include <bit>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <type_traits>

namespace anadius::config_storage {
namespace {
PersistenceResources checked(PersistenceResources r){
    if(!r.files||!r.xml||!r.numbers||!r.parser||!r.stats||!r.encoding||!r.errors)
        throw std::invalid_argument("Complete owned persistence resources are required");
    return r;
}
std::shared_ptr<Filesystem> checked(std::shared_ptr<Filesystem> fs){
    if(!fs)throw std::invalid_argument("Shared filesystem policy is required");
    return fs;
}
class StorageEffects final:public achievement_storage::Effects {
    Filesystem& filesystem_;PersistenceResources& resources_;
public:
    StorageEffects(Filesystem& fs,PersistenceResources& r):filesystem_(fs),resources_(r){}
    void create_directories_018e00(const configuration::Path& p) override{filesystem_.create_directories(p);}
    configuration::Path filesystem_widen_006d70(std::string_view text) override{
        return encoding_recovered::filesystem_widen_006d70(text,*resources_.encoding,*resources_.errors);
    }
    std::int64_t timestamp_to_epoch_041e10(std::string_view text) override{
        return timestamp_recovered::timestamp_to_epoch_041e10(text,*resources_.parser);
    }
    status_recovered::Status status_01ea20(const configuration::Path& p,std::uint32_t flags) override{
        return status_recovered::status_01ea20(p,flags,*resources_.stats);
    }
    [[noreturn]] void filesystem_error_018130(std::string_view operation,std::uint32_t code,const configuration::Path& path) override{
        throw std::filesystem::filesystem_error(std::string(operation),std::filesystem::path(path),
            std::error_code(std::bit_cast<std::int32_t>(code),std::system_category()));
    }
};
}
std::span<const std::uint8_t> embedded_png(configuration::EmbeddedPng value){
    switch(value){
    case configuration::EmbeddedPng::default_user_1800809d0:return png_data::default_user;
    case configuration::EmbeddedPng::default_avatar_180082710:return png_data::default_avatar;
    }
    throw std::invalid_argument("Unknown embedded PNG identity");
}
struct detail::PersistenceState {
    std::shared_ptr<Filesystem> filesystem;
    PersistenceResources resources;
    achievement_storage::Paths owned_paths;
    achievement_storage::Paths* selected_paths=&owned_paths;
    bool paths_bound=false,used=false,paths_exposed=false;
    StorageEffects effects;
    std::optional<achievement_storage::Storage> storage;
    PersistenceState(std::shared_ptr<Filesystem> fs,achievement_storage::Paths initial,PersistenceResources r)
        :filesystem(checked(std::move(fs))),resources(checked(std::move(r))),owned_paths(std::move(initial)),
        effects(*filesystem,resources),storage(std::in_place,*selected_paths,*resources.xml,*resources.numbers,effects,*resources.files){}
    void write_embedded_png(const configuration::Path& path,configuration::EmbeddedPng image);
    void update_achievement_storage(configuration::State& state){used=true;storage->update_01eb00(state);}
};
// Preserve the legacy class's exact opaque member declaration and allocation
// count while sharing all selected state and behavior with the direct engine.
struct ConfigurationPersistence::Impl:detail::PersistenceState {
    using detail::PersistenceState::PersistenceState;
};
Persistence::Persistence(std::shared_ptr<Filesystem> fs,achievement_storage::Paths paths,PersistenceResources resources)
    :impl_(std::make_unique<detail::PersistenceState>(std::move(fs),std::move(paths),std::move(resources))){}
Persistence::~Persistence()=default;
const achievement_storage::Paths& Persistence::paths() const noexcept{impl_->paths_exposed=true;return *impl_->selected_paths;}
bool Persistence::accepts_observed_empty_paths() const noexcept{
    const auto& v=*impl_;
    return !v.paths_bound&&!v.used&&!v.paths_exposed&&v.selected_paths->xml_path.empty()&&v.selected_paths->ini_path.empty();
}
void Persistence::bind_achievement_paths(achievement_storage::Paths& paths){
    auto& v=*impl_;
    if(v.paths_bound||v.used||v.paths_exposed)
        throw std::logic_error("Achievement paths must bind once before use or borrowing");
    if(&paths==&v.owned_paths||!paths.xml_path.empty()||!paths.ini_path.empty())
        throw std::invalid_argument("Shared achievement path destination must be distinct and empty");
    static_assert(std::is_nothrow_move_assignable_v<achievement_storage::Paths>);
    paths=std::move(v.owned_paths);
    // Storage's constructor only binds references; it allocates and calls no
    // opaque provider. The new reference set is installed before publication.
    v.storage.emplace(paths,*v.resources.xml,*v.resources.numbers,v.effects,*v.resources.files);
    v.selected_paths=&paths;v.paths_bound=true;
}
void Persistence::write_embedded_png_03c020(const configuration::Path& path,configuration::EmbeddedPng image){impl_->write_embedded_png(path,image);}
void Persistence::update_achievement_storage_01eb00(configuration::State& state){impl_->update_achievement_storage(state);}
ConfigurationPersistence::ConfigurationPersistence(std::shared_ptr<Filesystem> fs,achievement_storage::Paths paths,
    std::unique_ptr<configuration::Dependencies> lower,PersistenceResources resources)
    :impl_(std::make_unique<Impl>(std::move(fs),std::move(paths),std::move(resources))),lower_(std::move(lower)){
    if(!lower_)throw std::invalid_argument("Remaining configuration policy owner is required");
}
ConfigurationPersistence::~ConfigurationPersistence()=default;
const achievement_storage::Paths& ConfigurationPersistence::paths() const noexcept{impl_->paths_exposed=true;return *impl_->selected_paths;}
void detail::PersistenceState::write_embedded_png(const configuration::Path& path,configuration::EmbeddedPng image){
    used=true;
    const auto bytes=embedded_png(image);
    // Retained03c020 and03c1d0 both use open0x20/share0x40, initial-state
    // guard, write, explicit close. The caller ignores03c020's boolean.
    // Preserve that behavior: initial open failure is not fabricated success
    // and write/close exceptions still propagate; no new post-write check.
    (void)file_recovered::write_binary_03c1d0(path,
        std::string_view(reinterpret_cast<const char*>(bytes.data()),bytes.size()),*resources.files);
}
void ConfigurationPersistence::write_embedded_png_03c020(const configuration::Path& path,configuration::EmbeddedPng image){impl_->write_embedded_png(path,image);}
void ConfigurationPersistence::update_achievement_storage_01eb00(configuration::State& state){impl_->update_achievement_storage(state);}
configuration::Path ConfigurationPersistence::executable_path_03c6d0(){return lower_->executable_path_03c6d0();}
configuration::Path ConfigurationPersistence::parent_path_019820(const configuration::Path& p){return lower_->parent_path_019820(p);}
configuration::Path ConfigurationPersistence::join_016b00(const configuration::Path& p,const configuration::Path& q){return lower_->join_016b00(p,q);}
configuration::Path ConfigurationPersistence::widen_0145d0(std::string_view s){return lower_->widen_0145d0(s);}
configuration::Path ConfigurationPersistence::filesystem_widen_010540(std::string_view s){return lower_->filesystem_widen_010540(s);}
std::string ConfigurationPersistence::narrow_path(const configuration::Path& p){return lower_->narrow_path(p);}
configuration::Path ConfigurationPersistence::resolve_path_0192d0(std::string_view s,const configuration::Path& p,bool r){return lower_->resolve_path_0192d0(s,p,r);}
configuration::Path ConfigurationPersistence::program_data_03c920(){return lower_->program_data_03c920();}
configuration::Path ConfigurationPersistence::local_app_data_03c6f0(){return lower_->local_app_data_03c6f0();}
configuration::Node ConfigurationPersistence::load_document(const configuration::Path& p){return lower_->load_document(p);}
void ConfigurationPersistence::log_config_path(const configuration::Path& p){lower_->log_config_path(p);}
bool ConfigurationPersistence::crt_is_space(unsigned char c){return lower_->crt_is_space(c);}
int ConfigurationPersistence::crt_tolower(int c){return lower_->crt_tolower(c);}
void ConfigurationPersistence::apply_license_key_02a830(std::string_view s){lower_->apply_license_key_02a830(s);}
void ConfigurationPersistence::create_directories_018e00(const configuration::Path& p){lower_->create_directories_018e00(p);}
std::vector<configuration::Path> ConfigurationPersistence::directory_entries(const configuration::Path& p){return lower_->directory_entries(p);}
void ConfigurationPersistence::visit_directory_entries(const configuration::Path& p,configuration::DirectoryVisitor visitor){lower_->visit_directory_entries(p,visitor);}
configuration::Path ConfigurationPersistence::filename(const configuration::Path& p){return lower_->filename(p);}
void ConfigurationPersistence::remove_04948c(const configuration::Path& p){lower_->remove_04948c(p);}
bool ConfigurationPersistence::set_environment_variable(std::string_view k,std::string_view v){return lower_->set_environment_variable(k,v);}
static_assert(!std::is_abstract_v<ConfigurationPersistence>);
static_assert(!std::is_copy_constructible_v<Persistence>);
} // namespace anadius::config_storage
