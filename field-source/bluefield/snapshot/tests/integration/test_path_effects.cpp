// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/providers.hpp"
#include "src/configuration/defaults.hpp"
#include "src/profiles/online_session.hpp"
#include "src/xml/reader.hpp"
#include <iostream>
#include <type_traits>
#include <vector>
namespace {
using namespace preloader_l;
using Windows=paths::WindowsEffects;
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
void lexical(){
 struct Part {Path value,parent,name;};
 const std::vector<Part> parts={
  {u"",u"",u""},{u"a",u"",u"a"},{u"a/../b",u"a/..",u"b"},
  {uR"(C:\a\b\)",uR"(C:\a\b)",u""},
  {uR"(\\server\share\x)",uR"(\\server\share)",u"x"},
  {uR"(\\?\C:\a\..\f)",uR"(\\?\C:\a\..)",u"f"},
  {uR"(\??\C:\a)",uR"(\??\C:)",u"a"},
  {u"C:relative",u"C:",u"relative"},{uR"(\rooted)",uR"(\)",u"rooted"},
  {u"//server",u"//server",u""},{u"///a",u"///",u"a"},
  {Path{char16_t(0xd800),u'\\',u'x'},Path{char16_t(0xd800)},u"x"},
  {Path{u'a',0,u'b',u'\\',u'x'},Path{u'a',0,u'b'},u"x"}
 };
 for(const auto& c:parts){require(Windows::parent(c.value)==c.parent,"Windows parent literal");require(Windows::filename(c.value)==c.name,"Windows filename literal");}
 struct Join {Path left,right,result;};
 const std::vector<Join> joins={
  {uR"(C:\base)",u"child",uR"(C:\base\child)"},
  {uR"(C:\base)",u"C:child",uR"(C:\base\child)"},
  {uR"(C:\base)",u"c:child",u"c:child"},
  {uR"(C:\base)",uR"(D:\x)",uR"(D:\x)"},
  {uR"(C:\base)",uR"(\x)",uR"(C:\x)"},
  {uR"(\\srv\share)",u"/x",uR"(\\srv/x)"},
  {u"C:",u"",u"C:"},{uR"(\\srv)",u"",uR"(\\srv\)"},
  {u"",u"C:relative",u"C:relative"},{u"a",u"a",uR"(a\a)"}
 };
 for(const auto& c:joins)require(Windows::join(c.left,c.right)==c.result,"Windows join literal");
 const Path exe=uR"(C:\game\fixture.exe)";
 struct Resolve {Path value,result;};
 const std::vector<Resolve> resolutions={
  {u"a/../b",uR"(C:\game\a/../b)"},{u"C:child",uR"(C:\game\child)"},
  {u"c:child",u"c:child"},{uR"(\rooted)",uR"(C:\rooted)"},
  {uR"(C:\abs)",uR"(C:\abs)"},{uR"(\\server)",uR"(\\server)"},
  {uR"(\\?\C:\a)",uR"(\\?\C:\a)"},{u"",uR"(C:\game\)"},
  {u".",uR"(C:\game\.)"},{u"/rooted",u"C:/rooted"},
  {u"D:leaf",u"D:leaf"},{Path{u'a',0,u'b'},Path(uR"(C:\game\)")+Path{u'a',0,u'b'}}
 };
 for(const auto& c:resolutions){require(Windows::resolve(c.value,exe,true)==c.result,"Windows resolver literal");require(Windows::resolve(c.value,exe,false)==c.value,"Windows resolver false leaves code units intact");}
}
void callers(){
#ifdef _WIN32
 static_assert(std::is_same_v<PathEffects,Windows>);
#else
 static_assert(std::is_same_v<PathEffects,HostPathEffects>);
#endif
 namespace fs=std::filesystem;const auto root=fs::temp_directory_path()/"preloader_l_path_effects_no_io";
 Environment env{RuntimePaths::fixture(root/"base"/"fixture.exe",root/"base"/"surge.cfg",root/"data",root/"programdata")};
 auto state=configuration::initial_state();achievement_storage::Paths paths;
 xml_reader::Backend xml;achievement_storage::HostCrtNumbers numbers;timestamps::ClassicIso parser;
 StorageEffects storage_effects{parser,env.paths};files::NativeFactory files{env.paths};
 achievement_storage::Storage storage{paths,xml,numbers,storage_effects,files};ConfigurationEffects config{state,env,storage};
 std::uint8_t flag=0;OnlineSession online{state,env,flag};achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
 AvatarStore avatars{env.paths};ContentEffects content{state,paths,clock,formatter,storage_effects,files,env,online,avatars};LicenseEffects license{env,clock,formatter,online};
 const Path value=model_path(root/"left"/"a"/"b");const Path parent=model_path(root/"left"/"a");
 const Path joined=model_path(root/"left"/"a"/".."/"x");
 const Path tail=model_path(fs::path("..")/"x");
 require(config.parent_path_019820(value)==parent,"Configuration parent uses common owner");
 require(config.filename(value)==u"b","Configuration filename uses common owner");
 require(config.join_016b00(parent,tail)==joined,"Configuration join uses common owner");
 require(license.parent_path_019820(value)==parent,"License parent uses common owner");
 require(license.filename_00e120(value)==u"b","License filename uses common owner");
 require(license.join_016b00(parent,tail)==joined,"License join uses common owner");
 require(content.path_append_016b50(parent,tail)==joined,"Content join uses common owner");
 require(online.parent_path_019820(value)==parent,"Online parent uses common owner");
 require(online.join_016b00(parent,tail)==joined,"Online join uses common owner");
 require(config.resolve_path_0192d0("a/../b",model_path(env.executable),true)==model_path(root/"base"/fs::path("a/../b")),"Configuration resolver retains dot spelling");
#ifndef _WIN32
 require(PathEffects::resolve(u"/absolute",model_path(env.executable),true)==u"/absolute","POSIX absolute projection retained separately");
#endif
}
}
int main(){try{lexical();callers();std::cout<<"PASS path effects checks="<<checks<<"; pure source Windows lexical bodies + actual platform-selected callers; no native Windows IO/ABI acceptance\n";return 0;}catch(const std::exception& error){std::cerr<<"FAIL path effects: "<<error.what()<<'\n';return 1;}}
