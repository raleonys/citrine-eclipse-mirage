// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/runtime/runtime_paths.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
namespace preloader_l {
class PersistenceRecoveryRequired:public std::runtime_error{public:using std::runtime_error::runtime_error;};
struct BatchHooks {virtual ~BatchHooks()=default;virtual void before_commit(std::size_t)=0;};
struct FileUpdate {std::filesystem::path target;std::string bytes;};
// Opens an exclusive sibling stage without publishing/truncating the target.
// A stream backend can report open failure before accepting any payload.
class AtomicFileWriter final {
public:
    AtomicFileWriter(const std::filesystem::path&,const RuntimePaths* owner);
    ~AtomicFileWriter();
    AtomicFileWriter(const AtomicFileWriter&)=delete;
    AtomicFileWriter& operator=(const AtomicFileWriter&)=delete;
    void write(std::string_view);
    void commit();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Sibling exclusive temporaries, flushed before replace; originals are backed up
// before a batch commit. Errors are exceptions, never successful short writes.
void atomic_write(const std::filesystem::path&,std::string_view,const RuntimePaths* owner,bool create_only=false);
void atomic_batch(const std::vector<FileUpdate>&,const RuntimePaths&,BatchHooks* hooks=nullptr);
}
