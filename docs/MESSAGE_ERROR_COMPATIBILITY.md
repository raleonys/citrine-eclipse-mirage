# Message error compatibility

The owner selected the retained reference UnknownMessage continuation and its
console/dialog diagnostic. This replaces the previous Application policy that
closed the peer for these invalid dispatches. NoResponse continuation remains a
separate, already accepted control path.

## Dispatch scope

`Application::SessionHandler::invalid_message_02bd30_04d228` throws the separate
`messages::UnknownMessageError` marker. The dispatcher reaches this producer for:

- no root or a root other than `LSX`;
- no `Request`, a different first child, or an additional sibling of `Request`;
- missing request `id`, `recipient`, or first command;
- an unregistered first command other than `ChallengeResponse`;
- a registered handler returning false, including its own required-field checks;
- `ChallengeResponse` without its required key.

The existing first-command-only rule is unchanged. Well-formed empty attributes
remain present attributes; this change does not add new validation. XML parse
errors and decrypt failures retain their existing retry/empty-result behavior.
Ordinary handler exceptions, allocation failures, invalid Router integration and
resource-limit/IO errors are not converted to this marker.

Only the protected dispatch call in `process_message_033d70` catches the marker.
A caller must also implement the optional secondary `UnknownMessageEffects`
capability; otherwise the marker is rethrown. Existing Effects vtable methods
are unchanged. Failures before dispatch, in the diagnostic itself or after the
protected call propagate. No broad `std::invalid_argument` catch is added.

## Diagnostic and continuation

The Application uses `std::cout`, independently of ShowLSX and in Debug and
Release. This is a console stream, not a new log file or a change to the
Debug-only `surge.log` sink:

1. Insert `[UNKNOWN MESSAGE]`, then a newline and flush.
2. Sample the original request object and insert its full counted bytes,
   including embedded NUL, then a newline and flush.
3. Sample the original object again and copy its current bytes into an owned
   dialog body. The earlier borrowed view is not used after the second flush.
4. Convert the owned title `UNKNOWN MESSAGE` and body using the existing UTF-8
   Environment conversion. Call `MessageBoxW` with a null owner and flags
   `0x10` (`MB_ICONERROR`, with the default OK button). Ignore its return value.
   The SDK's wide C-string display stops at the first embedded NUL, after full
   counted conversion. A conversion or dialog exception prevents continuation.

The modal diagnostic blocks that handler worker until it returns. It displays
and writes the request contents, as explicitly selected by the owner. The
separate unknown-entitlement-field diagnostic remains nonmodal and redacted.
No new global UI default, caller-secret file, management export or vendor change
is introduced.

After normal diagnostic return, the message core transfers its current local
response (empty for the existing dispatcher). It bypasses response logging,
framing/NUL append, encryption and event selection. The ordered Session handles
that empty result without a socket write and admits the next frame on the same
connection. A queued event is reserved/settled only by a later eligible response
under the existing delivery policy. Diagnostic failure follows the existing
per-peer exception/close path; it cannot report successful continuation.

## Reference and proposal boundaries

The reference is pinned to reconstruction package
`56eab8795fa65325e4fa6fe31a645eb55cc58469`, especially the selected messages
implementation, `anadius-unknown-message-source/native-diagnostic.json` and
`control.evidence.json`, and the licensing-effects diagnostic. The retained
static observations place the protected dispatch at `[0340bf,0341b1)`, the
UnknownMessage funclet at `07bd70` and continuation at `0345da`. They support the
ordering above, not execution of the original DLL, original EH/object/private
CRT ABI, game acceptance or every foreign exception provenance.

Research PR #178 describes the previous difference. Open test proposals #258
and #264 pin the former policy or a deliberately rejecting test Effects
implementation; they do not implement this compatibility path. The Application
peer-close expectations in #258 need policy-aware adaptation before combining
that proposal with this change. A custom Effects implementation that throws
`std::invalid_argument` still propagates, preserving the generic error boundary
in #264. Neither existing PR is modified or merged here.

Message-core and ordered-Session regressions exercise controlled effects,
ciphers and memory transport. The Environment fixture intercepts MessageBoxW;
it retains the original nonmodal entitlement and Debug/Release logging checks. A test-only
10-second watchdog fails its own fixture process if an unexpected real modal
prevents return; it never acknowledges a production dialog or converts failure
to PASS, and the existing 60-second CTest budget is unchanged.
Host/sanitizer results do not establish native Windows/MSVC, real modal UI,
production DLL/Golden or original/Consumer equivalence. Required later native
validation remains separate from initial PR source integrity.
