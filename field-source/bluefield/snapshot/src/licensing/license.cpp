#include "src/licensing/license.hpp"
#include "src/licensing/literals.hpp"
#include <optional>
#include <stdexcept>

namespace preloader_l::licensing {
namespace {
using namespace literals;
// Keep the existing non-returning Effect contract visible at a nonvirtual
// call boundary. GCC does not retain it for these virtual-call end points.
[[noreturn]] void invoke_exit_effect(Effects& effects,std::int32_t code) {
    effects.exit_05cdc0(code);
#if defined(__GNUC__) || defined(__clang__)
    // A returning implementation already violates Effects' noreturn contract.
    // This hint adds no termination, exception translation or fallback policy.
    __builtin_unreachable();
#endif
}
// Input attributes are C strings in 002cc0. The portable reader exposes a view;
// stop at its first NUL so this boundary does not invent embedded-NUL behavior.
std::optional<std::string> attribute(const Node& request,std::string_view key) {
    auto value=request.attribute(key);
    if(!value) return std::nullopt;
    return std::string(value->substr(0,value->find('\0')));
}
void emit(Xml& xml,const char* element,const char* attribute_name,const char* value) {
    xml.open_004080(element,false);
    xml.attribute_005560(attribute_name,value);
    xml.close_virtual_48(false);
}
}
bool Handlers::auth_code_02e3b0(const Node& request,Xml& xml) {
    const auto client=attribute(request,c_085570);
    if(!client) return false; // present-but-empty is accepted
    std::string auth=state_.auth_code_0a3b18;
    if(state_.online_0a3c3b) {
        const char* online=effects_.online_auth_virtual_30(client->c_str());
        if(!online) throw std::invalid_argument("online_auth_virtual_30 returned a null C string (outside evidenced domain)");
        const std::string received(online);
        if(!received.empty()) auth=received; // replace, not append; empty preserves fallback
    } else if(!state_.fake_auth_0a3c39) {
        xml.open_004080(c_084c70,false);
        xml.attribute_005560(c_084c80,c_0855b0);
        xml.attribute_005560(c_084c88,c_085580);
        xml.close_virtual_48(false);
        return true;
    }
    emit(xml,c_0855c0,c_0855cc,auth.c_str());
    return true;
}
bool Handlers::invalidate_license_030c50(const Node&,Xml& xml) {
    // Only in-memory license is cleared. No file deletion, retry-counter reset,
    // blocked-flag reset, or invented token revocation occurs in this function.
    state_.license_0a3b58.clear();
    xml.open_004080(c_084c70,false);
    xml.attribute_005560(c_084c80,c_083870);
    xml.attribute_005560(c_084c88,c_085fa8);
    xml.close_virtual_48(false);
    return true;
}
[[noreturn]] void Handlers::fatal(std::string_view title,std::string_view body,std::int32_t code) {
    effects_.dialog_038bc0(title,body,0x10);
    invoke_exit_effect(effects_,code);
}
Path Handlers::token_path_02af30(bool cached) {
    auto path=state_.license_base_0a3b38;
    path+=effects_.filesystem_widen_006d70(cached?"_cached.dlf":".dlf");
    return path;
}
std::string Handlers::ticket_text(const char* request_ticket,const char* ticket_engine) {
    std::string text;
    text.reserve(std::char_traits<char>::length(request_ticket)+500);
    text+=state_.game_0a3d88[c_083960];
    text+=c_085c18;
    text+=state_.game_0a3d88[c_083978];
    text+=c_085c1c;
    text+=effects_.path_utf8_01e3f0(effects_.filename_00e120(state_.executable_path_0a39e0));
    text+=c_085c20;
    text+=c_085c40;
    text+=state_.exe_hash_0a3b78;
    text+=c_085c48;
    text+=state_.dll_hash_0a3b98;
    const auto dll_filename=effects_.filename_00e120(state_.dll_path_0a3bb8);
    if(effects_.path_utf8_01e3f0(dll_filename)!=c_083a08) {
        text+=c_085c50;
        // Original performs the conversion twice on this branch.
        text+=effects_.path_utf8_01e3f0(dll_filename);
    }
    text+=c_085c54;
    text+=request_ticket;
    text+=c_0851d8;
    text+=ticket_engine;
    text+=c_0851d8;
    text+=state_.content_id_0a3af8;
    return text;
}
[[noreturn]] void Handlers::ticket_required(const char* request_ticket,const char* ticket_engine) {
    const auto text=ticket_text(request_ticket,ticket_engine);
    if(effects_.wine_detected_045010()) effects_.dialog_038bc0(c_085e28,c_085c60,0x10);
    effects_.log_literal_006880(c_085e50);
    effects_.log_string_006860(text);
    effects_.log_literal_006880(c_085e38);
    auto prefix=effects_.filesystem_widen_006d70(c_085e68);
    auto parent=effects_.parent_path_019820(state_.executable_path_0a39e0);
    auto file=effects_.join_016b00(parent,prefix);
    const auto suffix=effects_.current_time_041b60_null_true()+c_085e78;
    file+=effects_.filesystem_widen_006d70(suffix); // append, not a second path join
    if(effects_.write_ticket_03c1d0(file,text)) {
        effects_.dialog_038bc0(c_085ea0,std::string(c_085e80)+effects_.path_utf8_01e3f0(file),0x40);
    } else if(effects_.wine_detected_045010()) {
        effects_.dialog_038bc0(c_085e28,c_085eb0,0x40);
    } else {
        effects_.dialog_038bc0(c_085f80,std::string(c_085f58)+text,0x40);
    }
    invoke_exit_effect(effects_,-1);
}
bool Handlers::request_license_0320c0(const Node& request,Xml& xml) {
    if(state_.blocked_0a39c0) effects_.no_response_02bcd0_04d228();
    if(state_.dll_hash_0a3b98.size()!=40) {
        state_.blocked_0a39c0=1;
        fatal(c_085978,c_085920,-4);
    }
    if(state_.dll_path_0a3bb8.empty()) {
        state_.blocked_0a39c0=1;
        fatal(c_0859e8,c_085990,-5);
    }
    const auto status=effects_.status_01ea20(state_.dll_path_0a3bb8,3);
    if(status.error && ((static_cast<std::uint32_t>(status.kind)-1U)&0xfffffff7U))
        effects_.filesystem_error_018130("status",status.error,state_.dll_path_0a3bb8);
    if(status.kind!=2) {
        state_.blocked_0a39c0=1;
        fatal(c_0859e8,c_085990,-5);
    }
    if(!effects_.file_hash_matches_042d20(state_.dll_path_0a3bb8,state_.dll_hash_0a3b98)) {
        state_.blocked_0a39c0=1;
        fatal(c_085978,std::string(c_085a7c)+effects_.path_utf8_01e3f0(state_.dll_path_0a3bb8)+c_085a00,-6);
    }
    while(state_.attempted_files_0a39bc<2) {
        if(!state_.license_0a3b58.empty()) break;
        const auto path=token_path_02af30(state_.attempted_files_0a39bc!=0);
        state_.license_0a3b58=effects_.load_token_02afa0(path);
        ++state_.attempted_files_0a39bc;
    }
    if(!state_.license_0a3b58.empty()) {
        emit(xml,c_085f90,c_0839a0,state_.license_0a3b58.c_str());
        return true;
    }
    state_.blocked_0a39c0=1;
    const auto ticket=attribute(request,c_085a80);
    const auto engine=attribute(request,c_085a90); // both lookups happen, even if ticket absent
    if(!ticket || !engine) return false;
    if(state_.content_id_0a3af8.empty() || state_.content_id_0a3af8==c_083870)
        fatal(c_085b18,c_085aa0,-2);
    if(state_.exe_hash_0a3b78.size()!=40) fatal(c_085b88,c_085b30,-3);
    if(!effects_.file_hash_matches_042d20(state_.executable_path_0a39e0,state_.exe_hash_0a3b78))
        fatal(c_085978,std::string(c_085a7c)+effects_.path_utf8_01e3f0(state_.executable_path_0a39e0)+c_085ba0,-7);
    ticket_required(ticket->c_str(),engine->c_str());
}
} // namespace preloader_l::license_recovered
