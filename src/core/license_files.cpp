#include "preloader_l/core/license_files.hpp"
#include "preloader_l/core/token.hpp"
#include <cctype>
#include <exception>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace preloader_l::file_recovered {
namespace {
class HashInput final:public hash_recovered::InputStream {
    std::unique_ptr<Input> stream_;
public:
    explicit HashInput(std::unique_ptr<Input> stream):stream_(std::move(stream)){
        if(!stream_)throw std::invalid_argument("File factory returned null input");
    }
    std::uint32_t initial_state()const override{return stream_->initial_state();}
    std::int32_t peek_043790()override{return stream_->peek_043790();}
    std::size_t read_03b2a0(std::span<std::uint8_t> bytes)override{return stream_->read_03b2a0(bytes);}
    void close_03ae70()override{stream_->close_03ae70();}
};
class HashFiles final:public hash_recovered::FileFactory {
    Factory& files_;
public:
    explicit HashFiles(Factory& files):files_(files){}
    std::unique_ptr<hash_recovered::InputStream> open_input_039f70(
            const Path& path,std::uint32_t mode,std::uint32_t share)override {
        return std::make_unique<HashInput>(files_.open_input_039f70(path,mode,share));
    }
};
class TokenFiles final:public token_recovered::Dependencies {
    license_recovered::Effects& effects_;
    Factory& files_;
public:
    TokenFiles(license_recovered::Effects& effects,Factory& files):effects_(effects),files_(files){}
    token_recovered::FileStatus status_01ea20(const Path& path,std::uint32_t flags)override {
        const auto status=effects_.status_01ea20(path,flags);return {status.kind,status.error};
    }
    [[noreturn]] void filesystem_error_018130(std::string_view op,std::int32_t error,const Path& path)override {
        effects_.filesystem_error_018130(op,error,path);std::terminate();
    }
    std::string read_binary_03b3f0(const Path& path)override{return file_recovered::read_binary_03b3f0(path,files_);}
};
}
std::string NativeTextEnvironment::format_digest_043270(const hash_recovered::DigestWords& words) {
    std::ostringstream output;
    output<<std::hex<<std::nouppercase<<std::setfill('0');
    for(const auto word:words)output<<std::setw(8)<<word;
    return output.str();
}
std::int32_t NativeTextEnvironment::tolower_05fca0(std::int32_t value) {
    if(value<-1||value>255)throw std::invalid_argument("Native C tolower input outside defined character domain");
    return std::tolower(value);
}
bool LicenseFiles::file_hash_matches_042d20(const Path& path,std::string expected) {
    HashFiles files(files_);return hash_recovered::file_hash_matches_042d20(path,std::move(expected),files,text_);
}
std::string LicenseFiles::load_token_02afa0(const Path& path) {
    TokenFiles files(*this,files_);return token_recovered::load_token_02afa0(path,files);
}
bool LicenseFiles::write_ticket_03c1d0(const Path& path,std::string_view bytes) {
    return write_binary_03c1d0(path,bytes,files_);
}
const char* LicenseFiles::online_auth_virtual_30(const char* value){return remaining_.online_auth_virtual_30(value);}
license_recovered::FileStatus LicenseFiles::status_01ea20(const Path& path,std::uint32_t flags){return remaining_.status_01ea20(path,flags);}
[[noreturn]] void LicenseFiles::filesystem_error_018130(std::string_view op,std::int32_t error,const Path& path){remaining_.filesystem_error_018130(op,error,path);std::terminate();}
Path LicenseFiles::filesystem_widen_006d70(std::string_view text){return remaining_.filesystem_widen_006d70(text);}
Path LicenseFiles::filename_00e120(const Path& path){return remaining_.filename_00e120(path);}
Path LicenseFiles::parent_path_019820(const Path& path){return remaining_.parent_path_019820(path);}
Path LicenseFiles::join_016b00(const Path& a,const Path& b){return remaining_.join_016b00(a,b);}
std::string LicenseFiles::path_utf8_01e3f0(const Path& path){return remaining_.path_utf8_01e3f0(path);}
std::string LicenseFiles::current_time_041b60_null_true(){return remaining_.current_time_041b60_null_true();}
bool LicenseFiles::wine_detected_045010(){return remaining_.wine_detected_045010();}
void LicenseFiles::log_literal_006880(std::string_view text){remaining_.log_literal_006880(text);}
void LicenseFiles::log_string_006860(std::string_view text){remaining_.log_string_006860(text);}
void LicenseFiles::dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags){remaining_.dialog_038bc0(title,text,flags);}
[[noreturn]] void LicenseFiles::exit_05cdc0(std::int32_t code){remaining_.exit_05cdc0(code);std::terminate();}
[[noreturn]] void LicenseFiles::no_response_02bcd0_04d228(){remaining_.no_response_02bcd0_04d228();std::terminate();}
} // namespace preloader_l::file_recovered
