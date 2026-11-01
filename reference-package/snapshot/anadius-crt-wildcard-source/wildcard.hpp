#pragma once
#include "../anadius-crt-wildcard-search-source/search.hpp"
#include "../anadius-crt-byteclass-source/byteclass.hpp"
#include "../anadius-crt-argv-bridge-source/argv_bridge.hpp"
#include "../anadius-crt-sort-source/sort.hpp"

namespace anadius::crt_wildcard_recovered {
using Address=crt_wildcard_storage::Address;
struct Result {
    Address base{};
    std::uint64_t pointer_slots{}; // new bounds metadata, not an original field
};
struct Effects:crt_wildcard_search::Effects {
    virtual crt_errors_recovered::Cell32 errno_cell_05d724()=0;
    virtual void invalid_parameter_05c954()=0;
    // Actual 063888 helper; successful storage has live zeroed char*[] followed
    // by live char[], with the requested bounds and trailing NULL pointer.
    virtual Address allocate_buffer_063888(std::uint64_t pointers,
        std::uint64_t characters,std::uint64_t element_size)=0;
};
// Bounded semantic search for the constant set "*?" used at 06e624. This does
// not reconstruct 076da0's CPU/SIMD paths or their speculative memory footprint.
Address first_wildcard(Address input,Effects&);
std::uint32_t expand_06e5bc(Address argv,Result* output,Effects&);
std::uint32_t thunk_expand_06ecd4(Address argv,Result* output,Effects&);

struct PreviousEffects:crt_byteclass_recovered::Effects {
    virtual crt_errors_recovered::Cell32 errno_cell_05d724()=0;
    virtual void invalid_parameter_05c954()=0;
    virtual std::uint32_t file_api_ansi_066c1c()=0;
};
Address previous_locale_077170(Address start,Address position,Address locale,PreviousEffects&);
Address previous_077168(Address start,Address position,PreviousEffects&);
// Extracted inline region of 06e984, not a separate original function.
std::uint32_t select_code_page_region(PreviousEffects&);
class SupportEffects:public Effects,public PreviousEffects,public crt_sort_recovered::Effects {
public:
    Address previous_077168(Address start,Address position) final;
    std::uint32_t choose_code_page() final;
    void sort_new_segment(Address begin,std::uint64_t count) final;
    std::int32_t compare(Address comparator,Address lhs,Address rhs) final;
};

// New source bridge to configure mode2. Caller guarantees argv storage and
// successful final storage have real live pointer arrays at these identities.
// Module path and multibyte setup remain mandatory inherited Policy methods.
class ExpansionPolicy:public crt_argv_bridge::Policy {
public:
    explicit ExpansionPolicy(Effects& effects):effects_(effects){}
    std::int32_t expand_06ecd4(std::span<char*>,crt_argv_recovered::Expansion&) final;
private:
    Effects& effects_;
};
}
