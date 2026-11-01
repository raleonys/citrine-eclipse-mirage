# B7 — Temel Services ve configuration platform bağlantıları

Bu aşama üç Services ve altı configuration bağlantısını somutlaştırır. Yeni native fonksiyon gövdesi veya tam DLL eşdeğerliği iddiası yoktur.

| Alan | Önce | B7 sonrası |
|---|---:|---:|
| Bağlanan Services metodu | 8/17 | 11/17 |
| Açık Services politikası | 9 | 6 |
| Bağlanan configuration metodu | 7/21 | 13/21 |
| Açık configuration politikası | 14 | 8 |

Bu oranlar bileşimdeki metot bağlantılarını sayar; DLL'in tamamlanma yüzdesi değildir. Services tarafındaki yedi mevcut uygulama algoritması tekrar yazılmaz. Genel CRT leaf kuyruğu ve numeric0033c0 beklemededir; öncelik 1→2→3→5→4 olarak korunur.

## Kullanım ve yaşam süresi

DomainStorage hazırlandıktan sonra make_memory() ile ApplicationMemory alınır. SourceServices bu sağlayıcıyı ve açık kalan Services sahibini alır. Worker yükleri artık NativeThreads ile aynı recovered heap/PTD/error/new-handler alanından ayrılır ve aynı alana bırakılır.

ConfigurationPlatform alttaki Dependencies sahibini sarar; B5 Configuration dış katmanda kalır. bind_configuration_platform önce, bind_configuration_and_console sonra uygulanır. Böylece B5'in yedi bağlantısı korunurken altı yeni bağlantı eklenir.

Kapatma sırası: B4 admission stop → B3 worker/FLS join → application cleanup → Runtime ve sağlayıcıların yok edilmesi → ApplicationMemory referanslarının bırakılması → DomainStorage.close(). Başarısız HeapFree bloğu kayıtlı tutar; açıkça tekrar denenebilir. Sonucu bilinmeyen native çağrı veya canlı tahsisle sağlayıcı yıkımı terminaldir. Hiçbir otomatik kaynak kaybı veya çift free varsayılmaz.

B6 domain.hpp ve domain.cpp bu aşamada açık before/after SHA kaydıyla genişletildi. Önceki diğer 724 kaynak dosyası byte düzeyinde aynıdır; yedi yeni dosyayla toplam 733 kaynak vardır. Önceki B6 doğrulama kayıtları tarihsel kaynak sürümüne aittir; güncel kaynak ortak B7 build'iyle tekrar sınanır.

Kontrol API'leri controller thread'e aittir. Bellek çağrıları hazır domain'in sahip olunan PTD worker'larında kullanılmalıdır. Shared fallback errno ve genel eşzamanlı OOM/EH davranışı doğrulanmış sayılmaz. Allocator bookkeeping ve C++ hata tipleri seçilen yeniden derleme politikalarıdır; özgün MSVC EH/nesne ABI'si değildir.

## Doğrulamanın sınırı

Çekirdek ve entegrasyon testleri kendi EXE/worker/terminal alt süreçlerini kullanır. Asıl DLL, oyun veya yeniden üretilmiş DLL çalıştırılmaz; orijinaller değiştirilmez. Gerçek kullanıcı CFG/token/achievement içerikleri ve AV tarafından engellenmiş version girdileri okunmaz. SDK yol sorguları ve storage/foreign etkilerinin hangi testte gerçek/enjekte olduğu test raporunda belirtilir.

Native kapsam 487 mapped adres, 491 karşılaştırılabilir derlenmiş etiket, 124 normal gövde iddiası ve 52 tarihsel kapanıştır; B7 bunları artırmaz. Tam DLL hazırlığı BLOCKED. Sonraki B8 configuration dosya/persistence işlemleri ve desteklenen kalan Services politikalarıdır.


---

# B7 Services allocation and final return

