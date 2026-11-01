# B8 — Configuration dosya ve kalıcı depolama bağlantıları

Beş mevcut Dependencies metodu somut kaynak/SDK yollarına bağlandı. Yapılandırma bağlantısı13/21'den18/21'e çıktı; document fallback, config logging ve license politikaları açık. Services11/17 ve altı açık politika bu aşamada değişmedi.

| Bağlantı | Uygulama |
|---|---|
| Dizin oluşturma018e00 | Korunan prefix/hata seçimi, mevcut status kaynağı ve typed Win32 |
| Dizin girdileri011a40/017110 | SDK sırası, dot atlama ve sahip olunan find handle |
| Remove04948c | Erişim retry, disposition fallback, readonly yan etkileri ve sahip olunan handle |
| PNG03c020 | Gerçek gömülü baytlar ve mevcut03c1d0 yazma akışı |
| Achievement01eb00 | Mevcut Storage/Writer/XML/status/timestamp kaynaklarının sahip olunan grafiği |

Bu sayılar kaynak bileşimindeki metotları sayar; toplam DLL tamamlanma yüzdesi değildir. Yeni native gövde/kapanış0; mapped487, eligible compiled491, normal body claims124 ve tarihsel kapanış52 korunur. Tam DLL hazırlığı **BLOCKED**.

## Somut veri kanıtı

PNG baytları önceden korunan REA rdata kaydından geldi. PNGsignature, bütün chunk CRC'leri, IEND, boyut ve SHA256 doğrulandı. Orijinalin0x1800a1108 ve0x1800a110c alanları root tarafından REA/Ghidra ile yalnız statik olarak8bayt okundu:401d00002a0f0000 →7488 ve3882. Bu boyutlar iki PNG'nin tam uzunluğuyla aynı. Evidence ID:ev_4ef179003503ecb3da7cac478ce0c307d43c59bd64f90052b3e6d3cfc189bf91. HedefSHA9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c doğrulandı ve oturum kapatıldı.

## Kullanım ve sahiplik

Paylaşılan Filesystem, ConfigurationFilesystem ve ConfigurationPersistence tarafından tutulur. ConfigurationPlatform → ConfigurationFilesystem → ConfigurationPersistence → B5 Configuration zincirinde her katmanın seçtiği metot korunur. Persistence başlangıç Paths değerleri çağıranın açık girdisidir. Factory/XML/Numbers/Parser/Stats/encoding/errors tek sahibi olan kaynaklardır; lower sahibi bunlardan önce yok edilir. Constructor I/O yapmaz.

Varsayılanlar açık yeniden derleme tercihleridir: NativeFactory, pinned XML kaynak Backend, HostCrtNumbers, ClassicIso, NativeStats, NativeCalls/PortableErrors. ClassicIso sabit classic ASCII formatıyla sınırlıdır. Derleyici stream/locale/EH/nesne ABI'si, keyfi overflow ve tüm bozuk XML davranışları özgün binary ile eşdeğer sayılmaz. Mutasyonlar seri configuration controller'da yapılır.

## Açık davranış farkı

Mevcut directory_entries arayüzü vector döndürdüğünden enumeration tamamlanmadan avatar filtresi/silme başlamaz. Özgün01c870 canlı iterator kullanır; geç enumeration hatasından önceki silme yan etkileri bu arayüzde birebir korunamaz. Collection sınırındaki sıra/hata/handle temizliği test edildi; canlı iterator prefix eşdeğerliği **açıktır** ve B9'un ilk işidir. Kaynak bağlantısı tamamlanmış olması bu farkı kapatmaz.

Status fallback FindClose başarısızlığında terminal davranış seçildi; korunan049104 bu sonucu görmezden gelebilir. Bu daha güçlü sahiplik politikası açık bir uyumluluk tercihidir. SDK callback'leri nonthrowing Win32 sözleşmesine uymalıdır. Unknown acquisition ve sahip olunan handle close başarısızlığı dört kendi terminal alt süreç vakasında sınandı. LastError kendi bookkeeping'i boyunca son SDK çağrısını, cleanup close dahil, korur; exception işlem hata kodunu ayrıca taşır.

