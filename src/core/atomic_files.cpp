// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/core/atomic_files.hpp"
#include <algorithm>
#include <exception>
#include <stdexcept>

namespace laureondo::file {
AtomicFiles::~AtomicFiles(){try{recover();}catch(...){/* Retain disk recovery files. */}}
bool AtomicFiles::recovery_pending() const noexcept {
    return std::any_of(entries_.begin(),entries_.end(),[](const Entry& e){return e.replaced;});
}
void AtomicFiles::discard() noexcept {
    for(auto& e:entries_)for(auto* path:{&e.staged,&e.backup}) {
        if(path->empty())continue;
        try{ops_.remove(*path);path->clear();}catch(...){}
    }
    // A failed deletion retains its exact owned path for a later retry.
    if(std::all_of(entries_.begin(),entries_.end(),[](const Entry& e){return e.staged.empty()&&e.backup.empty();}))
        entries_.clear();
}
void AtomicFiles::recover() {
    ops_.recover_staging();
    std::exception_ptr first;
    for(auto i=entries_.rbegin();i!=entries_.rend();++i) {
        if(!i->replaced)continue;
        try {
            if(i->existed){ops_.replace(i->backup,i->target);i->backup.clear();}
            else ops_.remove(i->target);
            i->replaced=false;
        } catch(...) {if(!first)first=std::current_exception();}
    }
    if(first)std::rethrow_exception(first);
    discard();
}
void AtomicFiles::replace(std::span<const FileWrite> writes) {
    recover(); // Never overwrite a previous incomplete rollback's ownership.
    if(!entries_.empty())throw std::runtime_error("Atomic file staging cleanup is incomplete");
    entries_.reserve(writes.size());
    try {
        for(const auto& write:writes) {
            if(write.path.empty())throw std::invalid_argument("Empty atomic destination");
            if(std::any_of(entries_.begin(),entries_.end(),[&](const Entry& e){return e.target==write.path;}))
                throw std::invalid_argument("Duplicate atomic destination");
            Entry entry;entry.target=write.path;
            entries_.push_back(std::move(entry));
            auto& e=entries_.back();
            e.staged=ops_.stage(e.target,write.bytes);
            e.existed=ops_.exists(e.target);
            if(e.existed)e.backup=ops_.backup(e.target);
        }
        for(auto& e:entries_) {
            ops_.replace(e.staged,e.target);
            e.staged.clear();e.replaced=true;
        }
    } catch(...) {
        const auto failure=std::current_exception();
        recover(); // Failed rollback retains all remaining recovery identities.
        std::rethrow_exception(failure);
    }
    for(auto& e:entries_)e.replaced=false; // Group committed; only garbage remains.
    discard(); // Cleanup failure cannot turn a committed update into a failure.
}
} // namespace laureondo::file
