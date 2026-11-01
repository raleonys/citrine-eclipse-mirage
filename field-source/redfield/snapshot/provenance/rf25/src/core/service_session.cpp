// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/service_session.hpp"
#include "redfield/xml/value_codec.hpp"
namespace redfield {
ServiceSession::ServiceSession(std::filesystem::path module_filename)
    : module_filename_(std::move(module_filename)),
      configuration_resource_{this,start_configuration,stop_configuration},
      runtime_({&configuration_resource_,1}) {}
bool ServiceSession::start_configuration(void* opaque) noexcept {
    auto& session=*static_cast<ServiceSession*>(opaque);
    try {
        const auto& module=session.module_filename_;
        const auto& native=module.native();
        if(!module.is_absolute() || !module.has_filename() ||
           native.find(std::filesystem::path::value_type{})!=std::filesystem::path::string_type::npos) {
            session.load_status_=FileConfigResult::invalid_path;return false;
        }
        // Encoding conversion is strict on Windows; semantic codec also rejects
        // invalid UTF-8, NUL and non-XML codepoints on portable platforms.
        const auto utf8=(module.parent_path()/"surge.png").u8string();
        std::string fallback(reinterpret_cast<const char*>(utf8.data()),utf8.size());
        std::size_t encoded=0;
        if(redfield::xml::escape_attribute_value({fallback.data(),fallback.size()},nullptr,0,&encoded)!=redfield::xml::ValueCodecStatus::ok) {
            session.load_status_=FileConfigResult::invalid_path;return false;
        }
        session.configuration_=std::make_unique<ConfigStore>(std::move(fallback));
        session.load_status_=load_module_config(session.module_filename_,*session.configuration_);
        return session.load_status()==FileConfigResult::loaded || session.load_status()==FileConfigResult::missing;
    } catch (const std::filesystem::filesystem_error&) { session.load_status_=FileConfigResult::invalid_path; return false; }
    catch (...) { session.load_status_=FileConfigResult::resource; return false; }
}
bool ServiceSession::stop_configuration(void* opaque) noexcept {
    static_cast<ServiceSession*>(opaque)->configuration_.reset();
    return true;
}
std::shared_ptr<const ConfigSnapshot> ServiceSession::configuration() {
    auto lease=runtime_.acquire();
    return lease && configuration_ ? configuration_->snapshot() : nullptr;
}
SessionParseResult ServiceSession::parse_request(std::string_view frame, xml::Request& request) {
    request={};
    auto lease=runtime_.acquire();
    if (!lease) return SessionParseResult::closed;
    return xml::parse(frame,request)==xml::Result::ok ? SessionParseResult::parsed : SessionParseResult::invalid_frame;
}
}
