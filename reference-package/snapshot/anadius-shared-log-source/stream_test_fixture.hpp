#pragma once
#include "stream.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

namespace b20_test {
namespace sl=anadius::shared_log;
inline unsigned checks=0;
inline void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& fn,const char* message){bool rejected=false;try{fn();}catch(const E&){rejected=true;}require(rejected,message);}
// Own buffer remains safe even if an incorrect implementation overlaps calls.
// No stringbuf virtual recursion is counted as a false concurrent invocation.
struct Buffer:std::streambuf {
    std::mutex data_gate;std::string data;
    std::atomic<unsigned> inside=0,overlap=0,syncs=0;
    std::streamsize maximum=-1;bool fail_sync=false;
    std::function<void()> on_write;
    struct Active {
        Buffer& b;explicit Active(Buffer& value):b(value){if(b.inside.fetch_add(1))++b.overlap;std::this_thread::yield();}
        ~Active(){--b.inside;}
    };
    std::streamsize xsputn(const char* p,std::streamsize n)override{
        Active active(*this);if(on_write)on_write();const auto count=maximum<0?n:std::min(n,maximum);
        std::lock_guard lock(data_gate);data.append(p,static_cast<std::size_t>(count));return count;
    }
    int_type overflow(int_type value)override{
        Active active(*this);if(traits_type::eq_int_type(value,traits_type::eof()))return traits_type::not_eof(value);
        if(on_write)on_write();std::lock_guard lock(data_gate);data+=traits_type::to_char_type(value);return value;
    }
    int sync()override{Active active(*this);++syncs;return fail_sync?-1:0;}
    std::string text(){std::lock_guard lock(data_gate);return data;}
};
} // namespace b20_test
