#include "search.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace recovered = anadius::crt_wildcard_search;
namespace storage = anadius::crt_wildcard_storage;
using recovered::Address;
namespace {
std::size_t checks{};
void require(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
struct Watson {};
struct Fixture final : recovered::Effects {
    std::map<Address, std::vector<std::uint8_t>> blocks;
    Address next{0x10000};
    std::vector<std::string> events;
    std::vector<std::string> names;
    std::vector<Address> narrow_addresses;
    std::vector<std::uint32_t> code_pages;
    std::vector<std::uint32_t> used_pages;
    std::vector<Address> frees;
    Address wide{}, find_input{}, sort_begin{};
    std::uint64_t sort_count{};
    std::uint32_t wide_status{}, narrow_status{};
    bool wide_owned{true}, narrow_owned{true}, invalid_find{};
    bool fail_vector_allocation{}, fail_reallocation{}, invalid_previous{};
    bool null_narrow{};
    std::int32_t next_true{1};
    std::size_t current{}, locale_calls{}, previous_calls{}, sorts{};
    Address dbcs_start{}, dbcs_end{};
    Address obtain(std::size_t size) {
        auto address = next;
        next += static_cast<Address>(size) + 0x100;
        blocks.emplace(address, std::vector<std::uint8_t>(size));
        return address;
    }
    Address text(const std::string& string) {
        const auto address = obtain(string.size() + 1);
        std::copy(string.begin(), string.end(), blocks.at(address).begin());
        return address;
    }
    std::uint8_t& byte(Address address) {
        auto it = blocks.upper_bound(address);
        if (it == blocks.begin()) throw std::runtime_error("unmapped fixture address");
        --it;
        if (address - it->first >= it->second.size())
            throw std::runtime_error("fixture address beyond allocation");
        return it->second[static_cast<std::size_t>(address - it->first)];
    }
    std::string string(Address address) {
        std::string result;
        for (;;) { const auto c = byte(address++); if (c == 0) return result; result += static_cast<char>(c); }
    }
    std::vector<std::string> strings(const storage::Vector& vector) {
        std::vector<std::string> result;
        for (auto position = vector.begin; position != vector.end; position += 8)
            result.push_back(string(read_word(position)));
        return result;
    }
    std::uint64_t length(Address source) override { return string(source).size(); }
    Address calloc_066848(std::uint64_t count, std::uint64_t size) override {
        events.push_back("calloc");
        if (fail_vector_allocation && count == 4 && size == 8) return 0;
        return obtain(static_cast<std::size_t>(count * size));
    }
    Address recalloc_070434(Address old, std::uint64_t count, std::uint64_t size) override {
        events.push_back("recalloc");
        if (fail_reallocation) return 0;
        const auto replacement = obtain(static_cast<std::size_t>(count * size));
        auto& previous = blocks.at(old);
        std::copy(previous.begin(), previous.end(), blocks.at(replacement).begin());
        blocks.erase(old);
        return replacement;
    }
    void free_0668c0(Address address) override {
        frees.push_back(address);
        events.push_back(address != 0 && address == wide ? "free-wide" :
            (std::find(narrow_addresses.begin(), narrow_addresses.end(), address) != narrow_addresses.end()
                ? "free-narrow" : "free-other"));
        if (address != 0 && blocks.erase(address) != 1)
            throw std::runtime_error("fixture invalid or duplicate free");
    }
    std::uint32_t memcpy_s_0767fc(Address destination, std::uint64_t capacity,
                                 Address source, std::uint64_t count) override {
        if (destination == 0 || source == 0 || capacity < count) return 22;
        for (std::uint64_t i = 0; i != count; ++i) byte(destination + i) = byte(source + i);
        return 0;
    }
    std::uint64_t read_word(Address source) override {
        std::uint64_t word{};
        for (unsigned i = 0; i != 8; ++i) word |= static_cast<std::uint64_t>(byte(source + i)) << (8 * i);
        return word;
    }
    void write_word(Address destination, std::uint64_t word) override {
        for (unsigned i = 0; i != 8; ++i) byte(destination + i) = static_cast<std::uint8_t>(word >> (8 * i));
    }
    [[noreturn]] void watson_05c9a4() override { throw Watson{}; }
    std::uint8_t read_u8(Address address) override { return byte(address); }
    Address previous_077168(Address start, Address position) override {
        ++previous_calls;
        require(position > start, "previous must not run at start");
        if (invalid_previous) return position;
        return position == dbcs_end ? dbcs_start : position - 1;
    }
    std::uint32_t choose_code_page() override {
        events.push_back("locale");
        const auto index = locale_calls++;
        return index < code_pages.size() ? code_pages[index] : 65001;
    }
    recovered::Conversion to_wide_06004c(Address, std::uint32_t page) override {
        events.push_back("wide"); used_pages.push_back(page);
        wide = text("synthetic UTF16 identity");
        return {wide, wide_owned, wide_status};
    }
    recovered::Conversion to_narrow_06e430(Address filename, std::uint32_t page) override {
        require(filename == 0xf100 + current, "filename record must follow find enumeration");
        events.push_back("narrow"); used_pages.push_back(page);
        const auto address = null_narrow ? 0 : text(names.at(current));
        narrow_addresses.push_back(address);
        return {address, narrow_owned, narrow_status};
    }
    Address find_first(Address pattern, Address& filename) override {
        events.push_back("first"); find_input = pattern; current = 0;
        filename = 0xf100;
        return invalid_find || names.empty() ? std::numeric_limits<Address>::max() : 0xcafe;
    }
    std::int32_t find_next(Address handle, Address& filename) override {
        require(handle == 0xcafe, "next handle identity");
        events.push_back("next");
        ++current;
        filename = 0xf100 + current;
        return current < names.size() ? next_true : 0;
    }
    void find_close(Address handle) override {
        require(handle == 0xcafe, "close handle identity"); events.push_back("close");
    }
    void sort_new_segment(Address begin, std::uint64_t count) override {
        events.push_back("sort"); sort_begin = begin; sort_count = count; ++sorts;
    }
};
std::size_t event(const Fixture& fx, const std::string& value) {
    const auto found = std::find(fx.events.begin(), fx.events.end(), value);
    require(found != fx.events.end(), "required event absent");
    return static_cast<std::size_t>(found - fx.events.begin());
}
void normal() {
    Fixture fx;
    storage::Vector vector;
    const auto existing = fx.text("existing");
    require(storage::append_06e800(existing, 0, 0, vector, fx) == 0, "initial append");
    const auto pattern = fx.text("C:\\sub\\*.txt");
    fx.names = {".", "..", "z.txt", "a.txt", "...", ".profile"};
    fx.code_pages = {65001, 1, 0, 1252, 65001, 1, 0};
    fx.next_true = std::numeric_limits<std::int32_t>::min();
    require(recovered::search_06e984(pattern, pattern + 7, vector, fx) == 0, "normal search");
    require(fx.strings(vector) == std::vector<std::string>{"existing", "C:\\sub\\z.txt", "C:\\sub\\a.txt", "C:\\sub\\...", "C:\\sub\\.profile"}, "prefix, dot-only filtering and fixture enumeration order");
    require(fx.sorts == 1 && fx.sort_begin == vector.begin + 8 && fx.sort_count == 4, "sort only appended segment after reallocation");
    require(event(fx, "sort") < event(fx, "close") && event(fx, "close") < event(fx, "free-wide"), "sort close wide-free order");
    require(fx.locale_calls == fx.names.size() + 1 && fx.used_pages == fx.code_pages, "locale selection repeated for every conversion");
    for (auto address : fx.narrow_addresses)
        require(std::count(fx.frees.begin(), fx.frees.end(), address) == 1, "each owned narrow allocation freed once");
    require(std::count(fx.frees.begin(), fx.frees.end(), fx.wide) == 1, "owned wide freed once");
}
void prefixes() {
    struct Case { std::string pattern; std::size_t wildcard; std::string result; bool fallback; };
    const std::vector<Case> cases{
        {"*",0,"result",false}, {"abc*",3,"result",false},
        {"C:*",2,"C:result",false}, {"C:/x*",4,"C:/result",false},
        {"foo/bar*",7,"foo/result",false}, {"/x*",2,"/result",false},
        {"foo:bar*",7,"foo:bar*",true}, {":x*",2,":x*",true},
        {"a\\b/c*",5,"a\\b/result",false}, {"a/b\\c*",5,"a/b\\result",false}
    };
    for (const auto& item : cases) {
        Fixture fx; storage::Vector vector;
        fx.names = {"result"};
        const auto pattern = fx.text(item.pattern);
        require(recovered::search_06e984(pattern, pattern + item.wildcard, vector, fx) == 0, "prefix search");
        require(fx.strings(vector) == std::vector<std::string>{item.result}, "prefix selection");
        require(fx.locale_calls == (item.fallback ? 0u : 2u), "colon fallback skips locale conversion");
        require(fx.sorts == (item.fallback ? 0u : 1u), "one appended item is sorted too");
    }
    // All nonzero single bytes: only '/' and '\\' split an ordinary prefix;
    // the colon is a drive separator at byte offset one.
    for (unsigned ch = 1; ch != 256; ++ch) {
        Fixture fx; storage::Vector vector;
        std::string input("A"); input += static_cast<char>(ch); input += '*';
        const auto pattern = fx.text(input); fx.names = {"f"};
        require(recovered::search_06e984(pattern, pattern + 2, vector, fx) == 0, "byte separator classification");
        const bool split = ch == '/' || ch == ':' || ch == '\\';
        require(fx.strings(vector) == std::vector<std::string>{split ? input.substr(0,2) + "f" : "f"}, "exact byte separator set");
    }
    Fixture fx; storage::Vector vector;
    const auto pattern = fx.text(std::string("a") + char(0x81) + char(0x5c) + '*');
    fx.dbcs_start = pattern + 1; fx.dbcs_end = pattern + 3; fx.names = {"name"};
    require(recovered::search_06e984(pattern, pattern + 3, vector, fx) == 0, "DBCS provider source traversal");
    require(fx.strings(vector) == std::vector<std::string>{"name"}, "DBCS trail backslash skipped by previous helper");
}
void lifetime_and_failures() {
    for (const bool wide_owned : {false,true}) for (const bool narrow_owned : {false,true}) {
        Fixture fx; storage::Vector vector;
        fx.wide_owned = wide_owned; fx.narrow_owned = narrow_owned; fx.names = {".", ".."};
        const auto pattern = fx.text("*");
        require(recovered::search_06e984(pattern, pattern, vector, fx) == 0, "all dots success");
        require(vector.begin == 0 && vector.end == 0 && fx.sorts == 0, "all dots yields empty vector without sort");
        require(std::count(fx.frees.begin(), fx.frees.end(), fx.wide) == (wide_owned ? 1 : 0), "wide ownership honored");
        for (auto address : fx.narrow_addresses)
            require(std::count(fx.frees.begin(), fx.frees.end(), address) == (narrow_owned ? 1 : 0), "dot ownership honored");
    }
    for (std::uint32_t status : {0u, 1u, 0x100u, 0x80000000u}) {
        Fixture fx; storage::Vector vector;
        fx.wide_status = status; fx.invalid_find = true;
        const auto pattern = fx.text("a*.txt");
        require(recovered::search_06e984(pattern, pattern + 1, vector, fx) == 0, "find-invalid literal fallback");
        require(fx.find_input == (status == 0 ? fx.wide : 0), "full DWORD conversion failure passes NULL to find");
        require(fx.strings(vector) == std::vector<std::string>{"a*.txt"} && fx.sorts == 0, "fallback literal not sorted");
        require(std::find(fx.events.begin(), fx.events.end(), "close") == fx.events.end(), "invalid handle never closed");
        require(event(fx, "calloc") < event(fx, "free-wide"), "fallback appended before wide cleanup");
    }
    for (const bool initial_failure : {false,true}) {
        Fixture fx; storage::Vector vector;
        fx.names = initial_failure ? std::vector<std::string>{"one"} : std::vector<std::string>{"one","two","three","four","five"};
        fx.fail_vector_allocation = initial_failure; fx.fail_reallocation = !initial_failure;
        const auto pattern = fx.text("dir/*");
        require(recovered::search_06e984(pattern, pattern + 4, vector, fx) == 12, "append allocation failure status");
        require(fx.strings(vector).size() == (initial_failure ? 0u : 4u), "partial entries retained on failure");
        require(fx.sorts == 0, "failure skips sort");
        const auto last_free = std::find(fx.events.rbegin(), fx.events.rend(), "free-narrow");
        require(last_free != fx.events.rend(), "failing narrow freed");
        const auto free_position = fx.events.size() - 1 - static_cast<std::size_t>(last_free - fx.events.rbegin());
        require(free_position < event(fx,"close") && event(fx,"close") < event(fx,"free-wide"), "failure cleanup narrow close wide order");
    }
    for (const auto status : {0u,1u,0x100u,0x80000000u}) {
        Fixture fx; storage::Vector vector;
        fx.names = {"file"}; fx.narrow_status = status; fx.null_narrow = status == 0;
        const auto pattern = fx.text("*");
        bool caught{};
        try { (void)recovered::search_06e984(pattern, pattern, vector, fx); }
        catch (const std::domain_error&) { caught = true; }
        require(caught, "NULL narrow dereference explicitly unsupported");
        require(std::find(fx.events.begin(), fx.events.end(), "close") == fx.events.end() && fx.frees.empty(), "fault domain invents no cleanup");
    }
    Fixture fx; storage::Vector vector;
    fx.invalid_previous = true;
    const auto pattern = fx.text("abc*");
    bool caught{};
    try { (void)recovered::search_06e984(pattern, pattern + 3, vector, fx); }
    catch (const std::domain_error&) { caught = true; }
    require(caught && fx.locale_calls == 0, "malformed previous provider rejected before effects");
}
}
int main() {
    try {
        normal(); prefixes(); lifetime_and_failures();
        std::cout << "PASS " << checks << " wildcard enumeration checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << " checks: " << error.what() << '\n'; return 1;
    }
}
