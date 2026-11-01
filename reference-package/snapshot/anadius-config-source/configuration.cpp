#include "configuration.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace anadius::configuration {
const Node& Node::child(std::string_view key) const {
    static const Node absent{};
    const auto found = children.find(key);
    return found == children.end() ? absent : found->second;
}

std::vector<const decltype(Node::children)::value_type*> Node::ordered_children() const {
    // An integration contract error must not silently invent hash iteration order.
    if (iteration_order.size() != children.size())
        throw std::logic_error("Config2 adapter must supply complete binary collection order");
    std::vector<const decltype(children)::value_type*> result;
    for (const auto& key : iteration_order) {
        const auto found = children.find(key);
        if (found == children.end() || std::find(result.begin(), result.end(), &*found) != result.end())
            throw std::logic_error("Config2 adapter supplied invalid binary collection order");
        result.push_back(&*found);
    }
    return result;
}

// 18001e1e0. Empty input never clears the existing destination.
void assign_if_nonempty(std::string& destination, std::string_view source) {
    if (!source.empty()) destination.assign(source);
}

// 18001de40; assembly ev_678897aced1c65efbc6f8f013f07b732f661c397126aa015e2fbacc64a4d243a.
void apply_boolean(std::uint8_t& destination, std::string_view text, std::uint8_t fallback) {
    if (text.empty()) return;
    if (text == "true" || text == "1") destination = 1;
    else if (text == "false" || text == "0") destination = 0;
    else destination = fallback;
}

// 18001d6c0 calls signed, decimal 32-bit CRT conversion 18005e4ac. Trailing
// characters are accepted. Original exception messages are preserved; C++ ABI
// and locale adapter equivalence still require separate integration validation.
std::int32_t parse_decimal(std::string_view text, Dependencies& deps) {
    std::size_t cursor = 0;
    while (cursor < text.size() && text[cursor] != '\0' &&
           deps.crt_is_space(static_cast<unsigned char>(text[cursor]))) ++cursor;
    bool negative = false;
    if (cursor < text.size() && (text[cursor] == '+' || text[cursor] == '-'))
        negative = text[cursor++] == '-';
    const auto first_digit = cursor;
    const std::uint64_t limit = negative ? 2147483648ULL : 2147483647ULL;
    std::uint64_t magnitude = 0;
    bool overflow = false;
    while (cursor < text.size() && text[cursor] >= '0' && text[cursor] <= '9') {
        const auto digit = static_cast<unsigned>(text[cursor++] - '0');
        if (magnitude > (limit - digit) / 10) overflow = true;
        if (!overflow) magnitude = magnitude * 10 + digit;
    }
    if (cursor == first_digit) throw std::invalid_argument("invalid stoi argument");
    if (overflow) throw std::out_of_range("stoi argument out of range");
    if (negative && magnitude == 2147483648ULL) return std::numeric_limits<std::int32_t>::min();
    const auto result = static_cast<std::int32_t>(magnitude);
    return negative ? -result : result;
}

// 18003cf00 called with limit 0: substring delimiter split including empties.
// There is no CSV quoting or trimming in this helper.
std::vector<std::string_view> split_delimited(std::string_view text, char delimiter) {
    std::vector<std::string_view> parts;
    std::size_t start = 0;
    while (true) {
        const auto end = text.find(delimiter, start);
        if (end == std::string_view::npos) { parts.push_back(text.substr(start)); return parts; }
        parts.push_back(text.substr(start, end - start));
        start = end + 1;
    }
}

// 18001df60. Nonempty input clears old vector before attempting conversions;
// empty segments are skipped; failed conversion can leave a partially filled list.
void apply_csv_paths(std::vector<Path>& destination, std::string_view text,
                     const Path& executable, Dependencies& deps) {
    if (text.empty()) return;
    destination.clear();
    for (auto part : split_delimited(text, ','))
        if (!part.empty()) destination.push_back(deps.resolve_path_0192d0(part, executable, true));
}

