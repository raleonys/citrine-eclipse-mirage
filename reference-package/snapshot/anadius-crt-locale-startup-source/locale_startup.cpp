#include "locale_startup.hpp"
namespace anadius::crt_locale_startup_recovered {
bool initialize_locale_064468(Effects& e) {
    // 06446f is a plain MOV. The shipped initial refcount already starts at1.
    const auto initial=e.initial_locale_0a17e0();
    e.write_address(e.current_locale_slot_0a52e0(),initial);
    return true;
}
void reset_locale_065104(const std::uint32_t& before,const std::uint32_t& after,Effects& e) {
    e.lock_0625f8(before);
    // The observed half-open cell range A52E0..A52E8 contains exactly one slot.
    const auto slot=e.current_locale_slot_0a52e0();
    const auto initial=e.initial_locale_0a17e0();
    if(e.read_address(slot)!=initial) {
        const auto result=e.update_locale_nolock_0709d8(slot,initial);
        // Preserve the separate 065152 store even if the helper wrote the cell.
        e.write_address(slot,result);
    }
    e.unlock_06264c(after);
}
void uninitialize_locale_065338(Effects& e) {
    const std::uint32_t before=4,after=4;
    reset_locale_065104(before,after,e);
}
SourceEffects::SourceEffects(Address cell,Address initial,crt_locale_thread_recovered::Effects& t)
    :slot_(cell),initial_(initial),thread_(t){}
Address SourceEffects::current_locale_slot_0a52e0(){return slot_;}
Address SourceEffects::initial_locale_0a17e0(){return initial_;}
Address SourceEffects::read_address(Address a){return thread_.read_address(a);}
void SourceEffects::write_address(Address a,Address value){thread_.write_address(a,value);}
void SourceEffects::lock_0625f8(std::uint32_t i){thread_.lock_0625f8(i);}
void SourceEffects::unlock_06264c(std::uint32_t i){thread_.unlock_06264c(i);}
Address SourceEffects::update_locale_nolock_0709d8(Address a,Address value) {
    return crt_locale_thread_recovered::update_locale_nolock_0709d8(a,value,thread_);
}
}
