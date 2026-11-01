#pragma once
#include "src/configuration/state.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace preloader_l::test_packet_binary_io {
using Path=configuration::Path;
enum class Seek:std::uint32_t {begin=0,end=2};
// Stream state/exception behavior belongs to the implementation. The recovered
// application bodies only inspect the initial state and explicitly close.
class Input {
public:
    virtual ~Input()=default;
    virtual std::uint32_t initial_state()const=0;
    virtual void seek_03b610(std::int64_t offset,Seek)=0;
    virtual std::uint64_t position_sum_03ba50()=0;
    virtual std::int32_t peek_043790()=0;
    virtual std::size_t read_03b2a0(std::span<std::uint8_t>)=0;
    virtual void close_03ae70()=0;
};
class Output {
public:
    virtual ~Output()=default;
    virtual std::uint32_t initial_state()const=0;
    virtual void write_03be80(std::string_view)=0;
    virtual void close_03ae70()=0;
};
class Factory {
public:
    virtual ~Factory()=default;
    virtual std::unique_ptr<Input> open_input_039f70(const Path&,std::uint32_t mode,std::uint32_t share)=0;
    virtual std::unique_ptr<Output> open_output_03a1a0(const Path&,std::uint32_t mode,std::uint32_t share)=0;
};
std::string read_binary_03b3f0(const Path&,Factory&);
bool write_binary_03c1d0(const Path&,std::string_view,Factory&);

// Working Windows/LLVM-MinGW std::fstream compatibility backend. This is a
// chosen source dependency, not the original MSVC CRT/filebuf implementation.
// Default exception mask is zero. Sharing/locale/ABI equality is not established.
class NativeFactory final:public Factory {
public:
    std::unique_ptr<Input> open_input_039f70(const Path&,std::uint32_t mode,std::uint32_t share) override;
    std::unique_ptr<Output> open_output_03a1a0(const Path&,std::uint32_t mode,std::uint32_t share) override;
};
} // namespace preloader_l::test_packet_binary_io
