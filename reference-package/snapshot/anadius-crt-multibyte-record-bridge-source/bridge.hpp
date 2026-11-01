#pragma once
#include "../anadius-crt-multibyte-builder-source/multibyte_builder.hpp"
#include "../anadius-crt-multibyte-bridge-source/bridge.hpp"
namespace anadius::crt_multibyte_record_bridge {
using Address=std::uint64_t;
using CPInfo=crt_multibyte_tables_recovered::CPInfo;
using WordOutput=crt_multibyte_tables_recovered::WordOutput;
using ByteOutput=crt_multibyte_tables_recovered::ByteOutput;
class Memory:public crt_multibyte_bridge::Memory {
public:
    virtual std::uint16_t read_u16(Address)=0;
    virtual void write_u16(Address,std::uint16_t)=0;
};
struct Identities {
    Address initial_multibyte_0a1c70{};
    // Live equivalents of pointer cells 08bf70/78/80/88: ja, zh-CN, ko, zh-TW.
    std::array<Address,4> locale_names{};
};
class NativeEffects {
public:
    virtual ~NativeEffects()=default;
    virtual std::uint32_t get_acp()=0;
    virtual std::uint32_t get_oemcp()=0;
    virtual std::uint32_t exchange_32(Address,std::uint32_t)=0;
    virtual std::uint32_t is_valid_code_page(std::uint32_t)=0;
    virtual std::uint32_t get_cp_info(std::uint32_t,CPInfo&)=0;
    // These two lower CRT wrappers are still mandatory, unrecovered bodies.
    // Providers explicitly define output cells; an ignored error must not
    // silently turn the original uninitialized stack bytes into zeroes.
    virtual std::uint32_t get_string_type_06e210(Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<WordOutput>,std::uint32_t,std::uint32_t)=0;
    virtual std::uint32_t map_string_06cc30(Address,Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<ByteOutput>,std::uint32_t,std::uint32_t)=0;
};
// Supplies the actual recovered builder to the previous multibyte bridge.
// Construct both objects, bind the live support interface, then initialize.
// All referenced state/storage/identities must share one synchronized domain.
class RecordNative final:public crt_multibyte_bridge::NativeEffects,
    private crt_multibyte_builder_recovered::Effects,
    private crt_multibyte_tables_recovered::Effects {
public:
    RecordNative(Memory&,crt_multibyte_bridge::Globals&,const Identities&,
        crt_multibyte_record_bridge::NativeEffects&);
    void bind_system_cp(crt_multibyte_support_recovered::Effects&);
    std::uint32_t get_acp() final;
    std::uint32_t get_oemcp() final;
    std::uint32_t exchange_32(Address,std::uint32_t) final;
    std::uint32_t build_multibyte_06f674(std::int32_t,Address) final;
private:
    std::uint8_t read_u8(Address) final;
    std::uint16_t read_u16(Address) final;
    std::uint32_t read_u32(Address) final;
    Address read_address(Address) final;
    void write_u8(Address,std::uint8_t) final;
    void write_u16(Address,std::uint16_t) final;
    void write_u32(Address,std::uint32_t) final;
    void write_address(Address,Address) final;
    std::int32_t system_cp_06efd0(std::int32_t) final;
    std::uint32_t system_cp_flag_0a59d8() final;
    std::uint32_t is_valid_code_page(std::uint32_t) final;
    std::uint32_t get_cp_info(std::uint32_t,CPInfo&) final;
    Address initial_multibyte_0a1c70() final;
    void reset_sbcs_06f050(Address) final;
    void build_case_tables_06f0e8(Address) final;
    Address locale_name_08bf70() final;
    Address locale_name_08bf78() final;
    Address locale_name_08bf80() final;
    Address locale_name_08bf88() final;
    std::uint32_t get_string_type_06e210(Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<WordOutput>,std::uint32_t,std::uint32_t) final;
    std::uint32_t map_string_06cc30(Address,Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<ByteOutput>,std::uint32_t,std::uint32_t) final;
    Memory& memory_;
    crt_multibyte_bridge::Globals& globals_;
    const Identities& identities_;
    crt_multibyte_record_bridge::NativeEffects& native_;
    crt_multibyte_support_recovered::Effects* support_{};
};
}