// 18001e0c0. The flag is written before conversion, including on conversion throw.
void apply_registry_path(State& state, std::string_view text, Dependencies& deps) {
    if (text.empty()) return;
    state.registry_spoof = 1;
    state.language_registry_key = deps.widen_0145d0(text);
}

static Achievement reset_achievement(std::string_view display_name = {}) {
    return {1, 0, 0, "0000-00-00T00:00:00", std::string(display_name), {}, 0};
}

// 18001da80. Nonempty set permits semicolon IDs to insert/reset records; empty
// set prevents all changes even if ids is nonempty. Existing records are reset.
void apply_achievement_ids(State& state, std::string_view set, std::string_view ids) {
    if (set.empty()) return;
    state.achievements_set.assign(set);
    for (auto id : split_delimited(ids, ';'))
        if (!id.empty()) state.achievements[std::string(id)] = reset_achievement();
}

static void lowercase_hash(std::string& value, Dependencies& deps) {
    for (char& character : value)
        character = static_cast<char>(deps.crt_tolower(static_cast<signed char>(character)));
}
static void path_if_nonempty(Path& destination, std::string_view text,
                             const Path& executable, Dependencies& deps) {
    if (!text.empty()) destination = deps.resolve_path_0192d0(text, executable, true);
}
static std::string defaulted(std::string_view text, const char* fallback) {
    return text.empty() ? fallback : std::string(text);
}