`SourceServices` selects three existing `bootstrap_owner::Services` routes:
`allocate`, `deallocate`, and `guard_return`. With B3's six application routes
and B4's two registrar routes this yields 11 bound routes and six remaining
policies: log insertion, log finish, thread error, CRT exit-wait cleanup,
terminate logging, and unexpected-terminate-return handling. Unselected methods
continue to the mandatory owned lower provider. `00e310` already reaches the
existing host initialization body through B3 and is not counted again.

## Actual domain ownership

Two prior production files, `anadius-thread-domain-source/domain.hpp` and
`domain.cpp`, are deliberately extended. `DomainStorage::make_memory()` may run
only in Ready on the original nonreentrant controller thread. It constructs an
`ApplicationMemory` with the same provider-lease mechanism as B6's actual thread
factory. The lease retains the root and prevents FLS/lock/runtime teardown while
the memory provider lives. No heap state or effects reference escapes publicly.

`ApplicationMemory::allocate` uses existing `malloc_base_06a308` and
`call_new_handler_0634b0` with that root's actual heap, error/PTD binding,
encoded handler, and ACRT lock 0. Nonzero allocations use flags 0; the existing
malloc source normalizes zero to one. Deallocation calls existing
`free_0668c0` through a stack observer of the same effects object. Size is ignored
as in the retained delete route.

Per-provider source tickets are allocated before calling the native allocator.
The ticket gate is released across native calls and user new handlers. It is
not the original CRT allocator layout or a new native function. Known null
outcomes remove the ticket; a throwing handler with a known null result may
propagate. A throwing injected heap boundary with an unknown outcome poisons
and retains the provider. Foreign, already released, concurrently released, or
other-provider pointers are explicitly outside this bounded ownership domain.

Known HeapFree failure still executes the recovered errno mapping, retains the
block, and raises a selected compiler `system_error` so the controller can retry.
This retry signal is an intentional rebuilt-adapter difference from the native
void free route. An unknown free outcome cannot be retried. Destructor with any
ticket or poison is terminal. This also exposes the original bootstrap's
unreleased payload on failed thread creation; it does not silently invent cleanup.

Worker calls do not read mutable domain phase. Callers must use the original
controller or workers created through this
same domain's recovered factory, with a present domain PTD. Foreign/CRT-only
threads and shared fallback-error/OOM concurrency are not established here.
B3/B4 must exclude new admission, join every worker including FLS destructors,
and release Runtime/Services/thread
providers before root close. Memory provider destruction must not race calls.
Direct users must enforce that same contract. Provider existence is not an
automatic worker join or automatic cancellation. Concurrent allocation does not
establish general CRT, new-handler reentrancy, DLL unload or loader-lock safety.

## Selected exception and extent policy

Exhausted handler uses the rebuilt compiler's `std::bad_alloc`; size `SIZE_MAX`
uses `std::bad_array_new_length`. This matches the source-level error choice,
not original MSVC exception object/EH ABI. Extents above PTRDIFF_MAX but at or
below original malloc maximum `0xffffffffffffffe0` remain explicitly rejected
by the existing native byte-array bridge; no enormous synthetic allocation is
performed. Above that original maximum the malloc-failure/handler path performs
no native allocation. No over-aligned allocation or cross-provider free support
is claimed. Internal tickets use the compiler CRT independently of returned
application storage.

## Retained static evidence

Target SHA-256 is
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
No new native session, decompilation, original execution, or original-byte write
was needed; retained full assembly and existing recovered source were inspected.

