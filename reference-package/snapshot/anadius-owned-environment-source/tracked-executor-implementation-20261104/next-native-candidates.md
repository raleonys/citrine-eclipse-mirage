# Next native candidates from stale presence records

Retained/source audit only. Read the149-record `outputs/dll-critical-path/current-application-map-after-source-error/candidates.json`, current selected production bodies, existing reports and retained pseudocode slices. No code/evidence/tracker changes, REA call, original-binary access, compile, tests, link, DLL execution or packaging. The historical presence labels are heuristic seed metadata, not evidence that a behavior is missing today.

## Selected next question:020950 system-category message

This is a concrete selected provider comparison, not a missing-name assertion. Current `anadius-session-policy-source/policy.cpp:18,52-63` routes system category text to `std::system_category().message(code)` unless explicit format_override is supplied. No concrete020950 adapter was found in current production source. Existing `misc-message-020a80.md:27` explicitly says generic/system compiler categories remain unchanged after the earlier concrete misc formatter correction, so this question is not already closed by that stage.

Retained020950 pseudocode (Evidence `ev_7dc0a7541a2129dbe8fd29bf0db855e7354ed5a21c04d8392048f304b57ab278`; assembly record `ev_b40dcf1bd705c1e8789cb95f6225b356173bf5382946e1e7f9ed7066404edd45`) calls FormatMessageA with flags0x1300, language0x400, allocation output, then conditionally removes final LF/CR, copies a C string, falls back to `asio.system error` and LocalFree's the returned buffer. Compiler system_category may differ in fallback, language, trimming or representation. A mismatch is not yet proven by comparing its name alone.

Actual selected callers use the virtual category message boundary: finish_write025a70 (`socket-source/write_finish.cpp:82`), finish_frame_read025480 (`frame_read.cpp:84`) and accept_session025720 (`session-flow-source/flow.cpp:75`) feed error text to the owned log stream. Original native linkage should be established through system-category getter028620/object/vtable slot+10, then020950's exact body/error prefixes. This is one bounded Win32 formatting question, not a new CRT or general exception framework.

**Proposed fresh seed:**028620 plus its selected category slot+10 linkage,020950 instructions/pseudocode and exact fallback literal; compare flags/language, LF/CR-only trimming, C-string boundary, zero-length/failure behavior and LocalFree ordering. Root has selected this focused next stage; do not query all149 records or change Policy before evidence review.

## Other bounded checks — no additional fresh seeds selected

| Record / retained caller | Current selected source and existing proof | Disposition |
| --- | --- | --- |
|020a70 name /020a80 message /0285b0 getter | `socket-source/native.cpp:7-25` implements misc exact1..4/fallback; `session-policy-source/misc-message-020a80.md` retains exact branch evidence and explains selected Category adapter. | Closed normal value behavior. System020950 remains distinct; do not redo misc. |
|028ab0, called by021b00's software error path | `anadius-accept-source/accept.cpp:12-19` copies defined Error into packet metadata, marks queued and posts/fallback-enqueues the same operation. Its existing architecture review explicitly labels the typed boundary without claiming native Error ABI. | No missing selected normal action found; absence of an exact function label does not justify another error-copy wrapper. |
|0348d0, called repeatedly by02c080 achievement element | Retained body performs signed32 decimal conversion, including negative magnitude, into a string. `content-handlers-source/content_handlers.cpp:27-28,103-110` emits the corresponding signed decimal attributes. `config-defaults-source/application-gap-audit.md` and `bootstrap-backend-source/synchronous-loader-audit.md` already exclude this helper as a new application gap. | Already represented value behavior; no fresh query. Native string allocation/CRT representation is not promoted into a business module. |
|02bcd0 /02bd30, consumed by message routing/error control | Retained bodies construct NoResponseRequiredError / UnknownMessageError identities. Graph/native profile raise the typed source control errors; `socket-source/native_message.cpp:69-74` catches those distinct cases. Native exception layout is explicitly not reconstructed by these source types. | Business control-flow bindings present. Do not treat exception constructor addresses as unimplemented handlers or launch an EH framework task. |
|02bc10, called by030010 under event lock | Existing `event-queue-source/flow-audit.md` traces exact achievement cold-seed DOWN-event + achievement-event order and lock/unlock sites. Current actual Graph event provider and already completed growth stages implement selected queue behavior. | Existing normal proof; no new event-lock algorithm seed. Static-init/CRT/unwind concerns are separate limits, not current business gap. |

The list above contains six bounded groups including the chosen seed. It intentionally does not fill eight slots with low-value heuristics.

## Exclusions and scope

Retained bodies confirmed that02b300 is string reserve/SSO adjustment,02b600 a tree insertion-position helper,02c2d0 vector allocation,02c370 C-string append,0349a0/035120 vector growth,036bc0 map construction, and037d90/037e20/037ee0 vector destruction. They are compiler/container mechanisms, not absent independent application features. Current standard-container/value code may represent their selected normal results without reproducing their allocator layout. No fresh helper scan is recommended.

All newly closed initial/tracked environment bodies are excluded despite their stale not_mapped labels. Existing owned-response normal-path lifetime code also explicitly avoids inventing024fb0 EH cleanup calls after its pointers have been consumed; generic native unwind remains a documented limit, not an inferred normal missing function.

No source percentage, completed-function count or frozen inventory was rewritten. The single actionable next comparison is the selected system-category message provider020950.
