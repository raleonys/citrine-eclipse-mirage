# Native image owner — 2026-11-03

Status: SOURCE_IMPLEMENTED_NOT_COMPILED_OR_EXECUTED.

`entry.cpp` now defines both loader integration functions. The attach definition
constructs an actual owning Image, not a fixture or a call into the original DLL.
The permanent root adopts it before Domain initialization or publication. Image
owns the managed compiler-stdout stream, native dialog, initialized Domain,
configuration/console resources, host assembly, Online/Worker inputs and Runtime.
The host uses this image's statically linked NativeApi/OwnedEngine. Resources,
Context and Online select the existing OriginalCrt source profiles; Worker uses
OriginalApplication. No cooperative stop/unload predicate is fabricated.

Domain is adopted before initialize; Runtime is adopted before publish. After
all providers and initial cells are assembled, Image publishes Runtime and calls
its real ApplicationEntry synchronously exactly once for DLL_PROCESS_ATTACH.
DllMain returns that BOOL and does not make a second application-entry call.
The incoming and returned LastError values surround the actual application call.
Unknown or partly published failure uses the explicit compiler-abort error policy;
it never reports attach success. This policy is not recovered MSVC exception ABI.

The application detach helper remains intentionally empty, matching the retained
038870 reason0 slice. The compiler CRT runs the actual registered source exit
callback in its normal order. DllMain does not duplicate cleanup, close Runtime
or join workers. This preserves the user's selected startup/detach direction.

Worker now fills an empty OriginalApplication Graph version from its exact live
Runtime configuration.sdk after configuration and before Graph construction.
Retained `anadius-messages-source/evidence.json` identifies the Challenge version
read at 0x18002d540 from global 0x1800a3c98; `anadius-config-source/evidence.json`
maps that cell to sdk. The separate bootstrap log literal is not this SDK value.
Existing explicit fixture versions are preserved. No fresh binary access occurred.

The mapped-image root deliberately has no automatic Runtime/Domain destructor.
Its retained graph allows later CRT callbacks to access their providers. This
source ownership does not pin the OS module, prove full native resource teardown,
or validate live FreeLibrary/FLS behavior. Native callback concurrency, loader-lock
execution, exception/CRT ABI and actual application behavior remain unvalidated.
Those unknowns are not converted into an extra cooperative unload prerequisite.

Both loader names are now source-defined, but the last real link result remains
the historical two-undefined-symbol failure. No new DLL exists from this step.
Future link helpers no longer classify those undefined symbols as an intended
success. Their source was edited only; none was executed. The old
verify-link-gate.py and package-fresh-full-build.py are historical workflows with
blocked-link assumptions; they are not current-source validation and must be
reviewed before reuse in a separately authorized build/package step.

No compilation, linking, project tests, DLL execution, original-file modification
or ZIP update was performed. See native-image-owner.evidence.json for current
source hashes and preserved build/link/receipt hashes.
