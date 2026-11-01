#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"

#include <cstring>
#include <string>

using namespace preloader_l::xml;

int main()
{
    RequestView v{};

    constexpr char wrapped[] =
        "<LSX>\n  <Request id=\"42\" sender='Client' recipient=\"EALS\">\n"
        "    <GetProfile UserId=\"7\"/>\n"
        "  </Request>\n</LSX>";
    if (parse_request_view(wrapped, sizeof(wrapped) - 1, &v) != RequestStatus::ok) return 1;
    if (!v.has_request_wrapper || !span_equals(v.command, "GetProfile")) return 2;
    if (!span_equals(v.id, "42") || !span_equals(v.sender, "Client") || !span_equals(v.recipient, "EALS")) return 3;
    TextSpan profile_id{};
    if (find_attribute(v.command_open_tag,"UserId",&profile_id)!=AttributeStatus::found || !span_equals(profile_id,"7")) return 4;
    if (!span_equals(response_sender_policy(v), "EALS")) return 22;

    constexpr char direct[] = "<LSX><QueryOffers/></LSX>";
    if (parse_request_view(direct, sizeof(direct) - 1, &v) != RequestStatus::ok) return 5;
    if (v.has_request_wrapper || !span_equals(v.command, "QueryOffers")) return 6;
    if (v.id.size != 0 || v.sender.size != 0 || v.recipient.size != 0) return 7;

    // HF18 Stage03 / F1499-001: direct/unwrapped requests have exactly one
    // command. A trailing sibling must be rejected rather than silently
    // ignored, for both self-closing and explicitly closed first commands.
    constexpr char direct_second_command[] =
        "<LSX><QueryOffers/><GrantAchievement/></LSX>";
    if (parse_request_view(direct_second_command, sizeof(direct_second_command) - 1, &v) != RequestStatus::malformed) return 44;

    constexpr char direct_second_after_closed[] =
        "<LSX><SetPresence value=\"online\"></SetPresence><QueryOffers/></LSX>";
    if (parse_request_view(direct_second_after_closed, sizeof(direct_second_after_closed) - 1, &v) != RequestStatus::malformed) return 45;

    constexpr char direct_trailing_space[] =
        "<LSX><QueryOffers/> \r\n\t </LSX>";
    if (parse_request_view(direct_trailing_space, sizeof(direct_trailing_space) - 1, &v) != RequestStatus::ok) return 46;

    constexpr char spaces[] = "<LSX> \r\n\t<SetPresence value=\"online\" ></SetPresence></LSX>";
    if (parse_request_view(spaces, sizeof(spaces) - 1, &v) != RequestStatus::ok) return 8;
    if (!span_equals(v.command, "SetPresence")) return 9;

    constexpr char no_command[] = "<LSX><Request id=\"1\"/></LSX>";
    if (parse_request_view(no_command, sizeof(no_command) - 1, &v) != RequestStatus::no_command) return 10;

    // HOTFIX14 / F1571-001: the self-closing Request branch must not bypass
    // wrapper-level trailing-sibling cardinality validation.
    constexpr char self_closing_request_sibling[] =
        "<LSX><Request id=\"1\"/><QueryOffers/></LSX>";
    if (parse_request_view(self_closing_request_sibling, sizeof(self_closing_request_sibling) - 1, &v) != RequestStatus::malformed) return 47;
    constexpr char self_closing_request_space[] =
        "<LSX><Request id=\"1\"/> \r\n\t </LSX>";
    if (parse_request_view(self_closing_request_space, sizeof(self_closing_request_space) - 1, &v) != RequestStatus::no_command) return 48;

    // HOTFIX14 / F1579-001: duplicate attributes are malformed, matching the
    // retained TinyXML2 policy. Cover both parser-owned and public helper
    // surfaces, including a duplicate unrelated to the requested key.
    constexpr char duplicate_request_id[] =
        "<LSX><Request id=\"1\" id=\"2\"><QueryOffers/></Request></LSX>";
    if (parse_request_view(duplicate_request_id, sizeof(duplicate_request_id) - 1, &v) != RequestStatus::malformed) return 49;
    constexpr char duplicate_command_attr[] =
        "<LSX><Request id=\"1\"><GetSetting Name=\"Online\" Name=\"Offline\"/></Request></LSX>";
    if (parse_request_view(duplicate_command_attr, sizeof(duplicate_command_attr) - 1, &v) != RequestStatus::malformed) return 50;
    TextSpan duplicate_value{};
    constexpr char duplicate_public_tag[] = "<GetSetting Name=\"Online\" Scope=\"Game\" Scope=\"Other\">";
    if (find_attribute({duplicate_public_tag, sizeof(duplicate_public_tag) - 1}, "Name", &duplicate_value) != AttributeStatus::malformed) return 51;

    // HOTFIX14 / F1573-001: raw-source decode/compare paths validate UTF-8 and
    // XML Char legality just like normalize_source_attribute_value().
    const char raw_control_bytes[] = {'A', static_cast<char>(0x01), 'B'};
    TextSpan raw_control{raw_control_bytes, sizeof(raw_control_bytes)};
    size_t codec_written = 0;
    if (decode_source_value(raw_control, nullptr, 0, &codec_written) != ValueCodecStatus::invalid_character) return 52;
    ValueCodecStatus codec_status = ValueCodecStatus::ok;
    if (source_value_equals(raw_control, raw_control, &codec_status) || codec_status != ValueCodecStatus::invalid_character) return 53;
    const char malformed_utf8_bytes[] = {static_cast<char>(0xC0), static_cast<char>(0xAF)};
    if (decode_source_value({malformed_utf8_bytes, sizeof(malformed_utf8_bytes)}, nullptr, 0, &codec_written) != ValueCodecStatus::invalid_character) return 54;
    const char valid_utf8_bytes[] = {static_cast<char>(0xC3), static_cast<char>(0xA9)};
    if (decode_source_value({valid_utf8_bytes, sizeof(valid_utf8_bytes)}, nullptr, 0, &codec_written) != ValueCodecStatus::ok || codec_written != 2u) return 55;

    // F323-S0381-001: an early semantic mismatch must not hide malformed
    // source later in the same value.
    constexpr char mismatch_malformed_tail[] = "X&bogus;";
    constexpr char different_semantic[] = "Y";
    codec_status = ValueCodecStatus::ok;
    if (source_value_equals({mismatch_malformed_tail, sizeof(mismatch_malformed_tail)-1u},
                            {different_semantic, sizeof(different_semantic)-1u},
                            &codec_status)) return 64;
    if (codec_status != ValueCodecStatus::malformed_entity) return 65;
    constexpr char valid_nonmatch[] = "X&amp;Z";
    codec_status = ValueCodecStatus::ok;
    if (source_value_equals({valid_nonmatch, sizeof(valid_nonmatch)-1u},
                            {different_semantic, sizeof(different_semantic)-1u},
                            &codec_status)) return 66;
    if (codec_status != ValueCodecStatus::ok) return 67;

    // F319-P111-001: SemanticValue must validate entity-free aliases under the same rules as
    // entity-bearing source. Raw invalid XML bytes cannot cross the semantic boundary.
    SemanticValue semantic_value;
    if (semantic_value.assign_source(raw_control) != ValueCodecStatus::invalid_character) return 56;
    if (semantic_value.view().data != nullptr || semantic_value.view().size != 0u) return 57;
    if (semantic_value.assign_source({malformed_utf8_bytes, sizeof(malformed_utf8_bytes)}) != ValueCodecStatus::invalid_character) return 58;
    const char raw_control_entity_bytes[] = {'A', static_cast<char>(0x01), '&', 'a', 'm', 'p', ';', 'B'};
    if (semantic_value.assign_source({raw_control_entity_bytes, sizeof(raw_control_entity_bytes)}) != ValueCodecStatus::invalid_character) return 59;
    constexpr char plain_semantic[] = "plain";
    if (semantic_value.assign_source({plain_semantic, sizeof(plain_semantic)-1u}) != ValueCodecStatus::ok) return 60;
    if (semantic_value.view().data != plain_semantic || semantic_value.view().size != sizeof(plain_semantic)-1u) return 61;
    constexpr char encoded_semantic[] = "A&amp;B";
    if (semantic_value.assign_source({encoded_semantic, sizeof(encoded_semantic)-1u}) != ValueCodecStatus::ok) return 62;
    if (!span_equals(semantic_value.view(), "A&B")) return 63;

    constexpr char empty[] = "<LSX>\n</LSX>";
    if (parse_request_view(empty, sizeof(empty) - 1, &v) != RequestStatus::no_command) return 11;

    constexpr char incomplete[] = "<LSX><Request id=\"1\"><GetProfile/>";
    if (parse_request_view(incomplete, sizeof(incomplete) - 1, &v) != RequestStatus::incomplete_frame) return 12;

    constexpr char malformed_attr[] = "<LSX><Request id=42><GetProfile/></Request></LSX>";
    if (parse_request_view(malformed_attr, sizeof(malformed_attr) - 1, &v) != RequestStatus::malformed) return 13;

    // F1425-001: a valid early selector/metadata attribute must not hide a
    // malformed trailing attribute or token in the same start tag.
    constexpr char malformed_request_tail[] =
        "<LSX><Request id=\"42\" sender=\"Client\" junk><GetProfile/></Request></LSX>";
    if (parse_request_view(malformed_request_tail, sizeof(malformed_request_tail) - 1, &v) != RequestStatus::malformed) return 28;

    constexpr char malformed_command_tail[] =
        "<LSX><Request id=\"42\"><GetProfile UserId=\"7\" broken= /></Request></LSX>";
    if (parse_request_view(malformed_command_tail, sizeof(malformed_command_tail) - 1, &v) != RequestStatus::malformed) return 29;

    // PIA315-004: retained TinyXML2 accepts adjacent quoted attributes without
    // an intervening whitespace byte. Preserve that parser compatibility.
    constexpr char adjacent_attribute[] =
        "<LSX><Request id=\"42\"sender=\"Client\"><GetProfile/></Request></LSX>";
    if (parse_request_view(adjacent_attribute, sizeof(adjacent_attribute) - 1, &v) != RequestStatus::ok) return 30;
    if (!span_equals(v.id, "42") || !span_equals(v.sender, "Client")) return 43;

    constexpr char malformed_self_close[] =
        "<LSX><Request id=\"42\"><GetProfile UserId=\"7\" / ></Request></LSX>";
    if (parse_request_view(malformed_self_close, sizeof(malformed_self_close) - 1, &v) != RequestStatus::malformed) return 31;

    // PIA315-004: retained TinyXML2 also accepts raw '<' inside a quoted
    // attribute value. Do not impose a stricter project-only rejection.
    constexpr char raw_lt_attribute[] =
        "<LSX><Request id=\"42\"><GetProfile UserId=\"A<B\"/></Request></LSX>";
    if (parse_request_view(raw_lt_attribute, sizeof(raw_lt_attribute) - 1, &v) != RequestStatus::ok) return 32;

    // AUDIT-HOTFIX04 / F1425-001: the first character of every element and
    // attribute name must satisfy the retained ASCII NameStartChar subset.
    // Digits, '-' and '.' remain legal only after the first character.
    constexpr char digit_element_name[] = "<LSX><1Bad/></LSX>";
    if (parse_request_view(digit_element_name, sizeof(digit_element_name) - 1, &v) != RequestStatus::malformed) return 35;

    constexpr char hyphen_element_name[] = "<LSX><-Bad/></LSX>";
    if (parse_request_view(hyphen_element_name, sizeof(hyphen_element_name) - 1, &v) != RequestStatus::malformed) return 36;

    constexpr char dot_element_name[] = "<LSX><.Bad/></LSX>";
    if (parse_request_view(dot_element_name, sizeof(dot_element_name) - 1, &v) != RequestStatus::malformed) return 37;

    constexpr char digit_wrapped_command[] =
        "<LSX><Request id=\"42\"><1Bad/></Request></LSX>";
    if (parse_request_view(digit_wrapped_command, sizeof(digit_wrapped_command) - 1, &v) != RequestStatus::malformed) return 38;

    constexpr char digit_attribute_name[] =
        "<LSX><Request 1id=\"42\"><GetProfile/></Request></LSX>";
    if (parse_request_view(digit_attribute_name, sizeof(digit_attribute_name) - 1, &v) != RequestStatus::malformed) return 39;

    constexpr char continuation_name_chars[] =
        "<LSX><Request id=\"42\"><A1-B.C _x1=\"ok\"/></Request></LSX>";
    if (parse_request_view(continuation_name_chars, sizeof(continuation_name_chars) - 1, &v) != RequestStatus::ok) return 40;

    constexpr char underscore_name_start[] = "<LSX><_Good/></LSX>";
    if (parse_request_view(underscore_name_start, sizeof(underscore_name_start) - 1, &v) != RequestStatus::ok) return 41;

    constexpr char colon_name_start[] = "<LSX><:Good/></LSX>";
    if (parse_request_view(colon_name_start, sizeof(colon_name_start) - 1, &v) != RequestStatus::ok) return 42;

    constexpr char bad_prefix[] = " <LSX><GetProfile/></LSX>";
    if (parse_request_view(bad_prefix, sizeof(bad_prefix) - 1, &v) != RequestStatus::malformed) return 14;

    if (parse_request_view(nullptr, 1, &v) != RequestStatus::invalid_argument) return 15;
    if (parse_request_view(direct, sizeof(direct) - 1, nullptr) != RequestStatus::invalid_argument) return 16;


    constexpr char child_text[] =
        "<LSX><Request id=\"17\"><GetAuthCode><ClientId>  client-42  </ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(child_text, sizeof(child_text) - 1, &v) != RequestStatus::ok) return 17;
    if (!span_equals(v.command, "GetAuthCode") || !v.command_content.document || !v.command_content.element) return 18;
    TextSpan client{};
    if (find_direct_child_text(v.command_content, "ClientId", &client) != ChildTextStatus::found) return 19;
    if (!span_equals(client, "client-42")) return 20;
    if (find_direct_child_text(v.command_content, "Missing", &client) != ChildTextStatus::not_found) return 21;

    // PRE-AUDIT Wave5 / F10: direct-child traversal follows the proven
    // element-DOM contract for ignorable comments and a single text-bearing
    // CDATA node. PI/declaration and nested-markup expansion remain fail-closed.
    constexpr char child_comment[] =
        "<LSX><Request id=\"67\"><GetAuthCode><!--before--><ClientId>client-comment</ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(child_comment, sizeof(child_comment)-1, &v) != RequestStatus::ok) return 67;
    if (find_direct_child_text(v.command_content, "ClientId", &client) != ChildTextStatus::found || !span_equals(client,"client-comment")) return 68;

    constexpr char child_cdata[] =
        "<LSX><Request id=\"69\"><GetAuthCode><ClientId><![CDATA[  client-cdata  ]]></ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(child_cdata, sizeof(child_cdata)-1, &v) != RequestStatus::ok) return 69;
    if (find_direct_child_text(v.command_content, "ClientId", &client) != ChildTextStatus::found || !span_equals(client,"client-cdata")) return 70;

    constexpr char child_pi[] =
        "<LSX><Request id=\"71\"><GetAuthCode><?edge keep='closed'?><ClientId>x</ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(child_pi, sizeof(child_pi)-1, &v) != RequestStatus::malformed) return 71;
    // HF11: stock TinyXML2 refuses non-XML processing instructions; this is
    // an explicit researched behavioral divergence, not silently ignored.

    constexpr char nested_child_text[] =
        "<LSX><Request id=\"73\"><GetAuthCode><ClientId><Nested>x</Nested></ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(nested_child_text, sizeof(nested_child_text)-1, &v) != RequestStatus::ok) return 73;
    if (find_direct_child_text(v.command_content, "ClientId", &client) != ChildTextStatus::malformed) return 74;

    constexpr char direct_sequence[] = "<LSX><GetAuthCode><!--a--><A>1</A><!--b--><B>2</B><!--tail--></GetAuthCode></LSX>";
    if (parse_request_view(direct_sequence,sizeof(direct_sequence)-1u,&v)!=RequestStatus::ok) return 75;
    TextSpan sequence=v.command_content;
    size_t direct_cursor=0;
    DirectChildView direct_child{};
    if(next_direct_child(sequence,&direct_cursor,&direct_child)!=DirectChildStatus::found || !span_equals(direct_child.name,"A")) return 75;
    if(next_direct_child(sequence,&direct_cursor,&direct_child)!=DirectChildStatus::found || !span_equals(direct_child.name,"B")) return 76;
    if(next_direct_child(sequence,&direct_cursor,&direct_child)!=DirectChildStatus::end) return 77;

    constexpr char mismatched_nested[] =
        "<LSX><Request id=\"23\"><GetAuthCode><A><B></C></A></GetAuthCode></Request></LSX>";
    if (parse_request_view(mismatched_nested, sizeof(mismatched_nested) - 1, &v) != RequestStatus::malformed) return 23;

    constexpr char mismatched_command_close[] =
        "<LSX><Request id=\"24\"><GetAuthCode><ClientId>x</ClientId></GetProfile></Request></LSX>";
    if (parse_request_view(mismatched_command_close, sizeof(mismatched_command_close) - 1, &v) != RequestStatus::malformed) return 24;

    constexpr char missing_request_close[] =
        "<LSX><Request id=\"25\"><GetProfile/></LSX>";
    if (parse_request_view(missing_request_close, sizeof(missing_request_close) - 1, &v) != RequestStatus::malformed) return 25;

    constexpr char wrong_request_close[] =
        "<LSX><Request id=\"26\"><GetProfile/></Response></LSX>";
    if (parse_request_view(wrong_request_close, sizeof(wrong_request_close) - 1, &v) != RequestStatus::malformed) return 26;

    constexpr char second_command[] =
        "<LSX><Request id=\"27\"><GetProfile/><QueryOffers/></Request></LSX>";
    if (parse_request_view(second_command, sizeof(second_command) - 1, &v) != RequestStatus::malformed) return 27;

    // F1425-003: mirror retained TinyXML2's document-wide DepthTracker. The
    // document/LSX/Request/command ancestry consumes four depth levels, so 495
    // additional nested elements reach depth 499 and 496 would push to 500.
    auto nested_depth_request = [](size_t nested) {
        std::string xml = "<LSX><Request id=\"35\"><GetAuthCode>";
        for (size_t i = 0; i < nested; ++i) xml += "<A>";
        for (size_t i = 0; i < nested; ++i) xml += "</A>";
        xml += "</GetAuthCode></Request></LSX>";
        RequestView depth_view{};
        return parse_request_view(xml.data(), xml.size(), &depth_view);
    };
    if (nested_depth_request(495u) != RequestStatus::ok) return 33;
    if (nested_depth_request(496u) != RequestStatus::malformed) return 34;


    constexpr char comment_struct[] =
        "<LSX><Request id=\"64\"><GetAuthCode><!-- > <Name> </GetAuthCode> --><ClientId>x</ClientId></GetAuthCode></Request></LSX>";
    if (parse_request_view(comment_struct, sizeof(comment_struct)-1, &v) != RequestStatus::ok) return 64;
    constexpr char cdata_struct[] =
        "<LSX><Request id=\"65\"><GetAuthCode><![CDATA[ > <Name> </GetAuthCode> ]]></GetAuthCode></Request></LSX>";
    if (parse_request_view(cdata_struct, sizeof(cdata_struct)-1, &v) != RequestStatus::ok) return 65;
    constexpr char pi_struct[] =
        "<LSX><Request id=\"66\"><GetAuthCode><?p x='>' text='</GetAuthCode>'?></GetAuthCode></Request></LSX>";
    if (parse_request_view(pi_struct, sizeof(pi_struct)-1, &v) != RequestStatus::malformed) return 66;
    return 0;
}
