#include "locale_ref.hpp"
#include <bit>
#include <initializer_list>
#include <limits>

namespace anadius::crt_locale_ref_recovered {
namespace {
std::int32_t time_reference(Address time, std::uint32_t increment,
                            const Identities& ids, Effects& effects) {
    if (time == 0 || time == ids.initial_time_08ad20)
        return std::numeric_limits<std::int32_t>::max();
    const auto previous = effects.atomic_fetch_add_32(time + 0x15c, increment);
    return std::bit_cast<std::int32_t>(std::uint32_t(previous + increment));
}

void locale_references(Address locale, std::uint32_t increment,
                       const Identities& ids, Effects& effects) {
    // Do not snapshot the pointed-to references: each load follows the preceding
    // atomic effect, in instruction order, including aliased/mutating providers.
    effects.atomic_fetch_add_32(locale + 0x10, increment);
    for (const Address offset : {Address{0xe0}, Address{0xf0}, Address{0xe8}, Address{0x100}}) {
        const Address counter = effects.read_address(locale + offset);
        if (counter != 0) effects.atomic_fetch_add_32(counter, increment);
    }
    for (Address index = 0; index != 6; ++index) {
        const Address cursor = locale + 0x38 + index * 0x20;
        if (effects.read_address(cursor - 0x10) != ids.category_name_0a1948) {
            const Address counter = effects.read_address(cursor);
            if (counter != 0) effects.atomic_fetch_add_32(counter, increment);
        }
        if (effects.read_address(cursor - 0x18) != 0) {
            const Address counter = effects.read_address(cursor - 8);
            if (counter != 0) effects.atomic_fetch_add_32(counter, increment);
        }
    }
    const Address time = effects.read_address(locale + 0x120);
    (void)time_reference(time, increment, ids, effects);
}
} // namespace

void add_locale_ref_070634(Address locale, const Identities& ids, Effects& effects) {
    locale_references(locale, 1, ids, effects);
}
void release_locale_ref_0708c0(Address locale, const Identities& ids, Effects& effects) {
    if (locale != 0) locale_references(locale, 0xffffffffU, ids, effects);
}
std::int32_t add_time_ref_070838(Address time, const Identities& ids, Effects& effects) {
    return time_reference(time, 1, ids, effects);
}
std::int32_t release_time_ref_070898(Address time, const Identities& ids, Effects& effects) {
    return time_reference(time, 0xffffffffU, ids, effects);
}
} // namespace anadius::crt_locale_ref_recovered
