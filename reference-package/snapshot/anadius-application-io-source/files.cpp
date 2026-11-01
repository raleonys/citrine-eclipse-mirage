#include "files.hpp"
#include <windows.h>
#include <exception>
#include <stdexcept>
#include <type_traits>

namespace anadius::application_io {
struct GuardedFiles::Impl {
    file_recovered::Factory& lower;Checkpoint checkpoint;
    std::unique_ptr<file_recovered::Input> input;
    std::unique_ptr<file_recovered::Output> output;
    bool input_retirable=false,output_retirable=false,poisoned=false;
    Impl(file_recovered::Factory& f,Checkpoint c):lower(f),checkpoint(c){if(!c.context||!c.check)throw std::invalid_argument("Complete file checkpoint required");}
    void ensure_known()const{if(poisoned)throw std::logic_error("File prefix is unknown");}
    void check()const{const auto saved=::GetLastError();struct Error{DWORD value;~Error(){::SetLastError(value);}} error{saved};ensure_known();checkpoint.check(checkpoint.context);}
    template<class F>std::invoke_result_t<F> leaf(F&& f){check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check();}else{std::invoke_result_t<F> result=f();check();return result;}}
    void retire_input()noexcept{if(!input)return;if(!input_retirable||std::uncaught_exceptions()){poisoned=true;return;}try{check();auto retired=std::move(input);retired.reset();check();}catch(...){poisoned=true;}}
    void retire_output()noexcept{if(!output)return;if(!output_retirable||std::uncaught_exceptions()){poisoned=true;return;}try{check();auto retired=std::move(output);retired.reset();check();}catch(...){poisoned=true;}}
    struct Input final:file_recovered::Input {Impl& v;explicit Input(Impl& v):v(v){}~Input()override{v.retire_input();}
        std::uint32_t initial_state()const override{const auto n=v.leaf([&]{return v.input->initial_state();});if(n)v.input_retirable=true;return n;}
        void seek_03b610(std::int64_t n,file_recovered::Seek s)override{v.leaf([&]{v.input->seek_03b610(n,s);});}
        std::uint64_t position_sum_03ba50()override{return v.leaf([&]{return v.input->position_sum_03ba50();});}
        std::int32_t peek_043790()override{return v.leaf([&]{return v.input->peek_043790();});}
        std::size_t read_03b2a0(std::span<std::uint8_t> b)override{return v.leaf([&]{return v.input->read_03b2a0(b);});}
        void close_03ae70()override{v.leaf([&]{v.input->close_03ae70();});v.input_retirable=true;}
    };
    struct Output final:file_recovered::Output {Impl& v;explicit Output(Impl& v):v(v){}~Output()override{v.retire_output();}
        std::uint32_t initial_state()const override{const auto n=v.leaf([&]{return v.output->initial_state();});if(n)v.output_retirable=true;return n;}
        void write_03be80(std::string_view b)override{v.leaf([&]{v.output->write_03be80(b);});}
        void close_03ae70()override{v.leaf([&]{v.output->close_03ae70();});v.output_retirable=true;}
    };
};
GuardedFiles::GuardedFiles(file_recovered::Factory& f,Checkpoint c):impl_(std::make_unique<Impl>(f,c)){}
GuardedFiles::~GuardedFiles()noexcept{if(has_pending())std::terminate();}
bool GuardedFiles::has_pending()const noexcept{return impl_->input||impl_->output;}
void GuardedFiles::ensure_known()const{impl_->ensure_known();}
std::unique_ptr<file_recovered::Input> GuardedFiles::open_input_039f70(const configuration::Path& p,std::uint32_t m,std::uint32_t s){
    auto& v=*impl_;if(has_pending())throw std::logic_error("File owner already retained");auto wrapper=std::make_unique<Impl::Input>(v);v.check();v.input_retirable=false;
    v.input=v.lower.open_input_039f70(p,m,s);v.check();if(!v.input)return {};return wrapper;
}
std::unique_ptr<file_recovered::Output> GuardedFiles::open_output_03a1a0(const configuration::Path& p,std::uint32_t m,std::uint32_t s){
    auto& v=*impl_;if(has_pending())throw std::logic_error("File owner already retained");auto wrapper=std::make_unique<Impl::Output>(v);v.check();v.output_retirable=false;
    v.output=v.lower.open_output_03a1a0(p,m,s);v.check();if(!v.output)return {};return wrapper;
}
}
