#include "preloader_l/runtime/achievement_writer.hpp"
#include "preloader_l/core/paths.hpp"
#include <charconv>
#include <stdexcept>

namespace preloader_l::achievement_writer {
namespace {
template<class Number> void append_decimal(std::string& output,Number value) {
    char digits[32];
    const auto result=std::to_chars(digits,digits+sizeof(digits),value);
    if(result.ec!=std::errc{})throw std::logic_error("decimal buffer contract");
    output.append(digits,result.ptr);
}
}
std::string serialize_ini(const configuration::State& state) {
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
    const configuration::State& state,Effects& effects,file_recovered::Factory& files) {
    if(ini_path.empty())return;
    const auto parent=path_recovered::parent_path_019820(ini_path);
    effects.create_directories_018e00(parent);
    const auto bytes=serialize_ini(state);
    const file_recovered::FileWrite write{ini_path,bytes};files.replace_atomic({&write,1});
}
} // namespace preloader_l::achievement_writer
