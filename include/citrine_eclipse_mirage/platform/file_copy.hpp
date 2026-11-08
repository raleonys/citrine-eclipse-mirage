// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace citrine_eclipse_mirage::file_copy {
using Path = std::u16string;
using Handle = std::intptr_t;
inline constexpr Handle invalid_handle = -1;

struct Result { bool copied; std::uint32_t error; };
struct OpenResult { Handle handle; std::uint32_t error; };
struct Identity {
    std::uint64_t volume = 0;
    std::array<std::uint8_t,16> file_id{};
    bool operator==(const Identity&) const = default;
};
struct LegacyIdentity { std::uint32_t volume, index_high, index_low; };

// Public Win32 boundaries for the complete observed filesystem copy helper.
// This is a source API, not the original compiler's binary return-value ABI.
struct SystemCalls {
    virtual ~SystemCalls() = default;
    virtual Result copy(const Path&,const Path&,bool fail_if_exists) = 0;
    virtual OpenResult open_attributes(const Path&,std::uint32_t share) = 0;
    virtual std::uint32_t file_id_info(Handle,Identity&) = 0;
    virtual std::uint32_t legacy_file_info(Handle,LegacyIdentity&) = 0;
    virtual std::uint32_t last_write(Handle,std::int64_t&) = 0;
    virtual bool close(Handle) = 0;
    [[noreturn]] virtual void abort_close_failure() = 0;
};

// RE04895c: FileIdInfo first, legacy fallback only for errors50 and87.
std::uint32_t file_identity_04895c(Handle,Identity&,SystemCalls&);
// RE048d20: retain all low-nibble options, including undocumented combinations.
// options0=none,1=skip,2=overwrite,4=update; other low-nibble values are not
// rejected or normalized. Close failure aborts, as in the original helper.
Result copy_file_048d20(const Path&,const Path&,std::uint32_t options,SystemCalls&);

class NativeSystemCalls final : public SystemCalls {
public:
    Result copy(const Path&,const Path&,bool) override;
    OpenResult open_attributes(const Path&,std::uint32_t) override;
    std::uint32_t file_id_info(Handle,Identity&) override;
    std::uint32_t legacy_file_info(Handle,LegacyIdentity&) override;
    std::uint32_t last_write(Handle,std::int64_t&) override;
    bool close(Handle) override;
    [[noreturn]] void abort_close_failure() override;
};
Result copy_file_windows_048d20(const Path&,const Path&,std::uint32_t options);
}
