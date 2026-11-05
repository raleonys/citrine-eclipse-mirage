# IOCP constructor error diagnostics

Result: two concrete context/value gaps in the actual024990 failure projection. Native084428 is **mutex**, native084528 is **iocp**; both raised error pairs use028620's system category. Current Construction::raise028240(error,address) supplies the guessed text IOCP constructor and compiler system-category message formatting. Correcting these two exact address mappings and binding the established native category is justified without migrating unrelated HOST SourceError cases.

Fresh static REA6.8.0/Ghidra12.1.4, 2026-11-04. binary_session verified aligned server/provider before own isolated open. Original C:/Users/Skysect/Downloads/anadius64.dll SHA256 at open and after close: 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. Four full Evidence records,33259 bytes, exported and own session closed successfully. No source edits, build/test/link, DLL/target execution, packaging, network, version.dll access or original writes.

Only024990 was inspected as a function. Prior fresh028240/0248b0/00a270 construction,020950 message, native system-category comparison methods and actual compiler-provider records supplement this audit; they were not queried again.

## Exact literals

- Complete32-byte read at084428 begins 6d7574657800: mutex followed by NUL. The subsequent aligned bytes belong to other data, not this context.
- Complete24-byte read at084528 begins696f637000: iocp followed by NUL. The separate pqcs literal at84530 is not part of this string.

These are measured C strings, not semantic labels inferred from function names.

## Gate failure ordering and category

Complete024990 listing covers598 bytes across024990–024bd4 and024bd6–024be6. It initializes its scheduler fields, performs the version check and calls027be0 with section at scheduler+0x60 at024a7f. The status is cached in EBX at024a84. It then calls028620 at024a86 and publishes the cached status/category pair at stack+30/+38.

Only nonzero status (TEST EBX at024a94, JNZ024a96) branches to024bd6. That branch loads **084428**, passes that exact pair and calls028240 at024be2. Thus the gate's category lookup occurs before the status test, and the nonzero diagnostic is mutex: plus actual system-category message. There is no new GetLastError read in this branch after the returned027be0 status; this audit did not descend into027be0 or infer its internals.

On gate error, completion-port creation and later scheduler/worker initialization have not run. The normal body does not establish native EH cleanup of the initialized prefix.

## Completion-port failure ordering and category

On gate success,024acf calls CreateIoCompletionPort with invalid-file handle, null existing port, key0 and the selected concurrency hint.024ad5 stores the returned handle into scheduler+0x28 before testing it at024ad9/024adc.

If the handle is NULL,024ade captures GetLastError,024ae4 caches its DWORD in EBX,024ae6 calls028620, and024aeb/024aef publish status/category. Only nonzero cached status (024af4/024af6) reaches024bc4, which loads **084528** and calls028240 at024bd0. Its normal nonzero diagnostic is iocp: plus the same native system-category message.

Important finite boundary: NULL handle with cached error0 does NOT throw here; it falls through to the ordinary own-thread branch. A reconstruction must not invent a throw solely from NULL, re-read a later error after category effects, retry, or roll back the stored handle/fields. A nonnull handle also skips the error branch.

## Existing source and actual gap

Source reviewed read-only at server.cpp SHA9516bed4855e3d144f4617ec73e44cf6116d2912b8e757646210ccfbd6fe5300:

- socket-source/iocp_constructor.cpp:19–29 already reproduces gate/port conditions, captures and category order, and passes exact84428/84528 context identities. This algorithm is not missing.
- owned-server-source/server.cpp:171 handles both addresses with fail(error, IOCP constructor, address), discarding their distinct measured string values.
- SourceError's string-context constructor at server.cpp:19–23 preserves logical error/operation/address metadata but currently chooses compiler std::system_category for logical system errors. The actual selected compiler message provider was independently shown to differ from native020950.
- Both native raised pairs here come directly from028620. Prior fresh binding028620→A1110→message020950 establishes the proper value provider. The current module-owned system_error_category028620 already supplies that native message/name/comparison domain.

A minimal next source target is these two exact Construction address cases: preserve typed SourceError/error code/address and std::system_error catch compatibility, map84428→mutex and84528→iocp, and explicitly select the established native system category there. Do not globally reinterpret every logicalsystem SourceError input or change other uninspected operation strings. Actual raised codes are nonzero, so the compiler's zero-message skip is not entered by these caller branches.

Existing028240 constructor evidence supplies nonempty context + colon-space + supplied category.message(code). Consequently the already-mapped constructor algorithm plus correct category/literal binding is sufficient for these normal values; no native exception framework or CRT expansion is needed. How a future typed overload/import selection is exposed remains a root implementation decision, not code changed by this audit.

## Evidence and limits

| Full reply | EID |
|---|---|
| bytes-180084428.json |ev_cd8aa487db91241779ed2833d3f23b9da36ac110c477832e08c9ac54662a6f9b |
| bytes-180084528.json |ev_c168b8f48804fbe819914ef830ce6f236b4f36020dd0ce02bbebf077cadd1ec0 |
| pseudo-180024990.json |ev_f2aea1d0ed5a3f22d9fedb0e19d56676735439db29d01749919ec20b6d006e75 |
| instructions-180024990.json |ev_ad79f398b9501d4195867f29af77a3f4fac3dba478f331fa60963108725f6bcd |

Full session/open/export/close replies and post-close hash are retained. Zero recorded bundle Unknown objects do not remove native EH/allocator/exception ABI, actual OS text, runtime or module-teardown limitations. No extra unload condition or whole-goal completion claim follows.
