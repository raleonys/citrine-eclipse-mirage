#pragma once
#include "../anadius-online-source/online.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <cstddef>
#include "../anadius-online-object-source/object.hpp"
#include "../anadius-online-query-source/query.hpp"

namespace anadius::online_version {
using Module=online_recovered::Module;
using Procedure=online_recovered::Procedure;
using Object=online_recovered::Object;
// Selected observed Win64 call shape, not a recovered complete foreign header.
using VersionFunction=std::int32_t(*)();
using CodeLease=std::function<std::shared_ptr<void>(Module)>;
enum class QueryPolicy {Serialized,Concurrent};
struct Snapshot {
    Module module;Procedure procedure;bool active,poisoned,code_retained;
    Procedure factory;Object object;bool object_unknown,factory_code_retained;
};
// Explicit opt-in for one OnlinePlatform assembly. Lease must actually keep the
// resolved code/module alive; a nonnull arbitrary token is not provenance proof.
// No implicit raw-module FreeLibrary, object destroy, DLL load or blind retry.
// Managed load/resolve/call/free reject overlap/reentry BEFORE lower effects;
// metadata snapshots are allowed during calls. No mutex spans lower/lease/SDK.
// Concurrent permits same-object queries to overlap; lifecycle/export changes
// still require zero queries. The immutable default preserves HOST serial use.
class Binding final {
public:
    explicit Binding(CodeLease,QueryPolicy=QueryPolicy::Serialized);
    ~Binding() noexcept;
    Binding(const Binding&)=delete;Binding& operator=(const Binding&)=delete;
    Module load(const std::function<Module()>&);
    Procedure resolve(Module,const std::function<Procedure()>&);
    std::int32_t call(Procedure);
    Procedure resolve_factory(Module,const std::function<Procedure()>&);
    Object create(Procedure,const char16_t*);
    void destroy(Object);
    std::int32_t status(Object);
    const char* getter(Object,online_query::Slot); // borrowed; caller serializes use
    std::string copy_getter(Object,online_query::Slot); // reservation spans copy
    // Selected0314f0/02e3b0 Win64 callers. Args must be valid NUL-terminated
    // UTF16 path / narrow client ID; foreign target and object remain leased.
    bool avatar(Object,const char16_t*,const char*,std::int32_t);
    const char* auth(Object,const char*); // borrowed; serialize until copied
    std::string copy_auth(Object,const char*); // NUL scan/allocation reserved
    // B26 compatibility path retains four lower query providers. Shared gate
    // only guards exact adopted-object lifetime during each synchronous call.
    // Borrowed strings require caller serialization until their consumer copies.
    template<class F>auto object_call(Object object,F&& lower)->decltype(lower()) {
        begin_object(object);decltype(lower()) result;
        try{result=lower();}catch(...){end_object();throw;}
        end_object();return result;
    }
    bool free(Module,const std::function<bool()>&);
    Snapshot snapshot()const;
private:
    struct QueryUse;
    CodeLease lease_;
    const QueryPolicy query_policy_;
    mutable std::mutex gate_;
    Module module_=0;Procedure procedure_=0;
    std::shared_ptr<void> code_;
    Procedure factory_=0;Object object_=0;
    std::shared_ptr<void> factory_code_;
    bool object_unknown_=false;
    bool active_=false,poisoned_=false;
    std::size_t queries_=0;
    void begin_object(Object);
    void end_object()noexcept;
};
// Normal lookup (including0) retires old binding before lower lookup. Lease
// bookkeeping preserves lookup LastError even on lease failure. Resolve failure
// leaves loaded module + no callable binding. Synchronous version throw retains
// binding and exception/error; no automatic retry. Unknown load/free throw
// poisons future managed operations. Free retires procedure before lower; normal
// TRUE/FALSE clears tracked module and releases lease, matching core cleanup's
// module=0 prefix on either normal return. False is not labeled unload success.
// B26 factory/object lifecycle shares this SAME reservation. Live/unknown object
// blocks rebind/free and makes last-owner destruction fail-stop (no auto destroy).
// Factory/destroy unknown outcome poisons and retains all object/module/code.
} // namespace anadius::online_version
