# Matched registry SDK nesting test source

Only `outputs/anadius-foreign-callback-source/foreign_integration_tests.cpp` changed. Exact previous bytes are retained in `foreign_integration_tests.cpp.before.txt`. No production edit, compiler, project test, link, native session, reconstructed DLL execution or packaging was performed by this subagent.

The added normal-main scenario `registry_matched_sdk_paths()` reuses the existing fixture, actual native_context::Owner, concrete Policy, Publication and Domain scopes. It exercises Owner's new SDK-admission binding with three controlled original callbacks and real typed rebuilt entries. LanguageKey/LanguageValue are selected to avoid the ClientPath worker-wait route. An initial language handle is supplied in the explicit pre-publication seed; no post-publication external set mutation is used.

1. Matched OpenW recursively calls matched OpenW with the exact same caller PHKEY cell. The parent SDK writes0xb01, the child overwrites it with0xb02, child source tracking occurs, and the parent's post-SDK body must reread and track0xb02.0xb01 must remain absent. This adds positive typed OpenW nesting coverage missing in the preceding stage.
2. A query of a tracked language handle calls nested real-close from its synthetic original. The close body removes membership after original success. The outer query must observe that removal and return its own original status0x80001234 and modified type/size/data without language replacement.
3. An initially untracked query invokes matched OpenW from its original. The nested open tracks the queried handle and writes caller size1. The outer body must observe newly added membership, restore its pre-original capacity16, and return replacement `en_US` with size6/type1. Treating nested size1 as capacity would instead fail.

The probe observes expected Context lease counts and one shared root PTD at nested SDK depth. Each root call runs on one actual joined foreign thread and retires its acquired PTD before the next root call. Existing typed close removes created logical handles; existing Original provider/Context close is used. The cases do not cover fabricated allocation, original set-node/CRT ABI, arbitrary concurrent mutation, actual Win32 registry state, or real Windows-induced reentry. No outgoing LastError assertion is made after a general body with encoding/CRT effects.

This is uncompiled/unexecuted test source and manual static review, not evidence that executable tests pass. Root is responsible for independent review and current source hash registration.
