// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/core/binary_io.hpp"
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace citrine_eclipse_mirage::file {
namespace {
std::uint32_t state(std::ios_base::iostate value) {
    return ((value&std::ios::eofbit)?1U:0U)|((value&std::ios::failbit)?2U:0U)|((value&std::ios::badbit)?4U:0U);
}
std::streamsize count(std::size_t size) {
    if(size>static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()))
        throw std::length_error("Native IO request exceeds streamsize");
    return static_cast<std::streamsize>(size);
}
void supported(std::uint32_t mode,std::uint32_t share) {
    if(mode!=0x20||share!=0x40)throw std::invalid_argument("Native IO supports only evidenced binary/default-share route");
}
class NativeInput final:public Input {
public:
    explicit NativeInput(const Path& path):stream_(std::filesystem::path(path),std::ios::in|std::ios::binary){}
    std::uint32_t initial_state()const override{return state(stream_.rdstate());}
    void seek_03b610(std::int64_t offset,Seek origin)override {
        stream_.seekg(static_cast<std::streamoff>(offset),origin==Seek::end?std::ios::end:std::ios::beg);
    }
    std::uint64_t position_sum_03ba50()override {
        const auto position = stream_.tellg();
        if(position == std::streampos(-1))
            throw std::ios_base::failure("Native input stream position unavailable");
        return static_cast<std::uint64_t>(static_cast<std::streamoff>(position));
    }
    std::int32_t peek_043790()override {
        const auto value=stream_.peek();
        return value==std::char_traits<char>::eof()?-1:static_cast<unsigned char>(value);
    }
    std::size_t read_03b2a0(std::span<std::uint8_t> target)override {
        stream_.read(reinterpret_cast<char*>(target.data()),count(target.size()));
        return static_cast<std::size_t>(stream_.gcount());
    }
    void close_03ae70()override{stream_.close();}
private:std::ifstream stream_;
};
class NativeOutput final:public Output {
public:
    explicit NativeOutput(const Path& path):stream_(std::filesystem::path(path),std::ios::out|std::ios::binary|std::ios::trunc){}
    std::uint32_t initial_state()const override{return state(stream_.rdstate());}
    void write_03be80(std::string_view value)override{stream_.write(value.data(),count(value.size()));}
    void close_03ae70()override{stream_.close();}
private:std::ofstream stream_;
};
}
std::unique_ptr<Input> NativeFactory::open_input_039f70(const Path& path,std::uint32_t mode,std::uint32_t share) {
    supported(mode,share);return std::make_unique<NativeInput>(path);
}
std::unique_ptr<Output> NativeFactory::open_output_03a1a0(const Path& path,std::uint32_t mode,std::uint32_t share) {
    supported(mode,share);return std::make_unique<NativeOutput>(path);
}
} // namespace citrine_eclipse_mirage::file
