#pragma once
#include "../anadius-crt-conversion-source/conversion.hpp"

namespace anadius::crt_module_path_recovered {
namespace cv = crt_conversion_recovered;
using Address = cv::Address;
std::uint32_t fixed_to_narrow_0601c8(Address source, cv::Buffer&,
                                    std::uint32_t cp, cv::Effects&);
struct WideResult { Address data; std::uint32_t count; };
class Effects : public cv::Effects {
public:
    // New provider contract: per-call scratch >=261 UTF-16 code units, valid
    // throughout this call, with a NUL-terminated string on the supported path.
    virtual WideResult get_module_filename_w(Address module,
                                              std::uint32_t capacity) = 0;
    // Includes the inline LocaleUpdate/file-API ANSI selection and cleanup.
    virtual std::uint32_t choose_code_page() = 0;
};
std::uint32_t module_filename_06ecdc(Address module, Address destination,
                                    std::uint64_t capacity, Effects&);
}
