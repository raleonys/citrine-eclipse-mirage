# Özgün RE paketi genel sapma denetimi — FIX13

**Başka sapmalar da vardı.** Bu rapor reddedilen altı kararın yanı sıra önceki geliştirme aşamalarındaki doğrulanmış farkları ve açık native eşdeğerlik alanlarını ayırır. Kaynağın tamamının orijinal DLL ile eşdeğer olduğu iddia edilmez.

Arşiv SHA256: `0734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9`. 2951 dosyanın tamamı sınıflandırıldı; 151 modül/payload grubu, 387 first-party source, 233 paket testi, 16 decompiler kaynak kaydı var. 4247 lexical function locator oluşturuldu. Bu sayılar fonksiyonların runtime'da çalıştığını veya bütün semantik farkların çözüldüğünü göstermez.

Doğrudan src eşleştirmesi 82 kaynak için exact origin SHA denetimi içerir. FIX11 karşılaştırmasında 54 dosya include/namespace/comment normalizasyonundan sonra eşleşir; 19 dosyada gövde farkı, 6 dosyada scoped extraction vardır. FIX13 ek-DLL onarımı için 3 source dosyası ve özgün test dosyası doğrudan paketten alındı. Test beklentileri değiştirilmedi.

## Düzeltilen bulgular

| ID | Önceki sapma | FIX13 | Kaynak kanıtı |
|---|---|---|---|
| R01 | Invented assets/<hash>, cache/provider-v2 and avatar nonce/session dirs | Plain SURGE/surge.png, remid_cookie, last_profile, last_avatar and original _AVATAR_UserId_Width spelling | Source original filenames + user-requested SURGE root; exact old root not restored; src/runtime/runtime_paths.cpp:35 |
| R02 | Existing module/default image required official byte seal, otherwise ignored/error | Existing file/sidecar is read-only and accepted without hash gate; embedded official bytes only missing-file fallback | User explicitly rejected hash gate; source asset bytes remain pinned; src/application/avatars.cpp:6 |
| R03 | 4KiB/raw-field/uint64/UTF8/control acceptance filter on profile cache | Source raw-byte reader; original split/application branch decides fields | anadius-online-source/online.cpp; anadius-file-source/binary_io.cpp; src/profiles/online_session.cpp:101 |
| R04 | 128-char/ASCII identifier restriction and Width normalization | No invented component/length/width rule; source filename text and OS path behavior | anadius-content-handlers-source/content_handlers.cpp; anadius-achievement-storage-source/storage.cpp; src/runtime/runtime_paths.cpp:39 |
| R05 | Missing achievement INI creation deferred until grant | Original startup status/missing-INI write branch restored | anadius-achievement-storage-source/storage.cpp; src/achievements/storage.cpp:102 |
| R06 | 256KiB log cutoff; 64 avatar files/8MiB each/32MiB total, PNG/IHDR gating and destroy cleanup | No replacement quota/cutoff/nonce/delete/PNG policy; direct provider path. Production disk logging stays compiled out | FIX12 user-rejected additions; original provider forwarding + latest user log decision; src/application/providers.cpp:37 |
| R07 | 1MiB config / 128 nesting application pre-parser, present already in FIX03 | No secondary scanner; original Config2 parser sees the full bytes | anadius-parser-source/parser.cpp; anadius-config-pipeline-source/pipeline.cpp; src/application/application.cpp:91 |
| R08 | 1MiB frame receive and Connection::process guard, present already in FIX03 | Source pending.max_size() boundary; duplicate message guard removed | anadius-server-source/server.cpp Session::read_0250e0 uses input_.max_size(); src/transport/service.cpp:111 |
| R09 | 32 live-client rejection present in FIX03 and carried into Asio rewrite | No invented client count cap | anadius-server-source/server.cpp accept path has no 32-client condition; src/transport/service.cpp:211 |
| R10 | 64MiB general read/output/preimage caps | No custom file-size caps; natural stream/string representation/allocation limits remain | anadius-file-source/native_io.cpp and binary_io.cpp; src/application/environment.cpp:44 |
| R11 | FIX12 NativeInput threw on short read | Source returns gcount and keeps caller zero-filled requested tail | anadius-file-source/native_io.cpp read_03b2a0; binary_io.cpp ignores gcount; src/files/native_io.cpp:44 |
| R12 | Game.KeyForLicense side effect stored pending until final/online profile | Runs at source Config2 Game application call point before later User keys | anadius-config-source/configuration.cpp call order; anadius-all-functions.pseudo.c FUN_18002a830; src/application/providers.cpp:31 |
| R13 | Extra DLL failure aborted startup; changed loader/error-mode/unload order | Original libraries.cpp + NativeSystemCalls used: dialog-and-continue, LoadLibraryW, process mode, forward unload | anadius-libraries-source/libraries.cpp, libraries.hpp, native_system.cpp; src/windows/dll_entry.cpp:41 |
| R14 | Provider suppressed thread error mode instead of source process mode | Imported source suppress_errors_0450a0 and SetErrorMode restoration | anadius-libraries-source/libraries.cpp get_error_mode/set_error_mode; src/profiles/online_session.cpp:37 |
| R15 | Crypt object reader stopped after 32768 code units | Read-until-terminator with natural address-overflow/readability bounds; no invented string cutoff | preloader-source/preloader_core.cpp crypt_query scans to NUL; src/windows/original_effects.cpp:23 |
| R16 | Application bypassed imported WindowsTimeSource with chrono-only wrapper | Imported WindowsTimeSource used for protocol time | anadius-crypto-source/crypto.cpp WindowsTimeSource; src/application/application.cpp:145 |
| R17 | FIX12 version-mismatch dialog was silently suppressed by native adapter | Original Win32 MessageBoxA restored; negative version case requires manual dialog acknowledgement in native operator flow | anadius-online-source/online.cpp dialog effect; FIX11 native adapter; src/profiles/online_session.cpp:89 |

