#include "binary_io.hpp"
#include <stdexcept>

namespace anadius::file_recovered {
std::string read_binary_03b3f0(const Path& path,Factory& factory) {
    std::string result;
    auto stream=factory.open_input_039f70(path,0x20,0x40);
    if(!stream)throw std::invalid_argument("Input factory returned null (integration contract)");
    if(stream->initial_state()!=0)return result;
    stream->seek_03b610(0,Seek::end);
    result.resize(stream->position_sum_03ba50(),'\0');
    stream->seek_03b610(0,Seek::begin);
    // Original ignores gcount and keeps the requested length on a short read.
    (void)stream->read_03b2a0({reinterpret_cast<std::uint8_t*>(result.data()),result.size()});
    stream->close_03ae70();
    return result;
}
bool write_binary_03c1d0(const Path& path,std::string_view value,Factory& factory) {
    auto stream=factory.open_output_03a1a0(path,0x20,0x40);
    if(!stream)throw std::invalid_argument("Output factory returned null (integration contract)");
    if(stream->initial_state()!=0)return false;
    stream->write_03be80(value);
    stream->close_03ae70();
    // Success reflects the original initial-state branch, not a new post-write
    // validation. A stream with exceptions enabled can still throw above.
    return true;
}
} // namespace anadius::file_recovered
