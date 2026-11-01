#pragma once
#include "../anadius-crt-locale-ref-source/locale_ref.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace anadius::crt_initial_state {
using Address=std::uint64_t;
struct Globals {
    Address initial_locale,initial_multibyte;
    Address current_locale=0,current_multibyte=0,type_table=0,case_table=0;
    std::uint32_t locale_mask,locale_changed,system_cp_flag,initialized;
};
// Owned, relocated IMAGE INITIAL DATA only. No CRT initializers run here.
// All views and counter operations require this nonmoving owner to stay alive.
// Destruction and byte views require quiescence; concurrent reference operations
// use the immutable registry and actual Interlocked LONG objects.
class ImageState final:public crt_locale_ref_recovered::Effects {
public:
    ImageState();
    ~ImageState() override;
    ImageState(const ImageState&)=delete;
    ImageState& operator=(const ImageState&)=delete;
    ImageState(ImageState&&)=delete;
    ImageState& operator=(ImageState&&)=delete;
    const Globals& globals() const noexcept;
    crt_locale_ref_recovered::Identities reference_identities() const noexcept;
    Address region_address(std::size_t) const;
    std::span<const std::uint8_t> region_bytes(std::size_t) const;
    std::span<const std::uint8_t> multibyte_bytes() const noexcept;
    Address multibyte_type_table() const noexcept;
    Address multibyte_case_table() const noexcept;
    Address read_address(Address) override;
    std::uint32_t atomic_fetch_add_32(Address,std::uint32_t) override;
    std::uint32_t counter_value(Address) const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
