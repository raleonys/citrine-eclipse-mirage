#pragma once
#include "../anadius-dispatch-source/router.hpp"
#include "auth_state.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace anadius::license_recovered {
using Path = configuration::Path;
using Xml = handlers_recovered::Xml;
using Node = messages_recovered::XmlNode;
struct FileStatus { std::int32_t kind; std::int32_t error; };

// Legacy references remain caller-supplied. Optional AuthenticationBinding
// selects the Runtime-owned, immutable native+138 cell before first dispatch.
// This object does not implement original MSVC layout or native I/O.
struct StateReferences {
    const std::string& auth_code_0a3b18;
    const std::uint8_t& online_0a3c3b;
    const std::uint8_t& fake_auth_0a3c39;
    std::string& license_0a3b58;
    const std::string& dll_hash_0a3b98;
    const std::string& exe_hash_0a3b78;
    const std::string& content_id_0a3af8;
    const Path& dll_path_0a3bb8;
    const Path& executable_path_0a39e0;
    const Path& license_base_0a3b38;
    handlers_recovered::Attributes& game_0a3d88;
    std::int32_t& attempted_files_0a39bc;
    std::uint8_t& blocked_0a39c0;
};
// Optional owned result. Native providers keep their object/code reservation
// through C-string scan/allocation. Legacy borrowed providers still require
// external serialization through their immediate compatibility copy.
class OwnedAuthEffects {
public:
    virtual ~OwnedAuthEffects()=default;
    virtual std::string copy_online_auth(const char* client_id)=0;
};
class Effects {
public:
    virtual ~Effects() = default;
    // Original return pointer must be a valid, NUL-terminated string.
    virtual const char* online_auth_virtual_30(const char* client_id) = 0;
    virtual FileStatus status_01ea20(const Path&,std::uint32_t flags) = 0;
    [[noreturn]] virtual void filesystem_error_018130(std::string_view operation,
                                                   std::int32_t error,const Path&) = 0;
    // Original consumes a COPY of expected text; its case normalization must
    // not mutate the configuration's global string. Internal SHA-1 I/O stays
    // an explicit separate subsystem, not a synthesized acceptance result.
    virtual bool file_hash_matches_042d20(const Path&,std::string expected) = 0;
    virtual std::string load_token_02afa0(const Path&) = 0;
    // __std_fs_code_page() conversion, distinct from the UTF-8 narrowing below.
    virtual Path filesystem_widen_006d70(std::string_view) = 0;
    virtual Path filename_00e120(const Path&) = 0;
    virtual Path parent_path_019820(const Path&) = 0;
    virtual Path join_016b00(const Path&,const Path&) = 0;
    virtual std::string path_utf8_01e3f0(const Path&) = 0;
    virtual std::string current_time_041b60_null_true() = 0;
    virtual bool wine_detected_045010() = 0;
    virtual void log_literal_006880(std::string_view) = 0;
    virtual void log_string_006860(std::string_view) = 0;
    virtual bool write_ticket_03c1d0(const Path&,std::string_view) = 0;
    virtual void dialog_038bc0(std::string_view title,std::string_view body,std::uint32_t flags) = 0;
    [[noreturn]] virtual void exit_05cdc0(std::int32_t code) = 0;
    [[noreturn]] virtual void no_response_02bcd0_04d228() = 0;
};
std::string copy_online_auth(Effects&,const char* client_id);

// Application-level reconstruction compatible with the existing router. These
// are functional bodies, but all unknown OS/token/provider work is mandatory.
class Handlers final : public dispatch_recovered::LicenseHandlers, public AuthenticationBinding {
public:
    Handlers(StateReferences state,Effects& effects):state_(state),effects_(effects){}
    void bind_authentication_code(const AuthenticationCode&) override;
    bool auth_code_02e3b0(const Node&,Xml&) override;
    bool request_license_0320c0(const Node&,Xml&) override;
    bool invalidate_license_030c50(const Node&,Xml&) override;
private:
    Path token_path_02af30(bool cached);
    std::string ticket_text(const char* request_ticket,const char* ticket_engine);
    [[noreturn]] void fatal(std::string_view title,std::string_view body,std::int32_t exit_code);
    [[noreturn]] void ticket_required(const char* request_ticket,const char* ticket_engine);
    StateReferences state_;
    Effects& effects_;
    const AuthenticationCode* authentication_=nullptr;
    bool used_=false;
};
} // namespace anadius::license_recovered
