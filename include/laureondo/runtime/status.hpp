// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/config/configuration.hpp"
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace laureondo::status {
using Path = configuration::Path;
struct OutOfEvidenceStatus : std::runtime_error { using std::runtime_error::runtime_error; };
struct Status { std::int32_t kind; std::uint32_t permissions; std::uint32_t error; };
struct StatsResult {
    std::uint32_t error = 0;
    std::uint32_t attributes = 0;
    // 049104 does not initialize tag for flags=3. A provider may supply one
    // only if its actual provenance is known; never synthesize tag zero.
    std::optional<std::uint32_t> reparse_tag;
};
struct Stats { virtual ~Stats() = default; virtual StatsResult query(const Path&, std::uint32_t flags) = 0; };
Status status_01ea20(const Path&, std::uint32_t flags, Stats&);

using Handle = std::intptr_t;
inline constexpr Handle invalid_handle = -1;
struct Metadata { std::uint32_t attributes = 0; std::uint64_t last_write = 0, size = 0; };
struct OpenResult { Handle handle; std::uint32_t error; };
struct FindResult { Handle handle; std::uint32_t error; Metadata data; };
struct SystemCalls {
    virtual ~SystemCalls() = default;
    virtual std::uint32_t attributes_ex(const Path&, Metadata&) = 0;
    virtual FindResult find_first(const Path&) = 0;
    virtual bool find_close(Handle) = 0;
    virtual OpenResult open_attributes(const Path&, std::uint32_t access,
                                       std::uint32_t share, std::uint32_t disposition,
                                       std::uint32_t flags) = 0;
    virtual std::uint32_t basic_information(Handle, Metadata&) = 0;
    virtual bool close(Handle) = 0;
    [[noreturn]] virtual void abort_close_failure_05f03c() = 0;
};
// Bounded 049104 domain: flags 3, cached attributes -1. Other flags throw a
// reconstruction-domain exception, not an invented original return code.
class UsedStats final : public Stats {
public:
    explicit UsedStats(SystemCalls& system) : system_(system) {}
    StatsResult query(const Path&, std::uint32_t flags) override;
private: SystemCalls& system_;
};
class NativeSystemCalls final : public SystemCalls {
public:
    std::uint32_t attributes_ex(const Path&, Metadata&) override;
    FindResult find_first(const Path&) override;
    bool find_close(Handle) override;
    OpenResult open_attributes(const Path&, std::uint32_t, std::uint32_t,
                              std::uint32_t, std::uint32_t) override;
    std::uint32_t basic_information(Handle, Metadata&) override;
    bool close(Handle) override;
    [[noreturn]] void abort_close_failure_05f03c() override;
};
// Owns the native boundary and bounded policy; no target DLL is loaded.
class NativeStats final : public Stats {
public: StatsResult query(const Path& path, std::uint32_t flags) override {
    UsedStats policy(system_); return policy.query(path, flags);
}
private: NativeSystemCalls system_;
};
}
