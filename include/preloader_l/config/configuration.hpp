#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

// Address-backed C++20 reconstruction. This is an application-logic module,
// not the original MSVC object layout or a standalone replacement DLL.
namespace preloader_l::configuration {
using Path = std::u16string;
struct Node {
    std::string value;
    // Lookup storage only. The original is an MSVC unordered/hash-list container,
    // so this std::map's lexical order MUST NOT become collection traversal order.
    // Adapter must preserve the observed original linked-list traversal separately.
    // Raw grammar, duplicate keys and original parser errors remain its responsibility.
    std::map<std::string, Node, std::less<>> children;
    std::vector<std::string> iteration_order;
    const Node& child(std::string_view key) const;
    std::string_view text(std::string_view key) const { return child(key).value; }
    std::vector<const decltype(children)::value_type*> ordered_children() const;
};
struct Content { std::string id, name, version, state; };
struct Entitlement { std::string id, group, version, type, tag; };
struct Achievement {
    // Names reflect offsets in the original map value. Only field00's use by
    // AchievementTotals is established here; other integer meanings stay open.
    std::int32_t field00, field04, field08;
    std::string timestamp, display_name, steam_id;
    std::uint64_t field70;
};

// CALLER-SUPPLIED INITIAL STATE. No factory supplies guessed binary defaults.
// Tests explicitly seed synthetic zero/old values. Global VAs are documented in
// evidence.json; the native/MSVC ABI adapter must populate every real field.
struct State {
    Path executable_path;                                      // a39e0
    std::map<std::string, std::string, std::less<>> game_attributes, emulator_attributes;
    std::string content_id, denuvo_token, exe_hash, dll_hash;
    Path license_directory, denuvo_dll, language_registry_key;
    std::uint8_t language_from_arguments, registry_spoof;
    Path documents;
    std::uint16_t server_port; // Runtime bound port; no configuration override.
    std::uint8_t pretend_connected, fake_auth, block_reruns;
    std::vector<Path> extra_dlls_from_main, extra_dlls;
    std::string username, persona_id, user_id;
    Path avatar;
    std::uint8_t use_last_online_profile;
    std::string country, currency, is_subscriber, is_steam_subscriber, subscriber_level, is_under_age;
    std::vector<Content> content;
    std::vector<Entitlement> entitlements;
    std::string achievements_set;
    std::map<std::string, Achievement, std::less<>> achievements;
    std::string steam_app_id;
    Path user_directory, cookie_path, last_profile_path, last_avatar_path, default_avatar;
};

enum class EmbeddedPng { default_user_1800809d0, default_avatar_180082710 };
// Synchronous source callback; the entry is borrowed only for this call.
struct DirectoryVisitor {
    void* context=nullptr;
    void (*visit)(void*,const Path&)=nullptr;
};
class Dependencies {
public:
    virtual ~Dependencies() = default;
    virtual Path executable_path_03c6d0() = 0; // GetModuleFileNameW(NULL)
    virtual Path parent_path_019820(const Path&) = 0;
    virtual Path join_016b00(const Path&, const Path&) = 0;
    virtual Path widen_0145d0(std::string_view) = 0;
    // 010540 / 017330 use __std_fs_code_page(); distinct from UTF-8 0145d0.
    virtual Path filesystem_widen_010540(std::string_view) = 0;
    virtual std::string narrow_path(const Path&) = 0;
    virtual Path resolve_path_0192d0(std::string_view, const Path& executable, bool relative) = 0;
    virtual Path program_data_03c920() = 0;
    virtual Path local_app_data_03c6f0() = 0;
    // Original file I/O + BOM handling + parser + Config2 document construction.
    // 019ad0 guards parser result, but retained014310 normal exits return true;
    // malformed input routes to throw helpers. Plain parse-false was NOT observed.
    // Preserve the result-guard contract without claiming a normal false path;
    // propagate original I/O/parser exceptions. Byte grammar is a separate module.
    virtual Node load_document(const Path&) = 0;
    virtual void log_config_path(const Path&) = 0;
    // These are actual CRT locale operations, not hard-coded ASCII assumptions.
    virtual bool crt_is_space(unsigned char) = 0;
    virtual int crt_tolower(int original_signed_char) = 0;
    // Original 02a830 is a distinct licensing/file subsystem, not configuration
    // assignment. Its internal processing and MSVC ABI remain unrecovered here.
    virtual void apply_license_key_02a830(std::string_view) = 0;
    virtual void create_directories_018e00(const Path&) = 0;
    virtual void write_embedded_png_03c020(const Path&, EmbeddedPng) = 0;
    virtual std::vector<Path> directory_entries(const Path&) = 0;
    virtual Path filename(const Path&) = 0;
    virtual void remove_04948c(const Path&) = 0;
    // Separate achievement persistence subsystem, 01eb00; state can be modified
    // by loading its file. This contract must supply its recovered behavior.
    virtual void update_achievement_storage_01eb00(State&) = 0;
    virtual bool set_environment_variable(std::string_view name, std::string_view value) = 0;
    // Appended source capability: legacy vector providers remain supported.
    virtual void visit_directory_entries(const Path&,DirectoryVisitor);
};

void cleanup_cached_avatars(const Path&,Dependencies&);

void assign_if_nonempty(std::string& destination, std::string_view source); // 01e1e0
void apply_boolean(std::uint8_t& destination, std::string_view text, std::uint8_t fallback); // 01de40
std::int32_t parse_decimal(std::string_view text, Dependencies&); // 01d6c0 / 05e4ac
std::vector<std::string_view> split_delimited(std::string_view, char); // 03cf00, limit=0
void apply_csv_paths(std::vector<Path>& destination, std::string_view text,
                     const Path& executable, Dependencies&); // 01df60
void apply_registry_path(State&, std::string_view, Dependencies&); // 01e0c0
void apply_achievement_ids(State&, std::string_view set, std::string_view ids); // 01da80

// Already-decoded Config2 subtree; applies the complete observed settings path
// of 019ad0 after parse/logging. Unknown filesystem/locale/ABI work is explicit.
void apply_config2(State&, const Node& config2, bool override_pass, Dependencies&);
void load_and_apply(State&, bool override_pass, Dependencies&); // 019ad0 outer path
// Source-level composition point, not an original binary virtual interface.
// A supplied pass can connect the mutable file/parser/hash implementation while
// preserving the single existing 01c870 post-configuration orchestration body.
class ConfigurationPass {
public:
    virtual ~ConfigurationPass() = default;
    virtual void load_and_apply(State&,bool override_pass,Dependencies&) = 0;
};
void initialize_configuration(State&, Dependencies&, ConfigurationPass&);
void initialize_configuration(State&, Dependencies&);         // 01c870 orchestration
} // namespace preloader_l::configuration
