# Fresh native I/O allocation basis — 2026-11-04

REA6.8.0 / Ghidra12.1.4, immutable anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Eight full query records plus session/open/close and exported Evidence bundle are retained. No original or reconstructed DLL was executed; the root analysis session is closed.

Observed021d10 requests0xe0 bytes at `0x180021d46` and calls0269b0 at `0x180021d4b`; the saved pseudocode identifies WSARecv at import call `0x180021f34`, followed immediately by WSAGetLastError at `0x180021f3c`. Pseudocode `ev_46edbce6a8ed2be2d72d414c87fabdc9f2540e6127c6291a580a213ab5ecd706`; instructions `ev_3754219ace1b2f30c24cc15acf10d1ead7f4edd05d14bbf53fa25fbc4381e7cf`.

Observed021fd0 requests0xd0 bytes at `0x18002200e`, calls0269b0 at `0x180022013`, then WSASend at `0x1800221b4` and WSAGetLastError at `0x1800221bc`. Pseudocode `ev_f876c51f0f83eff9b423b31fe89b880b19de3a1b875b68b138f88f458e49ae4e`; instructions `ev_bf70fd500bbd2572d91cbbbf576f52708444ad9885147adc724278195846e5f0`.

Observed0269b0 obtains the worker TLS state, then passes the requested size and alignment1 to021820. It contains no64-record admission table. Pseudocode `ev_1f48bdc98d18d8509f994418fce5fb6018a921300f1c38f788ece79272e0260f`; instructions `ev_2977674f396c7af551b4a98cdfd9e48a067dcc52107db70b2c839e98cde768b9`.

Observed021820 scans two worker-cache slots for a sufficiently large/aligned allocation. On no match it may free one cached block (`0x1800218c2`) and allocate `(requested+3)/4*4+1` bytes through04b0bc at `0x1800218cf`, retaining the marker byte. This two-slot reuse cache is not a limit of two or64 concurrent operations. Pseudocode `ev_ac55d6f1319dacc2d1d44612477186f0ef8fdc6c392d276cf9f1c844db8800bb`; instructions `ev_40841f5574172cffd243949fe5d9b6af4243a4560bb55bc3c43b4e107f7ffa03`.

Inference: OwnedIoMemory's selected64-record pre-allocator bad_alloc is an added reconstruction constraint on these observed per-operation paths. Removing it requires coherent storage/generation/packet registries, stable old Record/OVERLAPPED identities, repair of all receive/send/accept packet pointers and correct persistent/transient view lifetimes. A partial vector resize would not be sufficient. Native allocation/OS failures and exact allocator/EH ABI remain outside this static finding; no unlimited-resource guarantee follows.
