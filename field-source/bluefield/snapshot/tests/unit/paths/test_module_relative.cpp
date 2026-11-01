// SPDX-License-Identifier: GPL-3.0-only
#include "src/paths/module_relative.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace fs=std::filesystem;
using preloader_l::paths::module_sibling;
using preloader_l::paths::SiblingRoute;
namespace {
std::size_t checks=0,gulrak=0,preserved=0;
void require(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
void compare(const fs::path& module,const fs::path& leaf) {
 const auto expected=module.parent_path()/leaf;const auto before=module.native();const auto joined=module_sibling(module,leaf);
 require(joined.value.native()==expected.native(),"exact FIX10 native spelling");require(module.native()==before,"input unmodified");
 if(joined.route==SiblingRoute::gulrak)++gulrak;else ++preserved;
}
}
int main(){try{
 const auto canonical=module_sibling(fs::path("alpha")/"preloader_l.dll","surge.cfg");
 require(canonical.value==fs::path("alpha")/"surge.cfg","fixed expected path");require(canonical.route==SiblingRoute::gulrak,"actual gulrak route");
 require(module_sibling("preloader_l.dll","surge.cfg").value.native()==fs::path("surge.cfg").native(),"relative base");
 require(module_sibling({},"surge_override.cfg").value.native()==fs::path("surge_override.cfg").native(),"empty base");
 const std::vector<fs::path> modules={"preloader_l.dll","alpha/preloader_l.dll","/preloader_l.dll","/alpha/preloader_l.dll","/alpha//beta/preloader_l.dll","//server/share/preloader_l.dll","///alpha/preloader_l.dll","alpha/./preloader_l.dll","alpha/../beta/preloader_l.dll","alpha/",{},".","..",fs::path(u8"alpha_\u00e7\u0131_\u30c6\u30b9\u30c8_\U0001f3c8")/"preloader_l.dll",fs::path(fs::path::string_type(300,static_cast<fs::path::value_type>('x')))/"preloader_l.dll"};
 const std::vector<fs::path> leaves={"surge.cfg","surge_override.cfg","surge.png",fs::path(u8"profil_\u00e7_\U0001f3c8.cfg")};
 for(const auto& m:modules)for(const auto& l:leaves)compare(m,l);
 for(int i=0;i<250;++i)for(const auto& l:leaves)compare(fs::path("area"+std::to_string(i))/(i%2?"sub//dir":"sub/../dir")/"preloader_l.dll",l);
#ifdef _WIN32
 for(const auto* m:{LR"(C:\preloader_l.dll)",LR"(C:\Games\preloader_l.dll)",LR"(C:/Games/preloader_l.dll)",LR"(C:\Games\\module\preloader_l.dll)",LR"(\\server\share\preloader_l.dll)",LR"(\\?\C:\Games\preloader_l.dll)",LR"(\\?\UNC\server\share\preloader_l.dll)",LR"(\??\C:\Games\preloader_l.dll)",LR"(C:preloader_l.dll)",L"C:\\Unicode_\u00e7\u0131_\u30c6_\U0001f3c8\\preloader_l.dll"})for(const auto& l:leaves)compare(m,l);
 compare(fs::path(std::wstring(L"C:\\")+std::wstring(300,L'x')+L"\\preloader_l.dll"),L"surge.cfg");
 auto raw=std::wstring(L"C:\\raw_");raw.push_back(static_cast<wchar_t>(0xd800));raw+=L"\\preloader_l.dll";compare(fs::path(raw),L"surge.cfg");
#else
 std::string raw="/raw_";raw.push_back(static_cast<char>(0xff));raw+="/preloader_l.dll";compare(fs::path(raw),"surge.cfg");
#endif
 auto embedded=fs::path::string_type(1,static_cast<fs::path::value_type>('a'));embedded.push_back(0);embedded+=fs::path("/preloader_l.dll").native();compare(fs::path(embedded),"surge.cfg");
 for(const auto& bad:std::vector<fs::path>{{},".","..","/surge.cfg","sub/surge.cfg","sub\\surge.cfg","C:surge.cfg",fs::path(embedded)}){
  bool rejected=false;try{(void)module_sibling("preloader_l.dll",bad);}catch(const std::invalid_argument&){rejected=true;}require(rejected,"leaf validation");
 }
 require(gulrak>0 && preserved>0,"both compatibility routes exercised");
 std::cout<<"PASS module sibling checks="<<checks<<" gulrak="<<gulrak<<" preserved_native="<<preserved<<"; Windows-specific cases require native execution\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
