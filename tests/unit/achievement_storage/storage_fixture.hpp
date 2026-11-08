// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/runtime/achievement_storage.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

using namespace citrine_eclipse_mirage;
namespace {
unsigned checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
using Path=configuration::Path;
Path wide(std::string_view s){return {s.begin(),s.end()};}
Path join(const Path& a,const Path& b){return path::join_016b00(a,b);}
configuration::Achievement record(std::string name={}){return {5,9,8,"old-grant",std::move(name),"steam-id",77};}
struct Input final:file::Input {
    std::string bytes;bool exists;
    Input(std::string b,bool e):bytes(std::move(b)),exists(e){}
    std::uint32_t initial_state()const override{return exists?0:1;}
    void seek_03b610(std::int64_t p,file::Seek)override{require(p==0,"seek zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{std::copy(bytes.begin(),bytes.end(),out.begin());return bytes.size();}
    void close_03ae70()override{}
};
struct Output final:file::Output {
    std::string& bytes;bool bad;
    Output(std::string& b,bool fail):bytes(b),bad(fail){}
    std::uint32_t initial_state()const override{return bad?1:0;}
    void write_03be80(std::string_view value)override{bytes.assign(value);}
    void close_03ae70()override{}
};
struct Files final:file::Factory {
    std::map<Path,std::string> input,output;
    std::vector<Path> reads,writes;
    bool bad_output=false;
    void replace_atomic(std::span<const file::FileWrite> batch)override {
        if(bad_output)throw std::runtime_error("atomic staging failure");
        for(const auto& w:batch){writes.push_back(w.path);output[w.path]=w.bytes;}
    }
    std::unique_ptr<file::Input> open_input_039f70(const Path& p,std::uint32_t m,std::uint32_t s)override{
        require(m==0x20 && s==0x40,"input flags");reads.push_back(p);const auto it=input.find(p);return std::make_unique<Input>(it==input.end()?"":it->second,it!=input.end());
    }
    std::unique_ptr<file::Output> open_output_03a1a0(const Path& p,std::uint32_t m,std::uint32_t s)override{
        require(m==0x20 && s==0x40,"output flags");writes.push_back(p);return std::make_unique<Output>(output[p],bad_output);
    }
};
struct Effects final:achievement_storage::Effects {
    std::vector<std::string> widened,epochs,events;
    std::vector<Path> directories,status_paths;
    status::Status status{2,0,0};
    std::string fail_epoch;
    Path filesystem_widen_006d70(std::string_view value)override{widened.emplace_back(value);return wide(value);}
    std::int64_t timestamp_to_epoch_041e10(std::string_view value)override{
        epochs.emplace_back(value);events.emplace_back("epoch");if(value==fail_epoch)throw std::runtime_error("synthetic epoch failure");return value=="negative"?-1:11;
    }
    status::Status status_01ea20(const Path& path,std::uint32_t flags)override{require(flags==3,"status flags3");status_paths.push_back(path);events.emplace_back("status");return status;}
    [[noreturn]]void filesystem_error_018130(std::string_view op,std::uint32_t error,const Path&)override{require(op=="status" && error!=0,"status error contract");events.emplace_back("error");throw std::runtime_error("synthetic status failure");}
    void create_directories_018e00(const Path& path)override{directories.push_back(path);events.emplace_back("mkdir");}
};
struct Fixture {
    configuration::State state{};achievement_storage::Paths paths;xml_reader_bridge::Backend xml;achievement_storage::HostCrtNumbers numbers;Effects effects;Files files;
    achievement_storage::Storage storage{paths,xml,numbers,effects,files};
    Fixture(){state.user_directory=u"USER";}
};
}
