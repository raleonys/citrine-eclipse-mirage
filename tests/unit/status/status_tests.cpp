// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/status.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace citrine_eclipse_mirage::status;
namespace {
int checks = 0;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("status test failed at check " + std::to_string(checks)); }
struct Given final : Stats {
    StatsResult result;
    StatsResult query(const Path& p, std::uint32_t f) override { check(p == u"synthetic"); check(f == 3); return result; }
};
struct AbortSignal {};
struct Calls final : SystemCalls {
    std::vector<std::string> calls;
    std::uint32_t attribute_error = 0, basic_error = 0;
    Metadata attribute_data{0x20, 123, 456}, basic_data{0x10, 789, 0};
    FindResult found{21, 0, {0x20, 777, 888}};
    OpenResult opened{31, 0};
    bool find_close_ok = true, close_ok = true;
    std::uint32_t attributes_ex(const Path& p, Metadata& out) override { check(p == u"synthetic"); calls.push_back("attributes"); out = attribute_data; return attribute_error; }
    FindResult find_first(const Path&) override { calls.push_back("find"); return found; }
    bool find_close(Handle h) override { check(h == found.handle); calls.push_back("find-close"); return find_close_ok; }
    OpenResult open_attributes(const Path&, std::uint32_t access, std::uint32_t share, std::uint32_t disp, std::uint32_t flags) override {
        check(access == 0x80 && share == 7 && disp == 3 && flags == 0x02000000); calls.push_back("open"); return opened;
    }
    std::uint32_t basic_information(Handle h, Metadata& out) override { check(h == opened.handle); calls.push_back("basic"); out = basic_data; return basic_error; }
    bool close(Handle h) override { check(h == opened.handle); calls.push_back("close"); return close_ok; }
    [[noreturn]] void abort_close_failure_05f03c() override { calls.push_back("abort"); throw AbortSignal{}; }
    StatsResult run() { UsedStats stats(*this); return stats.query(u"synthetic", 3); }
};
void mapping() {
    Given given;
    for (std::uint32_t error = 1; error != 256; ++error) {
        given.result = {error, 0xffffffff, {}};
        const auto value = status_01ea20(u"synthetic", 3, given);
        check(value.error == error && value.permissions == 0xffff);
        check(value.kind == ((error == 2 || error == 3 || error == 53 || error == 123) ? 1 : 0));
    }
    for (std::uint32_t attrs = 0; attrs != 2048; ++attrs) {
        given.result = {0, attrs, 0x80000001};
        const auto value = status_01ea20(u"synthetic", 3, given);
        check(value.error == 0 && value.kind == static_cast<int>(((attrs >> 4) & 1) + 2));
        check(value.permissions == ((attrs & 1) ? 0x16d : 0x1ff));
    }
    for (const auto tag : {0xa000000cU, 0xa0000003U}) {
        given.result = {0, 0x410, tag};
        check(status_01ea20(u"synthetic", 3, given).kind == (tag == 0xa000000c ? 4 : 10));
    }
    given.result = {0, 0x400, {}};
    bool out = false;
    try { (void)status_01ea20(u"synthetic", 3, given); } catch (const OutOfEvidenceStatus&) { out = true; }
    check(out);
}
void route() {
    {
        Calls c; const auto r = c.run(); check(r.error == 0 && r.attributes == 0x20 && !r.reparse_tag);
        check(c.calls == std::vector<std::string>{"attributes"});
    }
    for (const auto error : {2U, 3U, 5U, 53U, 123U}) {
        Calls c; c.attribute_error = error; check(c.run().error == error); check(c.calls.size() == 1);
    }
    {
        Calls c; c.attribute_error = 32; c.find_close_ok = false;
        check(c.run().attributes == 0x20); check(c.calls == std::vector<std::string>{"attributes", "find", "find-close"});
    }
    {
        Calls c; c.attribute_error = 32; c.found.handle = invalid_handle; c.found.error = 18;
        check(c.run().error == 18); check(c.calls == std::vector<std::string>{"attributes", "find"});
    }
    {
        Calls c; c.attribute_data.attributes = 0x400;
        const auto r = c.run(); check(r.error == 0 && r.attributes == 0x10 && !r.reparse_tag);
        check(c.calls == std::vector<std::string>{"attributes", "open", "basic", "close"});
    }
    {
        Calls c; c.attribute_error = 32; c.found.data.attributes = 0x400;
        check(c.run().attributes == 0x10); check(c.calls == std::vector<std::string>{"attributes", "find", "find-close", "open", "basic", "close"});
    }
    for (const auto h : {invalid_handle, Handle{31}}) {
        Calls c; c.attribute_data.attributes = 0x400; c.opened = {h, 5};
        check(c.run().error == 5);
        check(c.calls == (h == invalid_handle ? std::vector<std::string>{"attributes", "open"} : std::vector<std::string>{"attributes", "open", "close"}));
    }
    {
        Calls c; c.attribute_data.attributes = 0x400; c.basic_error = 87;
        check(c.run().error == 87); check(c.calls.back() == "close");
    }
    for (const auto phase : {0, 1, 2}) {
        Calls c; c.attribute_data.attributes = 0x400; c.close_ok = false;
        if (phase == 0) c.opened.error = 5;
        if (phase == 1) c.basic_error = 87;
        bool aborted = false; try { (void)c.run(); } catch (const AbortSignal&) { aborted = true; }
        check(aborted && c.calls.back() == "abort");
    }
    {
        Calls c; c.attribute_data.attributes = 0x400; c.basic_data.attributes = 0x400;
        const auto raw = c.run(); check(raw.attributes == 0x400 && !raw.reparse_tag);
    }
    {
        Calls c; UsedStats stats(c); bool out = false;
        try { (void)stats.query(u"synthetic", 7); } catch (const OutOfEvidenceStatus&) { out = true; }
        check(out && c.calls.empty());
    }
}
void native(const char* executable) {
    namespace fs = std::filesystem;
    // This executable only creates and queries its own generated fixture tree.
    const auto fixture = fs::absolute(fs::path(executable)).parent_path() / "status-fixtures";
    fs::create_directories(fixture / "directory");
    const auto file = fixture / fs::path(u"synthetic-ç.txt");
    { std::ofstream out(file, std::ios::binary); out << "synthetic fixture only"; check(bool(out)); }
    NativeStats stats;
    auto result = status_01ea20(file.u16string(), 3, stats);
    check(result.kind == 2 && result.error == 0 && result.permissions == 0x1ff);
    result = status_01ea20((fixture / "directory").u16string(), 3, stats);
    check(result.kind == 3 && result.error == 0);
    result = status_01ea20((fixture / "missing-generated-name.bin").u16string(), 3, stats);
    check(result.kind == 1 && result.error == 2 && result.permissions == 0xffff);
    check(SetFileAttributesW(file.c_str(), FILE_ATTRIBUTE_READONLY) != FALSE);
    result = status_01ea20(file.u16string(), 3, stats);
    check(result.kind == 2 && result.permissions == 0x16d);
    check(SetFileAttributesW(file.c_str(), FILE_ATTRIBUTE_NORMAL) != FALSE);
    // Exercise actual CreateFileW/FileBasicInfo/CloseHandle without requiring
    // a privileged symlink or querying any original input.
    NativeSystemCalls calls;
    const auto opened = calls.open_attributes((fixture / "directory").u16string(), 0x80, 7, 3, 0x02000000);
    check(opened.error == 0 && opened.handle != invalid_handle);
    Metadata data;
    check(calls.basic_information(opened.handle, data) == 0 && (data.attributes & 0x10));
    check(calls.close(opened.handle));
    const auto found = calls.find_first(file.u16string());
    check(found.error == 0 && found.handle != invalid_handle && found.data.size == 22);
    check(calls.find_close(found.handle));
    std::cout << "Native fixture scope: " << fixture.string() << '\n';
}
}
int main(int argc, char** argv) {
    try { check(argc > 0); mapping(); route(); native(argv[0]); std::cout << "PASS " << checks << " status checks\n"; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
