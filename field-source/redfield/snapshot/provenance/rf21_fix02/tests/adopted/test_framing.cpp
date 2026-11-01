#include "preloader_l/xml/lsx_framing.hpp"

#include <cstddef>
#include <cstring>

using namespace preloader_l::xml;

int main()
{
    const char* partials[] = {"", "<", "<L", "<LS", "<LSX", "<LSX>"};
    for (const char* p : partials) {
        const auto r = scan_lsx_frame(p, std::strlen(p));
        if (r.status != FrameStatus::need_more || r.frame_length != 0) return 1;
    }

    constexpr char empty_frame[] = "<LSX></LSX>";
    auto r = scan_lsx_frame(empty_frame, sizeof(empty_frame) - 1);
    if (r.status != FrameStatus::frame_ready || r.frame_length != sizeof(empty_frame) - 1) return 2;

    constexpr char request[] = "<LSX><Request id=\"1\"/></LSX>";
    r = scan_lsx_frame(request, sizeof(request) - 1);
    if (r.status != FrameStatus::frame_ready || r.frame_length != sizeof(request) - 1) return 3;

    // A close tag split across receive calls remains incomplete until all six
    // bytes are present.
    constexpr char split[] = "<LSX><Request/></LSX>";
    for (size_t n = 5; n < sizeof(split) - 1; ++n) {
        r = scan_lsx_frame(split, n);
        if (n < sizeof(split) - 1 && r.status == FrameStatus::frame_ready) return 4;
    }
    r = scan_lsx_frame(split, sizeof(split) - 1);
    if (r.status != FrameStatus::frame_ready) return 5;

    // Multiple accumulated frames: caller removes frame_length and scans the
    // remaining bytes again.
    constexpr char two[] = "<LSX><A/></LSX><LSX><B/></LSX>";
    r = scan_lsx_frame(two, sizeof(two) - 1);
    constexpr size_t first_len = sizeof("<LSX><A/></LSX>") - 1;
    if (r.status != FrameStatus::frame_ready || r.frame_length != first_len) return 6;
    const auto second = scan_lsx_frame(two + r.frame_length, (sizeof(two) - 1) - r.frame_length);
    if (second.status != FrameStatus::frame_ready || second.frame_length != sizeof("<LSX><B/></LSX>") - 1) return 7;

    // Trailing decrypted padding/noise is outside the XML frame, matching the
    // observed end-tag truncation rule.
    const char padded[] = {'<','L','S','X','>','<','A','/','>','<','/','L','S','X','>', 0x10,0x10,0x10};
    r = scan_lsx_frame(padded, sizeof padded);
    if (r.status != FrameStatus::frame_ready || r.frame_length != 15) return 8;

    constexpr char leading_space[] = " <LSX></LSX>";
    if (scan_lsx_frame(leading_space, sizeof(leading_space) - 1).status != FrameStatus::invalid_prefix) return 9;
    constexpr char wrong_case[] = "<lsx></lsx>";
    if (scan_lsx_frame(wrong_case, sizeof(wrong_case) - 1).status != FrameStatus::invalid_prefix) return 10;
    if (scan_lsx_frame(nullptr, 1).status != FrameStatus::invalid_argument) return 11;

    // A close marker inside either quote style belongs to the attribute value,
    // not to the surrounding LSX frame.
    constexpr char quoted_double[] =
        "<LSX><Request id=\"a</LSX>b\"><GetConfig/></Request></LSX>";
    r = scan_lsx_frame(quoted_double, sizeof(quoted_double) - 1);
    if (r.status != FrameStatus::frame_ready || r.frame_length != sizeof(quoted_double) - 1) return 12;

    constexpr char quoted_single[] =
        "<LSX><Request id='a</LSX>b'><GetConfig/></Request></LSX>";
    r = scan_lsx_frame(quoted_single, sizeof(quoted_single) - 1);
    if (r.status != FrameStatus::frame_ready || r.frame_length != sizeof(quoted_single) - 1) return 13;

    // The quoted marker alone cannot complete an accumulated frame.
    constexpr char quoted_partial[] = "<LSX><Request id=\"a</LSX>b\">";
    r = scan_lsx_frame(quoted_partial, sizeof(quoted_partial) - 1);
    if (r.status != FrameStatus::need_more || r.frame_length != 0) return 14;

    constexpr char comment_marker[] = "<LSX><!-- > </LSX> --><A/></LSX><LSX><B/></LSX>";
    r=scan_lsx_frame(comment_marker,sizeof(comment_marker)-1);
    if(r.status!=FrameStatus::frame_ready || r.frame_length!=sizeof("<LSX><!-- > </LSX> --><A/></LSX>")-1) return 15;
    constexpr char cdata_marker[] = "<LSX><![CDATA[ > </LSX> <Name>x</Name> ]]><A/></LSX>";
    r=scan_lsx_frame(cdata_marker,sizeof(cdata_marker)-1);
    if(r.status!=FrameStatus::frame_ready || r.frame_length!=sizeof(cdata_marker)-1) return 16;
    constexpr char pi_marker[] = "<LSX><?p x='>' y='</LSX>'?><A/></LSX>";
    r=scan_lsx_frame(pi_marker,sizeof(pi_marker)-1);
    if(r.status!=FrameStatus::frame_ready || r.frame_length!=sizeof(pi_marker)-1) return 17;

    return 0;
}
