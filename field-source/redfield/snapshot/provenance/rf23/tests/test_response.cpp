// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml/response_builder.hpp"

#include <cstring>

using namespace redfield::xml;

static TextSpan s(const char* p) { return {p, std::strlen(p)}; }

int main()
{
    constexpr char expected[] =
        " <LSX>\n"
        "  <Response id=\"42\" sender=\"EbisuSDK\">\n"
        "    <Ok/>\n"
        "  </Response>\n"
        " </LSX>\n";

    char out[512]{};
    size_t written = 99;
    if (build_response_envelope(s("42"), s("EbisuSDK"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 1;
    if (written != sizeof(expected) - 1) return 2;
    if (std::strcmp(out, expected) != 0) return 3;
    if (out[written] != '\0') return 4;

    // No partial writes when the caller's buffer is too small.
    char small[16];
    for (char& c : small) c = '#';
    written = 77;
    if (build_response_envelope(s("42"), s("EbisuSDK"), s("<Ok/>"), small, sizeof small, &written) != ResponseStatus::capacity) return 5;
    if (written != sizeof(expected) - 1) return 6;
    for (char c : small) if (c != '#') return 7;

    // Explicit size query: nullptr/0 reports the same logical required length.
    written = 0;
    if (build_response_envelope(s("42"), s("EbisuSDK"), s("<Ok/>"), nullptr, 0, &written) != ResponseStatus::capacity) return 8;
    if (written != sizeof(expected) - 1) return 9;

    // F1426-001: semantic attribute values escape all five XML specials.
    constexpr char escaped[] =
        " <LSX>\n"
        "  <Response id=\"A&amp;B&lt;C&gt;&quot;D&apos;E\" sender=\"S&amp;T\">\n"
        "    <Ok/>\n"
        "  </Response>\n"
        " </LSX>\n";
    written = 0;
    if (build_response_envelope(s("A&B<C>\"D'E"), s("S&T"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 10;
    if (written != sizeof(escaped) - 1 || std::strcmp(out, escaped) != 0) return 11;

    // Parser-owned source attributes are decoded then re-escaped exactly once.
    constexpr char source_expected[] =
        " <LSX>\n"
        "  <Response id=\"61&amp;62\" sender=\"S&amp;T\">\n"
        "    <Ok/>\n"
        "  </Response>\n"
        " </LSX>\n";
    written = 0;
    if (build_response_envelope_from_source_attributes(s("61&amp;62"), s("S&#38;T"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 12;
    if (written != sizeof(source_expected) - 1 || std::strcmp(out, source_expected) != 0) return 13;
    if (std::strstr(out, "&amp;amp;") != nullptr) return 14;

    // AUDIT-HOTFIX05 / F1426-001: XML 1.0 TAB/LF/CR semantic attribute
    // characters are valid and retained TinyXML2 does not reject them. Keep
    // exact bytes on the semantic serializer path rather than failing the
    // whole response.
    constexpr char controls_value[] = {'A','\t','B','\n','C','\r','D','\0'};
    written = 0;
    if (build_response_envelope(s(controls_value), s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 19;
    const char* id_begin = std::strstr(out, "id=\"");
    if (!id_begin) return 20;
    id_begin += 4;
    if (std::memcmp(id_begin, controls_value, 7) != 0) return 21;

    // Source-owned numeric references decode once and are then serialized as
    // the same legal semantic controls, including CR.
    written = 0;
    if (build_response_envelope_from_source_attributes(
            s("A&#x9;B&#xA;C&#xD;D"), s("EALS"), s("<Ok/>"),
            out, sizeof out, &written) != ResponseStatus::ok) return 22;
    id_begin = std::strstr(out, "id=\"");
    if (!id_begin) return 23;
    id_begin += 4;
    if (std::memcmp(id_begin, controls_value, 7) != 0) return 24;

    // Other C0 controls remain invalid XML characters.
    constexpr char invalid_control[] = {'A','\x01','B','\0'};
    if (build_response_envelope(s(invalid_control), s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::invalid_attribute) return 25;

    // Unknown/malformed XML source entities do not leak into response markup.
    if (build_response_envelope_from_source_attributes(s("bad&unknown;"), s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::invalid_attribute) return 15;

    const TextSpan invalid{nullptr, 1};
    if (build_response_envelope(invalid, s("EbisuSDK"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::invalid_attribute) return 16;
    if (build_response_envelope(s("1"), s("EbisuSDK"), invalid, out, sizeof out, &written) != ResponseStatus::invalid_argument) return 17;
    if (build_response_envelope(s("1"), s("EbisuSDK"), s("<Ok/>"), out, sizeof out, nullptr) != ResponseStatus::invalid_argument) return 18;

    // HF18 Stage04 / F1500-001: attribute serialization accepts well-formed
    // UTF-8 XML characters and rejects malformed byte sequences before any
    // response markup is emitted. This remains a bounded XML/UTF-8 contract,
    // not a claim of full Unicode normalization or XML conformance.
    constexpr char euro_value[] = {'E', static_cast<char>(0xE2), static_cast<char>(0x82), static_cast<char>(0xAC), 'U', '\0'};
    written = 0;
    if (build_response_envelope(TextSpan{euro_value, 5u}, s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 26;
    id_begin = std::strstr(out, "id=\"");
    if (!id_begin || std::memcmp(id_begin + 4, euro_value, 5u) != 0) return 27;

    written = 0;
    if (build_response_envelope_from_source_attributes(TextSpan{euro_value, 5u}, s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 28;
    id_begin = std::strstr(out, "id=\"");
    if (!id_begin || std::memcmp(id_begin + 4, euro_value, 5u) != 0) return 29;

    constexpr char lone_continuation[] = {static_cast<char>(0x80)};
    constexpr char truncated_three[] = {static_cast<char>(0xE2), static_cast<char>(0x82)};
    constexpr char overlong_two[] = {static_cast<char>(0xC0), static_cast<char>(0xAF)};
    constexpr char surrogate_utf8[] = {static_cast<char>(0xED), static_cast<char>(0xA0), static_cast<char>(0x80)};
    constexpr char above_unicode_max[] = {static_cast<char>(0xF4), static_cast<char>(0x90), static_cast<char>(0x80), static_cast<char>(0x80)};
    constexpr char bad_continuation[] = {static_cast<char>(0xE2), '(', static_cast<char>(0xA1)};

    const TextSpan malformed_utf8[] = {
        {lone_continuation, sizeof lone_continuation},
        {truncated_three, sizeof truncated_three},
        {overlong_two, sizeof overlong_two},
        {surrogate_utf8, sizeof surrogate_utf8},
        {above_unicode_max, sizeof above_unicode_max},
        {bad_continuation, sizeof bad_continuation},
    };
    for (const TextSpan& bad : malformed_utf8) {
        written = 123u;
        if (build_response_envelope(bad, s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::invalid_attribute) return 30;
        if (written != 0u) return 31;
        written = 123u;
        if (build_response_envelope_from_source_attributes(bad, s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::invalid_attribute) return 32;
        if (written != 0u) return 33;
    }

    // Numeric source entities still decode to legal non-ASCII UTF-8 and are
    // serialized once without double escaping.
    written = 0;
    if (build_response_envelope_from_source_attributes(s("E&#x20AC;U"), s("EALS"), s("<Ok/>"), out, sizeof out, &written) != ResponseStatus::ok) return 34;
    id_begin = std::strstr(out, "id=\"");
    if (!id_begin || std::memcmp(id_begin + 4, euro_value, 5u) != 0) return 35;

    return 0;
}