// 180019ad0, ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd.
void apply_config2(State& state, const Node& config2, bool override_pass, Dependencies& deps) {
    const auto& game = config2.child("Game");
    if (!override_pass) {
        assign_if_nonempty(state.game_attributes["DISPLAY_NAME"], game.text("Name"));
        assign_if_nonempty(state.game_attributes["INSTALLED_VERSION"], game.text("Version"));
        assign_if_nonempty(state.content_id, game.text("ContentId"));
    }
    // This derived path is recomputed on BOTH passes using the retained ContentId.
    const auto wide_content = deps.filesystem_widen_010540(state.content_id);
    const Path license_component = u"License";
    const Path services_component = u"EA Services";
    const Path publisher_component = u"Electronic Arts";
    auto path = deps.join_016b00(deps.program_data_03c920(), publisher_component);
    path = deps.join_016b00(path, services_component);
    path = deps.join_016b00(path, license_component);
    state.license_directory = deps.join_016b00(path, wide_content);

    assign_if_nonempty(state.denuvo_token, game.text("DenuvoToken"));
    if (!state.denuvo_token.empty() && state.denuvo_token.size() <= 99) state.denuvo_token.clear();
    assign_if_nonempty(state.exe_hash, game.text("DenuvoExeHash"));
    lowercase_hash(state.exe_hash, deps);
    assign_if_nonempty(state.dll_hash, game.text("DenuvoDllHash"));
    lowercase_hash(state.dll_hash, deps);
    path_if_nonempty(state.denuvo_dll, game.text("DenuvoDllName"), state.executable_path, deps);
    if (state.denuvo_dll.empty())
        path_if_nonempty(state.denuvo_dll, "dbdata.dll", state.executable_path, deps);
    const auto license_key = game.text("KeyForLicense");
    if (!license_key.empty()) deps.apply_license_key_02a830(license_key);
    assign_if_nonempty(state.game_attributes["LANGUAGES"], game.text("Languages"));
    apply_registry_path(state, game.text("LanguageRegistryKey"), deps);
    if (state.language_from_arguments == 0) {
        assign_if_nonempty(state.game_attributes["INSTALLED_LANGUAGE"], game.text("Language"));
        apply_boolean(state.registry_spoof, game.text("LanguageRegistrySpoof"), 1);
    }

    const auto& emulator = config2.child("Emulator");
    path_if_nonempty(state.documents, emulator.text("Documents"), state.executable_path, deps);
    const auto port = emulator.text("ServerPort");
    if (!port.empty()) {
        if (port == "default") state.server_port = 0xc90;
        else if (port == "random") state.server_port = 0;
        else state.server_port = static_cast<std::uint16_t>(parse_decimal(port, deps));
        // 18001a52a MOV word ptr [1800a3c90], AX: no 0..65535 range check.
    }
    apply_boolean(state.pretend_connected, emulator.text("PretendConnected"), 1);
    apply_boolean(state.fake_auth, emulator.text("FakeAuth"), 1);
    assign_if_nonempty(state.sdk, emulator.text("SDK"));
    assign_if_nonempty(state.emulator_attributes["LANGUAGE"], emulator.text("Language"));
    apply_boolean(state.block_reruns, emulator.text("BlockReruns"), 0);
    apply_csv_paths(state.extra_dlls_from_main, emulator.text("LoadExtraDLLsFromMain"), state.executable_path, deps);
    apply_csv_paths(state.extra_dlls, emulator.text("LoadExtraDLLs"), state.executable_path, deps);

    const auto& user = config2.child("User");
    assign_if_nonempty(state.username, user.text("Username"));
    assign_if_nonempty(state.persona_id, user.text("PersonaId"));
    assign_if_nonempty(state.user_id, user.text("UserId"));
    path_if_nonempty(state.avatar, user.text("Avatar"), state.executable_path, deps);
    apply_boolean(state.use_last_online_profile, user.text("UseLastOnlineProfile"), 0);
    assign_if_nonempty(state.country, user.text("Country"));
    assign_if_nonempty(state.currency, user.text("Currency"));
    assign_if_nonempty(state.is_subscriber, user.text("IsSubscriber"));
    assign_if_nonempty(state.is_steam_subscriber, user.text("IsSteamSubscriber"));
    assign_if_nonempty(state.subscriber_level, user.text("SubscriberLevel"));
    assign_if_nonempty(state.is_under_age, user.text("IsUnderAge"));
    if (override_pass) return;

    // Base-only collections; vectors APPEND, rather than clear/replace.
    for (const auto* entry : config2.child("Content").ordered_children()) {
        const auto& [id, node] = *entry;
        state.content.push_back({id, defaulted(node.text("Name"), "GAME"),
            defaulted(node.text("Version"), "0"), defaulted(node.text("State"), "INSTALLED")});
    }
    for (const auto* entry : config2.child("Entitlements").ordered_children()) {
        const auto& [id, node] = *entry;
        state.entitlements.push_back({id, std::string(node.text("Group")),
            defaulted(node.text("Version"), "0"), defaulted(node.text("Type"), "DEFAULT"),
            defaulted(node.text("EntitlementTag"), "ONLINE_ACCESS")});
    }

    const auto& achievements = config2.child("Achievements");
    apply_achievement_ids(state, achievements.text("AchievementsSet"), achievements.text("AchievementIDs"));
    for (const auto* entry : achievements.child("AchievementNames").ordered_children()) {
        const auto& [id, node] = *entry;
        state.achievements[id] = reset_achievement(node.value);
    }
    for (const auto* entry : achievements.child("AchievementTotals").ordered_children()) {
        const auto& [id, node] = *entry;
        const auto found = state.achievements.find(id);
        if (!node.value.empty() && found != state.achievements.end())
            found->second.field00 = parse_decimal(node.value, deps);
    }
    assign_if_nonempty(state.steam_app_id, achievements.text("SteamAppId"));
    if (!state.steam_app_id.empty()) {
        const auto prefix = achievements.text("SteamIdPrefix");
        for (auto& [id, achievement] : state.achievements)
            achievement.steam_id = std::string(prefix) + id;
    }
    for (const auto* entry : achievements.child("SteamIdOverrides").ordered_children()) {
        const auto& [id, node] = *entry;
        const auto found = state.achievements.find(id);
        if (!node.value.empty() && found != state.achievements.end()) found->second.steam_id = node.value;
    }
}

