#include "registry.hpp"
#include <limits>

namespace anadius::registry_recovered {
std::string resolve_narrow_008980(const Values& v, ValueEffects& e, std::string_view k) {
    const auto it = v.find(k);
    return it == v.end() ? std::string{} : e.filesystem_narrow(it->second);
}
std::u16string resolve_wide_008b80(const Values& v, std::string_view k) {
    const auto it = v.find(k);
    return it == v.end() ? std::u16string{} : it->second;
}
int compare_narrow_0086b0(const Values& v, ValueEffects& e, const char* s, std::string_view k) {
    if (!s) return 1;
    const auto value = resolve_narrow_008980(v, e, k);
    return e.crt_stricmp(s, value.c_str());
}
int compare_wide_0087f0(const Values& v, ValueEffects& e, const char16_t* s, std::string_view k) {
    if (!s) return 1;
    const auto value = resolve_wide_008b80(v, k);
    return e.crt_wcsicmp(s, value.c_str());
}
Status write_value_a_0091a0(const Values& v, ValueEffects& e, void* data,
    DwordCell size, std::string_view key, std::uint32_t capacity) {
    if (!size) return 234;
    size.store(capacity);
    const auto value = resolve_narrow_008980(v, e, key);
    const auto captured_capacity = size.load(); // conversion may have observable effects
    if (value.size() == std::numeric_limits<std::size_t>::max())
        throw OutOfEvidenceInput("value size plus NUL overflows");
    const auto need = value.size() + 1;
    size.store(static_cast<std::uint32_t>(need));
    if (!data) return 0;
    if (need > captured_capacity) return 234;
    return e.crt_strcpy_s(static_cast<char*>(data), need, value.c_str()) ? 234 : 0;
}
Status write_value_w_0093a0(const Values& v, ValueEffects& e, void* data,
    DwordCell size, std::string_view key, std::uint32_t capacity) {
    if (!size) return 234;
    size.store(capacity);
    const auto value = resolve_wide_008b80(v, key);
    const auto captured_capacity = size.load();
    if (value.size() > (std::numeric_limits<std::size_t>::max() / 2) - 1)
        throw OutOfEvidenceInput("wide value byte size overflows");
    const auto chars = value.size() + 1;
    const auto need = chars * 2;
    size.store(static_cast<std::uint32_t>(need));
    if (!data) return 0;
    if (need > captured_capacity) return 234;
    return e.crt_wcscpy_s(static_cast<char16_t*>(data), chars, value.c_str()) ? 234 : 0;
}
void fabricate_010170(StateRefs s, Effects& e, HandleCell result, Status& status) {
    if (status == 0) return;
    if (!result) throw OutOfEvidenceInput("original fabricated-key path dereferences result");
    result.store(e.allocate_4_04b0bc());
    s.fabricated_0a3800.insert(result.load());
    e.log_literal_006880(0x180080528);
    e.finish_log_0088e0();
    status = 0;
}
namespace {
template<class Open, class Original, class Compare>
Status open_impl(StateRefs s, Effects& e, const Open& a, Original original, Compare compare) {
    auto status = original(a); // matching always follows original, including failure
    Handles* track = nullptr;
    if (compare(a.subkey, "ClientKey") == 0 || compare(a.subkey, "ClientKey2") == 0)
        track = &s.client_0a37e0;
    else if (s.language_enabled_0a3ad1 && compare(a.subkey, "LanguageKey") == 0)
        track = &s.language_0a37f0;
    if (!track) return status;
    fabricate_010170(s, e, a.result, status);
    if (!a.result) throw OutOfEvidenceInput("matched open dereferences result");
    track->insert(a.result.load());
    return status;
}
template<class Query, class Original, class Compare, class Write>
Status query_impl(StateRefs s, Effects& e, const Query& a, Original original, Compare compare, Write write) {
    const auto capacity = a.size ? a.size.load() : 0;
    const auto status = original(a);
    Status replacement = 0;
    bool replaced = false;
    if (s.client_0a37e0.contains(a.key) && compare(a.name, "ClientValue") == 0) {
        e.wait_036880(-1);
        e.log_literal_006880(0x1800808c8);
        e.finish_log_0088e0();
        replacement = write(a.data, a.size, "ClientPath", capacity);
        replaced = true;
    } else if (s.language_0a37f0.contains(a.key) && compare(a.name, "LanguageValue") == 0) {
        // Original obtains its narrow log string before emitting the prefix.
        const auto language = resolve_narrow_008980(s.values, e, "Language");
        e.log_literal_006880(0x1800808e0);
        e.log_text_007990(language);
        e.finish_log_0088e0();
        replacement = write(a.data, a.size, "Language", capacity);
        replaced = true;
    }
    if (replaced) {
        if (a.type && a.type.load() == 0) a.type.store(1);
        return replacement;
    }
    if (s.fabricated_0a3800.contains(a.key)) {
        e.log_literal_006880(0x180080900);
        e.finish_log_0088e0();
        return 2;
    }
    return status;
}
}
Status open_a_00f800(StateRefs s, Effects& e, const OpenA& a) {
    return open_impl(s,e,a,[&](const OpenA& x){return e.original_open_a(x);},
        [&](const char* x,std::string_view k){return compare_narrow_0086b0(s.values,e,x,k);});
}
Status open_w_00f960(StateRefs s, Effects& e, const OpenW& a) {
    return open_impl(s,e,a,[&](const OpenW& x){return e.original_open_w(x);},
        [&](const char16_t* x,std::string_view k){return compare_wide_0087f0(s.values,e,x,k);});
}
Status query_a_00fac0(StateRefs s, Effects& e, const QueryA& a) {
    return query_impl(s,e,a,[&](const QueryA& x){return e.original_query_a(x);},
        [&](const char* x,std::string_view k){return compare_narrow_0086b0(s.values,e,x,k);},
        [&](void* d,DwordCell n,std::string_view k,std::uint32_t c){return write_value_a_0091a0(s.values,e,d,n,k,c);});
}
Status query_w_00fb60(StateRefs s, Effects& e, const QueryW& a) {
    return query_impl(s,e,a,[&](const QueryW& x){return e.original_query_w(x);},
        [&](const char16_t* x,std::string_view k){return compare_wide_0087f0(s.values,e,x,k);},
        [&](void* d,DwordCell n,std::string_view k,std::uint32_t c){return write_value_w_0093a0(s.values,e,d,n,k,c);});
}
Status close_00f5b0(StateRefs s, Effects& e, Handle key) {
    if (s.fabricated_0a3800.contains(key)) {
        s.fabricated_0a3800.erase(key);
        e.free_04b0f8(key);
        e.log_literal_006880(0x180080540);
        e.finish_log_0088e0();
    } else {
        const auto status = e.original_close(key);
        if (status) return status;
    }
    s.client_0a37e0.erase(key);
    s.language_0a37f0.erase(key);
    return 0;
}
}
