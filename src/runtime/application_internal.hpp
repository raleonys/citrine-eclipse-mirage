// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/runtime/application.hpp"
#include "preloader_l/codec/encoding.hpp"
#include "preloader_l/core/license_files.hpp"
#include "preloader_l/runtime/achievement_runtime.hpp"
#include "preloader_l/runtime/timestamp.hpp"
#include "preloader_l/runtime/online.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/runtime/event_adapters.hpp"
#include "preloader_l/platform/hook_native_callbacks.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#include "preloader_l/xml/reader.hpp"
#include <filesystem>
#include <initializer_list>
#include <mutex>
#include <array>
#include <system_error>

namespace preloader_l::application::detail {
using Path=configuration::Path;
namespace fs=std::filesystem;
inline std::wstring wide(std::u16string_view p){return {p.begin(),p.end()};}
inline Path utf16(std::wstring_view p){return {p.begin(),p.end()};}
struct NoResponse:std::runtime_error {NoResponse():std::runtime_error("RE no-response") {}};
[[noreturn]] inline void thread_error(int index){
    struct Spec {int code;const char* context;};
    static constexpr std::array<Spec,7> errors{{
        {16,"device or resource busy"},{22,"invalid argument"},{3,"no such process"},
        {12,"not enough memory"},{1,"operation not permitted"},
        {36,"resource deadlock would occur"},{11,"resource unavailable try again"}
    }};
    if(index<0||static_cast<std::size_t>(index)>=errors.size())
        throw std::out_of_range("Thread error index outside supported 0..6 table");
    const auto& value=errors[static_cast<std::size_t>(index)];
    throw std::system_error(value.code,std::generic_category(),value.context);
}

class Mutex final:public content_handlers::Mutex {
    // Standard CRT storage permits detecting same-thread reentry without
    // deadlocking. The public contract is the RE type2, nonrecursive mutex.
    std::recursive_mutex mutex_;
    std::int32_t count_=0;
public:
    int lock_04a6ac() override {
        mutex_.lock();
        if(count_!=0){mutex_.unlock();return 3;}
        count_=1;return 0;
    }
    std::int32_t& count_4c() override{return count_;}
    void unlock_04a6b4() override{--count_;mutex_.unlock();}
    [[noreturn]] void thread_error_04a648(int c) override{thread_error(c);}
};
// RAII queue/mutex owner in place of private CRT guard/atexit machinery. The
// source adapters still determine per-request locking and lazy DOWN seeding.
class Events final:public event_queue::Initialization {
    std::recursive_mutex init_;
    std::int32_t epoch_=1;
public:
    event_queue::Guards guards{0,0};
    Mutex mutex;
    event_queue::OwnedStorage storage;
    event_queue::Context context{guards,*this,mutex,storage};
    event_queue::ContentQueue content{context};
    event_queue::MessageQueue messages{context};
    event_queue::IgoQueue igo{context};
    int thread_epoch() override{return -2147483647;}
    void header_04b838(std::int32_t& guard) override {init_.lock();if(guard==0)guard=-1;else init_.unlock();}
    void footer(std::int32_t& guard) override {guard=epoch_++;init_.unlock();}
    void abort_initialization(std::int32_t& guard) noexcept override {guard=0;init_.unlock();}
    void construct_mutex_04a698(content_handlers::Mutex&,std::uint32_t) override;
    int register_atexit(event_queue::Cleanup) override;
};

class Environment final:public configuration::Dependencies,
                        public achievement_storage::Effects,
                        public handlers::Effects,
                        public achievement_runtime::PersistenceEffects,
                        public content_handlers::AvatarRequest {
public:
    Environment(configuration::State&,Options&,file::NativeFactory&,
                xml_reader_bridge::Backend&,achievement_storage::Paths&,
                achievement_runtime::HostClock&,achievement_runtime::HostFormatter&);
    void configure();
    void log(std::string_view);
    void log(std::initializer_list<std::string_view>);
    void log_number(std::string_view,std::int64_t);
    // Keep the out-of-line logging source APIs, but let production callers
    // discard Debug-only calls and literals without link-time optimization.
    void debug_log(std::string_view value) {
#ifndef NDEBUG
        log(value);
#else
        (void)value;
#endif
    }
    void debug_log(std::initializer_list<std::string_view> values) {
#ifndef NDEBUG
        log(values);
#else
        (void)values;
#endif
    }
    void debug_log_number(std::string_view value,std::int64_t number) {
#ifndef NDEBUG
        log_number(value,number);
#else
        (void)value;(void)number;
#endif
    }
    Path filesystem_widen_006d70(std::string_view) override;
    Path filesystem_widen_010540(std::string_view) override;
    Path widen_0145d0(std::string_view) override;
    Path executable_path_03c6d0() override;
    Path parent_path_019820(const Path&) override;
    Path join_016b00(const Path&,const Path&) override;
    Path filename(const Path&) override;
    Path path_append_016b50(const Path&,const Path&) override;
    Path resolve_path_0192d0(std::string_view,const Path&,bool) override;
    Path program_data_03c920() override;
    Path local_app_data_03c6f0() override;
    std::string narrow_path(const Path&) override;
    std::string path_to_utf8_031ca0(const Path&) override;
    configuration::Node load_document(const Path&) override;
    void log_config_path(const Path&) override;
    bool crt_is_space(unsigned char) override;
    int crt_tolower(int) override;
    void apply_license_key_02a830(std::string_view) override;
    void create_directories_018e00(const Path&) override;
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng) override;
    std::vector<Path> directory_entries(const Path&) override;
    void visit_directory_entries(const Path&,configuration::DirectoryVisitor) override;
    void remove_04948c(const Path&) override;
    void update_achievement_storage_01eb00(configuration::State&) override;
    bool set_environment_variable(std::string_view,std::string_view) override;
    status::Status status_01ea20(const Path&,std::uint32_t) override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::uint32_t,const Path&) override;
    std::int64_t timestamp_to_epoch_041e10(std::string_view) override;
    void log_profile_name_084f48(std::string_view) override;
    std::string current_time_041b60() override;
    std::int32_t atoi_05ee08(std::string_view) override;
    void unknown_entitlement_field(std::string_view) override;
    bool online_avatar_virtual_28(const Path&) override;
    bool request_online_avatar(const Path&,const char*,std::int32_t) override;
    bool copy_overwrite_048d20(const Path&,const Path&) override;
    void log_saved_size_085530(std::int32_t) override;
    online::Object provider=0;
    std::string auth_code;
private:
    configuration::State& state_;
    Options& options_;
    file::NativeFactory& files_;
    achievement_runtime::HostClock& clock_;
    achievement_runtime::HostFormatter& formatter_;
    encoding::NativeCalls encoding_;
    encoding::PortableErrors errors_;
    status::NativeStats stats_;
    achievement_storage::HostCrtNumbers numbers_;
    timestamp::HostGetTime timestamp_;
    achievement_storage::Storage storage_;
#ifndef NDEBUG
    std::mutex log_mutex_;
    std::vector<std::string> pending_logs_;
#endif
};
}
