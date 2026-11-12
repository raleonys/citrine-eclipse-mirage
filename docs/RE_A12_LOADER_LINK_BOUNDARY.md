# Archive 12 loader and linked-image boundary

**PARTIAL_STATIC_A12_REVIEW / LINKED_NOT_RUNTIME_VALIDATED / RESEARCH_REMAINS_OPEN**

This is the fresh A12 follow-up requested when [PR #126](https://github.com/raleonys/citrine-eclipse-mirage/pull/126)
was closed. Preserve that closure and historical A11 report; the old missing-link
condition is superseded by A12, while loader/runtime equivalence is still open.

## Reproducible input boundary

Current source authority: retained `research/reconstruction-package@56eab8795fa65325e4fa6fe31a645eb55cc58469`.
Product comparison: `main@eea758600fe0801ec91c31f85de563afff1d75a7`.
The nine-part Archive 12 ZIP identity is
`539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`;
the [A12 inventory](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_FILES.json) records 6,514 files.
The six selected inputs were exported from that exact commit and their lengths
and SHA-256 verified against the inventory. The DLL identity was independently
matched to the supplied link receipt. Original ZIP parts were not reassembled.
No supplied script or executable was run; no DLL was loaded. The PE inspection
used the repository's existing read-only PE parser on the selected bytes.

| A12 input | Bytes | SHA-256 |
| --- | ---: | --- |
| [dll-link-gate-source/entry.cpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/entry.cpp) | 9902 | `c0810bfc385c7159bdc559ad7bb3329799ca212d17ae4aba7fa49cd2243e09ca` |
| [dll-link-gate-source/production_image.hpp](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/production_image.hpp) | 2764 | `7853bba247ba243ba59fce8ec67fe08066a6bba40b14ae1d89f68dc513940829` |
| [dll-link-gate-source/anadius64.def](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/anadius64.def) | 112 | `8a6cba4f2cf4b893c3a22ac312f675e9a68d3049ded9882dafe4232dbc2e46fe` |
| [dll-link-gate-source/export_leaf.S](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/export_leaf.S) | 411 | `f9f864181cc17d774e6be441d7321bcac6c8472793a4f377795c1eb0617e7a08` |
| [dll-link-gate-source/current-full-link.json](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/current-full-link.json) | 5013 | `0cb6b7f6a5333510d2d99eb988c79ea7e4ce636cde9a4a1a7fff411f6bf126d5` |
| [dll-link-gate-source/build-current-full/anadius64.reconstruction.dll](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/reference-package/snapshot/dll-link-gate-source/build-current-full/anadius64.reconstruction.dll) | 4872192 | `4a507e834c179bfe55ef2df87a60d3e423531aec506dcab3796a89f94d8e9381` |

## What A12 now establishes

The supplied `fresh-full-archive-dll-link.v3` receipt reports link exit code zero,
a created DLL, and no undefined symbols. Its selection is all core and hook
compatibility archive members with the separate preloader archive excluded.
It explicitly says `LINKED_NOT_EXECUTED`, `LINKED_NOT_RUNTIME_VALIDATED`,
`dll_loaded_or_executed: false` and `runtime_or_loader_equivalence_established: false`.
This review checks receipt/source/binary identity, not a fresh linker execution
or independent replay of every input archive/object/toolchain hash.

Static PE inspection confirms AMD64 machine `0x8664` and three named exports:
`CDX@1`, `RUNE@2`, `anadius@3`, all nonforwarded at RVA `0x51b0`.
The selected DEF aliases these names to the same assembly leaf. That source
leaf is only `ret`; it does not establish an initialized return value or
functional application behavior behind those three names. Matching an export
table is not proof of original exported behavior or game compatibility.

## Loader and lifetime comparison

The A12 entry creates an Image with permanent errors, Domain, Resources, factory
and Runtime, publishes the application and invokes its process-attach entry
synchronously from DllMain. It deliberately retains a mapped-image shared owner
and does not register automatic cooperative Runtime/Domain destruction. The
compiler owns CRT startup, TLS and exit callbacks.

`require_loader_detach_quiescence` performs no application detach work; the
source explains the recovered reason-zero no-op. Despite its name, this is not
a barrier proving foreign callers, provider callbacks, hooks or workers have
stopped. It expressly avoids Runtime close/join inside DllMain. The static link
receipt cannot demonstrate loader-lock safety, failure cleanup or real unload.

The accepted product deliberately creates a stop event and worker in DllMain;
the worker constructs Application and does runtime/network/hook work outside
loader lock. Explicit shutdown retains the same owner and image lease across
failed stages. Its public ABI remains `DllMain@1`, `SURGE@2`, `DllShutdown@3`.
Copying A12's entry strategy or export aliases would change product policy and
ABI rather than repair a demonstrated product bug.

| Current product contract | Comparison and existing owner |
| --- | --- |
| DLL entry and image lease | `src/platform/dll_entry.cpp`; explicit worker and retained owner. Open [#322](https://github.com/raleonys/citrine-eclipse-mirage/pull/322) already owns image-reference acquisition failure tests. |
| Ordered Application shutdown | `src/runtime/application.cpp`; merged #102/#125/#160/#168/#172 own distinct supported fault schedules. |
| Public ABI | `exports/citrine_eclipse_mirage.def` and `metadata/PUBLIC_DLL_ABI.json`; donor aliases are not product requirements. |
| Post-start cooperative ownership | Continuing [#119](https://github.com/raleonys/citrine-eclipse-mirage/pull/119) compares A12 Controller/runtime owners; this report covers the linked image and loader boundary. |

## Remaining work and closure criterion

The next retained reconstruction step needs an executable loader-safe host plan:
identify the exact image/toolchain/source closure, entry and exit responsibilities,
allowed calling thread/fiber, failure-atomic owner retention and explicit drain
observations. Check positive and negative attach/close schedules outside loader
lock, with callback/provider/hook lifetime evidence and the original ABI stated.
A passing whole-archive link or a no-op detach function cannot substitute for
those observations. Further original binary and real Consumer/game comparisons
remain separate acceptance classes.

No new shipping defect is confirmed by this bounded comparison, and no runtime,
CRT, public ABI or vendor change is proposed. Keep the research workstream open
until those lifetime/equivalence questions have independently scoped evidence;
do not retire the source archive because the old A11 link blocker was superseded.

## Local validation boundary

Selected A12 inventory rows, receipt-to-DLL identity and read-only PE tables were
checked locally. Source QA and clean-tree pre-push results are attached to the
new PR head. Fresh native Windows/MSVC, production Golden, original DLL,
loader execution and Consumer/game acceptance were not run here. Supplied
historical validation claims remain attributed to their original records.

## Reviewed product identities

| Path at pinned main | SHA-256 |
| --- | --- |
| [src/platform/dll_entry.cpp](../src/platform/dll_entry.cpp) | `d638b43a6b627d0182a9eb63590cd7c4eaad3c81d67f4265e1318566d238c84d` |
| [src/runtime/application.cpp](../src/runtime/application.cpp) | `7a2be80fd20f60a6e5b279ff8836c4be875040b5f3f32692219cce9265e4348b` |
| [exports/citrine_eclipse_mirage.def](../exports/citrine_eclipse_mirage.def) | `4553dc30226a83083be75f6b41c4eef463f55e571f1080454c1cf6f3640278b2` |
| [metadata/PUBLIC_DLL_ABI.json](../metadata/PUBLIC_DLL_ABI.json) | `d5c0126423ae1d4a288153f6f2a19864c43baf10550c09e3bb2ee0d04fbbcae0` |
