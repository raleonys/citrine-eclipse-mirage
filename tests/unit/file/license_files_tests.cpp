// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/core/license_files.hpp"
#include "citrine_eclipse_mirage/core/token_constants.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include <filesystem>
#include <iostream>
#include <locale>
#include <stdexcept>

namespace file=citrine_eclipse_mirage::file;
namespace license=citrine_eclipse_mirage::license;
namespace crypto=citrine_eclipse_mirage::crypto;
namespace token=citrine_eclipse_mirage::token;
namespace xml=citrine_eclipse_mirage::xml;
namespace reader=citrine_eclipse_mirage::xml_reader_bridge;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]] void unused(){throw std::logic_error("Unexpected unimplemented test dependency");}
struct NoResponse{};
// Controlled application effects; actual file contents/hash/CBC below are not
// simulated. This does NOT implement the original general status/path backend.
class Remaining final:public license::Effects {
public:
    const char* online_auth_virtual_30(const char*)override{unused();}
    license::FileStatus status_01ea20(const file::Path& path,std::uint32_t flags)override {
        check(flags==3,"status flags");
        return {std::filesystem::is_regular_file(std::filesystem::path(path))?2:1,0};
    }
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const file::Path&)override{unused();}
    bool file_hash_matches_042d20(const file::Path&,std::string)override{unused();}
    std::string load_token_02afa0(const file::Path&)override{unused();}
    file::Path filesystem_widen_006d70(std::string_view text)override {
        // Only the synthetic ASCII suffixes .dlf / _cached.dlf are used here.
        check(text==".dlf"||text=="_cached.dlf","test path conversion limited to known ASCII suffix");
        return file::Path(text.begin(),text.end());
    }
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
public:const char* attribute(citrine_eclipse_mirage::handlers::Request,const char*)override{unused();}
};
std::string encrypted_fixture(std::string_view value) {
    std::string bytes(65,'H');bytes+="<GameToken>";bytes+=value;bytes+="</GameToken>";
    crypto::Cipher cipher;cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    check(cipher.encrypt_with_offset(bytes,65),"synthetic token fixture encryption");return bytes;
}
void run(const std::filesystem::path& directory) {
    const auto root=std::filesystem::absolute(directory).lexically_normal();
    check(root.filename()=="file-io-fixtures","dedicated generated fixture path");
    std::filesystem::create_directories(root);
    file::NativeFactory io;file::NativeTextEnvironment text;Remaining remaining;
    file::LicenseFiles binding(remaining,io,text);
    const auto dll=(root/"hash-artifact.bin").u16string();
    check(file::write_binary_03c1d0(dll,"abc",io),"write independent known-hash fixture");
    const std::string expected="A9993E364706816ABA3E25717850C26C9CD0D89D";
    check(binding.file_hash_matches_042d20(dll,expected),"actual file reader + SHA1 known answer + lowercase");
    check(expected=="A9993E364706816ABA3E25717850C26C9CD0D89D","expected source string not consumed");
    check(!binding.file_hash_matches_042d20(dll,std::string(40,'0')),"actual file hash mismatch");
    check(!binding.file_hash_matches_042d20((root/"absent.bin").u16string(),expected),"actual file missing returns false");
    const auto base=(root/"synthetic-license").u16string();
    const auto normal=base+u".dlf",cached=base+u"_cached.dlf";
    check(file::write_binary_03c1d0(normal,encrypted_fixture("fixture-A<&"),io),"write synthetic normal token file");
    check(file::write_binary_03c1d0(cached,encrypted_fixture("fixture-B"),io),"write synthetic cached token file");
    check(binding.load_token_02afa0(normal)=="fixture-A<&","actual file read CBC token extraction");
    std::string auth,license_value,exe_hash(40,'0'),content_id="fixture-content";
    std::uint8_t online=0,fake_auth=0,blocked=0;
    std::int32_t attempts=0;
    citrine_eclipse_mirage::handlers::Attributes game;
    license::Handlers core({auth,online,fake_auth,license_value,expected,exe_hash,content_id,
        dll,dll,base,game,attempts,blocked},binding);
    reader::Backend backend;auto request=backend.parse_c_string("<RequestLicense/>");
    check(!request.has_error,"actual XML request parse");
    NoAttributes attributes;
    xml::MemoryPrinter printer;xml::HandlerWriter writer(printer,attributes);
    check(core.request_license_0320c0(*request.document->first_element(),writer),"license body with actual file/hash/token path");
    check(printer.bytes_without_terminator()=="<RequestLicenseResponse License=\"fixture-A&lt;&amp;\"/>\n","loaded token serialized/escaped");
    check(attempts==1&&blocked==0&&license_value=="fixture-A<&","first file state retained");
    xml::MemoryPrinter invalidated;xml::HandlerWriter invalid_writer(invalidated,attributes);
    check(core.invalidate_license_030c50(*request.document->first_element(),invalid_writer),"invalidate first token");
    xml::MemoryPrinter second;xml::HandlerWriter second_writer(second,attributes);
    check(core.request_license_0320c0(*request.document->first_element(),second_writer),"actual cached file fallback after invalidation");
    check(attempts==2&&license_value=="fixture-B"&&blocked==0,"cached state and token");
    check(core.invalidate_license_030c50(*request.document->first_element(),invalid_writer),"invalidate cached token");
    check(!core.request_license_0320c0(*request.document->first_element(),second_writer),"no reset/reload after exhausted attempts and missing ticket");
    check(attempts==2&&blocked==1&&license_value.empty(),"exhausted state persists");
    bool caught=false;try{(void)core.request_license_0320c0(*request.document->first_element(),second_writer);}catch(const NoResponse&){caught=true;}
    check(caught,"subsequent blocked call no response");
    const auto ticket=(root/"synthetic-ticket.txt").u16string();
    const std::string ticket_bytes("test\0ticket",11);
    check(binding.write_ticket_03c1d0(ticket,ticket_bytes),"actual ticket writer binding");
    check(file::read_binary_03b3f0(ticket,io)==ticket_bytes,"ticket stored length including NUL");
    bool unsupported=false;try{(void)text.tolower_05fca0(-128);}catch(const std::invalid_argument&){unsupported=true;}
    check(unsupported,"native text adapter marks negative C-API domain explicitly");
}
}
int main(int argc,char** argv){try{if(argc!=2)throw std::invalid_argument("Pass file-io-fixtures directory");run(argv[1]);
    std::cout<<"PASS: "<<checks<<" file/license integration checks; actual IO/SHA1/CBC/XML on generated fixtures, remaining effects controlled; original targets untouched\n";
    return 0;}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
