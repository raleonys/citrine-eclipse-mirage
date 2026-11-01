#pragma once
#include "src/configuration/state.hpp"
#include "src/handlers/information.hpp"
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <vector>

namespace preloader_l::content {
using Xml = handlers::Xml;
using Path = configuration::Path;
using Achievement = configuration::Achievement;
using AchievementMap = std::map<std::string,Achievement,std::less<>>;

// Decoded tinyxml2 view. Adapter preserves attribute/document order, absence,
// empty text and case. No file or XML parsing happens in this module.
struct ChildText { std::optional<std::string> name, text; };
struct RequestTree {
    std::vector<std::pair<std::string,std::string>> attributes;
    std::vector<ChildText> children;
    const std::string* attribute(std::string_view) const;
};
struct Filters {
    std::set<std::string,std::less<>> offers, groups, items;
    bool empty() const;
};
class Mutex {
public:
    virtual ~Mutex() = default;
    virtual int lock_04a6ac() = 0;
    virtual std::int32_t& count_4c() = 0;
    virtual void unlock_04a6b4() = 0;
    [[noreturn]] virtual void thread_error_04a648(int) = 0;
};
class DocumentWriter {
public:
    virtual ~DocumentWriter() = default;
    virtual Xml& xml() = 0;
    // Original copies XMLPrinter's CStr with strlen, excluding its NUL.
    virtual std::string c_string_without_nul() const = 0;
};
class PendingEvent {public:virtual ~PendingEvent()=default;virtual void commit() noexcept=0;};
class EventQueue {
public:
    virtual ~EventQueue() = default;
    virtual Mutex& mutex_02c620() = 0;
    virtual bool initialized_locked() const = 0;
    virtual void initialize_locked(std::string initial_igo_event) = 0;
    virtual void push_locked_034660(std::string) = 0;
    virtual std::unique_ptr<PendingEvent> prepare_event(std::string initial,std::string payload) = 0;
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::int32_t atoi_05ee08(std::string_view) = 0;
    // Supplies observed logging + UNKNOWN MESSAGE dialog, then continues.
    virtual void unknown_entitlement_field(std::string_view diagnostic) = 0;
    // 041b60 time(nullptr)/gmtime + ISO UTC formatting; failure: empty/0.
    virtual std::string utc_time_041b60(std::uint64_t& timestamp_field70) = 0;
    virtual std::unique_ptr<DocumentWriter> printer_0019b0() = 0;
    virtual void write_xml_03c1d0(const Path&,std::string_view) = 0;
    // Separate 01f9a0 Steam persistence subsystem, mandatory; not a no-op.
    virtual void save_steam_01f9a0(const AchievementMap&) = 0;
    virtual void persist_achievement(const Path& path,std::string_view xml,const AchievementMap& map){if(!path.empty())write_xml_03c1d0(path,xml);save_steam_01f9a0(map);}
    // 006d70 uses __std_fs_code_page(), not a guessed UTF-8 conversion.
    virtual Path filesystem_widen_006d70(std::string_view) = 0;
    virtual Path path_append_016b50(const Path&,const Path&) = 0;
    virtual Path avatar_cache_path(const Path&,std::string_view owner,std::string_view width)=0;
    virtual Path resolve_image(const Path& value){return value;}
    virtual bool online_avatar_virtual_28(const Path& cache_path,std::string_view owner,std::int32_t size) = 0;
    // 048d20 mode=2 invokes CopyFileW(source,destination,FALSE).
    virtual bool copy_overwrite_048d20(const Path& source,const Path& destination) = 0;
    virtual std::string path_to_utf8_031ca0(const Path&) = 0;
    virtual void log_saved_size_085530(std::int32_t) = 0;
};
struct StateReferences {
    std::span<const configuration::Content> content_0a3d98;
    std::span<const configuration::Entitlement> entitlements_0a3db0;
    handlers::Attributes& game_0a3d88;
    AchievementMap& achievements_0a3e58;
    const std::string& achievements_set_0a3dc8;
    const std::string& content_id_0a3af8;
    const std::string& user_id_0a3c18;
    const Path& achievement_file_0a3de8;
    const Path& avatar_0a3a90;
    const Path& default_avatar_0a3ab0;
    const Path& user_directory_0a3a00;
    const Path& last_avatar_0a3a60;
    const std::uint8_t& online_0a3c3b;
    std::int32_t& saved_avatar_size_0a1130;
    Mutex& achievements_mutex_0a3e08;
    EventQueue& events;
};

Filters parse_entitlement_filters_02c6a0(const RequestTree&,Effects&);
bool query_content_030ee0(Xml&,StateReferences);
bool query_entitlements_031030(const RequestTree&,Xml&,StateReferences,Effects&);
void achievement_set_header_02bf60(Xml&,StateReferences);
void achievement_element_02c080(Xml&,std::string_view id,const Achievement&);
bool query_achievements_030d30(const RequestTree&,Xml&,StateReferences);
bool grant_achievement_030010(const RequestTree&,Xml&,StateReferences,Effects&);
bool query_image_0314f0(const RequestTree&,Xml&,StateReferences,Effects&);
} // namespace preloader_l::content_handlers_recovered