## Test ve orijinal sınırı

174 çekirdek +34 entegrasyon =208 kontrol geçti. İki test grubu frozen üretim dosyalarıyla yeniden derlendi. Entegrasyon yalnız outputs/source-workspace/build/b8-fixtures altında yeni ve ayrı bir çalışma dizini yaratır; çözülmüş path sınırı I/O öncesi denetlenir. Recursive cleanup yapılmaz.

Gerçek Win32 mkdir/enumeration/remove, kendi readonly/Unicode dosyaları, gerçek FilePass/parser base+override, PNG baytları, XML import ve tam CRLF INI çıktısı, mevcut INI'yi koruma ve gerçek hata prefix'leri sınandı. Core kaynak state/error/close/status/reparse/timestamp hatalarını kontrollü sağlayıcılarla sınar. B7'nin worker graph'ı bu aşamada tekrar kopyalanmaz.

Orijinal DLL/oyun veya yeniden üretilmiş DLL çalıştırılmadı ve değiştirilmedi. Gerçek kullanıcı CFG/token/achievement içerikleri ve AV tarafından engellenmiş version girdileri okunmadı. Tek yeni orijinal veri sorgusu yukarıdaki statik8baytlık uzunluk alanıdır. Önceki733 C++/hook kaynak dosyası aynı; sekiz yeni dosyayla toplam741 kaynak. Öncelik1→2→3→5→4; genel CRT leaf ve numeric0033c0 beklemededir.

---

# B8 configuration filesystem scope

`ConfigurationFilesystem` binds exactly three existing `configuration::Dependencies` methods: directory creation, directory enumeration and removal. It owns a shared `Filesystem` helper and an owned lower provider for all other methods. The same helper can be supplied to achievement persistence. B5 and B7 outer wrappers preserve their thirteen routes; these three yield sixteen selected routes before the independent PNG/storage additions. This is source/SDK composition, not a newly claimed native body or original MSVC ABI closure.

## Directory creation

The normal prefix/error flow follows retained `018e00` and `048fd8`. Empty input reports path error3. The existing recovered root-name parser chooses the initial prefix, including the extended-root drive-component case. Each subsequent component is passed to `CreateDirectoryW`; trailing separators cause the retained extra final-prefix call. A root-only path has no create call. `ERROR_ALREADY_EXISTS` invokes existing bounded `UsedStats` (flags3) to distinguish an existing directory from a file, including its sharing-violation and reparse-target paths.

The retained error-selection rule is preserved: errors2/3/53/123 do not replace the remembered non-ignorable error; a final failure selects the last remembered non-ignorable error when present, while final success returns success despite earlier prefix failures. This is not a generic stop-on-first-error implementation or compiler `std::filesystem` substitute.

## Enumeration

The retained `017a40` checks nonempty input and compares its C-string length to stored length. Empty paths or embedded NUL therefore fail with error2 before the SDK. Other filesystem routes retain native c_str truncation while their source lexical processing still sees the full stored path. Enumeration appends `*` with the existing recovered join algorithm; it uses `FindFirstFileExW` with `FindExInfoBasic`, `FindExSearchNameMatch`, no filter and options0. Only exact `.` and `..` entries are skipped. Returned paths follow SDK order without sorting. Error18 means normal end; other errors propagate with the retained constructor/operator++ operation label.

An important existing source-interface limit remains: `Dependencies.directory_entries` returns a vector, so this adapter eagerly gathers the collection before configuration starts removing avatar files. Original `01c870` has a live iterator and may already have removed earlier entries when a later enumeration error occurs. The vector interface does not reproduce that partial-side-effect prefix or provide a filesystem snapshot. Original iterator/shared-pointer/STL object layout, native allocation failure prefixes, races and concurrent mutation are not claimed.

## Removal

