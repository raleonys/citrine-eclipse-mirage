# Large-string allocation headers — fresh native findings, 2026-11-04

REA6.8.0 identity and original anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` were verified before Ghidra12.1.4 queries. Ten complete replies are retained in `rea-evidence-bundle.json` and individual files: eight supporting records and two incidental address checks listed below. Session closed after export. No target execution or mutation.

## Actual selected source restriction

ProfileSettings/Dependencies default headers=256 reaches actual session_profile::Strings. Its former track() searched a fixed ledger and threw when all rows were live. Strings tracks aligned allocation headers for string capacities >=4095. A valid backlog of a few hundred large pending responses/events can therefore fail even though the actual output/event deque supports growth. This is a selected production adapter restriction, not an abstract unused interface or a native handler count.

## Supporting native observations

-009eb0 copy: string length determines allocation; >=4096-byte storage receives the aligned data pointer and raw allocation base stored at data-8. Native normal/aligned allocations are at0x180009f37/0x180009f7e, with the counted copy at0x180009f98. No global256-header inventory is used by this body. Existing native length/overflow/invalid-parameter branches remain part of the retained Evidence.
-028270 queue insertion: grows through026670 at0x1800282b8, moves the string prefix into the output queue and starts02a500 only for the first pending element. It does not impose the source ledger's256-row bound.
-025a70 completion: normal success reads the deque head, obtains a large string's allocation base directly from data-8, checks alignment and frees it at0x180025af2 before decrementing the queue and scheduling the next write. Error-message retirement similarly frees its own aligned base at0x180025c0c.
-00aa50 string destructor: obtains raw base from data-8 for the large allocation case, validates the <=31-byte adjustment and frees at0x18000aa8a, then resets size/capacity/NUL.

Complete pseudocode and raw instruction records for all four functions are supporting evidence. These establish the examined per-string metadata paths; no full CRT/allocator/EH ABI or unlimited successful allocation claim is made.

## Incidental query correction

The initial017a30 query resolves to the exported guard/return leaf shared by CDX/RUNE/anadius; it is not a string free/completion function. A request at02a5d0 resolves to the containing02a500 send starter, also not the free path. Both complete replies remain retained for traceability and are excluded from the supporting eight-record count. The correct025a70 and00aa50 paths above were queried before deciding the change.

## Required source coherence

The ledger is a source ownership/projection aid. Growing its vector alone would invalidate two cached span fields: FrameReadContext::allocation_headers and Effects' WriteFinishContext::allocation_headers. Other selected response/message/Runtime views retain those contexts by reference. Five primitive lookup loops consume the field and finish their header iteration before opaque calls. The selected correction must publish both new spans before old metadata storage is freed; payload allocation bases and ownership do not move.
