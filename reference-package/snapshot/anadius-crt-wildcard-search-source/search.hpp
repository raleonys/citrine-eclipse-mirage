#pragma once
#include "../anadius-crt-wildcard-storage-source/storage.hpp"
#include <cstdint>

namespace anadius::crt_wildcard_search {
using crt_wildcard_storage::Address;
using crt_wildcard_storage::Vector;
struct Conversion {
    Address data{};
    bool owned{};
    std::uint32_t status{};
};
struct Effects : crt_wildcard_storage::Effects {
    virtual std::uint8_t read_u8(Address) = 0;
    // Mandatory DBCS-aware previous-character helper; must make progress within
    // [start,current), except current==start. No single-byte approximation.
    virtual Address previous_077168(Address start, Address current) = 0;
    // Entire inlined LocaleUpdate/066c1c selection region, including its cleanup.
    // Called separately for the pattern and every enumerated filename.
    virtual std::uint32_t choose_code_page() = 0;
    virtual Conversion to_wide_06004c(Address source, std::uint32_t code_page) = 0;
    virtual Conversion to_narrow_06e430(Address source, std::uint32_t code_page) = 0;
    // Exact native arguments abstracted here: info=0, search=0, filter=NULL,
    // flags=0. filename receives the cFileName identity in the find-data record.
    virtual Address find_first(Address wide_pattern, Address& filename) = 0;
    virtual std::int32_t find_next(Address handle, Address& filename) = 0;
    virtual void find_close(Address handle) = 0;
    // Mandatory 0768f0 sort with element size 8 and comparator 06e41c.
    virtual void sort_new_segment(Address begin, std::uint64_t count) = 0;
};
// Normal-flow source reconstruction. All addresses are opaque provider-owned
// identities, never preferred-VA function pointers. original NULL-dereference
// and malformed previous-character domains throw domain_error, without invented
// cleanup or errno recovery.
std::uint32_t search_06e984(Address pattern, Address wildcard, Vector&, Effects&);
} // namespace anadius::crt_wildcard_search