The normal flow follows retained `04948c`, `049434` and `048a48`: open with access0x10180/share7/OPEN_EXISTING/flags0x02200000, then access0x10000 retry only after access denied. The first disposition request uses class21 and flags0x13. Unsupported results1/50/87 use flags3, followed by the retained one-byte legacy FileDispositionInfo request when needed. With full access, fallback access-denied reads FileBasicInfo and may clear readonly, retry and restore readonly only for repeated access-denied. Other retry errors retain the native readonly side effect. Missing-name errors2/3/53/123 from the initial open are remove-no-op; errors from the access-denied retry are not normalized. No recursive deletion is introduced; ordinary OS path and reparse behavior still applies.

## Ownership, errors and deliberate boundaries

All SDK write buffers use actual `wchar_t` and typed SDK structs. Per-call owners release file/find handles on normal return and C++ unwinding. Failed close is a compiler `abort` terminal policy; full BOOL bits are tested. There is one explicit stronger cleanup boundary: retained raw049104 ignores the sharing-fallback FindClose result, while B8 makes a known failed close terminal rather than discarding ownership. Unknown throwing acquisition outcomes are also terminal; SDK callbacks must obey the nonthrowing Win32 outcome contract.

Selected `std::system_error` and compiler allocation/destruction are rebuilt policies, not original MSVC filesystem_error/EH/SEH ABI. The last recorded SDK LastError is preserved through local bookkeeping and exception construction; calls restore that state before entering the next SDK boundary. Close calls are SDK calls too. No implicit permission elevation, guessed path fallback, sorting, cancellation or DLL unload support is added. Methods have only per-call state, but OS filesystem races and injected API lifetime/synchronization remain caller responsibilities.

No previous source file was edited. No original DLL/game/rebuilt DLL was loaded or executed, no original bytes were changed, no real user CFG/token/achievement contents or blocked version artifacts were read. Strict production compilation with the existing C++20 compiler and `-O2 -Wall -Wextra -Werror` passed. Independent tests and whole-build results belong to the B8 owner; compilation alone does not establish them.

## Retained evidence