R07–R10 sınırlamalarının ilk kısmı FIX03 kaynağında zaten görülüyor; yalnız FIX12 kaynaklı değiller. Yeni tests/integration/test_original_compat.cpp ve loopback karşı örnekleri bu sınırları doğru davranış diye sabitlemez. >1MiB config/frame, 140 nesting, 41 canlı istemci, >64MiB file, raw cache, short read ve key side-effect sırası kontrol edilir. Byte-level Windows kabulü henüz yok.

## Korunan, kullanıcı tarafından istenen farklar

- SURGE names/root/official embedded asset/new provider basename/trusted module directory — explicit user migration contract
- Production/Native/CRT log compile-out and fixed event privacy — explicit user migration contract; user later rejected invented cap values
- No legacy cache/root automatic migration/deletion and no last_avatar override of official/custom choice — explicit user data/avatar protection; intentionally differs from packet old-path/cache behavior
- Grant failure-atomic files/memory/prepared-event commit — explicit failed-write consistency/atomic write requirement; error-path semantics intentionally differ from original
- Pinned Asio/PEGTL/MinHook/TinyXML2/gulrak and public ABI — earlier explicit user dependency/public ABI requests; native parity remains separate

## Açık farklar ve kanıt sınırları

**OPEN_BOOTSTRAP — OPEN_NATIVE_EQUIVALENCE**
Original Bootstrap starts detached workers and has a synchronous from-main load then a waited worker load; candidate has a controlled bootstrap worker/prepare/hooks/activate and loads extra lists after activation. DLL lifetime, atexit/terminate and phase/timing are structurally different. Extra list per-file load/unload decisions are corrected; full phase schedule is not claimed identical.
Kaynaklar: anadius-source/bootstrap.cpp, anadius-config-startup-source/startup.cpp, anadius-thread-source/thread.cpp, src/windows/dll_entry.cpp, src/transport/service.cpp

**OPEN_FILESYSTEM — OPEN_SOURCE_MODEL_BOUNDARY**
Owned status adapter uses std::filesystem for native queries instead of the packet UsedStats/NativeStats Win32 path. Permissions, reparse/sharing/error behavior is not established equal. Packet UsedStats itself has an explicit unknown reparse-tag domain; blindly copying that research guard into production is not a completed native reconstruction.
Kaynaklar: anadius-status-source/status.cpp, anadius-status-source/status.hpp, src/application/providers.cpp, src/profiles/online_session.cpp

