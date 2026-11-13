> **Historical checkpoint.** Use [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md) for current guidance.
> Old counts, outcomes and pending work are source-bound; see [INDEX.md](INDEX.md).

# Historical application integration checkpoint

Checkpoint interface work item: `YF-PUBLIC-ABI-20261028`. Runtime,
configuration, resource, and layout changes are retained. Historical
comparisons are distinct from the current implementation.

| Area | Previous state | Integration at checkpoint | Validation and limitation |
|---|---|---|---|
| Configuration collections | Incorrectly generalized as lacking a complete path. | Application uses the existing full `config_hash` / `config_application` path. | 25 content, entitlement and achievement totals; actual main -> override |
| Asio | Vendor and I/O fixture existed; no actual application owner. | Actual RE ServerBackend/Session adapter, socket/completion/job lifetimes and drain. | Real loopback greeting/profile/content/encrypted/event/stop |
| Windows filesystem | NativeFactory previously used std, then ghc. | Standard filesystem/fstream restored on owner instruction; unused Gulrak removed. | Unicode binary I/O, configuration/achievement/avatar paths, Debug/Release log checks |
| Startup State | Only caller-seeded State existed. | `036d20` pseudocode/assembly application-field projection; SURGE display name. | Actual Application starts without configuration |
| DLL bootstrap | Only a source-API development DLL existed. | DllMain preparation and Application worker outside loader lock, with management API. | Actual DLL starts automatically, answers LSX profile over socket, and stops |
| Native hook owner | Eleven source bodies/slots existed without one concrete shared owner. | Single configuration/original/context/MinHook owner with source conditions and failure rollback. | Eleven-hook activation and actual folder/process/registry SDK calls; Wine evidence |
| Provider | Source Effects ports; new name not connected. | RE version2/factory/virtual ABI adapter, new name, owner cleanup. | Controlled DLL for profile/auth/avatar/failed auth/destructor; not acceptance of actual external provider ABI |
| Avatar/log/paths | Resource and policy metadata without runtime integration. | LocalAppData/SURGE, byte-exact resource -> handler, Debug-only event log. | Actual resource byte comparison; Release log absence and Debug log presence |
| Source/tool comparison error | ScalarDependencies boundary was incorrectly generalized to the whole application. | Explicit correction to historical table; new current-status record. | Earlier ZIPs and raw validation retained |
| Native product acceptance | Missing. | Separate acceptance required; source integration alone does not provide it. | Windows/MSVC/game NOT_RUN; withdrawn 548-byte ABI claim; original Consumer import names differ from this branded DLL |
| Brownfield resource metadata | PNG only; VERSIONINFO omitted. | Exact `version.rc` with retained fields/string/translation. Later fix: PNG exists only as read-only C++ avatar data, with no RCDATA. | Release/Debug actual PE resource verification and metadata tamper rejection PASS; other Golden/ABI values still pending |
| Brownfield internal module layout | Unnecessary nested module directories. | One module level; 177 files moved. | 261 bodies identical except for includes; new build/runtime/QA evidence |
| Application C export interface | Thousands of C++ source exports mixed into application DLL. | Three Brownfield C names/ordinals: DllMain@1, SURGE@2, DllShutdown@3; management exports removed; SURGE RE RET-only marker; DllShutdown performs real cleanup; source APIs in separate DLL. | Actual PE export check, seven negative mutations, C17 import consumer, three load/socket/shutdown/unload cycles; Wine evidence |

Current interface: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md).
The companion CSV predates removal of the two management exports; its
five-export row is an earlier historical state, not the current public ABI.

Raw results: [PUBLIC_ABI_STATUS.json](../metadata/evidence/PUBLIC_ABI_STATUS.json).
Earlier runtime evidence:
[RUNTIME_BINDING_STATUS.json](../metadata/evidence/RUNTIME_BINDING_STATUS.json).
