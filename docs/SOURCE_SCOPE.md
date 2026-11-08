# Source and build boundaries

## Correction of the earlier output

The former `src/recovered` layout largely reproduced the RE working tree.
Anadius application sources, custom CRT recovery, separate preloader
projects, and test sources were all in one tree; the main targets produced
only static archives. That was not a Brownfield-style application project.
The previous package and working tree are retained for reference but are
not build inputs to the current product.

## Current selection

- Anadius64 application sources include configuration, bootstrap contracts,
  core thread launch flow, protocol/messages, handlers, transport/IOCP, XML,
  registry, file, time, hash, and native platform adapters.
- Separate preloader Core/Once/process-enumeration/mutex and version-wrapper
  projects are not imported. Only the explicitly requested `CryptQueryObject`
  behavior is isolated behind an independent platform interface. The host
  now requires four allocation/trap operations instead of importing the
  entire old preloader Host/HookEngine dependency graph.
- Original custom CRT/VCRT recovery packages and the historical B2-B6
  composition experiments dependent on them are not part of the product
  target. They are not automatically linked in place of the standard CRT.
  They remain accessible from the RE inputs and earlier packages.
- Four active vendors are selected from the owner's separately supplied
  `3RD PARTY` archives. Gulrak was subsequently removed by explicit owner
  instruction; the original archive is retained. Older RE MinHook/TinyXML2
  copies are removed from the active tree, and Brownfield's vendor tree is
  not imported. PR #149 replaces the active TinyXML2 tree with the owner's
  exact pugixml archive; the external pinned control and historical member
  list preserve retired input provenance. The project depth-500 boundary
  remains in first-party XML policy. No vendor bytes are patched.
- Tests are separate from headers and application `.cpp` files. CRT/preloader
  tests excluded from the product scope are not modified to conceal failures;
  they remain in the earlier package and are itemized in the scope list.
- Brownfield adaptation covers build helpers, actual Golden tools
  and pipeline, compatible negative tests, metadata/script ownership,
  source QA, five operator profiles, and flat source packaging. It does
  not transfer application code.

See [the Brownfield adaptation](INFRASTRUCTURE_ADAPTATION.md) and transfer
matrix for the current scope. Final product Golden acceptance was still
unavailable at this checkpoint because metadata/ABI contracts were incomplete.

`metadata/evidence/SOURCE_LAYOUT.json` contains the earlier path and hash
for every transferred file, applied path/namespace transformations, and
selected/excluded modules. This is not proof of native behavioral equivalence
to the original or an independent owner signature.

## Production DLL and development interface

`citrine_eclipse_mirage` is a SHARED target: all selected application objects are
linked into the production DLL. `citrine_eclipse_mirage_source_host` links against the
separate `citrine_eclipse_mirage_source_api` development DLL, which also retains the
reference models. It does not expand the shipping three-export interface.
See [BUILD_PARTITION.md](BUILD_PARTITION.md) and
[PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md) for the current target boundaries.

Development C++ exports do not form the production game/Consumer ABI.
Automatic startup, the Application owner, provider adapters and the fixed
SURGE interface are bound as described in [CURRENT_RUNTIME.md](CURRENT_RUNTIME.md).
Native hook/teardown and real Consumer acceptance remain separate evidence
classes tied to the tested commit and DLL; these bindings alone do not prove
arbitrary host compatibility.
