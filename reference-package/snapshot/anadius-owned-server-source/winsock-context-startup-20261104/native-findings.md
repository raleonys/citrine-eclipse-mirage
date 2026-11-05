# Native context/Winsock startup ownership

**Confirmed behavioral differences:** native shared ownership/cached startup status, requested WORD0x0002, context-gate ordering and last-owner cleanup differ from the current per-Core WORD0x0202 scope. This is broader than diagnostic wording. No production fix was made.

Fresh static REA6.8.0/Ghidra12.1.4/Java25.0.5; configured server/package/catalog aligned. Original anadius64.dll SHA256 at open/after close:9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. Exported19 records/103106 bytes, successful close; post-session open=false/cleanup_pending=false. No build/test/link, execution, annotation, configuration change, ZIP/version.dll or original writes. No024990/028240/020950 body was re-queried. Exact EIDs/full-file hashes are in native-evidence-index.json.

## Observed native sequence

- **0241b0**,415 bytes: allocates0x38 at0241c8/0241cd and initializes that registry gate through027be0 at0241e5. It captures028620/status and nonzero→mutex/028240 before WSA acquisition. Then stores the registry pointer and calls023f30(context+8,check=1) at02421b. Scheduler allocation0xb0 and024990 follow at024226/024245. This additional registry gate is not the scheduler gate or socket-service gate.
- **023f30**,163 bytes:023f5a LOCK-XADDs+1 to module countA3900; only resulting count1 calls WSAStartup at023f71 with ECX2.023f77 XCHGs its result into cached statusA3904. If check flag is nonzero,023f84 reads status atomically via XADD0. Cached nonzero→028620 then exact84500 literal winsock→028240 at023fce. No in-body decrement, retry, startup-completion wait/spin or WSADATA-version validation appears. Native EH cleanup is unknown.
- **001310**,115 bytes: registers07df30 via04b770 at001330, then performs the same shared increment/first-owner WORD2 startup/cache exchange. It never checks cached failure. Xrefs include7f668; fresh table bytes contain001310 there. Registration/table membership are observed; complete CRT initializer-dispatch timing was not audited. Its separateA38C0 sentinel write is outside this WSA question.
- **029ea0**,346 bytes: successful run destroys both socket objects, then029f42 LOCK-XADDs-1; only oldcount1 calls WSACleanup at029f4f, with return ignored. This precedes registry service shutdown/deletion and registry-gate destruction. Nonzero run error instead raises028210 before that normal cleanup.
- Registered **07df30** has no Ghidra FunctionManager procedure. Rather than invent one, its32 raw bytes and six decoded instructions were retained: MOV-1; LOCK-XADD count; compare old1; nonlast→RET; last→tail-JMP WSACleanup IAT7f530. No annotation or synthetic function extent is claimed.

## Current source comparison and bounded conclusion

Reviewed owned-server/server.cpp SHA7aa4b341b8ef7e96e62833f320f5d30ad43386aa869155a4275e28a7b5bd2343 and server.hpp SHAb847284e3498e43e34213ec84125197111bd27aff3d3aa9ffcaf16db540e98f1; both stayed unchanged.

initialize_context calls WSAStartup(MAKEWORD(2,2)) for every Core **before its scheduler/service gates**, throws compiler-system/WSAStartup immediately on failure, and records only successful per-Core ownership. No corresponding sharedA3900/A3904 or001310 provider was found by the focused production symbol/API search. destroy_context closes IOCP/deletes gates **before** each successful Core's cleanup, checks cleanup failure and throws; native last-owner cleanup precedes service teardown and ignores that result.

A coherent next design must separate shared module owner/acquire/check/release from per-context ownership and restore the measured registry-gate/acquire/scheduler order. Merely changing the label or WORD is insufficient. Within0241b0, gate precedes acquire; an actual SDK startup may already have occurred through001310 if its owner ran first. Do not infer whole-module chronological startup solely from the context body, invent waits/retries, assume failed-constructor rollback/leaks, or add unloading proofs. Module initializer integration, original synchronous attach/compiler-CRT detach and exceptional prefixes require explicit source design. Native C++/CRT/EH ABI and runtime equivalence remain unproved.
