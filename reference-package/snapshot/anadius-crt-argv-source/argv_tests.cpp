#include "argv.hpp"
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace recovered = anadius::crt_argv_recovered;
namespace parser = anadius::crt_argv_parse_recovered;
namespace {
std::size_t checks{};
void require(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
template<class F> void domain(F&& fn) {
    bool caught{};
    try { fn(); } catch (const std::domain_error&) { caught = true; }
    require(caught, "expected declared source-domain rejection");
}
struct Fixture final : recovered::Effects, parser::Effects {
    recovered::State* state{};
    std::vector<std::string> events;
    struct Block { void* base; std::size_t size; };
    std::vector<Block> live;
    std::vector<void*> frees;
    std::vector<std::string> expanded_strings{"expanded.exe", "alpha.txt", "beta.txt"};
    std::vector<std::string> expansion_input;
    std::string filename{"fallback.exe"};
    std::uint32_t filename_return{12};
    std::uint32_t error{99};
    std::uint64_t allocation_count{}, allocation_size{};
    std::int32_t expansion_status{};
    bool fail_allocation{}, partial_expansion{}, invalid_throws{}, lead_enabled{};
    bool malformed_expansion{}, malformed_allocation{}, unterminated_expansion{};
    bool inspect_final_publication{};
    void* parse_base{};
    char** expansion_base{};
    std::size_t lead_calls{};
    ~Fixture() override { for (auto block : live) std::free(block.base); }
    void* obtain(std::size_t size) {
        auto* memory = std::calloc(size ? size : 1, 1);
        if (!memory) throw std::bad_alloc();
        live.push_back({memory, size});
        return memory;
    }
    void initialize_multibyte_06f5f8() override { events.push_back("multibyte"); }
    std::uint32_t module_filename_06ecdc(std::span<char, 260> output) override {
        events.push_back("filename");
        require(output.data() == state->module_filename.data(), "module filename exact state storage");
        require(filename.size() < output.size(), "fixture module filename bound");
        std::copy(filename.begin(), filename.end(), output.begin());
        output[filename.size()] = '\0';
        return filename_return;
    }
    anadius::crt_errors_recovered::Cell32 errno_05d724() override {
        events.push_back("errno");
        return anadius::crt_errors_recovered::Cell32(&error);
    }
    void invalid_parameter_05c954() override {
        events.push_back("invalid");
        require(error == 22, "errno written before invalid handler");
        if (invalid_throws) throw std::runtime_error("fixture invalid handler");
    }
    recovered::Allocation calloc_066848(std::uint64_t count, std::uint64_t bytes) override {
        events.push_back("calloc");
        allocation_count = count;
        allocation_size = bytes;
        require(state->program_name == state->module_filename.data(), "program name published before allocation");
        if (fail_allocation) return {};
        require(count < 100000 && bytes == 1, "bounded fixture allocation");
        parse_base = obtain(static_cast<std::size_t>(count));
        auto storage = std::span(static_cast<std::byte*>(parse_base), static_cast<std::size_t>(count));
        if (malformed_allocation) storage = storage.first(0);
        return {parse_base, storage};
    }
    void free_0668c0(void* memory) override {
        events.push_back(memory ? "free" : "free-null");
        frees.push_back(memory);
        if (inspect_final_publication && expansion_base && !memory) {
            require(state->argv == expansion_base, "expansion argv published before final NULL free");
            require(state->argc == static_cast<std::int32_t>(expanded_strings.size()), "expansion argc published before final NULL free");
        }
        if (!memory) return;
        auto found = std::find_if(live.begin(), live.end(), [&](auto block) { return block.base == memory; });
        require(found != live.end(), "free owns fixture block exactly once");
        std::free(memory);
        live.erase(found);
    }
    std::int32_t expand_06ecd4(std::span<char*> input, recovered::Expansion& output) override {
        events.push_back("expand");
        require(!input.empty() && !input.back(), "parser array has terminal NULL");
        for (auto* value : input) {
            if (!value) break;
            expansion_input.emplace_back(value);
        }
        require(state->argv == nullptr && state->argc == 0, "globals unchanged before expansion result");
        if (expansion_status && !partial_expansion) return expansion_status;
        std::size_t text_size{};
        for (const auto& value : expanded_strings) text_size += value.size() + 1;
        const auto slots = expanded_strings.size() + 1;
        auto* block = obtain(slots * sizeof(char*) + text_size);
        auto** pointers = static_cast<char**>(block);
        auto* text = reinterpret_cast<char*>(pointers + slots);
        for (std::size_t i = 0; i < expanded_strings.size(); ++i) {
            pointers[i] = text;
            std::memcpy(text, expanded_strings[i].c_str(), expanded_strings[i].size() + 1);
            text += expanded_strings[i].size() + 1;
        }
        pointers[slots - 1] = nullptr;
        expansion_base = pointers;
        output = {pointers, {pointers, slots}};
        if (malformed_expansion) output.base = nullptr;
        if (unterminated_expansion) output.pointers = output.pointers.first(slots - 1);
        return expansion_status;
    }
    std::int32_t is_mb_lead_06f9a8(std::int32_t value) override {
        ++lead_calls;
        return lead_enabled && value == -127 ? -7 : 0;
    }
};
struct Harness {
    recovered::State state;
    Fixture fx;
    Harness() { fx.state = &state; }
    std::uint32_t run(std::int32_t mode) { return recovered::configure_0638e8(state, mode, fx, fx); }
    void command(const std::string& text) { state.command_line = {text.c_str(), text.size() + 1}; }
};
std::vector<std::string> arguments(const recovered::State& state) {
    std::vector<std::string> values;
    for (std::int32_t i = 0; i < state.argc; ++i) values.emplace_back(state.argv[i]);
    require(state.argv[state.argc] == nullptr, "published array terminates after argc");
    return values;
}
void modes_and_failures() {
    {
        Harness h;
        h.state.argc = -7;
        h.state.argv = reinterpret_cast<char**>(std::uintptr_t{0x1000});
        const auto original = h.state;
        require(h.run(0) == 0, "mode zero success");
        require(h.fx.events.empty() && h.fx.lead_calls == 0, "mode zero no providers");
        require(h.state.argc == original.argc && h.state.argv == original.argv && h.state.program_name == nullptr,
                "mode zero leaves globals");
    }
    for (const auto mode : {std::numeric_limits<std::int32_t>::min(), -100, -1, 3, 4,
                           std::numeric_limits<std::int32_t>::max()}) {
        Harness h;
        require(h.run(mode) == 22, "invalid mode returns EINVAL");
        require(h.fx.events == std::vector<std::string>{"errno", "invalid"}, "invalid effect ordering");
        require(h.state.program_name == nullptr && h.state.argv == nullptr, "invalid mode no global publication");
        require(h.fx.error == 22 && h.fx.frees.empty(), "invalid mode writes errno without free");
    }
    {
        Harness h;
        h.fx.invalid_throws = true;
        bool caught{};
        try { (void)h.run(-1); } catch (const std::runtime_error&) { caught = true; }
        require(caught && h.fx.events == std::vector<std::string>{"errno", "invalid"},
                "invalid-handler exception has no invented cleanup");
    }
    for (auto mode : {1, 2}) {
        Harness h;
        h.fx.fail_allocation = true;
        h.state.argc = -23;
        require(h.run(mode) == 12, "allocation failure ENOMEM");
        require(h.fx.events == std::vector<std::string>{"multibyte", "filename", "calloc", "free-null", "errno", "free-null"},
                "allocation failure preserves both NULL frees and errno order");
        require(h.fx.error == 12 && h.state.argc == -23 && !h.state.argv, "allocation failure preserves argv globals");
        require(h.state.program_name == h.state.module_filename.data(), "filename pointer still published on allocation failure");
    }
    {
        Harness h;
        h.fx.malformed_allocation = true;
        domain([&] { (void)h.run(1); });
        require(h.fx.frees.size() == 1 && h.fx.frees.front() == nullptr,
                "malformed provider guard adds no RAII cleanup");
    }
}
void parsed_command_lines() {
    struct Case { std::string input; std::vector<std::string> expected; };
    const std::vector<Case> cases{
        {"prog.exe", {"prog.exe"}},
        {"\"C:\\A B\\prog.exe\" one two", {"C:\\A B\\prog.exe", "one", "two"}},
        {"prog.exe \"a b\" \"\"", {"prog.exe", "a b", ""}},
        {"prog.exe  a\t b  ", {"prog.exe", "a", "b"}},
        {" leading", {"", "leading"}},
        {"p \"a\"\"b\"", {"p", "a\"b"}},
        {"p a\\\"b", {"p", "a\"b"}},
        {R"(p \\"q)", {"p", R"(\q)"}},
    };
    for (const auto& test : cases) {
        Harness h;
        h.command(test.input);
        require(h.run(1) == 0, "mode one parse success");
        const auto actual = arguments(h.state);
        if (actual != test.expected) {
            std::cerr << "input=[" << test.input << "] actual=";
            for (const auto& value : actual) std::cerr << '[' << value << ']';
            std::cerr << " expected=";
            for (const auto& value : test.expected) std::cerr << '[' << value << ']';
            std::cerr << '\n';
        }
        require(actual == test.expected, "end-to-end quoted/backslash argv semantics");
        require(h.fx.error == 99, "success does not overwrite errno");
        require(h.state.program_name == h.state.module_filename.data(), "program name identity is module buffer");
        require(std::string(h.state.program_name) == "fallback.exe", "program name differs from command line argv0");
        require(h.fx.events == std::vector<std::string>{"multibyte", "filename", "calloc", "free-null", "free-null"},
                "mode one provider sequence");
        require(h.state.argv == h.fx.parse_base && h.fx.live.size() == 1, "mode one transfers parse allocation ownership");
        const auto pointer_begin = reinterpret_cast<std::uintptr_t>(h.state.argv);
        const auto text_begin = pointer_begin + (test.expected.size() + 1) * sizeof(char*);
        std::size_t expected_chars{};
        for (const auto& argument : test.expected) expected_chars += argument.size() + 1;
        require(reinterpret_cast<std::uintptr_t>(h.state.argv[0]) == text_begin, "pointer array and text are contiguous");
        require(h.fx.allocation_count == (test.expected.size() + 1) * 8 + expected_chars && h.fx.allocation_size == 1,
                "exact count-only pass sizes one contiguous allocation");
    }
    for (auto empty_command : {false, true}) {
        Harness h;
        std::string empty;
        if (empty_command) h.command(empty);
        h.fx.filename = "\"C:\\Fallback Path\\real.exe\"";
        h.fx.filename_return = 0; // return ignored even when source writes a path
        require(h.run(1) == 0, "empty/NULL command uses module buffer even return zero");
        require(arguments(h.state) == std::vector<std::string>{"C:\\Fallback Path\\real.exe"}, "module path fallback parsed");
    }
    {
        Harness h;
        h.fx.lead_enabled = true;
        const std::string command = std::string("p ") + static_cast<char>(0x81) + "\" tail";
        h.command(command);
        require(h.run(1) == 0, "DBCS provider composition success");
        require(arguments(h.state) == std::vector<std::string>{"p", std::string(1, static_cast<char>(0x81)) + "\"", "tail"},
                "DBCS trail quote copied as data");
    }
    {
        Harness h;
        const std::string first = "first.exe one";
        const std::string second = "second.exe two three";
        h.command(first);
        require(h.run(1) == 0, "first repeated configuration succeeds");
        auto** first_argv = h.state.argv;
        h.command(second);
        require(h.run(1) == 0 && h.state.argv != first_argv, "second configuration replaces published pointer");
        require(arguments(h.state) == std::vector<std::string>{"second.exe", "two", "three"}, "second argv contents");
        require(h.fx.live.size() == 2 && std::string(first_argv[1]) == "one",
                "no invented release of previously published allocation");
        require(std::all_of(h.fx.frees.begin(), h.fx.frees.end(), [](auto* pointer) { return pointer == nullptr; }),
                "repeated mode one only observed NULL frees");
    }
}
void expansion_paths() {
    const std::string command = "program.exe *.txt";
    {
        Harness h;
        h.command(command);
        h.fx.inspect_final_publication = true;
        require(h.run(2) == 0, "wildcard expansion success");
        require(h.fx.expansion_input == std::vector<std::string>{"program.exe", "*.txt"}, "expansion receives actual parsed array");
        require(arguments(h.state) == h.fx.expanded_strings, "expanded result published");
        require(h.fx.events == std::vector<std::string>{"multibyte", "filename", "calloc", "free-null", "expand", "free-null", "free"},
                "expansion success NULL free precedes temporary parse free");
        require(h.fx.frees.back() == h.fx.parse_base && h.fx.live.size() == 1,
                "expanded allocation survives temporary parse free");
    }
    {
        Harness h;
        h.command(command);
        h.fx.expanded_strings.clear();
        require(h.run(2) == 0 && h.state.argc == 0 && h.state.argv[0] == nullptr,
                "empty expansion success counts zero without dereference past sentinel");
    }
    for (const auto status : {7, -1, std::numeric_limits<std::int32_t>::min()}) {
        for (auto partial : {false, true}) {
            Harness h;
            h.command(command);
            h.fx.expansion_status = status;
            h.fx.partial_expansion = partial;
            require(h.run(2) == static_cast<std::uint32_t>(status), "expansion preserves low DWORD status");
            require(h.state.argv == nullptr && h.state.argc == 0 && h.fx.error == 99,
                    "expansion failure leaves argv and errno unchanged");
            require(h.fx.frees.size() == 3 && h.fx.frees[1] == h.fx.expansion_base && h.fx.frees[2] == h.fx.parse_base,
                    "expansion failure frees output then parse array");
            require(h.fx.live.empty(), "expansion failure releases partial allocation");
        }
    }
    for (auto bad : {1, 2}) {
        Harness h;
        h.command(command);
        h.fx.malformed_expansion = bad == 1;
        h.fx.unterminated_expansion = bad == 2;
        domain([&] { (void)h.run(2); });
        require(h.fx.frees.size() == 1 && h.fx.live.size() == 2 && h.state.argv == nullptr,
                "unsupported success provider causes no invented cleanup/publication");
    }
}
void size_guards() {
    constexpr auto max = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t size = 42;
    require(!recovered::allocation_size_063888(0x1fffffffffffffffULL, 0, 1, size) && size == 42,
            "pointer multiplication bound is strict");
    require(!recovered::allocation_size_063888(max, 0, 0, size), "pointer guard precedes DIV zero");
    domain([&] { (void)recovered::allocation_size_063888(0, 0, 0, size); });
    for (std::uint64_t width : {1ULL, 2ULL, 4ULL, 8ULL, 0xffffffffULL, 0xffffffffffffffffULL}) {
        const auto limit = max / width;
        require(!recovered::allocation_size_063888(0, limit, width, size), "character multiplication bound strict");
        require(recovered::allocation_size_063888(0, limit - 1, width, size) && size == (limit - 1) * width,
                "character multiplication below bound accepted");
    }
    for (std::uint64_t pointers : {0ULL, 1ULL, 2ULL, 31ULL, 1024ULL, 0x1ffffffffffffffeULL}) {
        const auto boundary = ~(pointers * 8);
        require(!recovered::allocation_size_063888(pointers, boundary, 1, size), "sum equality rejects UINT64_MAX total");
        require(recovered::allocation_size_063888(pointers, boundary - 1, 1, size) && size == max - 1,
                "sum one below strict bound accepted");
    }
    // Small exhaustive operand set checks meaningful allocation layout identities.
    for (std::uint64_t pointers = 0; pointers < 33; ++pointers) {
        for (std::uint64_t chars = 0; chars < 65; ++chars) {
            for (std::uint64_t width : {1ULL, 2ULL, 4ULL}) {
                require(recovered::allocation_size_063888(pointers, chars, width, size) && size == pointers * 8 + chars * width,
                        "bounded allocation byte calculation");
            }
        }
    }
    Harness h;
    auto result = recovered::allocate_buffer_063888(0x1fffffffffffffffULL, 3, 1, h.fx);
    require(!result.base && h.fx.events.empty(), "failed allocation guard no provider or free");
    h.state.program_name = h.state.module_filename.data();
    result = recovered::allocate_buffer_063888(0, 0, 1, h.fx);
    require(result.base && h.fx.allocation_count == 0 && h.fx.allocation_size == 1,
            "zero size helper preserves calloc(0,1) call");
    require(h.fx.events == std::vector<std::string>{"calloc", "free-null"}, "zero allocation helper free NULL order");
}
}
int main() {
    try {
        modes_and_failures();
        parsed_command_lines();
        expansion_paths();
        size_guards();
        std::cout << "PASS argv configure/allocation: " << checks << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL argv configure/allocation after " << checks << ": " << error.what() << '\n';
        return 1;
    }
}
