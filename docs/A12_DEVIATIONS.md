# Archive 12 differences and equivalence limits

**NOT_IDENTICAL / BOUNDED_A12_CROSSWALK / NO_GLOBAL_PARITY_CLAIM**

## Answer and baselines

The product is **not byte-identical, ABI-identical or proven behaviorally
identical to A12**. Some selected source contracts match; confirmed deliberate
and structural differences remain. Changing XML depth alone cannot establish
whole-application equivalence. A12 itself is partial reconstruction/reference
material, not an independently accepted original DLL or real Consumer oracle.

Product baseline for this review: `main@fec37ae534696727dd1efc7dedb1398b2e3b99e8`. The depth change described here
belongs to this proposal until owner integration; that main still accepts 498
paired levels and rejects 499. This proposal accepts 98 and rejects 99 at the
entered-child-list boundary, matching the selected A12 source profile.

Integration baseline for this synchronization: `main@d91c54c99afd12ef65868f7996bd589f148ae35c`.
It includes accepted #280 rollback-matrix coverage and later regressions through
#273. The 85-row A12 crosswalk and source probes retain the review snapshot
identified above; fresh XML validation belongs to the synchronized PR head
recorded in its description.

A12 authority: retained `research/reconstruction-package@56eab8795fa65325e4fa6fe31a645eb55cc58469`;
nine-part archive SHA-256
`539fc80596bfa058afdfe74b01cee95c1c1b807be3e983127510e4dc170acb6d`.
The [file inventory](https://github.com/raleonys/citrine-eclipse-mirage/blob/56eab8795fa65325e4fa6fe31a645eb55cc58469/docs/RE_PACKAGE_FILES.json) contains 6,514 files.
For this report, 85 unique selected source/test/description/record/binary
rows used by the linked crosswalks were re-read and length/SHA-256 checked
against that exact inventory. This is an identity check, **not** a claim of
complete semantic review of those rows, every object or the whole archive.
No whole-ZIP reassembly, original DLL, game or full donor runtime was executed.

Unlike earlier static-only review, this work additionally compiles the two
inventory-verified A12 TinyXML2 compatibility sources with a small locally
written depth/value probe. Its Linux source-level results are not original-binary,
Windows/native, whole-reader or complete malformed-input equivalence evidence.
Supplied archive scripts and test executables were not run.

## Confirmed design/policy differences, including intentional choices

“Keep” below records current disposition rather than claiming the difference
cannot matter. No difference is excluded merely because it was intentional.
The proposal changes only D02; other rows are reported without transplanting
A12 code or silently reversing accepted product policies.

| ID | Boundary | A12 source / archive | Product baseline and disposition |
| --- | --- | --- | --- |
| D01 | XML implementation | TinyXML2 11 compatibility copy with a modified header. | Immutable pugixml 1.16 plus first-party grammar/value/wrapper code, selected by #149. Different implementation and maintenance/allocator behavior remain even if selected parse results match. Keep current vendor; no equivalence from a shared interface name. |
| D02 | XML depth | Depth-100 profile: 98 paired levels succeed, 99 fails with error 18; a self-closed level-99 leaf remains allowed. | Main uses the historical depth-500 profile (498/499). **This proposal aligns the selected depth contract to A12**, with preserved error priority, explicit old-control deltas and no vendor modification. |
| D03 | Numeric/entity value decoding | Selected TinyXML2 compatibility value path is not the corrected #108 product decoder. | Checked Unicode bounds and literal preservation of malformed/unknown references remain deliberate safety/correctness differences. The 20 old corpus value corrections are retained. Depth agreement does not remove these differences or prove all malformed-input parity. |
| D04 | Asynchronous event delivery | Pure source message path eagerly dequeues an event even when encryption fails. | Pure source path retains its selected behavior; prepared asynchronous responses instead reserve/settle delivery, retain on failure and unlock exception-safely. Keep accepted #143/#173/#248/#340; do not copy eager dequeue into the asynchronous owner. |
| D05 | Achievement multi-file persistence | Separate plain XML/INI writes; supplied writer ignores its boolean write result. | Product uses a native atomic batch with explicit failure/rollback/recovery ownership. Keep accepted safety semantics; accepted #280 adds rollback-matrix coverage. A plain donor write is not a substitute for the product transaction. |
| D06 | Achievement path components | Selected source constructs paths without the complete accepted product Windows component policy. | Product rejects unsafe XML/watcher path components and reserved native names (#179/#261), including documented Windows scope/alias exceptions. Keep safety policy; do not claim all paths are now source-equivalent. |
| D07 | Achievement INI sections | Source serializer emits selected map keys directly. | Product rejects watcher section injection (#188). Names containing structural delimiters can have different outcomes. Keep the accepted safe serialization boundary. |
| D08 | Achievement numeric input | A12 storage requires Numbers effects; that source alone does not define every native overflow outcome. | Product provides checked 32-bit decimal/hex adapters (#205). This concrete guard/model difference is recorded; exact A12 native invalid-value behavior and private CRT conversion ABI remain unproved. |
| D09 | Persistence side-effect order | Selected A12 writer creates directories before serialization. | Product validates/serializes watcher INI before creating the parent (#296), leaving disk/grants/events untouched on invalid names. Keep the accepted failure ordering. |
| D10 | Registry fabricated-key failures | Selected A12 allocate/insert/log sequence does not independently undo a later throw. | Product's #98 rolls back new membership, restores result/status and frees the just-created allocation on the supported fault boundary. Keep the deliberate correction; it is not an arbitrary fallible-free retry protocol. |
| D11 | QPC cold-cache contract | Donor recursive source guard serializes the sampled owner to one frequency query. | Caller-owned product cache deliberately permits two racing getters publishing one value. Application preinitializes a positive frequency before worker launch. This is a confirmed model/test difference, not a newly proven shipping startup race. |
| D12 | Publication/observer model | Optional ReadyStage/Publication observers carry source milestones, issuer checks and port snapshots. | Product uses its Application/Asio owner and publication contracts (#128/#130). Donor observer names/layouts are not required product APIs. Keep distinct owner models; full native ordering equivalence is unproved. |
| D13 | Default user identity | A12 default username is `anadius`. | Product default username is `SURGE`, an explicit branding choice. Ordinary default response bytes therefore need not be identical. |
| D14 | DLL attach and startup | Current linked-image entry constructs/publishes Runtime and invokes attach synchronously from DllMain. | Product creates its owned worker and does runtime/network/hook initialization outside loader lock. This is an architectural difference; retain loader-safe product ownership. |
| D15 | Explicit close ownership | Cooperative Controller has original controller thread/fiber requirements, Runtime/Domain retention and poisoned unknown outcomes. | Product's explicit shutdown retains the same Application/image lease on its DLL worker across staged retries. Keep accepted #102/#125/#160/#168/#172. Do not adopt a foreign controller ABI from a similar close function. |
| D16 | Public DLL exports | Selected reconstruction image has CDX@1/RUNE@2/anadius@3 aliases at one RET leaf. | Product exports exactly DllMain@1/SURGE@2/DllShutdown@3. Both table and function purpose differ; the donor leaf does not prove functional return/init behavior. Keep the product ABI. |
| D17 | Private CRT source applicability | Archive includes private PTD/FLS/heap/onexit/error-cell reconstructions and their layout/Effects assumptions. | Product keeps standard compiler CRT and separate static/shared CRT link graphs. These modules are not adopted. This does **not** assert that the current A12 linked DLL uses a private CRT; its entry uses compiler CRT. |
| D18 | Authentication owner model | Optional immutable AuthenticationCode/OwnedAuthEffects binding adds owned auth state/copy capabilities. | Product retains a caller-owned fallback and immediately copies borrowed provider data (#186). This is a source/owner-model difference; it does not itself establish different normal auth bytes or a current bug. |
| D19 | License-file XML text | A12 KeyForLicense appends counted key/ContentId/UserId bytes without XML escaping. | Product-owned writer escapes XML text before CBC (#223). Ordinary safe text retains its selected format, while metacharacter-containing input can yield different bytes. Keep the accepted injection/correctness fix; do not confuse this with the checked ticket writer. |

### Directly reproduced D03 value examples

The locally compiled, inventory-verified A12 TinyXML2 compatibility source and
actual product reader produced these counted attribute values. This tests the
selected source value path, not an original DLL or whole-dialect oracle.

| Input attribute spelling | A12 compatibility source | Product |
| --- | --- | --- |
| `&#0;` | Empty C-string-visible value | Literal `&#0;` |
| `&#6;` | One byte `06` | Literal `&#6;` |
| `&#xD800;` | Bytes `ed a0 80` (surrogate encoding) | Literal `&#xD800;` |
| `&#x110000;` | Literal `&#x110000;` | Same literal (positive agreement control) |
| `&#65;&unknown;` | `A#unknown;` | `A&unknown;` |

Retain product checked decoding/literal preservation. A12 depth alignment is
not a reason to reproduce these invalid-value/data-corruption outcomes.

## Matches, non-equivalent fields and unresolved proof

These rows are not counted as confirmed behavior differences. A partial match
or a missing proof must not be silently promoted to full parity or a defect.

| ID | Boundary | Current finding / remaining evidence |
| --- | --- | --- |
| M01 | NoResponse | Shared typed terminal and early empty result are already implemented (#234/#266), before framing/encryption/event selection. Do not report the old missing-catch claim as current. Original private exception ABI remains outside that match. |
| M02 | UnknownMessage | Accepted #350 now implements optional dedicated-marker diagnostics and same-session continuation. Ordinary failures, missing reporters and diagnostic failures still propagate; #264 retains ordinary rejection isolation. Earlier fail-closed-only Application reports describe their old baseline. Real UI, arbitrary foreign host and private EH equivalence remain unproved. |
| M03 | Registry language temporary | A12 nested log scope and product #337 both retire it before the replacement writer. No duplicate fix is needed. #240 separately covers output alias ordering. |
| M04 | Registry source/build chronology | Earlier 4 November static-stage no-rebuild text is superseded by later supplied 5 November build records: current source-snapshot bytes, Registry source row and common build-record identity link to the current receipt. This is selected record identity, not independent machine-code/runtime proof. |
| M05 | Plain license file semantics | Selected paths preserve the existing regular-file skip, 65-byte encrypted prefix and ignored plain-write boolean. Do not label every product writer atomic/checked: achievement batch, plain license file and checked ticket writer are distinct owners. Full crypto/file/native equivalence remains unproved. |
| U01 | SDK identity | A12 defaults contain a State.sdk value `10,5,96,47512`; product's separately specified LSX Challenge SDK is `10,6,2,31`. These are different model fields/call sites. Comparing literals alone does **not** establish an omitted default or a proven same-wire-field mismatch. Further same-call-site response tracing is required. |
| U02 | HKEY origin | Both selected numeric-set models lack a trusted issuer/owner/generation carrier at callback ingress. Issue #109 remains open; #113 direct SDK issuance and #224/#255 controlled installed-hook tests are distinct existing evidence. A synthetic collision is not proof of an OS alias or universal safety. |
| U03 | Native/foreign quiescence | Controlled owned shutdown schedules do not prove arbitrary provider, late callback, foreign thread/fiber or actual Consumer unload equivalence. A no-op detach is not a drain barrier. |
| U04 | XML dialect completeness | Current corrected-corpus comparison is against immutable #108, not the full A12 reader or original DLL. A source-level depth probe and selected status records do not prove every malformed input, allocator failure, diagnostic, original TinyXML2 release or ABI matches A12/original. |
| U05 | Whole-archive coverage | Archive-only, description-only, excluded/reference-only and unreviewed modules remain. No complete A12-module-to-product behavioral matrix, original binary acceptance or meaningful global parity percentage is established by these reviews. More differences may remain undiscovered. |

## Why 100, and what security claim is justified

The A12 TinyXML2 header explains the recursive parser's stack-exhaustion guard;
it changes upstream 500 to 100 and attributes that profile to recovered code.
This review verifies that header and executes the selected compatibility source,
not a new analysis/run of the original binary. Our existing 500 value came from
the old unmodified TinyXML2 control and was preserved by the pugixml migration;
it was not a pugixml security requirement.

Selecting 100 aligns that bounded A12 source contract and lowers permitted
entered-child-list work/storage. The product scanner is iterative, checks before
pugixml DOM parsing and is tested to request no pugixml storage for a depth
rejection. Thus this is a conservative compatibility/resource bound, not proof
of a pugixml recursive-stack vulnerability or a general DoS fix. Width, input
bytes, elapsed work, transport budgets and other resource limits remain distinct.

The compatibility cost is real: formerly accepted paired depths 99..498 now
fail. No observed real Consumer requirement for that larger range was established
here; actual game/provider acceptance remains separate. The self-closed leaf
and attribute/EOF precedence exceptions are retained because this is the A12
profile, not a new blanket tree-level cap.

## Reproduction, evidence and follow-up ownership

The implementation, immutable-control differences and test recipes are in
[XML_BACKEND_COMPARISON.md](XML_BACKEND_COMPARISON.md). A12 input hashes, isolated
probe output and local results stay in ignored `build/a12-parity/`. Release and
ASan/UBSan checks use the actual production reader; Windows/MSVC, production
Golden, original DLL and real Consumer/game acceptance are not claimed locally.
Historical/native receipts in older PRs are not promoted to fresh runs here.

Continue detailed source work in existing #119/#132/#136/#139/#140/#141/#178/#191,
the separate retained ledger #236, loader #356 and Registry #357. Separate
coverage work is recorded in #312/#322/#324/#334; accepted #280/#341/#346/#348
provide their scoped regression evidence. #355 owns performance evidence rather
than automatic production tuning. The all-PR inventory is #358. These scopes
are not completion or retirement of
A12, other field archives, a module or the original-source reconstruction.

This is the consolidated list of differences and proof gaps identified in the
bounded reviewed scope, **not an exhaustive assertion about all 6,514 files**.
Even after D02 merges, D01 and D03-D19 remain and U01-U05 need further proof.
