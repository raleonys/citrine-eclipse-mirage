// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/core/binary_io.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace io=preloader_l::file;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Injected{};
struct Trace {
    std::vector<std::string> calls;
    std::string bytes,written;
    std::uint64_t size=0;
    std::uint32_t state=0;
    bool close_throws=false,close_fails=false,write_fails=false,write_throws=false;
};
class Input final:public io::Input {
    Trace& t;
public:
    explicit Input(Trace& value):t(value){}
    ~Input()override{t.calls.push_back("destroy-input");}
    std::uint32_t initial_state()const override{t.calls.push_back("state");return t.state;}
    void seek_03b610(std::int64_t offset,io::Seek origin)override{check(offset==0,"seek offset");t.calls.push_back(origin==io::Seek::end?"end":"begin");}
    std::uint64_t position_sum_03ba50()override{t.calls.push_back("tell");return t.size;}
    std::int32_t peek_043790()override{throw Injected{};}
    std::size_t read_03b2a0(std::span<std::uint8_t> buffer)override {
        t.calls.push_back("read");const auto length=std::min(buffer.size(),t.bytes.size());
        std::copy_n(t.bytes.begin(),length,buffer.begin());return length;
    }
    void close_03ae70()override{t.calls.push_back("close");if(t.close_throws)throw Injected{};}
};
class Output final:public io::Output {
    Trace& t;
public:
    explicit Output(Trace& value):t(value){}
    ~Output()override{t.calls.push_back("destroy-output");}
    std::uint32_t initial_state()const override{t.calls.push_back("state");return t.state;}
    void write_03be80(std::string_view bytes)override{t.calls.push_back("write");if(t.write_throws)throw Injected{};t.written=bytes;if(t.write_fails)t.state=2;}
    void close_03ae70()override{t.calls.push_back("close");if(t.close_fails)t.state=4;if(t.close_throws)throw Injected{};}
};
class Factory final:public io::Factory {
public:
    Trace t;
    std::unique_ptr<io::Input> open_input_039f70(const io::Path& path,std::uint32_t mode,std::uint32_t share)override {
        check(path==u"fixture","input path");check(mode==0x20&&share==0x40,"input raw modes");t.calls.push_back("open-input");return std::make_unique<Input>(t);
    }
    std::unique_ptr<io::Output> open_output_03a1a0(const io::Path& path,std::uint32_t mode,std::uint32_t share)override {
        check(path==u"fixture","output path");check(mode==0x20&&share==0x40,"output raw modes");t.calls.push_back("open-output");return std::make_unique<Output>(t);
    }
};
void core() {
    Factory f;f.t.bytes=std::string("a\0b",3);f.t.size=5;
    check(io::read_binary_03b3f0(u"fixture",f)==std::string("a\0b\0\0",5),"short read keeps original zero-filled tail");
    check(f.t.calls==std::vector<std::string>{"open-input","state","end","tell","begin","read","close","destroy-input"},"read operation order");
    for(const auto flags:{1U,2U,4U,16U}) {
        Factory bad;bad.t.state=flags;
        check(io::read_binary_03b3f0(u"fixture",bad).empty(),"nonzero initial state returns empty");
        check(bad.t.calls==std::vector<std::string>{"open-input","state","destroy-input"},"bad open skips read/explicit close");
        bad.t.calls.clear();check(!io::write_binary_03c1d0(u"fixture","x",bad),"nonzero initial state writer false");
    }
    Factory empty;check(io::read_binary_03b3f0(u"fixture",empty).empty(),"zero-sized read");
    Factory fail;fail.t.write_fails=true;
    check(io::write_binary_03c1d0(u"fixture",std::string("x\0y",3),fail),"later write state failure does not change initial success branch");
    check(fail.t.written==std::string("x\0y",3),"write includes NUL by stored length");
    check(fail.t.calls==std::vector<std::string>{"open-output","state","write","close","destroy-output"},"write operation order");
    Factory closing;closing.t.close_throws=true;
    bool thrown=false;try{(void)io::write_binary_03c1d0(u"fixture","x",closing);}catch(const Injected&){thrown=true;}
    check(thrown&&closing.t.calls.back()=="destroy-output","close exception propagates with cleanup");
    Factory writing;writing.t.write_throws=true;thrown=false;
    try{(void)io::write_binary_03c1d0(u"fixture","x",writing);}catch(const Injected&){thrown=true;}
    check(thrown&&writing.t.calls==std::vector<std::string>{"open-output","state","write","destroy-output"},
        "write exception propagates with exactly one destruction and no invented explicit close");
    Factory checked;
    check(io::write_binary_checked(u"fixture",std::string("x\0y",3),checked),
          "checked ticket writer accepts complete write/close");
    check(checked.t.calls==std::vector<std::string>{"open-output","state","write","state","close","state","destroy-output"},
          "checked ticket writer observes current state twice");
    check(checked.t.written==std::string("x\0y",3),"ticket writer preserves binary bytes");
    Factory failed_write;failed_write.t.write_fails=true;
    check(!io::write_binary_checked(u"fixture","x",failed_write),
          "checked writer returns false after nonthrowing write failure");
    check(failed_write.t.calls==std::vector<std::string>{"open-output","state","write","state","close","state","destroy-output"},
          "checked writer still closes after failed write");
    Factory failed_close;failed_close.t.close_fails=true;
    check(!io::write_binary_checked(u"fixture","x",failed_close),
          "checked writer returns false after close failure");
    Factory failed_open;failed_open.t.state=4;
    check(!io::write_binary_checked(u"fixture","x",failed_open),
          "checked writer returns false after open failure");
    check(failed_open.t.calls==std::vector<std::string>{"open-output","state","destroy-output"},
          "failed open cannot reach write");
    Factory throws_write;throws_write.t.write_throws=true;
    thrown=false;try{(void)io::write_binary_checked(u"fixture","x",throws_write);}
    catch(const Injected&){thrown=true;}
    check(thrown&&throws_write.t.calls==std::vector<std::string>{"open-output","state","write","destroy-output"},
          "checked write exception propagates without explicit close");
    Factory throws_close;throws_close.t.close_throws=true;
    thrown=false;try{(void)io::write_binary_checked(u"fixture","x",throws_close);}
    catch(const Injected&){thrown=true;}
    check(thrown&&throws_close.t.calls.back()=="destroy-output",
          "checked close exception retains destruction");
    Factory impossible;impossible.t.size=~std::uint64_t{};
    thrown=false;try{(void)io::read_binary_03b3f0(u"fixture",impossible);}catch(const std::length_error&){thrown=true;}
    check(thrown&&impossible.t.calls.back()=="destroy-input","invalid tell size reaches resize exception");
}
#if !defined(PRELOADER_L_PORTABLE_FILE_IO_TESTS)
void native(const std::filesystem::path& directory) {
    const auto root=std::filesystem::absolute(directory).lexically_normal();
    check(root.filename()=="file-io-fixtures","dedicated generated-fixture directory required");
    std::filesystem::create_directories(root);
    io::NativeFactory factory;
    const auto file=(root/std::filesystem::path(u"roundtrip_\u00e7\u0131\u015f.bin")).u16string();
    std::string bytes;for(unsigned i=0;i<256;++i)bytes.push_back(static_cast<char>(i));
    for(const auto size:{std::size_t(0),std::size_t(1),std::size_t(63),std::size_t(64),std::size_t(8191),std::size_t(8192),std::size_t(8193)}) {
        std::string value;for(std::size_t i=0;i<size;++i)value.push_back(bytes[i%bytes.size()]);
        check(io::write_binary_03c1d0(file,value,factory),"native fixture write");
        check(io::read_binary_03b3f0(file,factory)==value,"native fixture binary roundtrip");
        check(std::filesystem::file_size(std::filesystem::path(file))==size,"native writer truncation");
    }
    check(io::write_binary_checked(file,bytes,factory),"native checked file write");
    check(io::read_binary_03b3f0(file,factory)==bytes,"native checked file bytes");
    const auto missing=(root/"missing-directory"/"file.bin").u16string();
    check(io::read_binary_03b3f0(missing,factory).empty(),"missing input empty");
    check(!io::write_binary_03c1d0(missing,"x",factory),"writer does not create parent directories");
    auto input=factory.open_input_039f70(file,0x20,0x40);
    check(input->peek_043790()==0,"native peek byte");
    std::array<std::uint8_t,9000> chunk{};
    check(input->read_03b2a0(chunk)==8193,"native partial read gcount");
    check(input->peek_043790()==-1,"native EOF peek");
    // A failed tellg() must not wrap streampos(-1) into a huge file size.
    bool invalid_position=false;
    try{(void)input->position_sum_03ba50();}
    catch(const std::ios_base::failure&){invalid_position=true;}
    check(invalid_position,"native EOF-state tellg rejects invalid position");
    input->close_03ae70();
}
#endif
}
int main(int argc,char** argv){try{
    core();
#if defined(PRELOADER_L_PORTABLE_FILE_IO_TESTS)
    (void)argc;(void)argv;
#else
    if(argc!=2)throw std::invalid_argument("Pass a dedicated file-io-fixtures directory");
    native(argv[1]);
#endif
    std::cout<<"PASS: "<<checks<<" IO checks; actual IO only on generated fixture files; original targets not executed or modified\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
