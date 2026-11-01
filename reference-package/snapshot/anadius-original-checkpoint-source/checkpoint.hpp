#pragma once
#include <functional>

namespace anadius::original_checkpoint {
// Actual selected PTD permission; captures its owner. Metadata preserves
// incoming/outgoing WinLastError even if permission throws. No mutex spans calls.
class Boundary final {
public:
    explicit Boundary(std::function<bool()>);
    void verify();
    // Scalar value-returning lower SDK routes. A valid postcheck rethrows the
    // identical lower exception; invalid postcheck wins. Prefixes stay written.
    template<class F>auto invoke(F&& fn)->decltype(fn()) {
        verify();decltype(fn()) result;
        try{result=fn();}catch(...){verify();throw;}
        verify();return result;
    }
private:
    std::function<bool()> permission_;
};
} // namespace anadius::original_checkpoint
