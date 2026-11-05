# XML parse status audit — candidate closed without a source change

Result: **NO CONFIRMED STATUS-MAPPING GAP** in the examined encrypted-detection/version-retry paths. Original error7 is an attribute parse failure and error8 is an ordinary text parse failure, matching pinned tinyxml2 11.0.0 and the current reader. This closes the proposed numeric-status mismatch candidate only; it does not establish that all malformed-input acceptance, diagnostics, allocation effects or original embedded release match.

## Evidence and execution boundary

Own isolated REA6.8.0 identity/alignment and available Ghidra12.1.4 provider were verified. Opened immutable `C:/Users/Skysect/Downloads/anadius64.dll`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`, and retained twelve complete successful focused replies plus their exported bundle. The same original SHA-256 was independently rechecked after analysis. Own session exported12 records and closed successfully; root's independent IO session was not changed.

The first four requests, made under the initial shared-session assumption, returned target_unavailable because this agent had no open target. Those complete errors remain as `error-pseudo-*.json`. Root then explicitly authorized the isolated open; no unchanged failing request was repeated in the absent-target state, no configuration was changed, and no error was erased. Successful replies are separately named.

No original or reconstructed DLL was executed. No compilation, project tests, linking, production/test source edit, ZIP update, version.dll access or network search occurred. The existing release-compatibility disclaimer was treated as an uncertainty, not evidence of a defect.

## Native numeric producers and application consumers

| Native finding | Exact instruction / procedure evidence | Current source comparison |
| --- | --- | --- |
| Document constructed with processEntities1 and whitespace mode0 | `033d70` calls001830 at `0x180033db5`; saved pseudocode passes `(document,1,0)`.001830 writes entity byte+69, error0 at+6c, whitespace mode at+70. | `reader.cpp:26` constructs `XMLDocument(true,PRESERVE_WHITESPACE)`. |
| Parse returns document's actual numeric error |004280 clears prior state, handles C-string sentinel, copies input and returns doc+6c.033d70 calls at `0x180033e26` and tests doc+6c at `0x180033e2b`. | `reader.cpp:55–57` forwards Parse code and Error flag. NativeProfile `native.cpp:181–185` returns the code unchanged and projects error/nonerror. |
| **Attribute failure =7** | XMLElement ParseDeep0048b0 calls ParseAttributes004370. At `0x1800045d2` it sets EDX7 and calls005820 at `0x1800045da` after missing/invalid attribute syntax or duplicate-attribute failure. | Vendor header enum starts SUCCESS0 and places XML_ERROR_PARSING_ATTRIBUTE at7 (`tinyxml2.h:529`); vendor ParseAttributes reports it at `tinyxml2.cpp:1982`. |
| **Text failure =8** | XMLText ParseDeep004ea0 scans ordinary text for `<`; missing terminator/end-of-input reaches LEA EDX8 at `0x180004fda`, SetError005820 at `0x180004fde`. Its separate CDATA failure uses9. | XML_ERROR_PARSING_TEXT is8 (`tinyxml2.h:530`); vendor XMLText ParseDeep reports text failure at `tinyxml2.cpp:1253` and separate CDATA9. |
| Error8 selects first encrypted-frame detection |033d70 compares EAX8 at `0x180033e39`, rejects an already encrypted state at `0x180033e42`, searches exact `<LSX>`, and sets encrypted byte+0xc4 at `0x180033ee1`/`0x180033efb` before retry. | `anadius-socket-source/native_message.cpp:123–124` and semantic `anadius-messages-source/messages.cpp:161–163` use the same code/state/substring predicates. |
| Error7 selects one retry with optional empty-version erasure |EAX7 comparison at `0x180033f08`, retry flag test at `0x180033f11`; exact `version=""` occurrence removes10 bytes through034730 at `0x180033fbc`; flag set at `0x180033fc1` then retry. | Native source `native_message.cpp:125–129` and semantic `messages.cpp:165–169` retain the same one-time search/erase/retry. |

034730 is the generic owned-string replacement operation, not a different XML parser. The examined call passes remove length10, replacement length0, so it erases the selected literal. Error7 can also arise from other attribute failures; merely containing a valid empty quoted version is not itself proved to produce error7. Both native and vendor attribute parsing accept quoted values through matching quote termination and reject missing syntax/duplicates on the examined branches.

The existing reader test **source** already lists ordinary undecorated text as8 and malformed/duplicate attributes as7. Those tests were not run in this audit and are not presented as new validation.

## Address discovery provenance

To avoid broad symbol/string inventory queries, a read-only PE RTTI/vtable walk identified XMLText and XMLElement ParseDeep candidates: XMLText vftable `0x18007fb28`, slot+80→004ea0; XMLElement vftable `0x18007fd78`, slot+80→0048b0. The algorithm and observed addresses are recorded in `selector-discovery.json` as analyst-inference discovery, not REA Evidence. The behavioral conclusions above rely on the subsequent fresh Ghidra function replies, including the actual7/8 setters.

## Complete Evidence index

| Query | Evidence ID |
| --- | --- |
|001830 pseudocode|ev_33d474ce59dc540e525881e601baecdba1b1baa88683b16ba4a54fc16adc6b03|
|0032d0 pseudocode|ev_45841199a92e13a86765d97f8da70936d887d99fbc99b08c57e7fdf364952d4d|
|0041b0 pseudocode|ev_9297f27b74fa18a5ff1e29dc29b6d654d819d9f56879c643671b94749796414c|
|004280 pseudocode|ev_dbe6a134b333e4863c4757777a1f474c05ad6608f142cb31cdf85d17f2a9d78c|
|004370 pseudocode|ev_08c398e2a94fb6a5ac9e02e22da9b3a13570c81d4a360c1757fa3b0a2e814579|
|004370 instructions|ev_f63af54ede39fc1df1197edb7f9a7865dcc2356de7bcce9d5c0c5eb303ac2849|
|0048b0 pseudocode|ev_934aecdfc4ebd711b7e6f2297056c118df2c84f8b0bd9e5e91fe4a65b065130f|
|004ea0 pseudocode|ev_ecae12fd5d81771e551f0e3f2caaff259d43840d65fbcef940df2ade6a42ff9a|
|004ea0 instructions|ev_96900d0fe1d492a3871fc15f663d6d2bd7f400482a3e557b243ec5c7261ef716|
|033d70 pseudocode|ev_5639af3e8455b548a57d80c017576f5410353d221f993955d97e95571e124517|
|033d70 instructions|ev_4470b55e68a26f1c364675e1e592813753e285529837d06b074ddc3854004d46|
|034730 pseudocode|ev_9ace592be68c655d2007e7e480824476229ced0b3aec6faf4ea3c5326922c86a|

All function replies preserve provider/target identities, full pseudocode or instruction lists and complete limitations.0041b0 reports an unresolved virtual ParseDeep tail; the actual producer bodies were queried separately rather than treating that warning as a completed call-graph proof. Terminal/no-return warnings in033d70/034730 remain explicit in the saved replies.

## Reviewed unchanged source SHA-256

| Source | SHA-256 |
| --- | --- |
| reader.cpp |0c80c24764ad925ada9ca2c485a984ba490f07d7a596db1eb12c2f18af817d66|
| reader.hpp |f73003ab563784f7dbc51db9bf853b28f08c80f2cc398a67c49145a3a7bd118e|
| vendor/tinyxml2/tinyxml2.h |aed790f764e4663f6cb99839648dd642c7e7a730d12c67d3ab7ca075f0ac67c0|
| vendor/tinyxml2/tinyxml2.cpp |95f64e4cb06c9e147d5b12e8860c88898944858dfbc38fed92ebf918f1436554|
| anadius-native-profile-source/native.cpp |750f958bdc0d8209bafbc98f00325b8081892eb10ae0fe2ed63d8c3ccb933758|
| anadius-socket-source/native_message.cpp |362b81b7980cd9146564a4ead2618f6f8327b3fe10126b5fb4610b2cc92298cd|
| anadius-messages-source/messages.cpp |2eda8d8cdd82a783b694ef1c8f7ad97b643d3b50589694a76239c14e1a970f9c|

No production patch or numerical whole-DLL completion credit is justified by this candidate audit.
