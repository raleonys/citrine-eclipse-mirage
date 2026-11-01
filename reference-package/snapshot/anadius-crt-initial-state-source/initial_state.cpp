#include "initial_state.hpp"
#include "../anadius-crt-initial-locale-source/initial_locale_data.hpp"
#include "../anadius-crt-initial-multibyte-source/initial_mb.hpp"
#include "../anadius-crt-counter-native-source/counters.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <new>
#include <stdexcept>
#include <vector>

namespace anadius::crt_initial_state {
namespace d=crt::initial_locale_data;
namespace mb=crt_initial_multibyte;
namespace {
static_assert(sizeof(Address)==8&&sizeof(LONG)==4);
struct Block {
    std::size_t size;
    void* data;
    explicit Block(std::span<const std::uint8_t> source):size(source.size()),
        data(::operator new(size,std::align_val_t{8})) {
        std::memcpy(data,source.data(),size);
    }
    ~Block(){::operator delete(data,std::align_val_t{8});}
    Block(const Block&)=delete;
    Block& operator=(const Block&)=delete;
    Address address() const noexcept {return reinterpret_cast<std::uintptr_t>(data);}
    std::span<const std::uint8_t> bytes() const noexcept {
        return {static_cast<const std::uint8_t*>(data),size};
    }
    void* at(std::size_t offset,std::size_t count) const {
        if(offset>size||count>size-offset) throw std::out_of_range("initial data bounds");
        return static_cast<std::byte*>(data)+offset;
    }
};
}
struct ImageState::Impl {
    std::vector<std::unique_ptr<Block>> regions;
    Block multibyte{mb::record_bytes()};
    std::array<std::uint8_t,mb::type_size> types=mb::initial_type_table();
    std::array<std::uint8_t,mb::case_size> cases=mb::initial_case_table();
    crt_counter_native::Counters counters;
    std::array<LONG*,3> live{};
    Globals globals{};
    Impl() {
        for(const auto& r:d::regions()) regions.push_back(std::make_unique<Block>(r.bytes));
        for(const auto& r:d::relocations()) {
            auto& source=*regions.at(r.source_region);
            auto& target=*regions.at(r.target_region);
            if(r.target_offset>=target.size) throw std::out_of_range("initial relocation target");
            const auto address=target.address()+r.target_offset;
            std::memcpy(source.at(r.source_offset,sizeof(address)),&address,sizeof(address));
        }
        live[0]=start_counter(*regions.at(d::Locale),0x10);
        live[1]=start_counter(*regions.at(d::Time),0x15c);
        live[2]=start_counter(multibyte,0);
        const auto& s=mb::image_globals();
        globals={regions.at(d::Locale)->address(),multibyte.address(),0,0,0,0,
                 s.locale_mask,s.locale_changed,s.system_cp_flag,s.initialized};
    }
    LONG* start_counter(Block& b,std::size_t offset) {
        void* place=b.at(offset,sizeof(LONG));
        if(reinterpret_cast<std::uintptr_t>(place)%alignof(LONG))
            throw std::logic_error("unaligned initial counter");
        std::uint32_t bits;
        std::memcpy(&bits,place,sizeof(bits));
        auto* value=std::construct_at(static_cast<LONG*>(place),std::bit_cast<LONG>(bits));
        counters.bind(*value);
        return value;
    }
    ~Impl() {
        for(auto* value:live) if(value) {
            counters.unbind(*value);
            std::destroy_at(value);
        }
    }
};
ImageState::ImageState():impl_(std::make_unique<Impl>()){}
ImageState::~ImageState()=default;
const Globals& ImageState::globals() const noexcept {return impl_->globals;}
crt_locale_ref_recovered::Identities ImageState::reference_identities() const noexcept {
    return {impl_->regions[d::Category]->address(),impl_->regions[d::Time]->address()};
}
Address ImageState::region_address(std::size_t i) const {return impl_->regions.at(i)->address();}
std::span<const std::uint8_t> ImageState::region_bytes(std::size_t i) const {
    return impl_->regions.at(i)->bytes();
}
std::span<const std::uint8_t> ImageState::multibyte_bytes() const noexcept {
    return impl_->multibyte.bytes();
}
Address ImageState::multibyte_type_table() const noexcept {
    return reinterpret_cast<std::uintptr_t>(impl_->types.data());
}
Address ImageState::multibyte_case_table() const noexcept {
    return reinterpret_cast<std::uintptr_t>(impl_->cases.data());
}
Address ImageState::read_address(Address address) {
    for(const auto& b:impl_->regions) if(address>=b->address()&&address-b->address()<b->size) {
        Address value;
        std::memcpy(&value,b->at(static_cast<std::size_t>(address-b->address()),sizeof(value)),sizeof(value));
        return value;
    }
    if(address>=impl_->multibyte.address()&&address-impl_->multibyte.address()<impl_->multibyte.size) {
        Address value;
        std::memcpy(&value,impl_->multibyte.at(static_cast<std::size_t>(address-impl_->multibyte.address()),sizeof(value)),sizeof(value));
        return value;
    }
    throw std::out_of_range("unknown initial data address");
}
std::uint32_t ImageState::atomic_fetch_add_32(Address a,std::uint32_t n) {
    return impl_->counters.fetch_add(a,n);
}
std::uint32_t ImageState::counter_value(Address a) const {return impl_->counters.load(a);}
}
