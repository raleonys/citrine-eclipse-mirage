#include "greenfield/core/lifecycle.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>
int main() {
    using namespace greenfield::core;
    Lifecycle a;
    assert(a.phase()==Phase::dormant);
    assert(a.shutdown(0u)==0u);
    assert(a.phase()==Phase::stopped);
    for (unsigned i=0;i<32;++i) assert(a.shutdown(i)==0u);
    Lifecycle b;
    std::vector<std::thread> threads;
    for (unsigned i=0;i<8;++i) {
      threads.emplace_back([&b,i] {
        for (unsigned j=0;j<100;++j) assert(b.shutdown(i+j)==0u);
      });
    }
    for(auto& t:threads) t.join();
    assert(b.phase()==Phase::stopped);
    assert(runtime_lifecycle().shutdown(UINT32_MAX)==0u);
    std::cout << "GF00_CORE_TEST_PASS\n";
}
