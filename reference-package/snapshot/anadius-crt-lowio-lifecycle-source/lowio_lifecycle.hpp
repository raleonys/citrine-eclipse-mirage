#pragma once
#include <array>
#include <cstdint>
namespace anadius::crt_lowio_lifecycle_recovered {
using Address=std::uint64_t;
inline constexpr std::size_t table_slots=128,records_per_array=64,record_stride=0x48,array_bytes=0x1200;
struct State {
    std::array<Address,table_slots> arrays_0a54a0{};
    std::uint32_t capacity_0a58a0{};
};
// All storage/lower providers are borrowed and live. Lifecycle operations are
// externally serialized; lock7 must be recursive for the lower ensure helper.
// These sources do not acquire file handles, implement SEH, or create arrays.
struct Effects {
    virtual ~Effects()=default;
    virtual void lock_0625f8(std::uint32_t)=0;
    virtual void unlock_06264c(std::uint32_t)=0;
    virtual std::uint64_t ensure_fh_exists_0732a8(std::int32_t)=0;
    virtual void initialize_inherited_067698()=0;
    virtual void initialize_standard_067788()=0;
    virtual void destroy_handle_array_073258(Address)=0;
    // Provider resolves only an actual live typed critical section at each
    // address; the source does not cast arbitrary byte storage into native CS.
    virtual void delete_critical_section(Address)=0;
    virtual void free_0668c0(Address)=0;
};
bool initialize_lowio_067890(Effects&);
bool uninitialize_lowio_0678cc(State&,Effects&);
void destroy_handle_array_073258(Address,Effects&);
// New composition of the recovered array destructor into the table walker.
// Lower array creation/inheritance/standard-handle providers remain mandatory.
class ArrayEffects:public Effects {
public:
    void destroy_handle_array_073258(Address) final;
};
}
