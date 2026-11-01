#include "counters.hpp"
#include <array>
#include <cstddef>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>
namespace cn=anadius::crt_counter_native;
namespace {
unsigned checks;
void check(bool value,std::source_location where=std::source_location::current()){
    ++checks;if(!value)throw std::runtime_error("check "+std::to_string(checks)+" at line "+std::to_string(where.line()));
}
struct alignas(8) Record {std::array<std::byte,0x10> prefix{};LONG references=0;LONG secondary=0;};
struct Time {std::array<std::byte,0x15c> prefix{};LONG references=0;};
static_assert(offsetof(Record,references)==0x10);
static_assert(offsetof(Time,references)==0x15c);
static_assert(!std::is_copy_constructible_v<cn::Counters>);
static_assert(!std::is_move_constructible_v<cn::Counters>);
template<class F>void rejected(F&& operation){bool caught=false;try{operation();}catch(const std::exception&){caught=true;}check(caught);}
void identities_and_bits(){
    Record record;Time time;cn::Counters counters;
    const auto first=counters.bind(record.references),second=counters.bind(record.secondary),third=counters.bind(time.references);
    check(first==reinterpret_cast<cn::Address>(&record)+0x10&&third==reinterpret_cast<cn::Address>(&time)+0x15c);
    check(counters.contains(first)&&counters.contains(second)&&counters.contains(third)&&!counters.contains(0));
    rejected([&]{(void)counters.bind(record.references);});
    for(const auto value:{0U,1U,0x7fffffffU,0x80000000U,0xfffffffeU,0xffffffffU,0x12345678U,0xabcdef01U}){
        (void)counters.exchange(first,value);check(counters.load(first)==value);
        for(const auto add:{0U,1U,0xffffffffU,0x80000000U}){
            const auto before=counters.load(first);check(counters.fetch_add(first,add)==before);
            check(counters.load(first)==std::uint32_t(before+add));
        }
    }
    check(counters.load(second)==0&&counters.load(third)==0);
    const auto before=counters.load(first);
    for(auto unknown:{cn::Address{0},first+1,~cn::Address{0}}){
        rejected([&]{(void)counters.fetch_add(unknown,1);});
        rejected([&]{(void)counters.exchange(unknown,1);});
        rejected([&]{(void)counters.load(unknown);});
    }
    check(counters.load(first)==before);
    ::SetLastError(0xf1234567);(void)counters.fetch_add(first,7);(void)counters.exchange(second,0x87654321);(void)counters.load(third);check(::GetLastError()==0xf1234567);
    counters.unbind(record.references);check(!counters.contains(first));
    rejected([&]{counters.unbind(record.references);});rejected([&]{(void)counters.load(first);});
    check(counters.bind(record.references)==first&&counters.load(first)==std::uint32_t(before+7));
    counters.unbind(record.references);counters.unbind(record.secondary);counters.unbind(time.references);
}
void parallel_borrowed_field(){
    Record record;cn::Counters counters;const auto identity=counters.bind(record.references);
    constexpr unsigned worker_count=4,iterations=25000,total=worker_count*iterations;
    std::array<std::uint64_t,worker_count> sums{};std::vector<std::thread> workers;
    for(unsigned i=0;i<worker_count;++i)workers.emplace_back([&,i]{std::uint64_t sum=0;for(unsigned n=0;n<iterations;++n)sum+=counters.fetch_add(identity,1);sums[i]=sum;});
    for(auto& worker:workers)worker.join();workers.clear();
    check(counters.load(identity)==total);
    std::uint64_t sum=0;for(auto value:sums)sum+=value;check(sum==std::uint64_t(total)*(total-1)/2);
    for(unsigned i=0;i<worker_count;++i)workers.emplace_back([&]{for(unsigned n=0;n<iterations;++n)(void)counters.fetch_add(identity,0xffffffffU);});
    for(auto& worker:workers)worker.join();
    check(counters.load(identity)==0&&record.references==0);
    counters.unbind(record.references);
}
}
int main(){try{identities_and_bits();parallel_borrowed_field();std::cout<<"PASS "<<checks<<" borrowed native LONG counter checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
