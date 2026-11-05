# Independent static review: explicit environment projection seams

Status: no blocking finding in the frozen seven-file seam change below. This report covers five production files and two source-test files. The separately authored Environment core and its leaf tests require their own exact-byte review. No compilation, project test, link, DLL execution, or ZIP operation was performed.

| File under outputs/ | SHA-256 |
| --- | --- |
| anadius-socket-source/io_completion.hpp | 94d68e926e43e34d74f227c1e1624312013a914d78bb38ef48de89d4be575015 |
| anadius-socket-source/io_completion.cpp | 581322e4f59b87e151ed05187bf7a4ba904254a09167ae4315625ad7b7ce6b52 |
| anadius-socket-source/environment_constructor.hpp | a0d90a4b9316dfcf0816f9c2408cd2e1dd0555fafbae9f6cdf2839b187954d30 |
| anadius-socket-source/environment_constructor.cpp | 170351638c2b03953fa5de77ecdee6d76f77b7dd9262b2b5dbad1a8bf1fd662c |
| anadius-session-profile-source/profile.cpp | 50cac048aca16fcdab4c49d862ba86587c1f63dd771dfd22123023cdf82c5108 |
| anadius-socket-source/io_completion_tests.cpp | 7edbb145ca7239421077dcdc76f5083a10f59859991d1fe5b37175ccc60162b4 |
| anadius-socket-source/environment_constructor_tests.cpp | 410d05e4ad7eaf3fbdcb2953fd4f2e0fb15b83421812fafe29d5e21ff10bf75b |

## Native ordering

Saved Ghidra 12.1.4 evidence binds original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`:

- Complete 106-byte move `180023cb0..180023d19`, Evidence `ev_cd85a47c1ecc95b2c78a406e8478fe137fdc65a60d22d94b04b2f4dfa8d0ec65`: destination ops/schedule copied; source ops/schedule reset at `180023cd7/180023ce2`; slot invoked at `180023cee`; source state reset at `180023cf1`; source executor read live at `180023cf9`, copied at `180023cfd`, then source sentinel stored at `180023d08`.
- Complete 352-byte constructor `180023dd0..180023f2f`, Evidence `ev_a796bdbc87106dd39b97f8933a91e478c3113ea2ac400041f0e295bcba20e562`: type query and comparator precede branch; nonempty branch caches the factory58 function at `180023e6b` before query18 at `180023e7a`, invokes that cached function at `180023e87`, performs the first inlined move and guarded cleanup, then helper move at `180023efa` and unconditional intermediate destroy at `180023f0b`.

The optional seams retain those existing typed algorithm effects and callback order. These are HOST source projections, not newly recovered native vtable slots or ABI claims.

## Move seam

The optional `IoHandlerVtable::try_move_environment_023cb0` is attempted on a live source table before any helper prefix write. An owning Environment can therefore validate and reserve the full source and destination rather than discovering an unauthorized source only after its header was reset. A handled hook must run the same helper using a private already-admitted `IoEnvironmentMoveSlot`; that optional fourth argument dispatches only the actual slot and bypasses whole-hook recursion. The helper still owns header writes, source state reset, live executor reload, and final sentinel write.

The final source guards the whole-hook attempt with nonnull source operations. This preserves the existing unsupported null-table path's preceding assignments and later failure instead of introducing an earlier throw. Independent draft review checked that distinction. A default-false hook leaves the existing valid projection algorithm selected.

Authored move tests observe untouched full source/destination at owning-hook entry, one admitted slot without recursive whole dispatch, callback-controlled payload/state and live executor mutation, refusal before any prefix, and an admitted slot throw retaining only preceding native writes. The prior null-table and callback-throw prefix assertions remain. These routing assertions do not manufacture a Parent or real Scope permission.

## Constructor and Profile seams

`IoEnvironmentConstructorContext` gains a trailing optional `calls=nullptr`. Its five-operation interface routes comparison, query18, destroy00, cached factory58 and whole move. Existing null selection retains the previous direct providers. A caller makes a private context copy; the shared constructor is not temporarily rebound.

The raw slot00 type-query function still runs before comparison, as the existing native algorithm requires. It is therefore the owning Scope's whole constructor entry that must claim source and destination before calling the helper. Owned Environment's type-query thunks are pure policy tokens; arbitrary foreign constructor/context authority is not established by this seam alone. The core review must verify its canonical source/context checks and nested temporary claims.

The factory function is selected and cached before query18, then passed by value into the explicit factory route. No executor-this argument or reread of a potentially deleted executor is introduced. Both constructor moves use the whole-operation route, with the actual helper retaining prefix/tail ordering. Conditional temporary destroy and unconditional final intermediate destroy retain their original distinction.

The explicit constructor test uses a private context and a bounded forwarding provider. It verifies all five selected routes, unchanged shared calls pointer, exact callback order, cached factory survival despite executor deletion and factory-registry mutation during query, empty-type sentinel move without invented factory/work/query/destroy, and unknown-query stop without fabricated cleanup. This is a source routing test, not a real child Scope or concurrent Runtime fixture.

`Application::JobHandoff::clone_environment()` now calls `scope.construct_023dd0()` and explicit scope query/destroy instead of raw global constructor/vtable callbacks on the child. It retains the actual JobMemory Operation, copied permit, captured Profile child checkpoint, protected session environment and final scope close/checks. This is the intended explicit seam; successful core implementation, compilation and actual Runtime invocation remain separate questions. No normal NativeJobs response-tail reordering is claimed.

## Review boundaries

These seven files expose no ambient current-job, actor-TLS selection, shared constructor rebinding or new business root. They do not themselves prove stable Environment row claims, shared protected-source readers, same-scope nested-constructor authority, unknown prefix retention, callback-target retirement, absence of core ownership cycles, or a two-Scope tracked-family Runtime schedule. Those are core/integration obligations. All tests described above are authored and statically inspected, not run. Production Concurrent selection, native exception/ABI equivalence, full original child response/021770 tail and overall DLL equivalence remain open.