| Address | Static observation | Assembly Evidence ID |
|---|---|---|
| `0x18004b0bc` | malloc then explicit handler retry; exhausted SIZE_MAX branch | `ev_4b9d54867bdd71cc2cd07945df16f49f84fdecaa3c3535386e97bd20f64624ec` |
| `0x180060044` | `_malloc_base` jumps to `06a308` | `ev_660e9d43688c584c1b643f3db3b1648df0d8479f4a4c0042af86c26725db0708` |
| `0x18006a308` | flags 0 heap allocation, zero normalization and malloc failure | `ev_fec2970bf0d499ba0715ef0f1dd0028bfcd6d27a246fd3e25106a2d5f9825b64` |
| `0x1800634b0` | explicit selected handler with requested size | `ev_feeb37c1659fa2ad6ed36fb2e17ad1622a1893e7adb59a2a704c97c32c5c11ba` |
| `0x18004b0f8` | jumps to `04b788`, then `05caec`, then `0668c0` | `ev_c742d3bdc5602e2293140e3408473befa826c5e7f02f988ef02afb1709aa3adf` |
| `0x18004b788` | intermediate delete thunk | `ev_9c1d7141369ea0b1b9c0ca37156bcfddbb25e1a30aaf0a61f46b64920290d220` |
| `0x18005caec` | final thunk to free | `ev_dc2e8860cfa1a06b464563b31c9a5f586dc49fb16cd3579156e95db913e59895` |
| `0x1800668c0` | null no-op; free failure maps LastError through errno | `ev_c99f204c542cccdad396d7bc71515f1b5731e73657cfccf84d0a121761e73e4a` |
| `0x180017a30` | exactly `RET 0x0` | `ev_4ef5a2bb53b7728331fdd5c8debedafc4f7ebcce78b3dd4a30154920bfc23a9d` |

The actual RET justifies an empty source `guard_return`; it does not implement
PE Control Flow Guard policy. Native thread-error `04a648` uses an indexed
MSVC error table and C++ throw machinery. It remains lower-provided rather than
being guessed from a Win32 error number or the compiler's platform errno table.

These are composition-only bindings and do not add mapped native bodies,
native closures or original ABI equivalence. Full DLL readiness remains open.

---

# B7 configuration platform scope

The new `ConfigurationPlatform` binds **six** of the fourteen policies left to
B5's lower provider. B5 remains the outer `Configuration` wrapper, preserving
its seven routes: **13 of 21 selected routes bound, 8 lower policies remain**.
No new virtual provider interface or original native-body closure is claimed.

| Configuration route | Concrete implementation and observed boundary |
|---|---|
| `executable_path_03c6d0` | `GetModuleFileNameW(nullptr, ...)`, retained helper `03c7b0` |
| `program_data_03c920` | `GetEnvironmentVariableW(L"ProgramData", ...)`, retained `039bc0` |
| `local_app_data_03c6f0` | `GetEnvironmentVariableW(L"LocalAppData", ...)`, retained `039bc0` |
| `widen_0145d0` | Explicit UTF-8 codepage65001 through existing `convert_narrow_048ae8`, two passes and first-pass length |
| `narrow_path` | Existing `filesystem_codepage_048ac0` and `narrow_006e60`, matching inline `01c870` filename conversion |
| `resolve_path_0192d0` | UTF-8 conversion then existing `is_absolute_0195b0`, `parent_path_019820` and `join_016b00` when relative resolution is requested |

The environment helpers do **not** call a shell known-folder API. The retained
wrappers pass literal environment names into `039bc0`; replacing those with
known-folder lookup or fallback values would change behavior. Both native
buffer loops start with260 UTF-16 units and grow by260 until the SDK count is
strictly less than capacity. A zero return produces an empty path even if the
SDK records an error, as the retained branches show. No fabricated successful
path or silent substituted default is introduced.

The native buffer is a `std::wstring`; SDK writes never alias `char16_t` objects
as `wchar_t`. The final path copies code units. The selected compatibility
implementation preserves the last SDK call's Win32 LastError across its own
temporary allocation/destruction. This is a rebuilt boundary policy; the
original allocator's incidental LastError and errno side effects are not
claimed. SDK callbacks must obey their normal nonthrowing outcome contracts.
Growth that would exceed DWORD capacity throws `std::length_error` as an
explicit extreme-size policy, rather than wrapping the SDK capacity.

