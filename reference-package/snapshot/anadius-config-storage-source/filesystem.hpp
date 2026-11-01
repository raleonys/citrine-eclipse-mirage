#pragma once
#include "../anadius-config-source/configuration.hpp"
#include "../anadius-status-source/status.hpp"
#include <memory>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::config_storage {
using Path=configuration::Path;
struct FilesystemApi {
    decltype(&::CreateDirectoryW) create_directory;
    decltype(&::GetFileAttributesExW) attributes_ex;
    decltype(&::FindFirstFileW) find_first;
    decltype(&::FindFirstFileExW) find_first_ex;
    decltype(&::FindNextFileW) find_next;
    decltype(&::FindClose) find_close;
    decltype(&::CreateFileW) create_file;
    decltype(&::GetFileInformationByHandleEx) get_info;
    decltype(&::SetFileInformationByHandle) set_info;
    decltype(&::CloseHandle) close_handle;
    decltype(&::GetLastError) last_error;
    decltype(&::SetLastError) set_last_error;
};
FilesystemApi native_filesystem_api() noexcept;
// Concrete bounded SDK/source policy. The returned collection preserves SDK
// enumeration order, with exact "."/".." omission and no sorting. The selected
// compiler system_error/abort boundaries are not original MSVC STL/EH proof.
// SDK callbacks obey their nonthrowing Win32 outcome contracts. A callback
// throwing during handle acquisition has an unknown ownership result and is
// terminal. Known close failure is terminal. This matches retained owned-handle
// helpers but deliberately strengthens raw049104's ignored status FindClose.
// Methods own only per-call state; concurrent calls require callers' filesystem
// race policy. No recursive deletion, manual reparse traversal or directory
// snapshot atomicity is introduced; ordinary OS path resolution still applies.
// The visitor processes each logical entry before FindNext; its borrowed Path
// expires on return. The vector route explicitly collects the same stream.
class Filesystem final {
public:
    struct Checkpoint {void* context=nullptr;void (*check)(void*)=nullptr;};
    explicit Filesystem(FilesystemApi=native_filesystem_api());
    void create_directories(const Path&);
    void create_directories(const Path&,Checkpoint);
    // Same existing source algorithm, returning the observed SDK error so a
    // native caller can construct its own operation/path exception payload.
    // Allocation/provider exceptions still propagate; no guessed catch/retry.
    std::uint32_t create_directories_error(const Path&);
    std::vector<Path> directory_entries(const Path&);
    void visit_directory_entries(const Path&,configuration::DirectoryVisitor);
    void remove(const Path&);
private:FilesystemApi api_;
};
// B8 supplies exactly three methods of the existing Dependencies interface.
// All other application/license/PNG/storage policies remain owned lower routes.
class ConfigurationFilesystem final:public configuration::Dependencies {
public:
    ConfigurationFilesystem(std::shared_ptr<Filesystem>,std::unique_ptr<configuration::Dependencies>);
    Path executable_path_03c6d0() override;
    Path parent_path_019820(const Path&) override;
    Path join_016b00(const Path&,const Path&) override;
    Path widen_0145d0(std::string_view) override;
    Path filesystem_widen_010540(std::string_view) override;
    std::string narrow_path(const Path&) override;
    Path resolve_path_0192d0(std::string_view,const Path&,bool) override;
    Path program_data_03c920() override;
    Path local_app_data_03c6f0() override;
    configuration::Node load_document(const Path&) override;
    void log_config_path(const Path&) override;
    bool crt_is_space(unsigned char) override;
    int crt_tolower(int) override;
    void apply_license_key_02a830(std::string_view) override;
    void create_directories_018e00(const Path&) override;
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng) override;
    std::vector<Path> directory_entries(const Path&) override;
    void visit_directory_entries(const Path&,configuration::DirectoryVisitor) override;
    Path filename(const Path&) override;
    void remove_04948c(const Path&) override;
    void update_achievement_storage_01eb00(configuration::State&) override;
    bool set_environment_variable(std::string_view,std::string_view) override;
private:
    std::shared_ptr<Filesystem> filesystem_;
    std::unique_ptr<configuration::Dependencies> lower_;
};
} // namespace anadius::config_storage
