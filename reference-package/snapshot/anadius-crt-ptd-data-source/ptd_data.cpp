#include "ptd_data.hpp"
#include <array>

namespace anadius::crt_ptd_data_recovered {
static_assert(sizeof(Address)==8);
void replace_locale_064ae4(void* storage, Address replacement,
                          const Identities& ids, Effects& e) {
    const View ptd(storage);
    if (const auto previous=ptd.load<Address>(0x90); previous!=0) {
        e.release_locale_ref_0708c0(previous);
        // Re-read the slot and current global AFTER the dependency call.
        const auto observed=ptd.load<Address>(0x90);
        if (observed!=e.current_locale_0a52e0() && observed!=ids.initial_locale &&
            e.locale_ref_count_10(observed)==0)
            e.free_locale_0706c0(observed);
    }
    ptd.store<Address>(0x90,replacement);
    if (replacement!=0) e.add_locale_ref_070634(replacement);
}
void retain_multibyte_0647d4(void* storage, std::uint32_t before,
                           std::uint32_t after, Effects& e) {
    e.lock_0625f8(before);
    (void)e.atomic_fetch_add_32(View(storage).load<Address>(0x88),1);
    e.unlock_06264c(after);
}
void clear_locale_064814(void* storage, std::uint32_t before,
                        std::uint32_t after, const Identities& ids, Effects& e) {
    e.lock_0625f8(before);
    replace_locale_064ae4(storage,0,ids,e);
    e.unlock_06264c(after);
}
void current_locale_064854(void* storage, std::uint32_t before,
                          std::uint32_t after, const Identities& ids, Effects& e) {
    e.lock_0625f8(before);
    const auto locale=e.current_locale_0a52e0();
    replace_locale_064ae4(storage,locale,ids,e);
    e.unlock_06264c(after);
}
void release_multibyte_06489c(void* storage, std::uint32_t before,
                            std::uint32_t after, const Identities& ids, Effects& e) {
    e.lock_0625f8(before);
    if (const auto info=View(storage).load<Address>(0x88); info!=0) {
        const auto previous=e.atomic_fetch_add_32(info,0xffffffffU);
        if (previous==1 && info!=ids.multibyte_info) e.free_0668c0(info);
    }
    e.unlock_06264c(after);
}
void construct_0648fc(void* storage, const Identities& ids, Effects& e) {
    const View ptd(storage);
    ptd.store<std::uint32_t>(0x28,1);
    ptd.store<Address>(0,ids.exception_table);
    ptd.store<std::uint32_t>(0x3a8,1);
    ptd.store<Address>(0x88,ids.multibyte_info);
    ptd.store<std::uint16_t>(0xbc,0x43);
    ptd.store<std::uint16_t>(0x1c2,0x43);
    ptd.store<Address>(0x3a0,0);
    retain_multibyte_0647d4(storage,5,5,e);
    current_locale_064854(storage,4,4,ids,e);
}
void destroy_0649ec(void* storage, const Identities& ids, Effects& e) {
    const View ptd(storage);
    if (const auto table=ptd.load<Address>(0); table!=ids.exception_table)
        e.free_0668c0(table);
    // Read each field immediately before its free; earlier effects can mutate
    // later fields. Null calls are observed and retained.
    constexpr std::array<std::size_t,9> offsets{0x70,0x58,0x60,0x68,0x48,0x50,0x78,0x80,0x3c0};
    for (const auto offset:offsets) e.free_0668c0(ptd.load<Address>(offset));
    release_multibyte_06489c(storage,5,5,ids,e);
    clear_locale_064814(storage,4,4,ids,e);
}
void fls_destroy_0649cc(void* storage, const Identities& ids, Effects& e) {
    if (storage) {
        destroy_0649ec(storage,ids,e);
        e.free_0668c0(reinterpret_cast<Address>(storage));
    }
}
}
