#include "find_native.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace fn = anadius::crt_find_native;
namespace {
std::uint64_t checks = 0;
void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
template<class F> void domain(F&& f) {
    bool threw = false;
    try { f(); } catch (const std::domain_error&) { threw = true; }
    require(threw, "explicit host address/handle domain");
}
fn::Address address(const void* pointer) {
    return static_cast<fn::Address>(reinterpret_cast<std::uintptr_t>(pointer));
}
struct Plan {
    fn::Address handle = 0;
    std::vector<std::wstring> names;
    std::vector<BOOL> results;
    BOOL close_result = TRUE;
    std::size_t cursor = 0;
    WIN32_FIND_DATAW* data = nullptr;
    bool throwing_next = false;
};
struct Fixture {
    std::vector<Plan> plans;
    std::size_t first_calls = 0, next_calls = 0, close_calls = 0;
    std::vector<fn::Address> patterns, closed;
    std::vector<WIN32_FIND_DATAW*> first_outputs;
    std::vector<Plan*> live;
    void fill(Plan& plan, const std::wstring& name) {
        require(name.size() < 260, "fixture bounded filename");
        std::memset(plan.data, 0x6b, sizeof(*plan.data));
        plan.data->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN;
        std::fill(std::begin(plan.data->cFileName), std::end(plan.data->cFileName), WCHAR{});
        std::copy(name.begin(), name.end(), plan.data->cFileName);
        plan.data->cAlternateFileName[0] = L'X';
    }
};
thread_local Fixture* current = nullptr;
HANDLE WINAPI first_api(LPCWSTR pattern, FINDEX_INFO_LEVELS info, LPVOID output,
                        FINDEX_SEARCH_OPS search, LPVOID filter, DWORD flags) {
    auto& f = *current;
    require(info == FindExInfoStandard && static_cast<int>(info) == 0, "exact first info zero");
    require(search == FindExSearchNameMatch && static_cast<int>(search) == 0, "exact first search zero");
    require(filter == nullptr && flags == 0, "exact first filter and flags zero");
    require(output != nullptr, "typed output record exists");
    require(f.first_calls < f.plans.size(), "fixture first script");
    auto& plan = f.plans.at(f.first_calls++);
    plan.data = static_cast<WIN32_FIND_DATAW*>(output);
    f.patterns.push_back(address(pattern));
    f.first_outputs.push_back(plan.data);
    if (plan.handle != std::numeric_limits<fn::Address>::max()) {
        require(!plan.names.empty(), "successful first scripted name");
        f.fill(plan, plan.names.front());
        f.live.push_back(&plan);
    }
    return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(plan.handle));
}
BOOL WINAPI next_api(HANDLE handle, LPWIN32_FIND_DATAW output) {
    auto& f = *current;
    ++f.next_calls;
    const auto it = std::find_if(f.live.begin(), f.live.end(), [handle](const Plan* p) {
        return p->handle == address(handle);
    });
    require(it != f.live.end(), "fixture known next handle");
    auto& p = **it;
    require(output == p.data, "same stable typed record at next");
    if (p.throwing_next) throw std::runtime_error("fixture exceptional call");
    require(p.cursor < p.results.size(), "fixture next result script");
    const auto result = p.results.at(p.cursor++);
    if (result != 0) {
        require(p.cursor < p.names.size(), "fixture next name script");
        f.fill(p, p.names.at(p.cursor));
    } else {
        // Failed API output is unspecified; the bridge must not treat it as a
        // successful name or add an implicit OS error policy.
        f.fill(p, L"failed-next-scribble");
    }
    return result;
}
BOOL WINAPI close_api(HANDLE handle) {
    auto& f = *current;
    ++f.close_calls;
    f.closed.push_back(address(handle));
    const auto it = std::find_if(f.live.begin(), f.live.end(), [handle](const Plan* p) {
        return p->handle == address(handle);
    });
    require(it != f.live.end(), "fixture known close handle");
    const auto result = (*it)->close_result;
    f.live.erase(it);
    return result;
}
constexpr fn::Api api{&first_api, &next_api, &close_api};
Plan plan(fn::Address handle, std::vector<std::wstring> names,
          std::vector<BOOL> results = {}, BOOL close = TRUE) {
    Plan p;
    p.handle = handle;
    p.names = std::move(names);
    p.results = std::move(results);
    p.close_result = close;
    return p;
}
void check_name(const fn::Session& s, fn::Address name, const std::wstring& value) {
    for (std::size_t i = 0; i < value.size(); ++i) {
        const auto expected = static_cast<std::uint16_t>(value[i]);
        require(s.read_u16(name + i * 2) == expected, "exact UTF16 code unit");
        require(s.read_u8(name + i * 2) == (expected & 0xff), "low byte UTF16");
        require(s.read_u8(name + i * 2 + 1) == (expected >> 8), "high byte UTF16");
    }
    require(s.read_u16(name + value.size() * 2) == 0, "terminated UTF16 filename");
}
void abi_and_domains() {
    static_assert(!std::is_copy_constructible_v<fn::Session>);
    static_assert(!std::is_move_constructible_v<fn::Session>);
    static_assert(sizeof(WIN32_FIND_DATAW) == 592 && sizeof(WCHAR) == 2);
    static_assert(offsetof(WIN32_FIND_DATAW, cFileName) == 44);
    const auto defaults = fn::system_api();
    require(defaults.first == &::FindFirstFileExW && defaults.next == &::FindNextFileW &&
            defaults.close == &::FindClose, "default API binds SDK imports without calling them");
    for (int field = 0; field != 3; ++field) {
        auto invalid = api;
        if (field == 0) invalid.first = nullptr;
        if (field == 1) invalid.next = nullptr;
        if (field == 2) invalid.close = nullptr;
        bool threw = false;
        try { fn::Session session(invalid); } catch (const std::invalid_argument&) { threw = true; }
        require(threw, "reject absent API function pointer");
    }
    fn::Session empty(api);
    fn::Address name = 17;
    require(empty.active_count() == 0 && !empty.contains(0, 0), "empty storage");
    domain([&] { (void)empty.next(0, name); });
    domain([&] { empty.close(0); });
    domain([&] { (void)empty.filename_storage(0); });
    domain([&] { (void)empty.read_u8(0); });
    domain([&] { (void)empty.read_u16(0); });
}
void full_bool_and_record_lifetime() {
    constexpr fn::Address high_handle = 0xfedcba9876543210ULL;
    std::wstring unicode = L"\x0131\x20ac\x4e2d";
    unicode += WCHAR{0xd83d}; unicode += WCHAR{0xde80}; unicode += L".dat";
    Fixture f;
    f.plans.push_back(plan(high_handle, {unicode, L".", L"..", L"hidden-dir", L"last"},
                           {0x100, -0x100, std::numeric_limits<BOOL>::min(), 1, 0}, FALSE));
    current = &f;
    fn::Session session(api);
    const std::array<WCHAR, 6> pattern{L'C', L':', L'\\', L'*', L'?', 0};
    fn::Address name = 123;
    require(session.first(address(pattern.data()), name) == high_handle, "full 64bit handle preserved");
    require(f.patterns.front() == address(pattern.data()), "original pattern pointer passed directly");
    const auto first_name = name;
    require(first_name == address(f.first_outputs.front()->cFileName), "actual SDK cFileName pointer");
    require(session.filename_storage(high_handle).data() ==
            reinterpret_cast<const std::byte*>(f.first_outputs.front()->cFileName), "span borrows typed record");
    require(session.filename_storage(high_handle).size() == 520, "bounded name area only");
    require(session.active_count() == 1, "record tracked");
    require(session.contains(name, 520) && session.contains(name + 520, 0), "full and empty end span");
    require(!session.contains(name - 1, 1) && !session.contains(name + 520, 1), "neighbor fields excluded");
    require(!session.contains(name, std::numeric_limits<std::uint64_t>::max()), "overflow width rejected");
    require(!session.contains(std::numeric_limits<fn::Address>::max(), 2), "overflow address rejected");
    domain([&] { (void)session.read_u16(name + 519); });
    domain([&] { (void)session.read_u8(name - 1); });
    check_name(session, name, unicode);
    const std::array<BOOL, 4> successes{0x100, -0x100, std::numeric_limits<BOOL>::min(), 1};
    for (std::size_t i = 0; i < successes.size(); ++i) {
        name = 777;
        require(session.next(high_handle, name) == successes[i], "full signed BOOL returned");
        require(name == first_name, "stable pointer across next calls");
        check_name(session, name, f.plans.front().names[i + 1]);
    }
    name = 888;
    require(session.next(high_handle, name) == 0 && name == 888, "failed next does not publish name");
    require(session.active_count() == 1 && f.close_calls == 0, "failed next needs explicit close");
    session.close(high_handle);
    require(f.close_calls == 1 && f.closed.front() == high_handle, "ignored false close and full handle");
    require(session.active_count() == 0 && !session.contains(first_name, 1), "close failure still invalidates storage");
    domain([&] { (void)session.read_u8(first_name); });
    domain([&] { session.close(high_handle); });
    domain([&] { (void)session.next(high_handle, name); });
    require(f.close_calls == 1 && f.next_calls == 5, "invalid domains do not call APIs");
}
void invalid_and_zero_handles() {
    Fixture f;
    f.plans.push_back(plan(std::numeric_limits<fn::Address>::max(), {}));
    f.plans.push_back(plan(0, {std::wstring(259, L'x')}));
    current = &f;
    fn::Session session(api);
    fn::Address name = 333;
    require(session.first(0, name) == std::numeric_limits<fn::Address>::max(), "invalid handle bit pattern");
    require(name == 333 && session.active_count() == 0 && f.close_calls == 0, "invalid first no record or close");
    require(f.patterns.front() == 0, "NULL conversion result forwarded to SDK");
    const WCHAR star[]{L'*', 0};
    require(session.first(address(star), name) == 0, "zero handle accepted: only -1 is invalid");
    check_name(session, name, std::wstring(259, L'x'));
    require(session.read_u16(name + 518) == 0, "last WCHAR accessible");
    session.close(0);
    require(f.close_calls == 1 && f.closed.front() == 0, "zero handle passed unchanged");
}
void simultaneous_sessions_and_many_handles() {
    Fixture f;
    constexpr std::size_t count = 96;
    f.plans.reserve(count);
    for (std::size_t i = 0; i != count; ++i)
        f.plans.push_back(plan(0x100000000ULL + i, {L"item-" + std::to_wstring(i)}));
    current = &f;
    fn::Session one(api), two(api);
    const WCHAR pattern[]{L'*', 0};
    std::array<fn::Address, count> names{};
    std::array<fn::Address, count> handles{};
    for (std::size_t i = 0; i != count; ++i) {
        auto& session = i % 2 ? two : one;
        handles[i] = session.first(address(pattern), names[i]);
        for (std::size_t j = 0; j <= i; ++j) {
            const auto& owner = j % 2 ? two : one;
            const auto& other = j % 2 ? one : two;
            require(owner.contains(names[j], 520) && !other.contains(names[j], 1), "independent simultaneous storage");
            check_name(owner, names[j], L"item-" + std::to_wstring(j));
        }
    }
    require(one.active_count() == 48 && two.active_count() == 48, "separate session active maps");
    for (std::size_t i = count; i-- > 0;) {
        auto& session = i % 2 ? two : one;
        session.close(handles[i]);
        require(!session.contains(names[i], 1), "closed record excluded while others remain");
    }
    require(f.close_calls == count && one.active_count() == 0 && two.active_count() == 0, "all explicit reverse closes");
}
void exception_and_duplicate_domains() {
    Fixture f;
    f.plans.reserve(2);
    f.plans.push_back(plan(55, {L"original"}));
    f.plans.push_back(plan(55, {L"duplicate"}));
    current = &f;
    fn::Address old_name = 0;
    {
        fn::Session session(api);
        (void)session.first(0, old_name);
        fn::Address name = 999;
        domain([&] { (void)session.first(0, name); });
        require(name == 999 && session.active_count() == 1 && f.close_calls == 0, "duplicate handle no invented cleanup");
        check_name(session, old_name, L"original");
        f.plans.front().throwing_next = true;
        bool threw = false;
        try { (void)session.next(55, name); } catch (const std::runtime_error&) { threw = true; }
        require(threw && f.close_calls == 0 && session.active_count() == 1, "exception keeps explicit-close policy");
        check_name(session, old_name, L"original");
    }
    require(f.close_calls == 0, "Session destruction never invents FindClose");
}
}
int main() {
    try {
        abi_and_domains();
        full_bool_and_record_lifetime();
        invalid_and_zero_handles();
        simultaneous_sessions_and_many_handles();
        exception_and_duplicate_domains();
        std::cout << "find native: " << checks << " checks PASS\n";
    } catch (const std::exception& e) {
        std::cerr << "find native test failure: " << e.what() << '\n';
        return 1;
    }
}
