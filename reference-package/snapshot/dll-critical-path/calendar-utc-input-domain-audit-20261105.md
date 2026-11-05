# Calendar/UTC business-input audit

**Bounded negative:** no concrete numeric/date-domain discrepancy found in the twelve branches below. This reuses retained native evidence and reads current selected sources; it does not reopen the completed ClassicIso suffix correction. No source/status change, REA session, original access, build/test/link, execution or packaging was needed. Historical validation results were not rerun.

Selected callers are achievement Grant `030010` → UTC `041b60` → calendar `041ff0`; persisted Grant import `01eb00` → parser `041e10` → calendar; information `02da90` and UTC response `02fea0` → UTC with null epoch/mode0; license `0320c0` ticket filename → UTC with null epoch/mode1. Default Content/Information/Licensing options select HostClock/HostFormatter; default config persistence selects ClassicIso.

| Branch/domain | Native evidence and current behavior |
|---|---|
|1. Positive month overflow|`042000…04201f`: signed division/32-bit normalization; calendar.cpp:19 models it without validation.|
|2. Negative month, including INT_MIN|`042033` wraps `11-month` before signed division; calendar.cpp:23 uses explicit wrapping helpers. No signed-overflow UB.|
|3. BC/zero year|`04205a…04207e` uses signed truncation toward zero, not floor-era Gregorian normalization; calendar.cpp:31 preserves it.|
|4. Day/clock widths|Day intermediates wrap32 through `0420ee`; `0420f1` CDQE and later MOVSXD widen day/hour/minute/second. calendar.cpp:40–41 matches; final64-bit range cannot overflow for int32 fields.|
|5. Leap/invalid civil dates|Parser field bounds, not civil-date validation; e.g. 2000-02-31 normalizes arithmetically to March2, and second60 is accepted. No extra rejection was added.|
|6. Zero-date sentinel|`0000-00-00T00:00:00` remains stored text; parsing invalid month/day yields epoch0. It is not rewritten as a normalized date.|
|7. Pre-epoch value|1969-12-31T23:59:59 computes signed -1; storage.cpp:89 casts to uint64 and writer.cpp:32 serializes that bit pattern as unsigned decimal, matching retained `01eb00`/`01f9a0`.|
|8. Grant progress arithmetic|`03062a…030638` uses signed eligibility and ADD32; content_handlers.cpp:125–127 uses wrap_add, then timestamps. Unknown nonpositive Total skips the update as native does.|
|9. Clock failure input|`041b9e` time is followed by `041bad` gmtime even for raw -1; utc.cpp:28–29 adds no early rejection.|
|10. Missing gmtime|`041dc6…041de0`: optional epoch0, empty string, no formatter; utc.cpp:30–33 matches.|
|11. Success ordering/mode|`041bd9/041bde` recomputes/writes epoch before formatting; `041ca2` tests the mode byte for any nonzero value. utc.cpp:34–36 preserves both and returns partial/empty formatter output unchanged.|
|12. Response/filename routing|Calls `02e2fe` and `02fed4` pass null epoch/mode0; `032688` passes null epoch/mode1. handlers.cpp:110, misc_handlers.cpp:95 and licensing-effects.cpp:106 match. Expiration/release-date config values are passed through as text, not reparsed.|

Authority: calendar `evidence.json` assembly EID `ev_9d8c05e7881ace56f5053a11d8c9b6148320e73c05a8db0fd152a3ad78b0619c`; achievement-runtime `flow-audit.md`/`evidence.json` UTC EID `ev_7a1e946a32600dde2f69f281612ad78d0f37819a97d59a29a134a8f95817603d`, Grant EID `ev_62f5fc49da4ec38f00325c7e0b857c87556da27030afd17618aec1e4238cb16c`; storage/parser and license/handler retained indexes supply the caller bindings. Exact current-source/evidence hashes are in the adjacent JSON.

Host CRT clock range, arbitrary locale/time_put formatting, native tm/class/exception ABI and runtime differential remain bounded compatibility limits, not newly demonstrated defects. No focused unanswered numeric branch justified another native query in this audit.
