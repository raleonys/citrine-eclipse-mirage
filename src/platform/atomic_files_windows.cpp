// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/core/binary_io.hpp"
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <climits>
#include <filesystem>
#include <mutex>
#include <system_error>
#include <stdexcept>
#include <vector>
#include <exception>

namespace laureondo::file {
namespace {
[[noreturn]] void fail(const char* operation,DWORD error=GetLastError()) {
    throw std::system_error(static_cast<int>(error),std::system_category(),operation);
}
std::wstring wide(const Path& path){return {path.begin(),path.end()};}
// Windows paths may name one file with different casing. The generic
// AtomicFiles ledger compares literal Path bytes; reject unambiguous aliases
// before staging two independent rollback entries for the same destination.
// This is not a claim to resolve every hardlink, reparse or 8.3 alias.
void validate_windows_batch_targets(std::span<const FileWrite> writes) {
    for(std::size_t i=0;i<writes.size();++i) {
        const auto& target=writes[i].path;
        if(target.empty())throw std::invalid_argument("Empty atomic destination");
        if(target.find(u'\0')!=Path::npos)
            throw std::invalid_argument("Atomic destination contains an embedded NUL");
        if(target.size()>INT_MAX)throw std::length_error("Atomic destination exceeds Windows comparison length");
        const auto current=wide(target);
        for(std::size_t j=0;j<i;++j) {
            const auto earlier=wide(writes[j].path);
            const auto result=CompareStringOrdinal(earlier.c_str(),static_cast<int>(earlier.size()),
                current.c_str(),static_cast<int>(current.size()),TRUE);
            if(result==0)fail("compare Windows atomic destinations");
            if(result==CSTR_EQUAL)throw std::invalid_argument("Duplicate Windows atomic destination");
        }
    }
}
Path sibling(const Path& target) {
    static std::atomic<unsigned long long> next{0};
    const auto suffix=".surge-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(++next)+".tmp";
    auto result=target;result.append(suffix.begin(),suffix.end());return result;
}
class NativeOps final:public AtomicFileOps {
    struct Pending {Path path;HANDLE file=INVALID_HANDLE_VALUE;bool owned=false;};
    std::vector<Pending> pending_;
    Pending& reserve(const Path& target) {
        pending_.emplace_back();auto& entry=pending_.back();
        try {
            for(unsigned attempt=0;attempt<128;++attempt) {
                entry.path=sibling(target);
                entry.file=CreateFileW(wide(entry.path).c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
                if(entry.file!=INVALID_HANDLE_VALUE){entry.owned=true;return entry;}
                const auto error=GetLastError();
                if(error!=ERROR_FILE_EXISTS&&error!=ERROR_ALREADY_EXISTS)fail("create atomic staging file",error);
            }
            fail("atomic staging name exhausted",ERROR_FILE_EXISTS);
        }catch(...){pending_.pop_back();throw;}
    }
    void close(Pending& entry) {
        if(entry.file==INVALID_HANDLE_VALUE)return;
        if(!CloseHandle(entry.file))fail("close atomic staging file");
        entry.file=INVALID_HANDLE_VALUE;
    }
    void remove_pending(const Path& owned) {
        const auto path=wide(owned);
        if(DeleteFileW(path.c_str()))return;
        auto error=GetLastError();
        if(error==ERROR_ACCESS_DENIED) {
            const auto attributes=GetFileAttributesW(path.c_str());
            if(attributes==INVALID_FILE_ATTRIBUTES)error=GetLastError();
            else if(attributes&FILE_ATTRIBUTE_READONLY) {
                // CopyFile can copy readonly onto an owned backup before its
                // return-path allocation fails. Only cleanup changes this flag.
                const auto writable=attributes&~FILE_ATTRIBUTE_READONLY;
                if(!SetFileAttributesW(path.c_str(),writable?writable:FILE_ATTRIBUTE_NORMAL))
                    fail("clear failed atomic staging readonly attribute");
                if(DeleteFileW(path.c_str()))return;
                error=GetLastError();
            }
        }
        if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)
            fail("remove failed atomic staging file",error);
    }
    Path release(Pending& entry) {
        Path result=entry.path; // Allocate the return value while still owned.
        pending_.pop_back();return result;
    }
    void cleanup_after_failure() noexcept {try{recover_staging();}catch(...) {}}
public:
    ~NativeOps(){
        cleanup_after_failure();
        for(const auto& entry:pending_)if(entry.file!=INVALID_HANDLE_VALUE)std::terminate();
    }
    void recover_staging() override {
        std::exception_ptr first;
        for(auto& entry:pending_) {
            try {
                close(entry);
                if(entry.owned)remove_pending(entry.path);
                entry.owned=false;
            }catch(...){if(!first)first=std::current_exception();}
        }
        std::erase_if(pending_,[](const Pending& p){return !p.owned&&p.file==INVALID_HANDLE_VALUE;});
        if(first)std::rethrow_exception(first);
    }
    Path stage(const Path& target,std::string_view bytes) override {
        auto& entry=reserve(target);
        try {
            std::size_t offset=0;
            while(offset<bytes.size()) {
                const auto requested=static_cast<DWORD>(std::min(bytes.size()-offset,std::size_t{65536}));
                DWORD written=0;
                if(!WriteFile(entry.file,bytes.data()+offset,requested,&written,nullptr))fail("write atomic staging file");
                if(written==0||written>requested)fail("short atomic staging write",ERROR_WRITE_FAULT);
                offset+=written;
            }
            if(!FlushFileBuffers(entry.file))fail("flush atomic staging file");
            close(entry);
            return release(entry);
        }catch(...) {
            cleanup_after_failure();
            throw;
        }
    }
    bool exists(const Path& path) override {
        std::error_code error;
        const auto status=std::filesystem::status(std::filesystem::path(path),error);
        if(error==std::errc::no_such_file_or_directory)return false;
        if(error)throw std::system_error(error,"atomic destination status");
        if(std::filesystem::exists(status)&&!std::filesystem::is_regular_file(status))
            throw std::runtime_error("Atomic destination must be a regular file");
        return std::filesystem::exists(status);
    }
    Path backup(const Path& target) override {
        auto& entry=reserve(target); // Reserve ownership before CopyFile can fail.
        try {
            close(entry);
            if(!CopyFileW(wide(target).c_str(),wide(entry.path).c_str(),FALSE))fail("backup atomic destination");
            return release(entry);
        }catch(...){cleanup_after_failure();throw;}
    }
    void replace(const Path& source,const Path& target) override {
        if(!MoveFileExW(wide(source).c_str(),wide(target).c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
            fail("replace atomic destination");
    }
    void remove(const Path& path) override {
        std::error_code error;
        (void)std::filesystem::remove(std::filesystem::path(path),error);
        if(error)throw std::system_error(error,"remove owned atomic file");
    }
};
}
struct NativeFactory::AtomicOwner {
    std::mutex mutex;
    NativeOps ops;
    AtomicFiles files{ops};
};
NativeFactory::NativeFactory():atomic_(std::make_unique<AtomicOwner>()){}
NativeFactory::~NativeFactory()=default;
void NativeFactory::replace_atomic(std::span<const FileWrite> writes) {
    std::lock_guard lock(atomic_->mutex);
    // Preserve the original recovery-before-new-batch ordering, including
    // when the requested batch has invalid or aliased Windows names.
    atomic_->files.recover();
    validate_windows_batch_targets(writes);
    atomic_->files.replace(writes);
}
void NativeFactory::recover_atomic() {
    std::lock_guard lock(atomic_->mutex);atomic_->files.recover();
}
} // namespace laureondo::file
