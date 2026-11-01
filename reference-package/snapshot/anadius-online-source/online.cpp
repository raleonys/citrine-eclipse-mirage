#include "online.hpp"

namespace anadius::online_recovered {
bool exists_035b00(const Path& path, Effects& effects) {
    const auto status = effects.status_01ea20(path, 3);
    const auto kind = static_cast<std::uint32_t>(status.kind);
    if (status.error && ((kind - 1u) & 0xfffffff7u) != 0)
        effects.filesystem_error_018130("status", status.error, path);
    return status.kind == 2;
}

std::vector<std::string> split_profile_fields_03cf00(std::string_view value) {
    std::vector<std::string> fields;
    std::size_t begin = 0;
    for (;;) {
        const auto end = value.find(';', begin);
        if (end == std::string_view::npos) {
            fields.emplace_back(value.substr(begin));
            return fields;
        }
        fields.emplace_back(value.substr(begin, end - begin));
        begin = end + 1;
    }
}

static void log_line(Effects& effects, std::string_view literal) {
    effects.log_literal_006880(literal);
    effects.log_finish_0088e0();
}
static std::string copy_c_string(const char* value) {
    if (!value) throw OutOfEvidenceInput("035bd0 scans a null provider C string");
    return std::string(value);
}
std::string Effects::copy_username(Object o){return copy_c_string(username_virtual_10(o));}
std::string Effects::copy_persona(Object o){return copy_c_string(persona_virtual_18(o));}
std::string Effects::copy_user_id(Object o){return copy_c_string(user_id_virtual_20(o));}

void load_online_035bd0(StateRefs state, Effects& effects) {
    const auto previous_mode = effects.suppress_errors_0450a0();
    const auto library_name = effects.filesystem_widen_006d70("anadius64online.dll");
    const auto parent = effects.parent_path_019820(state.executable_path_0a39e0);
    const auto path = effects.join_016b00(parent, library_name);
    state.module_0a3a80 = effects.load_copy_0450d0(path);
    // Normal-path restoration: exceptions above skip it in the observed body.
    effects.restore_error_mode(previous_mode);
    if (!state.module_0a3a80) return;

    log_line(effects, "online dll found!");
    const auto version = effects.get_proc_address(state.module_0a3a80, "version_check");
    if (!version) {
        log_line(effects, "version function not found");
        return;
    }
    if (effects.call_version_check(version) != 2) {
        effects.dialog_038bc0("crack error", "Online library version mismatch!", 0x10);
        log_line(effects, "online library version mismatch!");
        return;
    }
    const auto factory = effects.get_proc_address(state.module_0a3a80, "create_online");
    if (!factory) {
        log_line(effects, "factory function not found");
        return;
    }
    {
        const Path cookie_copy(state.cookie_0a3a20);
        state.object_0a3a88 = effects.call_create_online(factory, cookie_copy.c_str());
    } // 035fb1 commits object;035ff3 releases cookie before036002 status
    if (!state.object_0a3a88)
        throw OutOfEvidenceInput("035bd0 dereferences a null create_online result");
    const auto status = effects.status_virtual_08(state.object_0a3a88);
    if (status != 0) {
        if (status == 1) effects.log_literal_006880("authentication failed, ");
        log_line(effects, "starting offline");
        if (!state.use_last_profile_0a3c38) return;
        if (!exists_035b00(state.cookie_0a3a20, effects)) return;
        if (exists_035b00(state.last_avatar_0a3a60, effects))
            state.avatar_0a3a90 = state.last_avatar_0a3a60;
        const auto profile = effects.read_binary_03b3f0(state.last_profile_0a3a40);
        const auto fields = split_profile_fields_03cf00(profile);
        if (fields.size() == 3) {
            state.persona_id_0a3bf8 = fields[0];
            state.user_id_0a3c18 = fields[1];
            state.username_0a3bd8 = fields[2];
        }
        return;
    }

    state.online_0a3c3b = 1; // precedes logging and all provider profile getters
    log_line(effects, "authentication ok");
    state.username_0a3bd8 = effects.copy_username(state.object_0a3a88);
    state.persona_id_0a3bf8 = effects.copy_persona(state.object_0a3a88);
    state.user_id_0a3c18 = effects.copy_user_id(state.object_0a3a88);
    if (exists_035b00(state.cookie_0a3a20, effects) &&
        exists_035b00(state.last_avatar_0a3a60, effects))
        state.avatar_0a3a90 = state.last_avatar_0a3a60;
    if (!state.use_last_profile_0a3c38) return;
    // String lengths, rather than C-string scans, control the saved profile.
    std::string saved = state.persona_id_0a3bf8;
    saved += ';';
    saved += state.user_id_0a3c18;
    saved += ';';
    saved += state.username_0a3bd8;
    (void)effects.write_binary_03c1d0(state.last_profile_0a3a40, saved);
}

void cleanup_0366f0(StateRefs state, Effects& effects) {
    if (state.object_0a3a88) {
        effects.destroy_virtual_00(state.object_0a3a88);
        state.object_0a3a88 = 0;
    }
    if (state.module_0a3a80) {
        (void)effects.free_library(state.module_0a3a80);
        state.module_0a3a80 = 0;
    }
}

void worker_035b60(StateRefs state, Effects& effects) {
    effects.wait_steps_036880(1);
    load_online_035bd0(state, effects);
    if (!state.online_0a3c3b) cleanup_0366f0(state, effects);
    effects.finish_flag_036820();
}
} // namespace anadius::online_recovered
