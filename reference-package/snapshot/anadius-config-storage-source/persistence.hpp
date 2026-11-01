#pragma once
#include "filesystem.hpp"
#include "../anadius-achievement-storage-source/storage.hpp"
#include "../anadius-timestamp-source/timestamp.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include <span>

namespace anadius::config_storage {
// Exact retained embedded PNG payloads, immutable for the containing image.
// Retained PNG extents equal the original length globals; provenance is in
// embedded-png-evidence.json.
std::span<const std::uint8_t> embedded_png(configuration::EmbeddedPng);
struct PersistenceResources {
    std::unique_ptr<file_recovered::Factory> files=std::make_unique<file_recovered::NativeFactory>();
    std::unique_ptr<messages_recovered::XmlBackend> xml=std::make_unique<xml_reader_bridge::Backend>();
    std::unique_ptr<achievement_storage::Numbers> numbers=std::make_unique<achievement_storage::HostCrtNumbers>();
    std::unique_ptr<timestamp_recovered::Parser> parser=std::make_unique<timestamp_recovered::ClassicIso>();
    std::unique_ptr<status_recovered::Stats> stats=std::make_unique<status_recovered::NativeStats>();
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
};
namespace detail {struct PersistenceState;}
// Owned persistence engine with no remaining configuration dependency. Shares
// the exact Storage/Paths implementation with the legacy decorator below.
// Construction performs no I/O; calls require serialized configuration use.
class Persistence final {
public:
    Persistence(std::shared_ptr<Filesystem>,achievement_storage::Paths,PersistenceResources={});
    ~Persistence();
    Persistence(const Persistence&)=delete;
    Persistence& operator=(const Persistence&)=delete;
    Persistence(Persistence&&)=delete;
    Persistence& operator=(Persistence&&)=delete;
    const achievement_storage::Paths& paths() const noexcept;
    // Passive preflight; unlike paths(), this does not expose/borrow cells.
    bool accepts_observed_empty_paths() const noexcept;
    // One passive transfer before use or paths() borrowing. Destination must
    // be empty and outlive this engine. Unbound legacy instances keep ownership.
    void bind_achievement_paths(achievement_storage::Paths&);
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng);
    void update_achievement_storage_01eb00(configuration::State&);
private:
    std::unique_ptr<detail::PersistenceState> impl_;
};
// Two selected routes: embedded PNG output and the existing achievement
// Storage algorithm. Construction performs no I/O. Explicit initial Paths
// replace source globals; they are not guessed from an installed user profile.
// Selected compiler file/XML/numeric/error/ASCII timestamp policies are not
// proof of the original MSVC object/EH ABI or arbitrary locale equivalence.
// Use on the serialized configuration controller. No concurrent mutation,
// callbacks, background writer, implicit flush or rollback is introduced.
class ConfigurationPersistence final:public configuration::Dependencies {
public:
    ConfigurationPersistence(std::shared_ptr<Filesystem>,achievement_storage::Paths,
        std::unique_ptr<configuration::Dependencies>,PersistenceResources={});
    ~ConfigurationPersistence() override;
    ConfigurationPersistence(const ConfigurationPersistence&)=delete;
    ConfigurationPersistence& operator=(const ConfigurationPersistence&)=delete;
    ConfigurationPersistence(ConfigurationPersistence&&)=delete;
    ConfigurationPersistence& operator=(ConfigurationPersistence&&)=delete;
    const achievement_storage::Paths& paths() const noexcept;
    configuration::Path executable_path_03c6d0() override;
    configuration::Path parent_path_019820(const configuration::Path&) override;
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&) override;
    configuration::Path widen_0145d0(std::string_view) override;
    configuration::Path filesystem_widen_010540(std::string_view) override;
    std::string narrow_path(const configuration::Path&) override;
    configuration::Path resolve_path_0192d0(std::string_view,const configuration::Path&,bool) override;
    configuration::Path program_data_03c920() override;
    configuration::Path local_app_data_03c6f0() override;
    configuration::Node load_document(const configuration::Path&) override;
    void log_config_path(const configuration::Path&) override;
    bool crt_is_space(unsigned char) override;
    int crt_tolower(int) override;
    void apply_license_key_02a830(std::string_view) override;
    void create_directories_018e00(const configuration::Path&) override;
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng) override;
    std::vector<configuration::Path> directory_entries(const configuration::Path&) override;
    void visit_directory_entries(const configuration::Path&,configuration::DirectoryVisitor) override;
    configuration::Path filename(const configuration::Path&) override;
    void remove_04948c(const configuration::Path&) override;
    void update_achievement_storage_01eb00(configuration::State&) override;
    bool set_environment_variable(std::string_view,std::string_view) override;
private:
    // The lower policy owner dies first; selected persistence dependencies and
    // their Filesystem remain valid through its destructor.
    struct Impl;std::unique_ptr<Impl> impl_;
    std::unique_ptr<configuration::Dependencies> lower_;
};
} // namespace anadius::config_storage
