// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/config/configuration.hpp"
#include <span>
#include <string_view>
#include <vector>

namespace citrine_eclipse_mirage::file {
struct FileWrite { configuration::Path path; std::string_view bytes; };
// All generated paths are exclusive, same-directory siblings. A replacement
// failure must leave the destination unchanged; absence is not an IO error.
class AtomicFileOps {
public:
    virtual ~AtomicFileOps()=default;
    virtual configuration::Path stage(const configuration::Path&,std::string_view)=0;
    virtual bool exists(const configuration::Path&)=0;
    virtual configuration::Path backup(const configuration::Path&)=0;
    virtual void replace(const configuration::Path& source,const configuration::Path& target)=0;
    virtual void remove(const configuration::Path&)=0;
    // Native staging can fail after acquiring a handle but before returning a
    // path. Its backend retains that ownership until this recovery succeeds.
    virtual void recover_staging() {}
};
// Caller serializes access. Multi-file rollback is process-local, not an
// operating-system transaction or a power-loss atomic group commit.
class AtomicFiles final {
public:
    explicit AtomicFiles(AtomicFileOps& ops):ops_(ops){}
    // Recovery paths belong to one stable ledger; duplicating them lets one
    // owner's cleanup consume the other owner's pending rollback files.
    AtomicFiles(const AtomicFiles&)=delete;
    AtomicFiles& operator=(const AtomicFiles&)=delete;
    AtomicFiles(AtomicFiles&&)=delete;
    AtomicFiles& operator=(AtomicFiles&&)=delete;
    ~AtomicFiles();
    void replace(std::span<const FileWrite>);
    void recover();
    bool recovery_pending() const noexcept;
private:
    struct Entry {
        configuration::Path target,staged,backup;
        bool existed=false,replaced=false;
    };
    AtomicFileOps& ops_;
    std::vector<Entry> entries_;
    void discard() noexcept;
};
} // namespace citrine_eclipse_mirage::file
