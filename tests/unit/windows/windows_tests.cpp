// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/windows_effects.hpp"
#include "laureondo/core/license_files.hpp"
#include "laureondo/core/token_constants.hpp"
#include "laureondo/xml/reader.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace win=laureondo::windows;
namespace file=laureondo::file;
namespace lic=laureondo::license;
namespace enc=laureondo::encoding;
namespace file_status=laureondo::status;
namespace token=laureondo::token;
namespace crypto=laureondo::crypto;
namespace xml=laureondo::xml;
namespace reader=laureondo::xml_reader_bridge;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]] void unused(){throw std::logic_error("Unexpected remaining application effect");}
struct NoResponse{};
class Remaining final:public lic::Effects {
public:
    const char* online_auth_virtual_30(const char*)override{unused();}
    lic::FileStatus status_01ea20(const file::Path&,std::uint32_t)override{unused();}
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const file::Path&)override{unused();}
    bool file_hash_matches_042d20(const file::Path&,std::string)override{unused();}
    std::string load_token_02afa0(const file::Path&)override{unused();}
    file::Path filesystem_widen_006d70(std::string_view)override{unused();}
    file::Path filename_00e120(const file::Path&)override{unused();}
    file::Path parent_path_019820(const file::Path&)override{unused();}
    file::Path join_016b00(const file::Path&,const file::Path&)override{unused();}
    std::string path_utf8_01e3f0(const file::Path&)override{unused();}
    std::string current_time_041b60_null_true()override{unused();}
    bool wine_detected_045010()override{unused();}
    void log_literal_006880(std::string_view)override{unused();}
    void log_string_006860(std::string_view)override{unused();}
    bool write_ticket_03c1d0(const file::Path&,std::string_view)override{unused();}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{unused();}
    [[noreturn]] void exit_05cdc0(std::int32_t)override{unused();}
    [[noreturn]] void no_response_02bcd0_04d228()override{throw NoResponse{};}
};
class NoAttributes final:public xml::AttributeReader {
public:const char* attribute(laureondo::handlers::Request,const char*)override{unused();}
};
std::string encrypted_fixture() {
    std::string bytes(65,'H');bytes+="<GameToken>synthetic-win-token<&</GameToken>";
    crypto::Cipher cipher;cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    check(cipher.encrypt_with_offset(bytes,65),"encrypt generated token fixture");return bytes;
}
void run(const std::filesystem::path& directory) {
    const auto root=std::filesystem::absolute(directory).lexically_normal();
    check(root.filename()=="windows-fixtures","dedicated fixture directory");std::filesystem::create_directories(root);
    Remaining remaining;file_status::NativeStats stats;enc::NativeCalls encoding;enc::PortableErrors errors;
    win::WindowsEffects windows(remaining,stats,encoding,errors);
    file::NativeFactory io;file::NativeTextEnvironment text;
    // Order matters: token status inside LicenseFiles must reach WindowsEffects.
    file::LicenseFiles complete(windows,io,text);
    const auto artifact=(root/std::filesystem::path(u"artifact_\u00e7_\u03a9.bin")).u16string();
    check(file::write_binary_03c1d0(artifact,"abc",io),"create own Unicode artifact");
    auto status=complete.status_01ea20(artifact,3);
    check(status.kind==2&&status.error==0,"actual native file status through composition");
    status=complete.status_01ea20(root.u16string(),3);check(status.kind==3&&status.error==0,"actual directory status");
    const auto missing=(root/"not-generated.bin").u16string();
    status=complete.status_01ea20(missing,3);check(status.kind==1&&status.error==2,"actual missing-file kind/error preserved");
    check(complete.filename_00e120(artifact)==u"artifact_\u00e7_\u03a9.bin","recovered filename source used");
    check(complete.parent_path_019820(artifact)==root.u16string(),"recovered parent source used");
    check(complete.join_016b00(root.u16string(),u"artifact_\u00e7_\u03a9.bin")==artifact,"recovered Windows join");
    check(complete.path_utf8_01e3f0(complete.filename_00e120(artifact))=="artifact_\xc3\xa7_\xce\xa9.bin","actual UTF8 conversion through composition");
    check(complete.filesystem_widen_006d70("_cached.dlf")==u"_cached.dlf","actual filesystem codepage conversion");
    const std::string expected="a9993e364706816aba3e25717850c26c9cd0d89d";
    check(complete.file_hash_matches_042d20(artifact,expected),"actual hash stream through composed Windows/file adapters");
    check(complete.load_token_02afa0(missing).empty(),"actual native missing status suppresses token read");
    const auto base=(root/std::filesystem::path(u"license_\u00e7_\u03a9")).u16string();
    check(file::write_binary_03c1d0(base+u".dlf",encrypted_fixture(),io),"create own token fixture");
    check(complete.load_token_02afa0(base+u".dlf")=="synthetic-win-token<&","native status/read/CBC/raw extraction");
    std::string auth,license_value,exe_hash(40,'0'),content_id="fixture-content";
    std::uint8_t online=0,fake_auth=0,blocked=0;std::int32_t attempts=0;
    laureondo::handlers::Attributes game;
    lic::Handlers handler({auth,online,fake_auth,license_value,expected,exe_hash,content_id,
        artifact,artifact,base,game,attempts,blocked},complete);
    reader::Backend backend;auto parsed=backend.parse_c_string("<RequestLicense/>");check(!parsed.has_error,"actual XML parse");
    NoAttributes attributes;xml::MemoryPrinter printer;xml::HandlerWriter writer(printer,attributes);
    check(handler.request_license_0320c0(*parsed.document->first_element(),writer),"license request without status/path/file/hash/token mocks");
    check(printer.bytes_without_terminator()=="<RequestLicenseResponse License=\"synthetic-win-token&lt;&amp;\"/>\n","actual loaded token XML bytes");
    check(attempts==1&&blocked==0&&license_value=="synthetic-win-token<&","normal file state");
    check(handler.invalidate_license_030c50(*parsed.document->first_element(),writer),"invalidate loaded state");
    check(!handler.request_license_0320c0(*parsed.document->first_element(),writer),"missing cached file plus missing ticket returns false");
    check(attempts==2&&blocked==1,"actual cached status result participates in persistent blocking");
    bool no_response=false;try{(void)handler.request_license_0320c0(*parsed.document->first_element(),writer);}catch(const NoResponse&){no_response=true;}
    check(no_response,"remaining typed no-response effect propagates");
}
}
int main(int argc,char** argv){try{if(argc!=2)throw std::invalid_argument("Pass windows-fixtures directory");run(argv[1]);
    std::cout<<"PASS: "<<checks<<" Windows/license integration checks; actual status/path/conversion/IO/hash/CBC/XML on generated fixtures; remaining application effects controlled\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
