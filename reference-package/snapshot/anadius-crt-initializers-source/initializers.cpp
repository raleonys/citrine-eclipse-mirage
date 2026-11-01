#include "initializers.hpp"
#include <type_traits>

namespace anadius::crt_initializers_recovered {
bool initialize_pairs_0704f4(std::span<Pair> entries,Effects& effects) {
    for(std::size_t index=0;index<entries.size();++index) {
        const auto callback=entries[index].initialize;
        if(callback!=0 && (effects.invoke_boolean_initializer(callback)&0xff)==0) {
            // Failing entry itself is excluded; previous pairs are re-read.
            for(std::size_t rollback=index;rollback!=0;--rollback) {
                const auto& pair=entries[rollback-1];
                if(pair.initialize!=0 && pair.uninitialize!=0)
                    effects.invoke_uninitializer(pair.uninitialize,0);
            }
            return false;
        }
    }
    return true;
}
bool uninitialize_pairs_070588(std::span<Pair> entries,Effects& effects) {
    for(std::size_t index=entries.size();index!=0;--index) {
        const auto callback=entries[index-1].uninitialize;
        // Unlike rollback, this path does NOT require a nonnull initializer.
        if(callback!=0)effects.invoke_uninitializer(callback,0);
    }
    return true;
}
void initterm_064614(std::span<Address> entries,Effects& effects) {
    for(std::size_t index=0;index<entries.size();++index) {
        const auto callback=entries[index];
        if(callback!=0)effects.invoke_void_initializer(callback);
    }
}
std::uint32_t initterm_e_064658(std::span<Address> entries,Effects& effects) {
    for(std::size_t index=0;index<entries.size();++index) {
        const auto callback=entries[index];
        if(callback!=0) {
            const auto result=static_cast<std::uint32_t>(effects.invoke_int_initializer(callback));
            if(result!=0)return result;
        }
    }
    return 0;
}
bool acrt_initialize_064594(std::span<Pair> entries,Effects& effects) {
    return initialize_pairs_0704f4(entries,effects);
}
bool acrt_uninitialize_0645cc(bool terminating,State& state,std::span<Pair> entries,Effects& effects) {
    if(terminating) {
        if(state.flush_enabled_0a4ef0!=0)(void)effects.common_flush_all(true);
        return true;
    }
    return uninitialize_pairs_070588(entries,effects);
}
bool acrt_thread_attach_0645a8(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& effects) {
    return crt_ptd_recovered::optional_preserve_error_064d08(state,effects)!=nullptr;
}
bool acrt_thread_detach_0645bc(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& effects) {
    crt_ptd_recovered::free_ptd_064b4c(state,effects);return true;
}
bool acrt_late_cleanup_064604(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& effects) {
    (void)crt_ptd_recovered::uninitialize_064ecc(state,effects);return true;
}
AcrtTablesBridge::AcrtTablesBridge(State& state,std::span<Pair> pairs,
    std::span<Address> c_initializers,std::span<Address> cpp_initializers,
    crt_initializers_recovered::Effects& effects,
    crt_ptd_recovered::State& ptd,crt_ptd_recovered::Effects& ptd_effects)
    :state_(state),pairs_(pairs),c_(c_initializers),cpp_(cpp_initializers),effects_(effects),ptd_(ptd),ptd_effects_(ptd_effects){}
std::uint64_t AcrtTablesBridge::acrt_initialize_064594(){return crt_initializers_recovered::acrt_initialize_064594(pairs_,effects_);}
void AcrtTablesBridge::acrt_uninitialize_0645cc(bool terminating){(void)crt_initializers_recovered::acrt_uninitialize_0645cc(terminating,state_,pairs_,effects_);}
std::uint64_t AcrtTablesBridge::acrt_thread_attach_0645a8(){return crt_initializers_recovered::acrt_thread_attach_0645a8(ptd_,ptd_effects_);}
void AcrtTablesBridge::acrt_thread_detach_0645bc(){(void)crt_initializers_recovered::acrt_thread_detach_0645bc(ptd_,ptd_effects_);}
void AcrtTablesBridge::acrt_late_cleanup_064604(bool){(void)crt_initializers_recovered::acrt_late_cleanup_064604(ptd_,ptd_effects_);}
std::uint32_t AcrtTablesBridge::initterm_e_064658(){return crt_initializers_recovered::initterm_e_064658(c_,effects_);}
void AcrtTablesBridge::initterm_064614(){crt_initializers_recovered::initterm_064614(cpp_,effects_);}
static_assert(std::is_abstract_v<AcrtTablesBridge>);
}
