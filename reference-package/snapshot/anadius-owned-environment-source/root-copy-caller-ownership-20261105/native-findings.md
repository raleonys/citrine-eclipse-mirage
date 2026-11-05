# Addressed static findings

Original anadius64.dll SHA2569e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c. REA6.8/Ghidra12.1.4 Evidence ev_c1e35ff0c5c57064a1ddf6f2ba08016bd1eecedf05e9c21ef99d15c95c75e2c7, complete saved reply and one-record exported bundle, session closed.

028d30 analyzed body ranges180028d30..180028faf,180028fb1..180028fbb,180028fbd..180028fc1 are656 totalbytes over658spanbytes. Provider terminal flags at180028fab/180028fb7/180028fbd are not independent callee termination or EH proof.

First operations/schedule stores180028e1f/180028e2a precede cached slot08 lookup180028e2e and call180028e3a. LIVE source executor load180028e3d and destination store180028e44 FOLLOW return. Second stores180028e4c/180028e54, cached lookup180028e58, call180028e64, LIVE load180028e67/store180028e6b repeat that sequence. These addressed instructions require whole-caller ownership; moving executor assignment into raw slot08 changes the measured order. Response021420 follows at180028e7d and was not moved.

This supports the selected source refactoring only. No target execution, runtime overlap, new recovered-function credit or original-source/ABI/EH equivalence is claimed.
