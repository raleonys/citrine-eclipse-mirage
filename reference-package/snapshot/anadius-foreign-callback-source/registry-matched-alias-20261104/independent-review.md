# Independent static review — matched registry paths and aliases

Static review: **PASS within the selected scope**. No concrete blocking defect found in the two added test-source scenarios or their expected results. This is source inspection, not a compiler or executable test result. The reviewer made no production edits and performed no compilation, project tests, linking, REA session, target execution or packaging.

## Final reviewed bytes

| Source | Before SHA-256 | Final SHA-256 |
| --- | --- | --- |
| `outputs/anadius-registry-source/registry_tests.cpp` | `14f9f7148e517d4241904add1c8aaa1c55b57aa97d20aa54830a67b896683da8` | `8d8bc4717034b6cac5cc3d5893270fde4fe695fe6a02b0f88ca4f745d32450c1` |
| `outputs/anadius-foreign-callback-source/foreign_integration_tests.cpp` | `dbde68ae4021e68881b2601b0d5234a3c49f1f3a362715cb4f416e513192e761` | `6ccb3eff5fd7f2fd773f24f265723b4b7191f8822207e618c9e051acc7f518ea` |

## Alias oracle

The primitive tests use one byte-backed DWORD cell as type, size and data for an empty replacement. Mock original query writes its size then zero type into that same cell, while the source wrapper has already captured the caller's capacity. For capacity4 the writer restores4, writes required size1(A)/2(W), and copies one narrow/two wide NUL bytes. On the selected little-endian Windows target this zeros the complete DWORD. The subsequent type-zero reread writes1 to the same four bytes. The expected final bytes `01 00 00 00` are therefore intentional rather than an empty-string expectation. The adjacent four canary bytes remain untouched.

With null data there is no NUL copy, so the nonzero required size1/2 remains in the shared type/size cell. With incoming capacity0, the writer returns234 without copying and leaves required size1/2. These cases distinguish actual rereads from cached type/output writeback.

The saved native instruction slices corroborate the oracle: narrow restore/resolve/reload/size store at0x1800091dd/91f5/91fd/9208 precede secure copy at0x1800092ce; wide equivalents are0x1800093d4/93ec/93f4/9402 and copy0x180009429. Narrow helper returns at writer call0x1800097ab before the actual caller type comparison0x1800097b7 and write0x1800097bc. The saved wide helper has the corresponding post-writer order. These are root-acquired fresh Evidence artifacts reviewed from disk, not a separate reviewer REA run.

ScalarCell uses memcpy for scalar loads/stores. The mock wide copy uses memcpy to output and never dereferences the byte storage as char16_t objects; aligned byte backing therefore adds no typed output aliasing violation. Both A/W cases are registered in the existing normal main.

## Matched integration coverage

The integration uses actual native_context::Owner, concrete Policy, Publication and explicit Domain permission scopes with controlled typed SDK originals. It does not invoke the outer host assembly wrapper or real system registry hooks.

- Recursive matched OpenW passes the identical PHKEY to parent and child. The child replaces0xb01 with0xb02; post-SDK tracking must reread0xb02, never track the superseded0xb01. This supplies positive typed OpenW nesting coverage missing from the preceding stage.
- The tracked-query original invokes nested real close. The child removes language membership, and the outer query must observe that current state and preserve its original return bits/type/size/data rather than override them.
- An initially untracked query invokes matched OpenW. The child adds queried membership and writes size1; the parent must see the new membership but restore its pre-SDK capacity16 before writing the six-byte `en_US` replacement. Using post-SDK size1 as capacity would fail this case.

Language paths avoid the client worker-wait route. Seeded membership is supplied before publication; the scenario does not mutate private sets externally after publication. Counts are coherent: opens3, queries2, closes3. Per-depth leases and PTD identity are inspected, each successive root call retires its acquired PTD, and all seeded/created logical handles are closed before ordinary provider/Context cleanup.

## Limits

No new production mismatch or normal native algorithm recovery was established. The scenarios remain uncompiled/unexecuted and use synthetic SDK boundaries. They do not establish actual Windows-induced reentry, arbitrary concurrent mutation, fabricated-key allocation failure behavior, original set-node/CRT/EH ABI equivalence, or every caller-buffer alias combination. No completion-percentage increase or new production-DLL validity claim follows from this static review.