// 019ad0 outer path, logging, decoded Config2 lookup. Reader and grammar are
// required adapters; the source does not parse user CFG using guessed syntax.
void load_and_apply(State& state, bool override_pass, Dependencies& deps) {
    const auto name = override_pass ? "anadius_override.cfg" : "anadius.cfg";
    const auto filename = deps.filesystem_widen_010540(name);
    const auto parent = deps.parent_path_019820(state.executable_path);
    const auto path = deps.join_016b00(parent, filename);
    const auto document = deps.load_document(path);
    deps.log_config_path(path);
    apply_config2(state, document.child("Config2"), override_pass, deps);
}

// 18001c870; ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e.
// Filesystem APIs are mandatory contracts; tests replace them entirely.
void initialize_configuration(State& state, Dependencies& deps) {
    class DocumentPass final : public ConfigurationPass {
        void load_and_apply(State& s,bool pass,Dependencies& d) override {
            configuration::load_and_apply(s,pass,d);
        }
    } document_pass;
    initialize_configuration(state,deps,document_pass);
}

void initialize_configuration(State& state, Dependencies& deps, ConfigurationPass& passes) {
    state.executable_path = deps.executable_path_03c6d0();
    passes.load_and_apply(state, false, deps); // 18001c8d1
    passes.load_and_apply(state, true, deps);  // 18001c8d8
    const auto product_component = deps.filesystem_widen_010540("LSX emu");
    const auto vendor_component = deps.filesystem_widen_010540("anadius");
    state.user_directory = deps.join_016b00(deps.local_app_data_03c6f0(), vendor_component);
    state.user_directory = deps.join_016b00(state.user_directory, product_component);
    deps.create_directories_018e00(state.user_directory);
    state.cookie_path = deps.join_016b00(state.user_directory, deps.filesystem_widen_010540("remid_cookie"));
    state.last_profile_path = deps.join_016b00(state.user_directory, deps.filesystem_widen_010540("last_profile"));
    state.last_avatar_path = deps.join_016b00(state.user_directory, deps.filesystem_widen_010540("last_avatar"));
    const bool default_user_avatar = state.avatar.empty();
    if (default_user_avatar)
        state.avatar = deps.join_016b00(state.user_directory, deps.filesystem_widen_010540("anadius.png"));
    state.default_avatar = deps.join_016b00(state.user_directory, deps.filesystem_widen_010540("default.png"));
    if (default_user_avatar)
        deps.write_embedded_png_03c020(state.avatar, EmbeddedPng::default_user_1800809d0);
    deps.write_embedded_png_03c020(state.default_avatar, EmbeddedPng::default_avatar_180082710);
    // Process each logical entry before advancing the source iterator. Older
    // providers use the optional visitor route's collecting fallback.
    deps.visit_directory_entries(state.user_directory,{&deps,[](void* context,const Path& entry){
        auto& directory=*static_cast<Dependencies*>(context);
        if (directory.narrow_path(directory.filename(entry)).find("_AVATAR_") != std::string::npos)
            directory.remove_04948c(entry);
    }});
    state.game_attributes["AVAILABLE_VERSION"] = state.game_attributes["INSTALLED_VERSION"];
    std::replace(state.sdk.begin(), state.sdk.end(), '.', ',');
    deps.update_achievement_storage_01eb00(state);
    (void)deps.set_environment_variable("ContentId", state.content_id);
    (void)deps.set_environment_variable("EAGameLocale", state.game_attributes["INSTALLED_LANGUAGE"]);
}
void Dependencies::visit_directory_entries(const Path& path,DirectoryVisitor visitor) {
    if(!visitor.visit)throw std::invalid_argument("Directory visitor callback required");
    for(const auto& entry:directory_entries(path))visitor.visit(visitor.context,entry);
}
} // namespace anadius::configuration
