// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/ptd_runtime.hpp"
using namespace preloader_l::runtime;
int main(){
 const auto*p=default_exception_action_table(); if(!p||default_exception_action_count()!=12)return 1;
 const uint32_t ex[12]={0xC0000005u,0xC000001Du,0xC0000096u,0xC000008Du,0xC000008Eu,0xC000008Fu,0xC0000090u,0xC0000091u,0xC0000092u,0xC0000093u,0xC00002B4u,0xC00002B5u};
 const int sig[12]={11,4,4,8,8,8,8,8,8,8,8,8};
 for(int i=0;i<12;i++)if(p[i].exception_number!=ex[i]||p[i].signal_number!=sig[i]||p[i].action!=0)return 2+i;
 if(ptd_size!=0x3C8||ptd_offset::errno_value!=0x20||ptd_offset::invalid_parameter_handler!=0x3B8)return 20;
 return 0;
}
