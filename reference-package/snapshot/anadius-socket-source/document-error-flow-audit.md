# N24 — 0x180005820 belge hatası kaynak gövdesi

26 Ekim 2026. `document_error.hpp/cpp`, `set_document_error_005820` normal gövdesini ve `RecoveredDocumentErrorEffects::set_error_005820` final sağlayıcı bağlantısını ekler. Bir native adrese normal gövde ve override olmak üzere iki tagged kaynak tanımı bağlanır. Bu aşama tek yeni adres ve tek sağlayıcı bağlantısı ekler; 43→44 historical typed source closure olur. Önceki iki semantic session adapter ve ayrı allocation adapter korunur.

## Kimlik ve kanıt

Anadius64 SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA 6.5.0 server/catalog identity, kullanılabilir Ghidra ve kapalı hedef checkpoint'i `document-error-rea-session.json` içindedir. Yeni native import/data read yoktur; önceden tamamlanmış REA kayıtları kullanılır. Kaynak ve destek dosyası hash'leri `document-error-evidence.json` içindedir.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
| 0x180005820 | `ev_37353162f9f968089bd3417314183c72cd998b61cac28d808f7e0959bf88f494` | `ev_188aa01332032bd7a9db079cf67ce66428557de9e6be37670dd091066a5e2ce8` |

005820 önceki N23 envanterinde `module_record_only`, sıfır definition, uncompiled ve normal-claim false idi. Yalnız bu bucket `source_definition_mapped` olur; `no_definition_mapping` sayısı bu hedef nedeniyle azalmaz. 0041b0, 004280, 0060a0, 006110 ve 004970 yalnız context olarak tutulur; bu aşamada yeni native recovery sayılmaz.

## İşlem sırası ve çağrı ilişkileri

| Adres | Kaynak gövdesinde korunan işlem |
|---|---|
| 005833–005843 | Signed error id cache; doc+6c hata ve doc+90 satır DWORD store |
| 00584d–005871 | Live doc+78 bit0x200 testi, koşullu live doc+80 pointer-only free; flags/start/end sıralı reset |
| 005878–0058a4 | 1000 bayt scratch allocate; signed64 error id ile error-name QWORD load; header format çağrısı |
| 0058ae–005905 | Nonzero optional-format için NUL tarama, separator format, yeniden tarama ve opaque caller-argument ile detail format |
| 00590a–005922 | Format sonrası live owned-bit ve start yeniden okunur; ikinci koşullu free ve üç sıralı reset |
| 005930–00594f | Scratch NUL uzunluğu; len+1 allocation, doc+80 pointer store ve len+1 copy |
| 005954–00596b | Copy sonrası live doc+80 tekrar okunur; flags=0x200 ve end=liveStart+cachedLen store; scratch free |

0058a4 header çağrısı `0060a0(temp,1000,header,name,id,id,line)` argümanlarını taşır. Assembly 005880/00588b/00589d, decompiler'ın kısalttığı stack argümanlarını doğrular. Separator 0058da ile 0060a0'a, optional detail 005905 ile 006110'a gider. Formatter dönüşleri kullanılmaz. Kapasite çıkarma unsigned64 olarak korunur; yapay enum bounds check, null allocation guard, uzunluk sınırı veya rollback eklenmez. Bounded mapping ve lower failure testleri yalnız korunmuş kaynak prefix'lerini doğrular; native fault/EH davranışı sayılmaz.

`NativeErrorArgumentTail` çağıranın açıkça sağladığı opaque invocation identity'dir. Final bridge her çağrıda bu bağlamı kullanır; native stack'ten varargs yakaladığı, host `va_list` oluşturduğu veya çağıran ABI'ını yeniden ürettiği iddia edilmez. `format_header_0060a0`, `format_separator_0060a0` ve `format_detail_006110` zorunlu alt etkilerdir. Allocate/copy/free sağlayıcıları açık kalır.

## Sabitler ve veri kanıtının sınırı

`document-error-retained-data.json`, `outputs/anadius-rea-evidence.json` içindeki tam historical read `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082` kaydına bağlıdır. Parent SHA-256: `c6d1daab6e5cce17f48285d07f7b7a1f6c9be4b37f043f19694009393b401495`. Aralık 0x18007f000..0x1800a0dff, 138752/138752 bayt complete'tir.

0x180080068'deki NUL dahil 42 bayt `Error=%s ErrorID=%d (0x%x) Line number=%d`, 0x180080094'teki NUL dahil üç bayt `: ` ve 19 hata adı string'i bu eski raw read içinde doğrulanır. **0x1800a1010 pointer tablosu bu aralık dışındadır.** Tablo slotlarının 19 pointer/ad eşlemesi yalnız eski focused PE-audit metadata olarak etiketlenir; raw REA pointer-byte kanıtına yükseltilmez. Üretim gövdesi seçilen identities ve mapped QWORD table yüklemesini korur; test fixture tabloları native slot doğrulaması değildir. Başka retained dosyalarda yokluk iddiası yoktur. Initialized Ghidra memory, runtime pointer gözlemi sayılmaz.

## Doğrulama ve kalan işler

**248 çekirdek + 99 entegrasyon = 347 yeni kontrol PASS**. Authoritative test grupları ortak build'deki `socket_document_error` ve `socket_document_error_integration` olur; standalone module script çalıştırılmış sayılmaz. Bağımsız sınırlı statik review üretim cpp/hpp hash'lerine bağlıdır; full native equivalence review false kalır.

Ortak build: 205 üretim nesnesi (201 C++ / 4 C), 223 normal + 19 terminal grup ve 96 socket grubu. Önceki 661 kaynak dosyası ve CRT baytları korunur; toplam 666 source ve 238 documentation snapshot girdisi. 48 validation mirror çifti = 47 historical + bir current; 94 historical config referansı ve 147 dependency hash. Bu sayılar tam DLL tamamlanma yüzdesi değildir; güvenilir toplam yüzde henüz ölçülmedi.

0060a0/006110 formatting, caller-supplied variadic boundary, hata pointer tablosunun raw slot kanıtı, native heap/copy/free/ABI/EH/concurrency ve 004970 parser/DOM açık kalır. Original DLL/oyun yürütülmez/değiştirilmez; gerçek CFG/token/achievement içeriği okunmaz. Hash-only integrity ayrı kaydedilir. Önceden antivirüs tarafından engellenen version.dll ve generated version DLL yeniden okunmaz, koruma ayarları değiştirilmez.

Öncelik 1→2→3→5→4. Genel CRT aşama 5 beklemede. Sıradaki N25, kayıtlı 0x180004970 parser kanıtlarının envanter ve sözleşme incelemesidir; bu hazırlık yeni parser gövdesi sayılmaz.
