#pragma once
#include "../anadius-crt-locale-thread-source/locale_thread.hpp"
#include <cstdint>
namespace anadius::crt_locale_startup_recovered {
using Address=std::uint64_t;
struct Effects {
    virtual ~Effects()=default;
    virtual Address current_locale_slot_0a52e0()=0;
    virtual Address initial_locale_0a17e0()=0;
    virtual Address read_address(Address)=0;
    virtual void write_address(Address,Address)=0;
    virtual void lock_0625f8(std::uint32_t)=0;
    virtual void unlock_06264c(std::uint32_t)=0;
    virtual Address update_locale_nolock_0709d8(Address slot,Address replacement)=0;
};
bool initialize_locale_064468(Effects&);
// Recovered normal flow only: the two DWORD lvalues are read separately at
// lock/unlock time. They and all borrowed storage must remain live.
// This does not implement MSVC SEH funclets or exceptional unwind behavior.
void reset_locale_065104(const std::uint32_t& lock,const std::uint32_t& unlock,Effects&);
void uninitialize_locale_065338(Effects&);

// New composition, not another original body. Cells/identities use one owned
// memory domain shared with the thread provider. No preferred VA is invoked.
class SourceEffects final:public Effects {
public:
    SourceEffects(Address current_slot,Address initial,crt_locale_thread_recovered::Effects&);
    Address current_locale_slot_0a52e0() final;
    Address initial_locale_0a17e0() final;
    Address read_address(Address) final;
    void write_address(Address,Address) final;
    void lock_0625f8(std::uint32_t) final;
    void unlock_06264c(std::uint32_t) final;
    Address update_locale_nolock_0709d8(Address,Address) final;
private:
    const Address slot_,initial_;
    crt_locale_thread_recovered::Effects& thread_;
};
}
