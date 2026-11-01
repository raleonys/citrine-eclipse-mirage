#pragma once
#include "../anadius-crt-recalloc-source/recalloc.hpp"
#include "../anadius-crt-wildcard-source/wildcard.hpp"
#include "../anadius-crt-onexit-source/onexit.hpp"

namespace anadius::crt_recalloc_bridge {
using Address=std::uint64_t;
// Borrowed state/providers: one shared heap identity and error/new-handler policy.
// Nonzero addresses must identify provider-owned live allocations, never target VAs.
class Allocator {
public:
    Allocator(crt_heap_recovered::State& state,crt_recalloc_recovered::Effects& effects)
        :state_(state),effects_(effects){}
    Address calloc(std::uint64_t count,std::uint64_t size);
    Address recalloc(Address old,std::uint64_t count,std::uint64_t size);
    void free(Address);
private:
    crt_heap_recovered::State& state_;
    crt_recalloc_recovered::Effects& effects_;
};
class WildcardHeapEffects:public crt_wildcard_recovered::SupportEffects {
public:
    explicit WildcardHeapEffects(Allocator& allocator):allocator_(allocator){}
    Address calloc_066848(std::uint64_t count,std::uint64_t size) final;
    Address recalloc_070434(Address old,std::uint64_t count,std::uint64_t size) final;
    void free_0668c0(Address) final;
private:
    Allocator& allocator_;
};
class OnexitHeapEffects:public crt_onexit_recovered::Effects {
public:
    explicit OnexitHeapEffects(Allocator& allocator):allocator_(allocator){}
    Address recalloc_070434(Address old,std::uint64_t count,std::uint64_t size) final;
    void free_0668c0(Address) final;
private:
    Allocator& allocator_;
};
}
