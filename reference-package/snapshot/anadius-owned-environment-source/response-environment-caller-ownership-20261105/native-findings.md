# Response environment preparation: fresh static evidence

Evidence collection only at this point; no response source change, compiler, tests, link, DLL execution or ZIP update. Original anadius64.dll SHA-2569e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

REA6.8/Ghidra12.1.4 read_function_instructions, Evidence ev_a50f8ce27dcc6920425abd344e46e1fc3cb4f854d5b55e78e8f5336c92da6121. Complete analyzed body is one inclusive range180021420..18002176d,846bytes. The full reply is saved and exported as one record; native session closed. Ghidra marks180021769→18005c974 terminal/no-return and excludes fallthrough. That is a provider flag, not independent proof of callee termination or native EH behavior.

- Incoming task function/string/object/control are moved into locals at180021458..180021493, with source string/object/control cleared BEFORE any executor/cache/query lookup. A new preparation boundary must stay after this already-consumed prefix.
- Source executor read180021497, slot20 cached18002149b, source query18 called1800214a7, then cached factory20 called1800214b8 with literal180084425.
- First inline move callback1800214f8 is followed by state reset1800214fa and live executor load/store1800214ff/180021504, then source executor sentinel store180021512. The moved-from temporary's LIVE operations/destroy target is read180021517/18002151c and guard compared180021526.
- If the temporary is destroyed at180021530, first.executor is reloaded at180021532. Otherwise the earlier executor cached in RBX is unchanged. Slot88 is cached180021541 BEFORE first.query18 at180021557, then the cached function is invoked180021566.
- The second factory output is moved at180021595, then the state/executor tail180021597..1800215a3 completes before LIVE guard lookup1800215a7/1800215ab, compare1800215ae, conditional destruction1800215b8.
- Intermediate is moved again at1800215e7, followed by state/executor tail1800215e9..1800215f5. Intermediate destruction at18002160c is UNCONDITIONAL, including the sentinel guard case.
- Only afterward is the final scheduler read18002160f and borrowed slot18 tested180021613/180021617. Response publication and final environment/task cleanup therefore remain separate from this preparation segment.

The prospective source boundary is pure response20/88 preparation after incoming-task consumption and before publication, with the first/final outputs and three actual internal temporary addresses owned through complete field/callback tails. It must not equate20/88 with factory58 or grant Scope scheduling authority merely from a projection capability. Explicit actor context should precede source field access inside this selected boundary, while the native task prefix remains earlier. No implementation or runtime equivalence is claimed by this evidence note.
