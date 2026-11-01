#include "key_processor.hpp"
#include "../anadius-crypto-source/crypto.hpp"
#include "../anadius-path-source/paths.hpp"
#include "../anadius-token-source/constants.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <filesystem>
#include <system_error>

namespace anadius::license_recovered {
namespace {
constexpr std::array<char,8> header{
    '\x30','\x06','\x02','\x01','\x00','\x02','\x01','\x00'
};
constexpr std::string_view opening=
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
    "<License xmlns=\"http://ea.com/license\"><CipherKey>";
constexpr std::string_view content=
    "</CipherKey><MachineHash>1</MachineHash><ContentId>";
constexpr std::string_view user="</ContentId><UserId>";
constexpr std::string_view closing=
    "</UserId><GrantTime>2024-01-01T00:00:00Z</GrantTime>"
    "<StartTime>2024-01-01T00:00:00Z</StartTime></License>";
static_assert(opening.size()==0x69&&content.size()==0x33&&
              user.size()==0x14&&closing.size()==0x69);
}

// Pseudocode ev_f896d1d300fedafcee6570b765c7cecc78a5c0ae23182cea35a454ec6d0199a7
// Assembly   ev_15aca81fc2f57a75ce4a92b8ce30822f78fa1d0cee3366522019ef53c167e64e
void apply_license_key_02a830(std::string owned_key,KeyState state,KeyDependencies deps) {
    auto path=state.license_base;
    path+=encoding_recovered::filesystem_widen_006d70(".dlf",deps.encoding,deps.errors);
    const auto status=status_recovered::status_01ea20(path,3,deps.stats);
    const auto kind=static_cast<std::uint32_t>(status.kind);
    if(status.error&&((kind-1u)&0xfffffff7u)!=0) {
        throw std::filesystem::filesystem_error("status",std::filesystem::path(path),
            std::error_code(std::bit_cast<std::int32_t>(status.error),std::system_category()));
    }
    if(status.kind==2)return; // Existing regular file skips all creation work.

    std::string document;
    document.reserve(0x40f); // Native allocation0x410 includes the terminator.
    document.resize(token_recovered::constants::encrypted_offset,'\0');
    std::copy(header.begin(),header.end(),document.begin());
    document.append(opening);
    document.append(owned_key); // Counted bytes; no XML escaping or NUL scan.
    document.append(content);
    document.append(state.content_id);
    document.append(user);
    document.append(state.user_id);
    document.append(closing);

    crypto_recovered::Cipher cipher;
    cipher.set_key_and_iv_042760(token_recovered::constants::key_084a50,
                               token_recovered::constants::iv_084a60);
    // 02ad39 ignores the low-byte result, as 02ae66 ignores the file result.
    static_cast<void>(cipher.encrypt_with_offset(document,
        token_recovered::constants::encrypted_offset));
    {
        const auto parent=path_recovered::parent_path_019820(path);
        const auto error=deps.filesystem.create_directories_error(parent);
        if(error) {
            throw std::filesystem::filesystem_error("create_directories",
                std::filesystem::path(parent),
                std::error_code(std::bit_cast<std::int32_t>(error),std::system_category()));
        }
    } // 02ae1c..02ae57 releases this Path before the output is opened.
    static_cast<void>(file_recovered::write_binary_03c1d0(path,document,deps.files));
}
} // namespace anadius::license_recovered
