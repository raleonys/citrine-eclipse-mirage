// SPDX-License-Identifier: GPL-3.0-only
#include "src/files/atomic_io.hpp"
#include <atomic>
#include <algorithm>
#include <exception>
#include <fstream>
#include <mutex>
#include <memory>
#include <optional>
#include <stdexcept>
#include <system_error>
#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#endif
namespace preloader_l {
namespace fs=std::filesystem;
namespace {
std::atomic<unsigned long long> sequence{0};std::recursive_mutex write_mutex;
[[noreturn]] void fail(const char* text){
#ifdef _WIN32
 throw std::system_error(static_cast<int>(GetLastError()),std::system_category(),text);
#else
 throw std::system_error(errno,std::generic_category(),text);
#endif
}
struct Stage {
 fs::path path;bool owned=false;
#ifdef _WIN32
 HANDLE handle=INVALID_HANDLE_VALUE;
#else
 int descriptor=-1;
#endif
 explicit Stage(const fs::path& target){
  fs::create_directories(target.parent_path());
#ifdef _WIN32
  const auto pid=GetCurrentProcessId();
#else
  const auto pid=getpid();
#endif
  for(unsigned attempt=0;attempt<100;++attempt){
   path=target.parent_path()/(".preloader_l-"+std::to_string(pid)+"-"+std::to_string(sequence.fetch_add(1))+".tmp");
#ifdef _WIN32
   handle=CreateFileW(path.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY,nullptr);
   if(handle==INVALID_HANDLE_VALUE){if(GetLastError()==ERROR_FILE_EXISTS)continue;fail("create atomic temporary");}
   owned=true;
#else
   descriptor=::open(path.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC|O_NOFOLLOW,0600);
   if(descriptor<0){if(errno==EEXIST)continue;fail("create atomic temporary");}owned=true;
#endif
   return;
  }
  throw std::system_error(std::make_error_code(std::errc::file_exists),"atomic temporary names exhausted");
 }
 Stage(const fs::path& target,std::string_view bytes):Stage(target){
  try{write(bytes);finish();}catch(...){close_noexcept();cleanup();throw;}
 }
 void write(std::string_view bytes){
#ifdef _WIN32
  std::size_t n=0;while(n<bytes.size()){DWORD got=0;const auto want=static_cast<DWORD>((std::min)(bytes.size()-n,std::size_t(0x7fffffff)));if(!WriteFile(handle,bytes.data()+n,want,&got,nullptr)||got==0)fail("write atomic temporary");n+=got;}
#else
  std::size_t n=0;while(n<bytes.size()){auto got=::write(descriptor,bytes.data()+n,bytes.size()-n);if(got<0&&errno==EINTR)continue;if(got<=0)fail("write atomic temporary");n+=static_cast<std::size_t>(got);}
#endif
 }
 void finish(){
#ifdef _WIN32
  if(!FlushFileBuffers(handle))fail("flush atomic temporary");
  if(!CloseHandle(handle)){handle=INVALID_HANDLE_VALUE;fail("close atomic temporary");}handle=INVALID_HANDLE_VALUE;
#else
  if(::fsync(descriptor))fail("flush atomic temporary");const int old=descriptor;descriptor=-1;if(::close(old))fail("close atomic temporary");
#endif
 }
 void close_noexcept()noexcept{
#ifdef _WIN32
  if(handle!=INVALID_HANDLE_VALUE){CloseHandle(handle);handle=INVALID_HANDLE_VALUE;}
#else
  if(descriptor>=0){::close(descriptor);descriptor=-1;}
#endif
 }
 void cleanup() noexcept{if(owned){std::error_code ec;fs::remove(path,ec);owned=false;}}
 ~Stage(){close_noexcept();cleanup();}
 Stage(const Stage&)=delete;
 void commit(const fs::path& target,bool create_only=false){
#ifdef _WIN32
  if(create_only){if(!MoveFileExW(path.c_str(),target.c_str(),MOVEFILE_WRITE_THROUGH))fail("publish new file");}
  else {std::error_code ec;if(fs::exists(target,ec)){if(!ReplaceFileW(target.c_str(),path.c_str(),nullptr,0,nullptr,nullptr))fail("replace file");}else if(!MoveFileExW(path.c_str(),target.c_str(),MOVEFILE_WRITE_THROUGH))fail("publish file");}
#else
  if(create_only){if(::link(path.c_str(),target.c_str()))fail("publish new file");if(::unlink(path.c_str()))fail("unlink published temporary");}
  else if(::rename(path.c_str(),target.c_str()))fail("replace file");
#endif
  owned=false;
 }
};
void validate_target(const fs::path& p,const RuntimePaths* owner){if(owner)owner->require_write(p);std::error_code ec;auto st=fs::symlink_status(p,ec);if(fs::exists(st)&&(!fs::is_regular_file(st)||fs::is_symlink(st)))throw std::invalid_argument("atomic target is not a regular owned file");}
}
struct AtomicFileWriter::Impl {
 fs::path target;const RuntimePaths* owner;std::optional<Stage> stage;bool failed=false,committed=false;
 Impl(const fs::path& p,const RuntimePaths* o):target(p.lexically_normal()),owner(o){
  std::lock_guard lock(write_mutex);validate_target(target,owner);stage.emplace(target);
 }
};
AtomicFileWriter::AtomicFileWriter(const fs::path& p,const RuntimePaths* owner):impl_(std::make_unique<Impl>(p,owner)){}
AtomicFileWriter::~AtomicFileWriter()=default;
void AtomicFileWriter::write(std::string_view bytes){
 if(impl_->failed||impl_->committed)throw std::logic_error("atomic stream is no longer writable");
 try{impl_->stage->write(bytes);}catch(...){impl_->failed=true;throw;}
}
void AtomicFileWriter::commit(){
 if(impl_->failed||impl_->committed)throw std::logic_error("atomic stream cannot commit");
 std::lock_guard lock(write_mutex);
 try{impl_->stage->finish();if(impl_->owner)impl_->owner->require_write(impl_->target);impl_->stage->commit(impl_->target);impl_->committed=true;}
 catch(...){impl_->failed=true;throw;}
}
void atomic_write(const fs::path& p,std::string_view bytes,const RuntimePaths* owner,bool create_only){std::lock_guard lock(write_mutex);auto target=p.lexically_normal();validate_target(target,owner);Stage stage(target,bytes);if(owner)owner->require_write(target);stage.commit(target,create_only);}
void atomic_batch(const std::vector<FileUpdate>& writes,const RuntimePaths& owner,BatchHooks* hooks){
 std::lock_guard lock(write_mutex);
 struct Entry {fs::path target;std::unique_ptr<Stage> fresh,backup;bool existed=false,committed=false;};std::vector<Entry> entries;entries.reserve(writes.size());
 for(const auto& w:writes){auto target=w.target.lexically_normal();validate_target(target,&owner);for(const auto& e:entries)if(e.target==target)throw std::invalid_argument("duplicate atomic target");Entry e;e.target=target;e.fresh=std::make_unique<Stage>(target,w.bytes);e.existed=fs::exists(target);if(e.existed){std::ifstream in(target,std::ios::binary);in.exceptions(std::ios::badbit|std::ios::failbit);const auto size=fs::file_size(target);std::string old(static_cast<std::size_t>(size),'\0');if(size)in.read(old.data(),static_cast<std::streamsize>(size));e.backup=std::make_unique<Stage>(target,old);}entries.push_back(std::move(e));}
 try{std::size_t index=0;for(auto& e:entries){if(hooks)hooks->before_commit(index++);owner.require_write(e.target);e.fresh->commit(e.target);e.committed=true;}}
 catch(...){auto original=std::current_exception();bool restored=true;for(auto i=entries.rbegin();i!=entries.rend();++i)if(i->committed){try{if(i->existed)i->backup->commit(i->target);else{std::error_code ec;fs::remove(i->target,ec);if(ec)restored=false;}}catch(...){restored=false;}}if(!restored){for(auto& e:entries){if(e.backup)e.backup->owned=false;}throw PersistenceRecoveryRequired("persistence rollback incomplete; stop application, retained backups");}std::rethrow_exception(original);}
}
}
