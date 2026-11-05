# Registry SDK nesting — Domain source implementation

Only `foreign.hpp` and `foreign.cpp` were edited by this subagent. Exact pre-edit copies are retained beside this note. No compilation, project tests, linking, REA operation, DLL execution or packaging was performed by the subagent; root owns the native evidence session and integration.

The Original publication profile gains four optional Admission operations delegated to Domain. Two automatic TLS-linked token types distinguish actual registry original-SDK intervals from nested general registry bodies. Only open A/W, query A/W and close tokens participate. Existing pure-forward/connect actor phases remain excluded as general registry SDK parents.

The outer general admission stays active throughout all descendants. Its CallbackThreads object and existing PTD stay owned by the root: nested entry checks the selected hook's publication and root `permits_current()` but never invokes CallbackThreads enter/leave. Other threads therefore retain the preexisting general-overlap rejection. Same-thread/fiber descendants run their shared-set bodies sequentially while each ancestor is suspended inside its typed SDK call.

The SDK token captures the exact current body identity. Child admission first publishes its own automatic body token and switches the global actor metadata to Checking, then performs opaque publication/root-PTD checks. An ancestor SDK token cannot authorize another callback from that Checking phase or from the child's later CRT/log/set body, because its captured body no longer matches the current child token. A deeper SDK interval can capture that exact child and grant the next registry descendant.

Successful admission becomes Full. Existing Full Policy permission remains present while a selected SDK marker is active, as required by root integration; the marker adds only bounded nested admission. Exact LIFO leave restores the parent's callback and Full mode without releasing the root PTD. Normal root leave rejects outstanding SDK or nested-body tokens. Metadata operations preserve LastError.

Known missing publication and a known stop unwind child metadata and retain the parent actor. Opaque proof/PTD exceptions or a failed PTD validation poison the Domain; exact frame cleanup still restores the parent. Frame stack corruption fails through existing fail-stop conventions rather than inventing a safe teardown. SDK begin does not reject merely because pause closed new admission during an already-owned call; new descendants still reject closing. SDK/nested cleanup works with poison/closing when its exact metadata is intact.

This is a source implementation and manual static review, not compiler/runtime validation or proof that Windows actually reenters registry APIs. It adds no general cross-thread container concurrency, native CRT ABI claim, global-ready gate or new unload protocol.
