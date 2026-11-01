// SPDX-License-Identifier: GPL-3.0-only
#include "../src/platform/registry_value_output.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

using namespace preloader_l::platform;
namespace writer=preloader_l::platform::registry_value_output;

int main() {
    const uint16_t wide[]={0x41,0x20ac,0xd83d,0xde00,0};
    const char narrow[]={'a',static_cast<char>(0xff),0,'z'};
    const std::array<uint8_t,10> expected_w={0x41,0,0xac,0x20,0x3d,0xd8,0,0xde,0,0};
    const std::array<uint8_t,5> expected_a={'a',0xff,0,'z',0};
    // All offsets/capacities use the pre-original capacity even if the current
    // size cell has been overwritten by the original Win32 function.
    for (bool is_wide:{false,true}) {
        const uint32_t required=is_wide?10u:5u;
        for (size_t offset=0;offset<8;++offset) {
            for (uint32_t capacity=0;capacity<14;++capacity) {
                alignas(8) std::array<uint8_t,32> buffer;
                buffer.fill(0xa5);
                uint32_t bytes=UINT32_MAX,type=99;
                const writer::Output output{capacity,&type,buffer.data()+offset,&bytes};
                const auto status=is_wide?writer::write_w(wide,4,output)
                                         :writer::write_a(narrow,4,output);
                assert(type==host_reg_sz && bytes==required);
                assert(status==(capacity<required?host_error_more_data:host_error_success));
                for(size_t i=0;i<buffer.size();++i) {
                    const bool written=capacity>=required && i>=offset && i<offset+required;
                    const auto expected=written?(is_wide?expected_w[i-offset]:expected_a[i-offset]):0xa5;
                    assert(buffer[i]==expected);
                }
            }
        }
        uint32_t type=99,bytes=999;
        const auto query=is_wide?writer::write_w(wide,4,{0,&type,nullptr,&bytes})
                                :writer::write_a(narrow,4,{0,&type,nullptr,&bytes});
        assert(query==host_error_success && bytes==required && type==host_reg_sz);
        assert(writer::write_a(nullptr,0,{0,nullptr,nullptr,nullptr})==host_error_success);
        assert(writer::write_w(nullptr,0,{0,nullptr,nullptr,nullptr})==host_error_success);
    }
    std::array<uint8_t,8> buffer;buffer.fill(0xa5);
    uint32_t type=99,bytes=999;
    assert(writer::write_w(nullptr,0,{2,&type,buffer.data()+1,&bytes})==host_error_success);
    assert(bytes==2 && buffer[1]==0 && buffer[2]==0 && buffer[0]==0xa5 && buffer[3]==0xa5);
    buffer.fill(0xa5);type=99;bytes=999;
    assert(writer::write_a(nullptr,0,{1,&type,buffer.data()+1,&bytes})==host_error_success);
    assert(bytes==1 && buffer[1]==0 && buffer[2]==0xa5);
    // Invalid requests publish neither REG_SZ nor a size and never touch data.
    for(bool is_wide:{false,true}) {
        for(unsigned invalid=0;invalid<3;++invalid) {
            buffer.fill(0xa5);type=99;bytes=999;
            const writer::Output output{8,&type,buffer.data(),invalid==1?nullptr:&bytes};
            const auto length=invalid==2?std::numeric_limits<size_t>::max():1u;
            const auto status=is_wide?writer::write_w(invalid==0?nullptr:wide,length,output)
                                     :writer::write_a(invalid==0?nullptr:narrow,length,output);
            assert(status==host_error_invalid_parameter && type==99 && bytes==999);
            for(auto byte:buffer) assert(byte==0xa5);
        }
        // The precise largest count is accepted for size-only requests;
        // the next one is rejected without reading the deliberately tiny source.
        const size_t width=is_wide?2u:1u;
        const auto maximum=(static_cast<size_t>(UINT32_MAX)-width)/width;
        type=99;bytes=999;
        const auto valid=is_wide?writer::write_w(wide,maximum,{0,&type,nullptr,&bytes})
                                :writer::write_a(narrow,maximum,{0,&type,nullptr,&bytes});
        assert(valid==host_error_success && bytes==(maximum+1u)*width && type==host_reg_sz);
        type=99;bytes=999;
        const auto invalid=is_wide?writer::write_w(wide,maximum+1u,{0,&type,nullptr,&bytes})
                                  :writer::write_a(narrow,maximum+1u,{0,&type,nullptr,&bytes});
        assert(invalid==host_error_invalid_parameter && bytes==999 && type==99);
    }
    // The type output is optional on a successful non-query request.
    bytes=999;
    assert(writer::write_a("a",1,{2,nullptr,buffer.data(),&bytes})==host_error_success);
    assert(bytes==2 && buffer[0]=='a' && buffer[1]==0);
}
