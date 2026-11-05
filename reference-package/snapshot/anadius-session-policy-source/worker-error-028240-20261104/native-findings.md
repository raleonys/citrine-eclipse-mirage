# Worker error028240 diagnostic value comparison

Result: a concrete diagnostic-value gap, distinct from native EH ABI. The selected stop error literal at `0x180084530` is **`pqcs`**, not `post`. The native constructor assembles that nonempty context, `: `, and the supplied category's message. Current original and tracked-child Policy throw projections instead pass `post` to compiler std::system_error and use compiler category formatting.

Fresh static REA6.8.0/Ghidra12.1.4, 2026-11-04. binary_session verified aligned configured server/provider before own isolated open. Original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 at open and after successful close: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Seven complete Evidence records,45717 bytes, exported; zero recorded Unknown objects do not erase the explicit limits below.

No production/test source edits, target execution, compile/tests/link, package update, version.dll access, network, provider reconfiguration or original writes. Only the requested028240 and its two immediate diagnostic-construction functions0248b0/00a270 were inspected; no CRT/EH descent or duplicate020950 query.

## Exact selected literal

`bytes-180084530.json` returns32 complete bytes. Its prefix is `70 71 63 73 00`, the C string **pqcs**. This is a measured literal, not an inferred label for PostQueuedCompletionStatus. The existing native stop evidence and current `worker_run.cpp:11` select this address for02a190's nonzero failed-post error.

## Value/diagnostic order

1. Complete028240 listing (`028240–028269`,42 bytes) copies16 bytes of the incoming error pair at028244/028247, retains the caller's context pointer in R8, and calls0248b0 at028259. It then calls023990 at028264 and ends in INT3. This audit did not descend into the throw helper.
2. Complete0248b0 listing (`0248b0–024988`,217 bytes) initializes a local string representation, scans the incoming context C string to NUL at0248f0–0248f8, constructs that string via006b30, copies the error pair and calls00a270 at02491d. After normal return it cleans the temporary context string and writes the std::system_error vtable at02495c–024963. No decode/translation changes `pqcs` into `post`.
3.00a270 copies the supplied context string at00a2a8. It checks its length; only when nonempty,00a2e5 appends word0x203a (little-endian `: `) and a NUL, or00a30c takes the equivalent growth helper.
4.00a311–00a32f extracts the supplied category pointer from the error pair, loads its vtable and invokes **slot+0x10** with the low32 error code. The returned string is appended by pointer plus its stored count at00a34d–00a358. Category name and decimal code are not appended by this function's visible normal body.
5. After cleaning that category-message temporary,00a3b2 onward moves the combined diagnostic to another local, invokes the existing native exception text copy at00a40a, writes runtime_error/_System_error vtables, and stores the original16-byte error pair at object+0x18 at00a45c. The outer0248b0 then supplies the system_error vtable. This is finite observed value assembly, not a reconstruction of those C++ class layouts or CRT copy helper internals.

For the selected stop literal, the constructed text value is therefore `pqcs: ` plus the **supplied category.message(code)**. The empty-context constructor case would omit the separator. This audit does not infer that every other028240 caller uses pqcs.

## System-category supplement

The preceding fresh `../system-message-020950-20261104` six-record audit established028620→objectA1110→vtable84248→slot10=020950. Its body uses FormatMessageA flags1300/language400, at most one LF then CR trim, subsequent C-string length copy, fallback `asio.system error`, and normal LocalFree after result construction. Those retained records supplement this audit;020950 was not queried again.

Thus the selected native system stop error constructs `pqcs: ` followed by that020950 message value. This is stronger evidence than guessing a context from the source function name. It does not require assuming any particular current OS message text or localization.

## Actual current source comparison

Read-only compared sources at audit time:

| Source | SHA256 |
|---|---|
| outputs/anadius-session-policy-source/policy.cpp |5c42ccbff002069dc4b1bdb111f6ed59efcb634ecc01efaef7890fd55b06307f |
| outputs/anadius-owned-server-source/server.cpp |9516bed4855e3d144f4617ec73e44cf6116d2912b8e757646210ccfbd6fe5300 |
| outputs/anadius-socket-source/worker_run.cpp |75d617268edcb4b8ad610dd4509571a5420d1918e92e60b485d66d0d564fedf6 |

- worker_run.cpp:11 passes the logical error/category and opaque context84530 to raise028240 after the exact existing stop/error sequence.
- Policy::raise at policy.cpp:264 validates84530 but throws `std::system_error(e.value, compiler_category(...), "post")`.
- The separately admitted tracked child/original finish adapter's raise at policy.cpp:244–247 also validates84530 and throws with `"post"`.
- The new020950 adapter is selected by Policy's ordinary system-category message_slot10 logging path. These terminal compiler std::system_error projections do not use that adapter, so the logging correction alone does not complete the diagnostic constructor path discovered here.
- Other typed `OwnedServerBackend::SourceError` constructors at server.cpp:16–23 retain a logical source error and context address while delegating text to compiler std::system_error. Their other literal inputs/callers were not newly audited; this finding does not authorize replacing all their labels with pqcs or claiming every wrapper has this same value gap.

The literal mismatch is concrete independent of the compiler library's final what() punctuation. The use of compiler system_category.message rather than the proven supplied native category's020950 value is a second remaining value-path boundary; no current compiler what() result was executed or guessed here.

## Bounded next candidate

A justified source follow-up is the actual84530 terminal diagnostic path in both Policy raise projections: preserve the logical error/category and use the recovered context `pqcs` plus the supplied native category-message value, in the observed context→separator→message order. Keep the real body/admission/error prefix and unknown-outcome behavior; do not merely add an unused helper or change every SourceError context.

Changing only the word post would fix the literal input gap but would not by itself establish native system-message value equivalence for the thrown diagnostic. Conversely, exact MSVC exception object layout, RTTI, throw dispatch, unwinding, allocator/SSO and catch ABI remain separate reconstruction work. This report does not propose or claim a generic native exception framework, EH completion, or runtime pass.

## Retained evidence

| Full reply | EID |
|---|---|
| pseudo-180028240.json |ev_caec7170b7511491f6246f8be2c0ade711b4a9f8f832c135160483a471f02c0b |
| instructions-180028240.json |ev_ec5499f2bbbabf0cc78f8929ea229f5fa12d3aab6ba6eb1f7418806c34d66320 |
| pseudo-1800248b0.json |ev_39e04a3db4b2186c186f81478d7a5f8ccb9684791dc349a14fa470b897f368d1 |
| instructions-1800248b0.json |ev_57aaaf1a60a65732e76c3d58854c2a8a4301a1347d3a591e2b1322fab9a66303 |
| pseudo-18000a270.json |ev_c4164937f426fe4363d7acf2c3b366d8481957236da7b03b09501e1b77279f18 |
| instructions-18000a270.json |ev_de16e118a373e30ee8d8b81684c382c26798cce6f3283f4aa17584f47ca4c5d3 |
| bytes-180084530.json |ev_9e3850282dbec7d20b4d26ecef4374ea44cd967544f4cf322f655cc55e91227d |

Full session/open/export/close replies and post-close original hash are retained. No agent analysis session remains open.
