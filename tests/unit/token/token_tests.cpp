// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/core/token.hpp"
#include "preloader_l/core/token_constants.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace token = preloader_l::token;
namespace crypto = preloader_l::crypto;
namespace {
unsigned checks;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("token check failed at " + std::to_string(checks)); }
struct FilesystemFailure {};
struct ReadFailure {};
struct Io final : token::Dependencies {
    token::FileStatus status{2,0};
    std::string bytes;
    std::vector<std::string> calls;
    bool fail_read = false;
    token::FileStatus status_01ea20(const token::Path& path, std::uint32_t flags) override {
        check(path == u"synthetic-token.bin" && flags == 3); calls.emplace_back("status"); return status;
    }
    [[noreturn]] void filesystem_error_018130(std::string_view operation, std::int32_t error, const token::Path& path) override {
        check(operation == "status" && error == status.error && path == u"synthetic-token.bin");
        calls.emplace_back("error"); throw FilesystemFailure{};
    }
    std::string read_binary_03b3f0(const token::Path& path) override {
        check(path == u"synthetic-token.bin"); calls.emplace_back("read");
        if (fail_read) throw ReadFailure{};
        return bytes;
    }
};
std::string load(Io& io) { return token::load_token_02afa0(u"synthetic-token.bin", io); }
std::string encrypted(std::string body, char prefix = 'H') {
    std::string value(65,prefix); value += body;
    crypto::Cipher cipher; cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    check(cipher.encrypt_with_offset(value,65)); return value;
}
// Build CBC ciphertext from raw blocks without the high-level padding helper.
std::string raw_cbc(std::string plaintext) {
    check((plaintext.size() & 15) == 0);
    std::string result(65,'H');
    crypto::Cipher cipher; cipher.set_key_042780(token::constants::key_084a50);
    crypto::Key previous{};
    for (std::size_t pos = 0; pos < plaintext.size(); pos += 16) {
        crypto::Key block{};
        for (unsigned j = 0; j != 16; ++j) block[j] = static_cast<unsigned char>(plaintext[pos+j]) ^ previous[j];
        cipher.encrypt_block_045670(block);
        result.append(reinterpret_cast<const char*>(block.data()),block.size()); previous = block;
    }
    return result;
}
void search_goldens() {
    const std::string prefix(65,'H');
    check(token::extract_game_token(prefix + "<GameToken>synthetic</GameToken>") == "synthetic");
    check(token::extract_game_token(prefix + "<GameToken></GameToken>").empty());
    check(token::extract_game_token(prefix + "<GameToken>A&amp;B&#65;</GameToken>") == "A&amp;B&#65;");
    check(token::extract_game_token(prefix + "<GameToken>A<Other>B</Other>C</GameToken>") == "A<Other>B</Other>C");
    check(token::extract_game_token(prefix + "<GameToken>first</GameToken><GameToken>second</GameToken>") == "first");
    check(token::extract_game_token(prefix + "<GameToken>outer<GameToken>inner</GameToken>") == "outer<GameToken>inner");
    check(token::extract_game_token(prefix + "<GameToken a='x'>wrong</GameToken>").empty());
    check(token::extract_game_token(prefix + "<gametoken>wrong</gametoken>").empty());
    check(token::extract_game_token(prefix + "<GameToken>missing</GameToken").empty());
    check(token::extract_game_token(prefix + "<GameToken>").empty());
    check(token::extract_game_token(prefix + "</GameToken><GameToken>right</GameToken>") == "right");
    std::string zero_body = "<GameToken>A"; zero_body.push_back('\0'); zero_body += "B</GameToken>";
    check(token::extract_game_token(prefix + zero_body) == std::string("A\0B",3));
    check(token::extract_game_token(std::string(65,'\0') + "<GameToken>after-zero</GameToken>") == "after-zero");
    // Position 64 is rejected even when a complete token follows; 65 is inclusive.
    check(token::extract_game_token(std::string(64,'H') + "<GameToken>before</GameToken>").empty());
    for (std::size_t before = 65; before < 97; ++before)
        check(token::extract_game_token(std::string(before,'H') + "<GameToken>x</GameToken>") == "x");
    for (std::size_t length = 0; length < 100; ++length) check(token::extract_game_token(std::string(length,'<')).empty());
}
void input_domains() {
    for (std::size_t length = 0; length <= 65; ++length) {
        Io io; io.bytes.assign(length,'H');
        const bool unsupported = length == 1 || length == 17 || length == 33 || length == 49;
        bool threw = false;
        try { check(load(io).empty()); } catch (const crypto::OutOfEvidenceInput&) { threw = true; }
        check(threw == unsupported); check(io.calls == std::vector<std::string>({"status","read"}));
    }
    // With no ciphertext blocks the padding validator still examines the HEADER.
    Io io; io.bytes.assign(65,'H'); io.bytes.back() = '\x01'; check(load(io).empty());
}
void status_and_errors() {
    for (auto kind : std::array<std::int32_t,8>{0,1,3,4,9,10,-1,2}) {
        Io io; io.status = {kind,0}; check(load(io).empty());
        check(io.calls == (kind == 2 ? std::vector<std::string>{"status","read"} : std::vector<std::string>{"status"}));
    }
    for (auto kind : std::array<std::int32_t,9>{0,1,2,3,4,9,10,-1,17}) {
        Io io; io.status = {kind,5}; bool threw = false;
        try { check(load(io).empty()); } catch (const FilesystemFailure&) { threw = true; }
        check(threw == (kind != 1 && kind != 9));
        check(io.calls == (threw ? std::vector<std::string>{"status","error"} : std::vector<std::string>{"status"}));
    }
    Io io; io.fail_read = true; bool threw = false;
    try { static_cast<void>(load(io)); } catch (const ReadFailure&) { threw = true; }
    check(threw); check(io.calls == std::vector<std::string>({"status","read"}));
}
void concrete_cbc() {
    for (const auto& body : std::array<std::string,6>{
        "<GameToken>synthetic</GameToken>",
        "<GameToken>A&amp;B</GameToken>",
        std::string("<GameToken>A\0B</GameToken>",26),
        "<GameToken>first</GameToken><GameToken>second</GameToken>",
        "not-an-xml-document", "<GameToken>no-close"}) {
        Io io; io.bytes = encrypted(body);
        check(load(io) == token::extract_game_token(std::string(65,'H') + body));
    }
    // False decrypt result caused by nonalignment leaves original bytes intact.
    Io plain; plain.bytes = std::string(65,'H') + "<GameToken>raw</GameToken>!";
    check(((plain.bytes.size()-65)&15) != 0); check(load(plain) == "raw");
    // Invalid padding does NOT roll back in-place decryption or suppress scanning.
    std::string invalid = "<GameToken>mutated</GameToken>";
    invalid.resize(32,'\0');
    Io bad; bad.bytes = raw_cbc(invalid); check(load(bad) == "mutated");
    crypto::Cipher cipher; cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    auto decoded = bad.bytes; check(!cipher.decrypt_with_offset(decoded,65));
    check(decoded == std::string(65,'H') + invalid);
    check(bad.bytes != decoded); // Dependency fixture itself is copied, never modified.
    Io before; before.bytes = encrypted("nothing");
    before.bytes.replace(0,24,"<GameToken>x</GameToken>"); check(load(before).empty());
}
}
int main() {
    try { search_goldens(); input_domains(); status_and_errors(); concrete_cbc();
        std::cout << checks << " synthetic token checks PASS\n"; return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
