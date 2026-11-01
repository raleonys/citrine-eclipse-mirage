#include "src/contracts/status.hpp"
namespace preloader_l::status_recovered {
Status status_01ea20(const Path& path, std::uint32_t flags, Stats& stats) {
    const auto raw = stats.query(path, flags);
    Status result{0, 0xffff, raw.error};
    if (raw.error) {
        if (raw.error == 2 || raw.error == 3 || raw.error == 53 || raw.error == 123) result.kind = 1;
        return result;
    }
    result.permissions = (raw.attributes & 1) ? 0x16d : 0x1ff;
    if (raw.attributes & 0x400) {
        if (!raw.reparse_tag) throw OutOfEvidenceStatus("01ea20 would read an uninitialized 049104 tag for this flags=3 result");
        if (*raw.reparse_tag == 0xa000000c) { result.kind = 4; return result; }
        if (*raw.reparse_tag == 0xa0000003) { result.kind = 10; return result; }
    }
    result.kind = static_cast<std::int32_t>(((raw.attributes >> 4) & 1) + 2);
    return result;
}
StatsResult UsedStats::query(const Path& path, std::uint32_t flags) {
    if (flags != 3) throw OutOfEvidenceStatus("UsedStats only reconstructs 049104 flags=3, cached attributes=-1");
    Metadata data;
    auto error = system_.attributes_ex(path, data);
    if (error) {
        if (error != 32) return {error, 0, {}};
        const auto found = system_.find_first(path);
        if (found.handle == invalid_handle) return {found.error, 0, {}};
        (void)system_.find_close(found.handle); // Original ignores this result.
        data = found.data;
    }
    if (!(data.attributes & 0x400)) return {0, data.attributes, {}};
    // Follow the reparse target: FILE_READ_ATTRIBUTES, all three share modes,
    // OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS (no OPEN_REPARSE_POINT).
    const auto opened = system_.open_attributes(path, 0x80, 7, 3, 0x02000000);
    if (opened.error) {
        if (opened.handle != invalid_handle && !system_.close(opened.handle)) system_.abort_close_failure_05f03c();
        return {opened.error, 0, {}};
    }
    error = system_.basic_information(opened.handle, data);
    if (opened.handle != invalid_handle && !system_.close(opened.handle)) system_.abort_close_failure_05f03c();
    if (error) return {error, 0, {}};
    // Flags 3 consumes FileBasicInfo only; neither FileAttributeTagInfo nor
    // FileStandardInfo is queried, and tag remains explicitly unknown.
    return {0, data.attributes, {}};
}
}