UTF-8 conversion uses explicit lengths, so embedded NUL remains data in these
value-level operations. Empty UTF-8 input does not query a codepage. Narrow path
conversion selects the filesystem codepage even for an empty path, as `01c870`
does before its empty test. Existing encoding failures, substitution rejection
and specific fallback behavior stay in the recovered encoding source. Portable
C++ exceptions, compiler string allocation and the explicit executable-path
argument replace native EH, MSVC string layout and global `0x1800a39e0` access;
none proves original ABI or full runtime equivalence.

`bind_configuration_platform(Resources, ...)` wraps only the owned config
provider. Apply it **before** B5 `bind_configuration_and_console`. Construction
performs no SDK path lookup, file access, callback registration or worker
creation. Existing parser/FilePass code remains unchanged.

Eight policies remain explicit in the lower provider: document fallback,
configuration logging, license subsystem, directory creation, embedded PNG
writing, directory enumeration, removal and achievement persistence. This
stage deliberately makes no storage success or reconstructed license claim.
No actual user CFG/token/achievement contents or blocked version artifact was
read, and no original target or rebuilt DLL was executed or modified.

## Retained static evidence

Target SHA-256:
`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
No new binary session or decompilation is required for these claims. They use
the existing full pseudocode and assembly, whose hashes remain respectively
`26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`
and
`9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Address | Pseudocode Evidence ID | Assembly Evidence ID |
|---|---|---|
| `0x1800145d0` | `ev_b7669ec63c588e3e0c3c3ea478118b765778ea8ceeaac18635081dec14844c4d` | `ev_57ba03c192271ff8c347201fe9ae78a51f6632cff0e5a891c609f018940e6aee` |
| `0x1800192d0` | `ev_f97924b6178fd1c17ef606a5e5153562cafc5f04e36eecb08e5cdb93f2a31286` | `ev_aeef0b48642561e9aa088e2e457d40ad95a8159f366b6578e122c959b494e3a0` |
| `0x18003c6d0` | `ev_ba410f99cd0312ac92597ec449496bc2ce3eecfccc66c8f885b9c05f4b586a2d` | `ev_a25bcb6c136d83d915ae0affe5aa508e2c257aaf5193287cbff852b374d11743` |
| `0x18003c6f0` | `ev_cdae4b9b195f2bd0c3b2ecf95a78f67285dadd02ff0b1173c5af0f58698d7f79` | `ev_651ded00d21fdb7e5a9835dcbdc4f5844ee80600929155f268af3d7f708b137d` |
| `0x18003c920` | `ev_79c12e3986d5d1dafc3ee8c3b30a5bfba0c6175c8706b19219fa84deddd096bc` | `ev_037c4e2de199069d759e25ae4d49959902cb9789a34f27c2ca1cb66d01544094` |
| `0x180039bc0` | `ev_72d75fc9fa12d5c38ef034c259adc6d350c8c6000666bab5cd18fc7be06eb529` | `ev_86ba6440a8aa53a23ef4afa027e52749cacaf3379338fdd6370a3d8d0d08c130` |
| `0x18003c7b0` | `ev_17ad912a154a63c866cd96d402845c0371db388a41f12981ec499db6cd784d9f` | `ev_d8977d7a67dd2c948e4482f0547e26f9d9ef13e7e01747bb0f231017608f506c` |
| `0x18001c870` | `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e` | Existing configuration evidence ledger |

Additional path and encoding callees retain their own module evidence files.
The new adapter's address-suffixed names are composition entries, not new
canonical native function mappings.

Production compile check: `clang++ -std=c++20 -O2 -Wall -Wextra -Werror -static
-c config_platform.cpp` passed. Independent core/integration and common build
results are recorded by the B7 delivery owner, not inferred from this compile.

## B7 ortak doğrulama

**92 çekirdek + 96 entegrasyon = 188 kontrol PASS.** Ortak build 222 nesne (218 C++ / 4 C), 245 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 724 kaynak aynı; 2 önceki dosya açık hash kaydıyla güncellendi; toplam 733 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
