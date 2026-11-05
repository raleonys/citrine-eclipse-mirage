# Independent Original query-admission production review

No blocking source defect found in the final five files below, compared with this stage's captured `before/` files. The confirmed eligible window and ordinary hooked3216 wait qualification are separately recorded in `source-reachability-review.md` and `fresh-reachability-addendum.md`. No source edit, build, test, linking, DLL/child execution or packaging was performed by this reviewer.

The immutable QueryPolicy defaults to Serialized and validates its enum. Only Online's existing OriginalCrt selection constructs Concurrent; Cooperative and existing one-argument Binding callers retain serialization. No readiness wait or broad foreign-provider thread-safety requirement was added. Binding's private layout changes require consumers to be rebuilt consistently; no existing compiled output validates this source.

Lifecycle-exclusive active_ and counted queries are separate. All seven lifecycle/export operations (load, resolve, version call, resolve_factory, create, destroy, free) reject nonzero queries before lower effects or retiring code/object ownership. Query admission rejects lifecycle activity, poison, unknown/foreign/null object or missing module/factory lease. Serialized additionally requires zero queries; Concurrent increments under the same metadata mutex without adding actor affinity. The destructor includes outstanding queries in its unresolved-owner check. Existing snapshot layout is unchanged and active reports lifecycle activity OR nonzero query count, preserving consumers' busy/empty predicates.

Every successful begin has one end decrement; end does not clear lifecycle activity or another query. QueryUse's constructor reserves before reading vptr/slot. Its invalid-layout catch first poisons/marks unknown, then ends only that constructor's reservation; its destructor does not run for a throwing constructor. Other admitted queries remain counted until their own guards finish, so a failed second query cannot expose the first as quiescent. Normal foreign/copy exceptions unwind the existing guard once without erasing shared poison/unknown state. New admissions and lifecycle cleanup remain denied after invalid layout.

No metadata mutex spans foreign code, vptr/slot reads, string allocation/copy or code-lease retirement. Lifecycle rejection keeps the existing exact factory-code lease and adopted object alive throughout all admitted calls. Owned getter/auth copies retain QueryUse through foreign return, first-NUL scan and actual string construction. Their existing per-thread LastError guards still capture outgoing foreign error before metadata/allocation and restore it during guard release; status/avatar value results likewise release only metadata afterward. Raw borrowed return methods retain their preexisting caller-consumption/lifetime limitation.

This corrects wrapper admission of the measured Original caller schedule. It does not reconstruct arbitrary concurrent Configuration/string publication, underlying foreign provider buffers/locks, complete native class/EH ABI or runtime scheduling. Controlled tests must synchronize initial object/online publication and hold the first username getter before return, rather than claiming all later profile updates race safely.

| Final file | SHA256 |
|---|---|
| `anadius-online-version-source/version.hpp` | `59be644b1ad458f14a35f6673fd795ff81eb3c247fbd1d9782fc3f1609aeeb4a` |
| `anadius-online-version-source/version.cpp` | `8c4bedbcac6fe06ec876948feb49f37b2ec64eadf647594deb741e29bc067457` |
| `anadius-online-object-source/object.cpp` | `8ec06369de0d4bb2d327fb59cab61ad3d4ca13be86def5e161819d28c15b2064` |
| `anadius-online-query-source/query.cpp` | `aef7a62d8a77ca6426d88b3a5a9275d67230cc573f58022003c83dc374039287` |
| `anadius-online-factory-source/factory.cpp` | `f681a819749056ecb9a29c2f20dca74a1514974b75c561a1dc45c7f8210e724f` |
