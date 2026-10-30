#include "preloader_l/hash/hash.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
using namespace preloader_l::hash_recovered;
namespace {
std::size_t checks=0;
void check(bool good) {++checks;if(!good)throw std::runtime_error("hash check failed at "+std::to_string(checks));}
template<class F> void throws(F&& operation) {
    bool caught=false;try{operation();}catch(const std::exception&){caught=true;}check(caught);
}
std::string sha(std::string_view input) {Sha1 state;state.update(input);return classic_hex_digest(state.finalize_words_043270());}
std::string pattern(std::size_t n) {std::string s(n,'\0');for(std::size_t i=0;i<n;++i)s[i]=static_cast<char>((i*37+11)%256);return s;}
constexpr std::pair<std::size_t,const char*> patterns[]{
{0,"da39a3ee5e6b4b0d3255bfef95601890afd80709"},
{1,"067d5096f219c64b53bb1c7d5e3754285b565a47"},
{55,"c4622048cfef59b72875839ee7ae1cbcf55e7658"},
{56,"ddc12942656468475970fa4fa49161f52ed138e4"},
{57,"c3299df7d6a56509d38a55d09ccf403d4a56d044"},
{63,"7f8c3fa49f1297bd8b9feb964b6b419987f9f0d1"},
{64,"a334b47180c61fd522f99905ec02c36f9e848211"},
{65,"dd27d9eb923d39687e10872c3e8133ba2f0a68a1"},
{119,"bea949473b1ec34747ce121c3293624b5d9d8f84"},
{120,"bf05266acd3ec21592b4d42aaea97fa6f3e51926"},
{127,"b2b4bfd7b2112a167b77a600cca227593523c406"},
{128,"3b1953091899492377f686c266b81d84b5d40f70"},
{129,"4fd6558b2a93925fb7129447e1d1fac8cff56287"},
{8191,"9dc1ee7f066f90afe5b2c74e97e4a45c3a4b0e3d"},
{8192,"706e3fc6a93a816c4bb435f166fa60e969a18d35"},
{8193,"97a5881d35343154191c8f5e692c0eeb565ec878"},
{16384,"d93c49629ae0053dc6df04e92aa1b87228033b9c"}
};
struct MockState {
    std::string data;
    std::vector<std::string> events;
    std::vector<std::int32_t> lowercase_inputs;
    std::uint32_t initial=0;
    std::size_t position=0,read_limit=8192;
    bool stop=false,close_failure=false,null_stream=false;
    std::string throw_at;
    void event(std::string name) {events.push_back(name);if(throw_at==name)throw std::runtime_error(name);}
};
class MockStream final:public InputStream {
    MockState& s_;
public:
    explicit MockStream(MockState& state):s_(state){}
    ~MockStream() override {s_.events.push_back("destruct");}
    std::uint32_t initial_state() const override {s_.event("initial");return s_.initial;}
    std::int32_t peek_043790() override {
        s_.event("peek");return s_.stop||s_.position==s_.data.size()?-1:static_cast<unsigned char>(s_.data[s_.position]);
    }
    std::size_t read_03b2a0(std::span<std::uint8_t> destination) override {
        s_.event("read");check(destination.size()==8192);
        const auto n=std::min({destination.size(),s_.read_limit,s_.data.size()-s_.position});
        std::copy_n(s_.data.begin()+s_.position,n,destination.begin());s_.position+=n;
        if(n<destination.size())s_.stop=true;
        return n;
    }
    void close_03ae70() override {s_.event("close");if(s_.close_failure)s_.stop=true;}
};
class MockFiles final:public FileFactory {
    MockState& s_;
public:
    explicit MockFiles(MockState& s):s_(s){}
    std::unique_ptr<InputStream> open_input_039f70(const Path& p,std::uint32_t mode,std::uint32_t protection) override {
        s_.event("open");check(p==u"synthetic-file.bin");check(mode==0x20);check(protection==0x40);
        return s_.null_stream?nullptr:std::make_unique<MockStream>(s_);
    }
};
class MockText final:public TextEnvironment {
    MockState& s_;
public:
    std::string override_text;
    bool use_override=false;
    explicit MockText(MockState& s):s_(s){}
    std::string format_digest_043270(const DigestWords& w) override {
        s_.event("format");return use_override?override_text:classic_hex_digest(w);
    }
    std::int32_t tolower_05fca0(std::int32_t c) override {
        s_.event("lower");s_.lowercase_inputs.push_back(c);return c>='A'&&c<='Z'?c+32:c;
    }
};
bool match(MockState& state,std::string expected) {MockFiles files(state);MockText text(state);return file_hash_matches_042d20(u"synthetic-file.bin",std::move(expected),files,text);}
void core_tests() {
    check(sha("")=="da39a3ee5e6b4b0d3255bfef95601890afd80709");
    check(sha("abc")=="a9993e364706816aba3e25717850c26c9cd0d89d");
    check(sha("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")=="84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    check(sha(std::string(1000000,'a'))=="34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    for(const auto& [n,expected]:patterns) {
        const auto input=pattern(n);
        check(sha(input)==expected);
        for(std::size_t stride=1;stride<=129;++stride) {
            Sha1 digest;
            digest.update(std::string_view{});
            for(std::size_t p=0;p<input.size();p+=stride)
                digest.update(std::string_view(input).substr(p,stride));
            check(classic_hex_digest(digest.finalize_words_043270())==expected);
            check(classic_hex_digest(digest.finalize_words_043270())==patterns[0].second);
            digest.update("abc");
            check(classic_hex_digest(digest.finalize_words_043270())=="a9993e364706816aba3e25717850c26c9cd0d89d");
        }
    }
    check(classic_hex_digest({0,1,0xf,0x100,0xffffffffu})=="00000000000000010000000f00000100ffffffff");
}
void file_tests() {
    for(const auto& [n,expected]:patterns) {
        MockState s;s.data=pattern(n);
        check(match(s,expected));
        check(s.position==n);check(s.events.back()=="destruct");
        const auto reads=(n+8191)/8192;
        check(static_cast<std::size_t>(std::count(s.events.begin(),s.events.end(),"read"))==reads);
        check(static_cast<std::size_t>(std::count(s.events.begin(),s.events.end(),"peek"))==reads+1);
        check(std::count(s.events.begin(),s.events.end(),"close")==1);
        check(s.lowercase_inputs.size()==80);
    }
    {MockState s;s.data="abc";const std::string configured="A9993E364706816ABA3E25717850C26C9CD0D89D";
     check(match(s,configured));check(configured=="A9993E364706816ABA3E25717850C26C9CD0D89D");}
    for(auto initial:{1u,2u,4u,0x17u,0x20u}) {
        MockState s;s.initial=initial;check(!match(s,patterns[0].second));
        check(s.events==std::vector<std::string>{"open","initial","destruct"});
    }
    {MockState s;s.data="abc";check(!match(s,""));}
    {MockState s;s.data="abc";check(!match(s,"a9993e364706816aba3e25717850c26c9cd0d89d0"));}
    {MockState s;s.data="abc";check(!match(s,"a9993e364706816aba3e25717850c26c9cd0d89d\n"));}
    {MockState s;s.data="abc";s.close_failure=true;check(match(s,sha("abc")));}
    // An early short read consumes its prefix, then stream fail state makes
    // the next peek EOF. The original compares that prefix digest.
    {MockState s;s.data=pattern(8193);s.read_limit=63;check(match(s,patterns[5].second));check(s.position==63);}
    {MockState s;s.data="abc";s.read_limit=0;check(match(s,patterns[0].second));check(s.position==0);}
    for(const auto& at:{"open","initial","peek","read","close","format","lower"}) {
        MockState s;s.data="abc";s.throw_at=at;throws([&]{match(s,sha("abc"));});
        if(s.throw_at!="open")check(s.events.back()=="destruct");
    }
    {MockState s;s.null_stream=true;throws([&]{match(s,"");});}
    // Required signed-byte promotion, full length including embedded NUL,
    // and actual-then-expected conversion order.
    {MockState s;MockFiles files(s);MockText text(s);text.use_override=true;
     text.override_text=std::string({'A','\0','Z',char(0x80),char(0xff)});
     check(file_hash_matches_042d20(u"synthetic-file.bin",std::string({'a','\0','z',char(0x80),char(0xff)}),files,text));
     check(s.lowercase_inputs==std::vector<std::int32_t>{65,0,90,-128,-1,97,0,122,-128,-1});}
    // Formatting belongs to the required locale contract. The matcher itself
    // does not force length 40 or validate that either string is hexadecimal.
    {MockState s;MockFiles files(s);MockText text(s);text.use_override=true;
     check(file_hash_matches_042d20(u"synthetic-file.bin","",files,text));}
}
} // namespace
int main() {try{core_tests();file_tests();std::cout<<checks<<" deterministic hash checks PASS\n";return 0;}
catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