Target SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
Full pseudocode SHA-256: `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`.
Full assembly SHA-256: `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.
No fresh decompilation or original input read was needed for this source composition.

| Address | Pseudocode Evidence ID | Assembly Evidence ID |
|---|---|---|
| `0x180018e00` | `ev_aa91855c8f9a5cb7fc35e491b954dcb179e2fadbcad6947f765d1d8919c42aff` | `ev_c6c55d80f26c20c8a545576572b5bd9ce5ef777e51a5eaf971c2284a97bb0b5a` |
| `0x180048fd8` | `ev_ec2405434788aadef0cbc5881a31c3033b65cda584484ca8b02f2245de8f276c` | `ev_92f41332945d6575b5aeb3c071a6fa18d3c66f57143de347cc9e251d02632bc4` |
| `0x180049104` | `ev_005bb220a469430c8acd184c5ceaace3b27a578b843f9774e9842a86fcdb9122` | `ev_5c914d480dd2ba111f50296dae9a6904e1fc7b536ca1f76e0d57f5d3ea3b24fc` |
| `0x18004948c` | `ev_e72194d20aa35a443ed071ea2568e1157254a50f348d17fb16fc5638fff78b0d` | `ev_f466f245bd1484fd956c8dee43b3d19147868172fa2580f7ddc06039e39ff800` |
| `0x180049434` | `ev_55280ecfb3c34265727e88d7f2e18fcda8dc52b6f97e2bc017c934ac978d90b4` | `ev_7b745f5e76486d9818cd43f80a889202153ab46ab790662f974a21f9ab121c1b` |
| `0x180048a48` | `ev_83cbb2f8ee62f066318ab6451a975c49dcf1c2792c5f53a864b19ae81a00cc72` | `ev_5baee9bfa31c7624534f1964cbe75f3f6b45b318e9d5b20498ea34f76e9e5bc0` |
| `0x180011a40` | `ev_ce0c5dec45c662c9f2baac77b5e61e47cb04a6ad1950c85f881ed16e13b2a781` | `ev_b6a386f368816e9c88abd16557006ead0cdbd676e44597ed950e8d426c704dce` |
| `0x180017a40` | `ev_43d76c5092901a642bc14870de0f639acd0a7988b96aa1b64e002477da21b4a7` | `ev_073421e7009ee6649c0534af37f1fe40035a04ddfc4a8e9723c4f4bbc033f747` |
| `0x180018020` | `ev_d532a7075a66b9f651d5bb641f3f891753c1a39cd132f3cc392bdbc437e11713` | `ev_3908d9cb30feee1d07d5a1d41292b7d56375e463003c98ffe9d68c330988ba01` |
| `0x180017110` | `ev_b73cfdf1cde3954d962ee0f307723dae4ede3cb6ce29cbe6c2092083fd65cb47` | `ev_47c64d27e32e48a3c5bd00e18a8048d29f8b97869ceb18d26aa73c29f086ba57` |
| `0x180017e80` | `ev_aabdd47ad28fda81aba724b13dc93508b9f396aa91e73e4ff09953209501fa2e` | `ev_7587a6b3adbb997ddbffeb0fe567b20f06a43087f095e23a897c74658e5f5fcf` |
| `0x180049090` | `ev_77de3b02f24450f016f913598b06f9c4b2430d28fc397ca2f66214ad72ee00c0` | `ev_884a18cda4c9376da3bc0364e8d7beafae8e29b85d269fc044eb892813fafc26` |
| `0x180049050` | `ev_79d8bde589d46498589382b7ad7fcd6393a1d33c3dedc7d44e5676cfe0fb3196` | `ev_903f1816da43db010ffe952a406e509e3941b4223c563a8961a3ab3033783330` |
| `0x180049070` | `ev_68e0cf0d5ebe35359569ce91801d9e71db632e1701f28c670aaf740c69141256` | `ev_e7c6921d71ee74b642c3b6ba3ae8a6729379866c535f4b822d42338e387fd21f` |

---

# B8 — Embedded PNG and achievement persistence

ConfigurationPersistence binds exactly two existing configuration methods:
write_embedded_png_03c020 and update_achievement_storage_01eb00. Other nineteen
methods forward to its mandatory owned lower provider. Combined with B5/B7 and
the independent three-route filesystem wrapper, this supplies **18/21 selected
configuration routes**. Document fallback, logging and the separate license
subsystem remain explicit. These are source bindings, not new native bodies,
a complete replacement DLL, or a completion percentage for the original.

## Immutable embedded payloads

| Original VA | Length | SHA-256 |
|---|---:|---|
| 0x1800809d0 | 7488 | 833d5a6444b352bec759221e1973d7045b88967f4507059c6ffd95aa15cf34dd |
| 0x180082710 | 3882 | 9584cb4afe2734f4b30f0fe30d8390e76dd8703ac603fcf4a13c8f2df11133f5 |

Exact bytes came from retained read_bytes Evidence
ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082,
base0x18007f000/length138752 in outputs/anadius-rea-evidence.json, SHA-256
c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495.
Generation checked both PNG signatures, every chunk CRC32 and IEND. The parent
separately read eight bytes at0x1800a1108 through static Ghidra:
401d00002a0f0000 = little-endian7488/3882. Thus the extents also equal the
original caller length globals; padding was not guessed from IEND.
Length evidence:
ev_4ef179003503ecb3da7cac478ce0c307d43c59bd64f90052b3e6d3cfc189bf91.
The complete provenance is embedded-png-evidence.json. Byte arrays are in
embedded_png_data.hpp; embedded_png returns a stable read-only span and rejects
unknown enum identities.

Retained03c020 and existing write_binary_03c1d0 have the same application output
sequence: open mode0x20/share0x40, initial stream-state guard, one byte write,
explicit close. PNG output reuses that existing source. The bootstrap caller
ignores the boolean: failed initial open therefore performs no write and
returns without a new exception. Write/close exceptions still propagate.
No post-write integrity check, mkdir, retry or rollback is added.

## Owned storage graph

Constructor inputs are shared Filesystem, explicit initial achievement Paths,
owned lower configuration and PersistenceResources. Construction performs no
file I/O. Member order keeps resources/paths/effects/filesystem alive through
Storage and lower-provider destruction. The wrapper is immovable. paths()
returns a read-only view for the same quiescent controller lifetime.

Existing Storage.update_01eb00 supplies XML path construction/import,
count/progress/name/timestamp merge, optional Steam INI path and epoch setup,
status lookup and source INI writing. Concrete effects reuse existing
filesystem_widen_006d70, timestamp_to_epoch_041e10/epoch_041ff0,
status_01ea20, the same shared Filesystem.create_directories as the outer
wrapper, and existing binary file/INI source. No timezone or OS mktime
substitution is made.

Source failure prefixes remain: empty achievement set does not clear an
explicit old XML path; malformed XML does not invent successful records;
timestamp/status/mkdir errors can leave earlier paths or records changed.
The INI writer's boolean remains ignored as in the original caller.
Atomic replacement, backups, automatic retry and rollback are not added.

## Selected compatibility policies and limits

Default resources are NativeFactory, pinned xml_reader_bridge.Backend,
HostCrtNumbers, ClassicIso, NativeStats, NativeCalls and PortableErrors.
All are injectable through existing interfaces; null resources are rejected
before I/O. Compiler file sharing/streams/EH and MSVC object ABI are not proven
equivalent. Pinned XML source does not prove the original tinyxml2 release or
all malformed-input outcomes. HostCrtNumbers uses rebuilt host CRT; numeric
overflow and arbitrary locale equality remain unproven. ClassicIso is the
already recovered classic ASCII fixed-format parser and explicitly rejects
non-ASCII input. Its selection does not adopt an arbitrary OS locale.
Filesystem errors use selected compiler filesystem_error with operation, path
and preserved32-bit code, not original MSVC error-object/EH ABI.

Mutation/storage effects belong to the serialized controller. No background
writer, concurrent persistence consistency, cross-thread callback ownership,
loader-lock or DLL-unload proof is added. B3/B4 joins and B6/B7 domain lifetime
contracts still apply.

## Retained static evidence

Target SHA-256:
9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c.

| Address | Source/observation | Assembly Evidence ID |
|---|---|---|
| 0x18003c020 | raw span PNG output | ev_4dca6b0cf6f142a6ac6b4bd34a482777b9a683e3546558323a1b48efbf73b0b1 |
| 0x18003c1d0 | existing string output sequence | ev_97eaf0d1a5c0d1e7ad14855daa321d9f92e4aabb4a1d614bc467ef53f3010812 |
| 0x18001eb00 | XML import and INI setup | ev_bd10a5a82a8307f47db504ea903686ef7b68be9597de0c3a48f59707493363e8 |
| 0x18001f9a0 | existing Steam INI writer | ev_488d569fd12cfa9fe60d4d35d04b56c9d44b52396427f3060d14d2a7e0125a5a |
| 0x180041e10 | timestamp parse/error check | ev_c6b31cf960ca4a4bf60c21d21f3719c757a8a088e5e42c6c5b0a0784d2fc858d |
| 0x180041ff0 | arithmetic epoch conversion | ev_9d8c05e7881ace56f5053a11d8c9b6148320e73c05a8db0fd152a3ad78b0619c |
| 0x18001ea20 | status/error classification | ev_b2ed2f94d1e70182e9c643a28823161406f5467cb3c96081b5325e82fd10f8e8 |

Strict production compile passed with C++20/O2/Wall/Wextra/Werror/static.
Independent tests/common build are recorded by their authors and delivery
owner. This agent did not read original inputs or user CFG/token/achievement
content, execute a target or DLL, or modify prior source. Parent performed
only the declared static eight-byte length-global read of selected anadius.
No blocked version input or historical generated DLL was accessed.

## B8 ortak doğrulama

**174 çekirdek + 34 entegrasyon = 208 kontrol PASS.** Ortak build 224 nesne (220 C++ / 4 C), 247 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 733 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 741 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
