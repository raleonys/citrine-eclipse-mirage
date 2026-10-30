#include "preloader_l/hash/hash.hpp"
#include <algorithm>
#include <bit>
#include <stdexcept>

namespace preloader_l::hash_recovered {
void Sha1::transform_043900(const std::array<std::uint8_t,64>& block) {
    std::array<std::uint32_t,80> w{};
    // 042aa0 forms each initial word in big-endian byte order.
    for (std::size_t i=0;i<16;++i) {
        const auto p=i*4;
        w[i]=(std::uint32_t(block[p])<<24)|(std::uint32_t(block[p+1])<<16)|
             (std::uint32_t(block[p+2])<<8)|std::uint32_t(block[p+3]);
    }
    for (std::size_t i=16;i<80;++i)
        w[i]=std::rotl(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    auto [a,b,c,d,e]=state_;
    for (std::size_t i=0;i<80;++i) {
        std::uint32_t f,k;
        if (i<20) {f=(b&c)|(~b&d);k=0x5a827999u;}
        else if (i<40) {f=b^c^d;k=0x6ed9eba1u;}
        else if (i<60) {f=(b&c)|(b&d)|(c&d);k=0x8f1bbcdcu;}
        else {f=b^c^d;k=0xca62c1d6u;}
        const auto t=std::rotl(a,5)+f+e+k+w[i];
        e=d;d=c;c=std::rotl(b,30);b=a;a=t;
    }
    state_[0]+=a;state_[1]+=b;state_[2]+=c;state_[3]+=d;state_[4]+=e;
    ++block_count_;
}

void Sha1::update(std::span<const std::uint8_t> bytes) {
    while (!bytes.empty()) {
        const auto count=std::min<std::size_t>(64-pending_size_,bytes.size());
        std::copy_n(bytes.begin(),count,pending_.begin()+pending_size_);
        pending_size_+=count;
        bytes=bytes.subspan(count);
        if (pending_size_==64) {transform_043900(pending_);pending_size_=0;}
    }
}
void Sha1::update(std::string_view bytes) {
    update(std::span(reinterpret_cast<const std::uint8_t*>(bytes.data()),bytes.size()));
}
DigestWords Sha1::finalize_words_043270() {
    const auto bits=(block_count_*std::uint64_t{64}+pending_size_)*std::uint64_t{8};
    pending_[pending_size_++]=0x80;
    std::fill(pending_.begin()+pending_size_,pending_.end(),0);
    if (pending_size_>56) {transform_043900(pending_);pending_.fill(0);}
    for (std::size_t i=0;i<8;++i)
        pending_[56+i]=static_cast<std::uint8_t>(bits>>(56-i*8));
    transform_043900(pending_);
    const auto digest=state_;
    *this=Sha1{};
    return digest;
}
std::string classic_hex_digest(const DigestWords& words) {
    constexpr char digits[]="0123456789abcdef";
    std::string result;
    result.reserve(40);
    for (auto word:words)
        for (int shift=28;shift>=0;shift-=4)
            result.push_back(digits[(word>>shift)&15]);
    return result;
}

static void lowercase_03d2c0(std::string& text,TextEnvironment& environment) {
    for (char& value:text) {
        const auto byte=static_cast<std::uint8_t>(value);
        const std::int32_t promoted=byte<128 ? byte : std::int32_t(byte)-256;
        value=static_cast<char>(environment.tolower_05fca0(promoted));
    }
}
bool file_hash_matches_042d20(const Path& path,std::string expected,
                            FileFactory& files,TextEnvironment& text) {
    auto stream=files.open_input_039f70(path,0x20,0x40);
    if (!stream) throw std::invalid_argument("039f70 adapter returned no stream object");
    if (stream->initial_state()!=0) return false;
    Sha1 digest;
    std::array<std::uint8_t,8192> buffer{};
    while (stream->peek_043790()!=-1) {
        const auto count=stream->read_03b2a0(buffer);
        if (count>buffer.size())
            throw std::invalid_argument("03b2a0 adapter gcount exceeds destination");
        digest.update(std::span(buffer.data(),count));
    }
    stream->close_03ae70();
    auto actual=text.format_digest_043270(digest.finalize_words_043270());
    lowercase_03d2c0(actual,text);
    lowercase_03d2c0(expected,text);
    return expected==actual;
}
} // namespace preloader_l::hash_recovered
