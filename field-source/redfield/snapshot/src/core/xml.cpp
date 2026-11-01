// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/xml.hpp"
namespace preloader_l::xml {
Result parse(std::string_view frame, Request& output) noexcept {
    output = {};
    if (frame.size() > max_bytes) return Result::too_large;
    const auto scan = preloader_l::xml::scan_lsx_frame(frame.data(), frame.size());
    if (scan.status == preloader_l::xml::FrameStatus::need_more) return Result::incomplete;
    if (scan.status != preloader_l::xml::FrameStatus::frame_ready) return Result::malformed;
    if (scan.frame_length != frame.size()) return Result::trailing_data;
    // One adopted TinyXML2 parse; the scanner only owns wire boundaries.
    Request staged{};
    if (preloader_l::xml::parse_request_view(frame.data(), frame.size(), &staged) != preloader_l::xml::RequestStatus::ok)
        return Result::malformed;
    output = std::move(staged);
    return Result::ok;
}
}
