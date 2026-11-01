// SPDX-License-Identifier: GPL-3.0-only
#include "src/application/environment.hpp"
#include <chrono>
#if PRELOADER_L_DISK_DIAGNOSTICS
#include <filesystem>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#endif
namespace preloader_l {
const char* log_event_name(LogEvent e) noexcept {switch(e){
#define EVENT(n) case LogEvent::n:return #n;
EVENT(startup) EVENT(stopped) EVENT(ready) EVENT(failure) EVENT(phase) EVENT(hook) EVENT(config_base_present) EVENT(config_base_absent) EVENT(config_override_present) EVENT(config_override_absent) EVENT(config_unsupported) EVENT(profile_local) EVENT(profile_online) EVENT(profile_request) EVENT(network_receive) EVENT(network_send) EVENT(network_event) EVENT(network_parse) EVENT(network_crypto) EVENT(network_failure) EVENT(listener_started) EVENT(listener_stopped) EVENT(dispatch_failed) EVENT(accept_rearm_failed) EVENT(provider) EVENT(license) EVENT(avatar) EVENT(unknown_field) EVENT(startup_failed) EVENT(shutdown_failed)
#undef EVENT
}return "event";}
void Environment::log(LogEvent event) noexcept {
#if PRELOADER_L_DISK_DIAGNOSTICS
 try {
  std::lock_guard lock(log_mutex);
  const auto path=paths.diagnostic_log();paths.require_write(path);
  const auto stamp=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  const auto line="utc_ms="+std::to_string(stamp)+" event="+log_event_name(event)+"\n";
  std::filesystem::create_directories(path.parent_path());
#ifdef _WIN32
  struct File {HANDLE h;~File(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);}} file{CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr)};
  if(file.h==INVALID_HANDLE_VALUE)return;
  OVERLAPPED range{};
  if(!LockFileEx(file.h,LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,0,MAXDWORD,MAXDWORD,&range))return;
  struct Lock {HANDLE h;OVERLAPPED* range;~Lock(){UnlockFileEx(h,0,MAXDWORD,MAXDWORD,range);}} held{file.h,&range};
  LARGE_INTEGER end{};if(!SetFilePointerEx(file.h,end,nullptr,FILE_END))return;
  DWORD written=0;if(!WriteFile(file.h,line.data(),static_cast<DWORD>(line.size()),&written,nullptr)||written!=line.size())return;
  FlushFileBuffers(file.h);
#else
  struct File {int fd;~File(){if(fd>=0)::close(fd);}} file{::open(path.c_str(),O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC|O_NOFOLLOW,0600)};
  if(file.fd<0||::flock(file.fd,LOCK_EX|LOCK_NB))return;
  (void)::write(file.fd,line.data(),line.size()); // best-effort diagnostics; no sensitive payload
#endif
 }catch(...){}

#else
 (void)event;
#endif
}
}
