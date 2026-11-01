// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml.hpp"
namespace redfield::xml {
Result parse(std::string_view frame, Request& output) noexcept {
    output = {};
    if (frame.size() > max_bytes) return Result::too_large;
    const auto scan = redfield::xml::scan_lsx_frame(frame.data(), frame.size());
    if (scan.status == redfield::xml::FrameStatus::need_more) return Result::incomplete;
    if (scan.status != redfield::xml::FrameStatus::frame_ready) return Result::malformed;
    if (scan.frame_length != frame.size()) return Result::trailing_data;
    // One adopted TinyXML2 parse; the scanner only owns wire boundaries.
    Request staged{};
    if (redfield::xml::parse_request_view(frame.data(), frame.size(), &staged) != redfield::xml::RequestStatus::ok)
        return Result::malformed;
    output = std::move(staged);
    return Result::ok;
}
}
