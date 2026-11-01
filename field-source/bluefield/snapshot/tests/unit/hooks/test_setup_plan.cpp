// SPDX-License-Identifier: GPL-3.0-only
#include "src/hooks/setup.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace preloader_l;
namespace {
unsigned checks=0;
void require(bool value){++checks;if(!value)throw std::runtime_error("hook staging contract "+std::to_string(checks));}
struct Mock final:host::Effects {
    unsigned initialized=0,created=0,queued=0;int fail_create=-1,fail_queue=-1;
    bool missing_module=false,missing_symbol=false;
    std::vector<std::string> names,traces;
    std::u16string filesystem_widen_006d70(std::string_view s) override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t n,std::uint32_t,std::uint8_t) override{return n;}
    bool verify_version(const host::VersionRequest&,std::uint32_t,std::uint64_t) override{return true;}
    std::int32_t initialize_engine_046a20() override{++initialized;return 0;}
    host::Address get_module_handle(std::u16string_view) override{return missing_module?0:0x1000;}
    host::Address get_proc_address(host::Address,std::string_view name) override{
        names.emplace_back(name);return missing_symbol?0:0x2000+names.size()*16;
    }
    std::int32_t create_hook_0466e0(host::Address,host::Callback callback,host::Address& original) override{
        ++created;const auto index=static_cast<int>(host::slot(callback));original=0x3000+static_cast<unsigned>(index)*16;
        return index==fail_create?7:0;
    }
    std::int32_t queue_hook_enable(host::Address) override{
        const auto index=static_cast<int>(queued++);return index==fail_queue?8:0;
    }
    void trace_hook(std::string_view phase,std::string_view name,std::int32_t status) noexcept override{
        traces.emplace_back(std::string(phase)+":"+std::string(name)+":"+std::to_string(status));
    }
    void log_text_006880(std::string_view) override{}
    void log_integer_00af90(std::int32_t) override{}
    void finish_log_0088e0() override{}
};
}
int main(){try{
    for(bool processes:{false,true})for(bool folders:{false,true}){
        configuration::State config{};config.block_reruns=processes;
        if(folders)config.documents=u"C:\\fixture\\docs";
        host::Values values;std::vector<host::Address> targets;std::array<host::Address,host::callback_count> originals{};Mock mock;
        host::initialize_host_00e310({config,values,targets,originals},mock);
        const auto expected=6u+(processes?2u:0u)+(folders?2u:0u);
        require(mock.initialized==1);require(mock.created==expected);require(mock.queued==expected);require(targets.size()==expected);
        for(std::size_t i=0;i<6;++i)require(originals[i]!=0);
        require(originals[host::slot(host::Callback::crypt_query_001264)]==0); // native owner stages this separately
        require(mock.traces.front()=="resolve begin:connect:0");
        require(mock.traces.back().starts_with("queue end:"));
        require(values.contains("Documents")==folders);if(folders)require(values.at("Documents")==config.documents);
    }
    for(bool queue_failure:{false,true}){
        configuration::State config{};host::Values values;std::vector<host::Address> targets;std::array<host::Address,host::callback_count> originals{};Mock mock;
        if(queue_failure)mock.fail_queue=0;else mock.fail_create=0;
        host::initialize_host_00e310({config,values,targets,originals},mock);
        require(targets.size()==5);require(mock.created==6);require(originals[0]!=0);
        require(mock.queued==(queue_failure?6u:5u)); // outer native owner must reject incomplete staging
    }
    for(bool symbol:{false,true}){
        Mock mock;mock.missing_module=!symbol;mock.missing_symbol=symbol;
        host::Address original=0x1122,target=0x3344;
        const auto status=host::create_named_hook_046970(mock,u"fixture","connect",host::Callback::connect_00fd00,original,&target);
        require(status==(symbol?12:11));require(original==0x1122);require(target==0x3344);require(mock.created==0);require(mock.queued==0);
    }
    std::cout<<"PASS source hook stage-only plan, optional surfaces, partial create/queue and missing target checks="<<checks<<"; no native engine executed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
