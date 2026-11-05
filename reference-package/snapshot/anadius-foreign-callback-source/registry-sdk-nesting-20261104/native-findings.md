# Registry SDK-phase nesting: native findings

Date: 2026-11-04. REA6.8.0 server identity verified before opening original anadius64.dll with Ghidra12.1.4. SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Six complete fresh Evidence records, full individual tool replies, session identity and open/close replies are retained in this directory. The native session was closed after export. No binary was executed or modified.

| Body | Typed original call | Shared-body phase |
| --- | --- | --- |
| RegOpenKeyExA00f800 | 0x18000f823 -> slot0x1800a3840 | Configured-name compare starts at0x18000f867; subsequent match fabrication/tracking occurs after original returns. |
| RegOpenKeyExW00f960 | 0x18000f983 -> slot0x1800a3848 | First configured-name helper at0x18000f9c7; match processing follows the original. |
| RegQueryValueExA00fac0 | 0x18000fb01 -> slot0x1800a3858 | Caller capacity is snapshotted before original; replacement helper009520 at0x18000fb35 follows it. |
| RegQueryValueExW00fb60 | 0x18000fba1 -> slot0x1800a3860 | Same scalar snapshot pattern; replacement helper009850 at0x18000fbd5 follows original. |
| RegCloseKey00f5b0 | 0x18000f6bb -> slot0x1800a3850 | Fabricated membership lookup completes before the real-original branch. Client set root is reloaded at0x18000f6cb and language set root at0x18000f75e after successful return. The fabricated-free branch does not call original. |

These function bodies contain no reconstructed Domain active/reentry guard. The observations establish call/body ordering, not that Windows actually causes reentry. They do not establish safe concurrent set mutation, native allocator/CRT/EH equivalence, or general same-thread reentry from compare/allocator/log/wait phases. Ghidra body/flow limitations remain attached to each original Evidence record.

Before this stage, source general admission rejected a second same-thread registry body even while the parent was inside the original SDK function and held no live source set operation. The intended bounded source correction permits only such SDK intervals. Root admission and its PTD remain active; child registry bodies execute sequentially on that exact actor. Other threads and non-SDK body reentry remain outside this correction.
