#include "preloader_l/config/store_runtime.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <string>
using namespace preloader_l::config;
static bool eq(ValueView v,const char* s){const size_t n=std::strlen(s);return v.present&&v.size==n&&!std::memcmp(v.data,s,n);}
int main(){
    std::array<int,4> order{{0,1,2,3}};
    unsigned checked=0;
    do {
        std::string xml="<Config><Store><Entitlement>";
        for(int x:order){
            if(x==0) xml+="<FeatureDesc>D1</FeatureDesc>";
            else if(x==1) xml+="<FeatureDesc>D2</FeatureDesc>";
            else if(x==2) xml+="<FeatureFlag>F1</FeatureFlag>";
            else xml+="<FeatureFlag>F2</FeatureFlag>";
        }
        xml+="</Entitlement></Store></Config>";
        clear_runtime_store();
        if(set_runtime_store_xml(xml.data(),xml.size())!=StoreRuntimeStatus::ok) return 1;
        const char* desc_expected[2]={nullptr,nullptr};
        const char* flag_expected[2]={nullptr,nullptr};
        size_t di=0,fi=0;
        for(int x:order){
            if(x==0) desc_expected[di++]="D1";
            else if(x==1) desc_expected[di++]="D2";
            else if(x==2) flag_expected[fi++]="F1";
            else flag_expected[fi++]="F2";
        }
        if(!eq(runtime_store_feature_desc(0,0),desc_expected[0]) || !eq(runtime_store_feature_desc(0,1),desc_expected[1])) return 2;
        if(!eq(runtime_store_feature_flag(0,0),flag_expected[0]) || !eq(runtime_store_feature_flag(0,1),flag_expected[1])) return 3;
        ++checked;
    } while(std::next_permutation(order.begin(),order.end()));
    clear_runtime_store();
    return checked==24 ? 0 : 4;
}
