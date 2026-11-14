// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/core/atomic_files.hpp"
#include <array>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
namespace io=laureondo::file;
using Path=laureondo::configuration::Path;
using Files=std::map<Path,std::string>;
const std::array<Path,4> targets{u"a",u"b",u"c",u"d"};
const std::array<std::string,4> original{std::string("old-a\0kept",10),"","old-c","old-d"};
const std::array<std::string,4> replacement{"new-a",std::string("\0new-b",6),"new-c","new-d"};
unsigned checks=0,cases=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct CommitError final:std::runtime_error {CommitError():std::runtime_error("fourth replacement denied"){} };
struct RestoreError final:std::runtime_error {
    Path target;
    explicit RestoreError(const Path& path):std::runtime_error("rollback denied"),target(path){}
};
// An in-memory backend with independent replacement and rollback faults. Real
// AtomicFiles owns all staging/backup identities; no private ledger is exposed.
struct MatrixOps final:io::AtomicFileOps {
    Files files;
    std::map<Path,Path> backups;
    std::set<Path> blocked;
    std::vector<Path> restore_attempts;
    unsigned stages=0;
    bool fail_last=true;
    static bool destination(const Path& path){return path==u"a"||path==u"b"||path==u"c"||path==u"d";}
    Path stage(const Path&,std::string_view bytes)override{
        const auto number=std::to_string(++stages);
        Path path=u"stage-";path.append(number.begin(),number.end());
        files.emplace(path,bytes);return path;
    }
    bool exists(const Path& path)override{return files.contains(path);}
    Path backup(const Path& target)override{
        const auto number=std::to_string(++stages);
        Path path=u"backup-";path.append(number.begin(),number.end());
        files.emplace(path,files.at(target));backups[target]=path;return path;
    }
    void restoration(const Path& target){
        restore_attempts.push_back(target);
        if(blocked.contains(target))throw RestoreError(target);
    }
    void replace(const Path& from,const Path& to)override{
        if(from.starts_with(u"backup-"))restoration(to);
        else if(fail_last&&to==targets.back())throw CommitError{};
        files[to]=files.at(from);files.erase(from);
    }
    void remove(const Path& path)override{
        if(destination(path))restoration(path);
        files.erase(path);
    }
    Files destinations()const{
        Files result;
        for(const auto& target:targets)
            if(const auto i=files.find(target);i!=files.end())result.emplace(i->first,i->second);
        return result;
    }
};
std::vector<Path> failed_order(unsigned mask){
    std::vector<Path> result;
    for(unsigned i=3;i>0;--i)if(mask&(1u<<(i-1)))result.push_back(targets[i-1]);
    return result;
}
template<class Action>void failure(Action action,unsigned mask){
    enum class Observed {none,commit,restore};
    auto observed=Observed::none;Path target;
    try{action();}
    catch(const CommitError&){observed=Observed::commit;}
    catch(const RestoreError& error){observed=Observed::restore;target=error.target;}
    require(observed==(mask?Observed::restore:Observed::commit),
        "rollback failure remains visible; successful rollback preserves the original commit exception");
    if(mask)require(target==failed_order(mask).front(),"multiple rollback failures preserve the first reverse-order error");
}
void recovery_matrix(){
    const std::array<io::FileWrite,4> writes{{
        {targets[0],replacement[0]},{targets[1],replacement[1]},
        {targets[2],replacement[2]},{targets[3],replacement[3]}}};
    for(unsigned present=0;present<8;++present)for(unsigned blocked=0;blocked<8;++blocked){
        ++cases;MatrixOps ops;
        for(unsigned i=0;i<3;++i){
            if(present&(1u<<i))ops.files[targets[i]]=original[i];
            if(blocked&(1u<<i))ops.blocked.insert(targets[i]);
        }
        ops.files[targets[3]]=original[3];
        const auto before=ops.files;
        io::AtomicFiles owner(ops);
        failure([&]{owner.replace(writes);},blocked);
        require(ops.restore_attempts==std::vector<Path>{targets[2],targets[1],targets[0]},
            "one denied restoration cannot prevent attempts on the other changed destinations");
        auto partial=before;
        for(unsigned i=0;i<3;++i)if(blocked&(1u<<i))partial[targets[i]]=replacement[i];
        require(ops.destinations()==partial,"only denied restorations retain new bytes; all others regain exact prior bytes or absence");
        require(owner.recovery_pending()==(blocked!=0),"pending recovery tracks the destinations still awaiting restoration");
        for(unsigned i=0;i<3;++i)if((present&blocked)&(1u<<i))
            require(ops.files.at(ops.backups.at(targets[i]))==original[i],"each failed existing destination retains its exact original backup");
        if(blocked){
            const auto retained=ops.files;const auto stages=ops.stages;
            ops.restore_attempts.clear();
            failure([&]{owner.replace(writes);},blocked);
            require(ops.files==retained&&ops.stages==stages,"blocked replacement cannot overwrite recovery files or create another batch");
            require(ops.restore_attempts==failed_order(blocked),"retry visits only failed owners, without restoring completed destinations twice");
        }else require(ops.files==before,"complete rollback also removes all unused staging and backup files");
        ops.blocked.clear();ops.restore_attempts.clear();
        owner.recover();
        require(ops.restore_attempts==failed_order(blocked),"repaired recovery resumes the exact remaining rollback owners");
        require(!owner.recovery_pending()&&ops.files==before,"recovery restores the entire original group and clears every temporary file");
        ops.restore_attempts.clear();owner.recover();
        require(ops.restore_attempts.empty()&&ops.files==before,"completed recovery is inert on repeated calls");
        ops.fail_last=false;owner.replace(writes);
        Files committed;for(unsigned i=0;i<4;++i)committed.emplace(targets[i],replacement[i]);
        require(!owner.recovery_pending()&&ops.files==committed,"the same owner commits a fresh binary group after complete recovery");
    }
}
}
int main()try{
    recovery_matrix();
    std::cout<<"PASS: "<<checks<<" checks across "<<cases<<" atomic recovery cases; controlled filesystem operations\n";
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<" checks: "<<error.what()<<'\n';return 1;}
