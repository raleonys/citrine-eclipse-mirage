# Native mutex owner and release order

Three fresh complete instruction replies from aligned REA6.8.0/Ghidra12.1.4 are saved inline and exported (three records, 18465 bytes). Session closed; original anadius64.dll SHA256 remains 9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. Static analysis only.

04a6ac body is seven bytes at 18004a6ac..18004a6b2: XOR EDX,EDX then JMP18004a6d4. This selects null deadline. Evidence ev_4347280839492f11ba4421f73c4198672732eced8d34e45394539dbd8e091fb2.

04a6d4 complete body is 305 bytes at 18004a6d4..18004a804. The type1 branch checks owner+48 at 18004a713 before calling the actual lock boundary at 18004a71c, then assigns owner at 18004a722 and increments count+4c at 18004a725. Null-deadline general path tests RDI at 18004a72d, reads owner at 18004a736 before actual acquisition at 18004a73e, then updates count at 18004a7a1..18004a7a7. Signed count<=1 publishes owner at 18004a7df; nonrecursive count>1 restores the previous count at 18004a7b7 and null deadline returns3 via 18004a7d8. Evidence ev_4fa832afb7d8df63c59e00e1725e2ccf4e419b9938014f3c15738dbb6d4f0ea4. Timed/try branches are visible in the complete body but are outside the existing selected source contract; they are not newly implemented here.

04a6b4 complete body is 31 bytes at 18004a6b4..18004a6d2. Count decrements at 18004a6b8. When zero, 18004a6be clears owner to ffffffff, then 18004a6c6 calls the actual release boundary using mutex+10. Evidence ev_cf0dc296024dde7ce00aa201ca3a686d0fff48453b9f9a88d1caf0cce6bf4baa.

The actual pre-lock owner observation can overlap another native owner's store. In portable reconstructed C++, plain overlapping access would be a data race. A lock-free relaxed atomic_ref projection at these existing reads/stores preserves the selected order and raw field layout while the same real SRW remains responsible for protected data and count. Each temporary atomic_ref ends before the next platform callback, especially before actual release can admit a distinct raw lock-owning borrower. This is an implementation memory-model choice, not an assertion that native MOV instructions use a C++ atomic class or that raw native Runtime/MSVC ABI/EH and every foreign SDK are equivalent.

No new function-count credit, concurrency schedule observation, runtime validation or production concurrent policy selection follows from this static evidence.
