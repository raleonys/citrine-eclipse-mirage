# Environment reference ledger: focused native comparison

Static-only evidence collected on 2026-11-04 through this agent's owned MCP target lifecycle. `binary_session` verified aligned REA 6.8.0 at `C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs`; the selected provider is Ghidra 12.1.4 from `C:/tools/ghidra_12.1.4_PUBLIC` with Java 25.0.5. Full session replies are retained in `session.json` and `session-active.json`. Original `C:/Users/Skysect/Downloads/anadius64.dll` SHA256 before opening and after closing was `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, matching `open.json` and every Evidence subject. Original bytes were not changed or executed. No version.dll, network, compiler, linker, test runner, or packaging action was used.

## Bounded conclusion

The inspected constructor, move, copy caller and prepare caller operate on individual environment projections through type/query/factory/copy/move/destroy functions. Their observed code does not admit projections through a global 4096-row reference registry. The source `Environment` default capacity 4096 and `Control::rows` allocation/copy exhaustion checks are host metadata choices; removing that adapter cap can preserve the selected operations without claiming native container or ABI identity.

The actual nonempty native operations and executor vtable targets remain unresolved. Consequently this evidence does **not** prove a complete native copy/prepare implementation, absence of limits inside those unknown virtual callees, or unlimited native capacity. Other resource/counter/allocator boundaries remain outside this comparison.

## Address evidence

- `0x180023dd0–0x180023f2f`: actual constructor. `023df8/023dfc` calls source schedule slot0, then `023e09` compares the type through `04d2d0`. The nonempty branch caches executor slot `+0x58` at `023e67/023e6b`, queries operations slot `+0x18` at `023e6f–023e7a`, and invokes that cached factory at `023e87`. It invokes the returned operations' move slot `+0x10` at `023ebc`, performs guarded temporary destruction at `023ef0`, calls `023cb0` at `023efa`, then destroys the intermediate at `023f0b`. No row-count admission check appears in this observed body. Unknown indirect targets remain unknown.
- `0x180023cb0–0x180023d19`: actual move primitive called above. `023cc0–023ce2` copies operations/schedule identities and replaces the source with empty sentinels; `023cee` calls destination operations slot `+0x10`. `023cf1` clears source state, `023cf9–023d08` reloads/copies executor after that callback and resets the source executor. No per-reference ledger traversal/allocation or 4096 admission appears in this body.
- `0x180028d30`: actual message-dispatch copy caller, selected from the source's native selector rather than a guessed nearby symbol. `028e18–028e3a` loads the owner environment operations/schedule and invokes copy slot `+8` from source environment `owner+0x78` into a first local. `028e3d–028e44` reloads executor after that copy. `028e48–028e64` invokes copy slot `+8` again into a second local. `028e7d` dispatches through `021420`, and `028e8e/028e9c` destroys second then first. This is evidence of two per-projection copies and paired cleanup, not recovery of the unknown copy target itself.
- `0x1800229d0`: actual send scheduling/prepare caller. The fresh pseudocode caches executor slot `+0x40`, queries environment operations slot `+0x18`, invokes that factory into a local, moves via operations slot `+0x10`, conditionally destroys the moved-from temporary, schedules borrowed or owned work, and destroys the remaining local. It does not add a 4096 reference-row gate. Its virtual targets were not guessed or decompiled.

`028eed/028f29/028f6d` compare string allocation byte counts with `0x1000`; these are the string large-allocation/free backpointer paths, **not** a 4096 environment-reference count. `021770` was also read as a narrow path-navigation query; it is a message-thread closure trampoline and its string cleanup contributes no environment ledger proof. Its full reply remains retained, but it is not counted as a recovered copy/prepare body.

## Complete retained queries (8)

| Reply file | Evidence ID |
|---|---|
| pseudo-180023dd0.json | ev_ea109552dd64ba204d1b426aae3a069cd59ae7f679c6f0bf6e9b27e2fafd2697 |
| instructions-180023dd0.json | ev_a796bdbc87106dd39b97f8933a91e478c3113ea2ac400041f0e295bcba20e562 |
| pseudo-180023cb0.json | ev_6e1345372ea03b6c6d0fa80ed13e6d847513b82e3fc968d795d0b3d1f543924c |
| instructions-180023cb0.json | ev_cd85a47c1ecc95b2c78a406e8478fe137fdc65a60d22d94b04b2f4dfa8d0ec65 |
| pseudo-180021770.json | ev_4e07be3971e944c0b4b45203a5ef8c2ce26e9cc9360c485270be3ba9f15b82ee |
| pseudo-180028d30.json | ev_d541cc789ed813d1046213cefe05b6e94a66cd62b9092a7cccd3404931f2390c |
| instructions-180028d30.json | ev_c1e35ff0c5c57064a1ddf6f2ba08016bd1eecedf05e9c21ef99d15c95c75e2c7 |
| pseudo-1800229d0.json | ev_87d2582ea82ea7330341c295f540cf69fe0cd98477f562b2e038f53705f90cee |

The exported `rea-evidence-bundle.json` contains all 8 records (61073 bytes). Export reported zero recorded Unknown objects; that does not mean the virtual targets or other limitations above were resolved. `close.json` confirms successful close (`result:null`); no shared target state was left open. Provider limitations and complete inline Evidence are retained in every reply. Pseudocode is a Ghidra reconstruction, not original C++ or observed runtime behavior.
