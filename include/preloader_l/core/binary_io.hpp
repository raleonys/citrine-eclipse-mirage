// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/config/configuration.hpp"
#include "preloader_l/core/atomic_files.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <string>

namespace preloader_l::file {
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
    // Achievement persistence requires a checked, rollback-capable backend.
    // Unsupported adapters throw rather than silently reverting to truncation.
    virtual void replace_atomic(std::span<const FileWrite>);
    virtual void recover_atomic() {}
};
std::string read_binary_03b3f0(const Path&,Factory&);
bool write_binary_03c1d0(const Path&,std::string_view,Factory&);

// Standard filesystem path/fstream backend restored from the supplied RE adapter.
// Windows/MSVC uses its compiler-owned CRT/filebuf. Default exception mask is zero.
// Exact parity with the historical compiler's sharing/locale/ABI is not claimed.
class NativeFactory final:public Factory {
public:
    NativeFactory();
    ~NativeFactory();
    std::unique_ptr<Input> open_input_039f70(const Path&,std::uint32_t mode,std::uint32_t share) override;
    std::unique_ptr<Output> open_output_03a1a0(const Path&,std::uint32_t mode,std::uint32_t share) override;
    void replace_atomic(std::span<const FileWrite>) override;
    void recover_atomic() override;
private:
    struct AtomicOwner;
    std::unique_ptr<AtomicOwner> atomic_;
};
} // namespace preloader_l::file