**OPEN_ENCODING — OPEN_NATIVE_EQUIVALENCE**
filesystem_widen uses AreFileApisANSI/MultiByteToWideChar; general UTF8/path conversions use std::filesystem. Packet conversion retry/default-character/error/locale branches are not all wired or validated against native original.
Kaynaklar: anadius-encoding-source/encoding.cpp, anadius-encoding-source/native_encoding.cpp, src/application/environment.cpp

**OPEN_CRT_THREADS — HOST_LIBRARY_SUBSTITUTION**
CRT/VCRT/thread models are replaced by MSVC static CRT/std threading and modern ownership; packet allocator/locale/TLS/exception/cleanup binary behavior is not all reconstructed. These are cataloged dependencies, not automatically missing application features.
Kaynaklar: ORIGINAL_PACKAGE_MODULES_FIX13.json, src/windows/startup_platform.cpp, src/application/events.hpp

**OPEN_ASIO_EXECUTION — OPEN_NATIVE_EQUIVALENCE**
Socket/execution/server packet models are implemented with requested pinned Asio. FIFO/cancel/OOM/stop are tested, but worker counts, scheduler timing, completion/error/allocator and original native interleavings are not fully equivalent by proof. No unsupported byte/client cap remains.
Kaynaklar: anadius-socket-source/*, anadius-execution-source/*, anadius-server-source/*, src/transport/service.cpp

**OPEN_XML_ERRORS — OPEN_DIAGNOSTIC_EQUIVALENCE**
Upstream TinyXML2 is kept pristine as requested. Successful-tree original depth=100 is retained in adapter; malformed/deep error precedence and all diagnostic strings are not proven same. The depth 100 is present in the original packet vendor; it is not an arbitrary FIX12 rule.
Kaynaklar: anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h, src/xml/reader.cpp

**OPEN_PROVIDER_ABI — NOT_RUN**
Version2 exports/vtable/synthetic model are not real renamed-provider native ABI acceptance; arbitrary provider-internal filesystem side effects remain Unknown.
Kaynaklar: src/profiles/online_session.cpp, tests/native/online_fixture.cpp

**OPEN_INVALID_DOMAINS — EXPLICIT_MODEL_DIFFERENCE**
Unreadable/invalid UTF16 forwarding, exception types, abstract OutOfEvidence guards and safe unload scans are owned model choices outside established ordinary input parity. No assertion that invalid/native fault behavior matches the original.
Kaynaklar: src/hooks/crypt_query.cpp, src/windows/original_effects.cpp, src/windows/unload_guard.cpp

**OPEN_PACKET_PROJECTION — NOT_FULL_BINARY_REBUILD**
Only part of the packet is direct source reuse. Scoped pseudocode/asm projections, constants, headers and renamed bridges are distinct from complete original memory-layout/global-state reconstruction. Unimported modules and function token mapping are listed, not silently treated as implemented.
Kaynaklar: ORIGINAL_IMPORT_COMPARISON_FIX13.json, ORIGINAL_FUNCTION_INDEX_FIX13.json

**OPEN_NATIVE_ACCEPTANCE — NOT_RUN**
Windows/MSVC/IOCP/CRT/actual DLL Golden/provider/consumer/Watcher acceptance not run in Linux. Static catalog and portable test counts are not substitutes.
Kaynaklar: docs/WINDOWS_FIX13.md

## Paket genelinde coverage

ORIGINAL_PACKAGE_FILES_FIX13.json her arşiv dosyasının sınıf/hash/import eşleştirmesini içerir. ORIGINAL_PACKAGE_MODULES_FIX13.json ve ORIGINAL_MODULE_COVERAGE_FIX13.csv doğrudan alınmayan dosyaları da gösterir. ORIGINAL_FUNCTION_INDEX_FIX13.json her lexical locator için source/line ve yalnız isim/RVA token eşleşmesini verir; token bulunması uygulama binding veya native eşdeğerlik kanıtı değildir. CRT/Asio alt modellerinin host runtime/upstream ile değiştirilmesi, doğrudan eksik oyun işlevi olarak sayılmaz; sınır açıkça yazılır.

Dosya stat/reparse modelindeki unknown domain, XML hata önceliği, bootstrap worker zamanlaması ve gerçek provider/native Windows sonuçları kapanmadan **“bütün sapmalar giderildi” denemez**. Bu raporun tam arşivi kapsaması, her mümkün runtime davranışının kanıtlandığı anlamına gelmez.
