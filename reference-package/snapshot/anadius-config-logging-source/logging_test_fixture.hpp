#pragma once
#include "logging.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include <iomanip>
#include <sstream>

namespace b11_test {
using namespace anadius;
using Path=configuration::Path;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct Buffer:std::stringbuf {
    unsigned flushes=0;std::streamsize maximum=-1;bool fail_flush=false;
    std::streamsize xsputn(const char* p,std::streamsize n)override{
        return std::stringbuf::xsputn(p,maximum<0?n:std::min(n,maximum));
    }
    int sync()override{++flushes;return fail_flush?-1:0;}
};
// Base construction precedes ostream; the shared owner owns its streambuf too.
struct OwnedStream:Buffer,std::ostream {OwnedStream():std::ostream(static_cast<Buffer*>(this)){} };
struct ConversionState {
    OwnedStream* stream=nullptr;std::vector<std::string> prefixes;
    std::vector<std::uint32_t> codepages,flags;
    std::vector<Path> inputs;unsigned locale=0,ansi=0;
    unsigned fail_call=0,short_fill=0;bool inject_tail=false;
};
class Calls final:public encoding_recovered::Calls {
    std::shared_ptr<ConversionState> state_;encoding_recovered::NativeCalls actual_;
public:
    explicit Calls(std::shared_ptr<ConversionState> s):state_(std::move(s)){}
    std::uint32_t locale_codepage_0629ec()override{++state_->locale;return 1254;}
    bool are_file_apis_ansi()override{++state_->ansi;return true;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t f,const char* p,int n,char16_t* out,int cap)override{return actual_.multi_byte_to_wide(cp,f,p,n,out,cap);}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t f,const char16_t* p,int n,char* out,int cap,bool* used)override{
        state_->codepages.push_back(cp);state_->flags.push_back(f);state_->inputs.emplace_back(p,p+n);
        if(state_->stream)state_->prefixes.push_back(state_->stream->str());
        if(state_->fail_call==state_->codepages.size())throw std::runtime_error("injected conversion outcome");
        if(state_->inject_tail){if(!out)return 4;out[0]='Z';return 1;}
        return actual_.wide_to_multi_byte(cp,f,p,n,out,cap,used);
    }
    std::uint32_t last_error()override{return actual_.last_error();}
};
struct Graph {
    std::shared_ptr<OwnedStream> output=std::make_shared<OwnedStream>();
    std::shared_ptr<ConversionState> conversion=std::make_shared<ConversionState>();
    std::shared_ptr<b8_test::LowerState> lower=std::make_shared<b8_test::LowerState>();
    config_logging::ConfigurationLogging selected;
    Graph():selected(output,std::make_unique<b8_test::Lower>(lower),std::make_unique<Calls>(conversion)){
        conversion->stream=output.get();
    }
};
} // namespace b11_test
