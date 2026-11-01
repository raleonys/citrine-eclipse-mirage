#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include <cassert>
#include <string>
using namespace preloader_l::config;
static void test_many_cfg_settings(){
 std::string cfg="\"Config2\" { \"User\" { ";
 for(int i=0;i<96;++i)cfg+="\"K"+std::to_string(i)+"\" \"V"+std::to_string(i)+"\" ";
 cfg+="} }";
 assert(set_runtime_configuration_cfg(cfg.data(),cfg.size())==RuntimeConfigurationStatus::ok);
 const auto v=runtime_configuration_setting("K95");assert(v.present&&std::string(v.data,v.size)=="V95");
}

static void test_long_cfg_key(){
 std::string key(512u,'K');
 std::string cfg="\"Config2\" { \"User\" { \""+key+"\" \"VALUE\" } }";
 assert(set_runtime_configuration_cfg(cfg.data(),cfg.size())==RuntimeConfigurationStatus::ok);
 const auto v=runtime_configuration_setting(key.c_str());
 assert(v.present&&std::string(v.data,v.size)=="VALUE");
}
static void test_cfg_depth_boundary(){
 std::string ok="\"Config2\" { \"A\" {";
 for(int i=0;i<30;++i)ok+="\"N\" {";
 ok+="\"Value\" \"x\"";
 for(int i=0;i<32;++i)ok+="}";
 assert(set_runtime_configuration_cfg(ok.data(),ok.size())==RuntimeConfigurationStatus::ok);
 std::string bad="\"Config2\" { \"A\" {";
 for(int i=0;i<40;++i)bad+="\"N\" {";
 bad+="\"Value\" \"x\"";
 for(int i=0;i<42;++i)bad+="}";
 assert(set_runtime_configuration_cfg(bad.data(),bad.size())==RuntimeConfigurationStatus::invalid_cfg);
}
static void test_large_cfg(){
 std::string cfg="\"Config2\" { \"User\" { \"Big\" \"";
 cfg.append((1024u*1024u)+8192u,'A');cfg+="\" } }";
 assert(set_runtime_configuration_cfg(cfg.data(),cfg.size())==RuntimeConfigurationStatus::ok);
 const auto v=runtime_configuration_setting("Big");assert(v.present&&v.size==(1024u*1024u)+8192u);
}
static void test_store_over_old_record_cap(){
 std::string xml="<Root><Store>";constexpr size_t count=4100;
 for(size_t i=0;i<count;++i)xml+="<Entitlement NumericId=\""+std::to_string(i+1)+"\"/>";
 xml+="</Store></Root>";assert(set_runtime_store_xml(xml.data(),xml.size())==StoreRuntimeStatus::ok);
 assert(runtime_store_entitlement_count()==count);
}
int main(){test_many_cfg_settings();test_long_cfg_key();test_cfg_depth_boundary();test_large_cfg();test_store_over_old_record_cap();clear_runtime_configuration();clear_runtime_store();return 0;}
