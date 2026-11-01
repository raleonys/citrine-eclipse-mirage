#include "../../src/config/runtime_configuration_command_line.hpp"
#include <cstring>
using namespace preloader_l::config::runtime_command_line_detail;
static bool eq(CommandToken t,const char* s){ return token_equals(t,s); }
int main(){
    { const char* p="app -emuGameLang tr_TR"; CommandToken t{};
      if(next_command_token(&p,&t)!=CommandTokenStatus::token||!eq(t,"app"))return 1;
      if(next_command_token(&p,&t)!=CommandTokenStatus::token||!eq(t,"-emuGameLang"))return 2;
      if(next_command_token(&p,&t)!=CommandTokenStatus::token||!eq(t,"tr_TR"))return 3; }
    { const char* p="app -emuGameLang \"fr_FR\"junk"; CommandToken t{};
      if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 4;
      if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 5;
      if(next_command_token(&p,&t)!=CommandTokenStatus::malformed)return 6; }
    { const char* p="app -emuGameLang \"fr_FR"; CommandToken t{};
      if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 7;
      if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 8;
      if(next_command_token(&p,&t)!=CommandTokenStatus::malformed)return 9; }
    { const char* p="app -emuGameLang en_US -emuGameLang \"tr_TR\""; CommandToken t{},selected{};
      if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 10;
      for(;;){ auto st=next_command_token(&p,&t); if(st==CommandTokenStatus::end)break; if(st!=CommandTokenStatus::token)return 11;
        if(!eq(t,"-emuGameLang"))continue; if(next_command_token(&p,&t)!=CommandTokenStatus::token)return 12; selected=t; }
      if(!eq(selected,"tr_TR"))return 13; }
    return 0;
}
