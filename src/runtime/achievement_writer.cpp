// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/achievement_writer.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include <charconv>
#include <stdexcept>

namespace citrine_eclipse_mirage::achievement_writer {
namespace {
template<class Number> void append_decimal(std::string& output,Number value) {
    char digits[32];
    const auto result=std::to_chars(digits,digits+sizeof(digits),value);
    if(result.ec!=std::errc{})throw std::logic_error("decimal buffer contract");
    output.append(digits,result.ptr);
}
// Reconstructed serialization allows arbitrary steam_id bytes. Production
// watcher INI output must not let one section name forge another record.
// This is an explicit compatibility/security policy, not donor-byte parity.
void validate_section_names(const configuration::State& state) {
    for(const auto& [key,record]:state.achievements) {
        (void)key;
        if(record.steam_id.find_first_of("[]\r\n")!=std::string::npos ||
           record.steam_id.find('\0')!=std::string::npos)
            throw std::invalid_argument("Unsafe achievement INI section name");
    }
}
}
std::string serialize_ini(const configuration::State& state) {
    validate_section_names(state);
    std::string output;
    for(const auto& [key,record]:state.achievements) {
        (void)key;
        output.push_back('[');
        output.append(record.steam_id); // Explicit-length bytes, including NUL.
        output.append("]\r\nAchieved=");
        append_decimal(output,record.field04);
        output.append("\r\n");
        if(record.field00>1) {
            output.append("CurProgress=");
            append_decimal(output,record.field08);
            output.append("\r\nMaxProgress=");
            append_decimal(output,record.field00);
            output.append("\r\n");
        }
        output.append("UnlockTime=");
        append_decimal(output,record.field70);
        output.append("\r\n\r\n");
    }
    return output;
}
void write_ini_01f9a0(const configuration::Path& ini_path,
    const configuration::State& state,Effects& effects,file::Factory& files) {
    if(ini_path.empty())return;
    // Reject unsafe section IDs before creating the output directory. Keep
    // normal source order (mkdir before serialization) for accepted inputs.
    validate_section_names(state);
    const auto parent=path::parent_path_019820(ini_path);
    effects.create_directories_018e00(parent);
    const auto bytes=serialize_ini(state);
    const file::FileWrite write{ini_path,bytes};files.replace_atomic({&write,1});
}
} // namespace citrine_eclipse_mirage::achievement_writer
