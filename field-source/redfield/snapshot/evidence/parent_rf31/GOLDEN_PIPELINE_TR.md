## Tek Golden zinciri

MSVC Build (gerçek compile/link komut denetimi dahil) → URSYNEX Golden Finalizer
→ ARYRA Golden Layout → MUSTAVEX Golden Metadata → CORVEXON Golden Trail
→ FALCION Golden Cleaning. Son atomik dist yazımı ek bir protokol aşaması değildir.

| Sorumluluk | Yeni yapıdaki uygulama |
|---|---|
| PE okuma | Immutable bytes;header/section/RVA/import/export/resource/VERSIONINFO sınırları,cycle ve range reddi |
| URSYNEX | Sadece pinned linker/timestamp,Rich kaldırma ve checksum değişikliği;byte allowlist,atomik yazım,hash/readback |
| ARYRA | Section sırası/alignment/raw-virtual backing/overlap,image ve static CRT .fptable sözleşmesi |
| MUSTAVEX | Exact compatibility VERSIONINFO,resource tür/id/dil,export internal name,OS/subsystem;debug/certificate/COFF/identity sızıntısı reddi |
| CORVEXON | Üç public export/ordinal,CRT startup entry ilişkisi,ASLR/NX/high entropy,GS cookie,import/delay import ve checksum |
| FALCION | Current source/role/import/manifest ve exact ZIP üyeliği;generated binary/arşiv ve sensitive materyal reddi;.data VS<=0x1800 ve static .fptable |
| Dist | Live candidate hash'ine bağlı typed five-stage outcome;tool/input/object/resource/output yeniden kontrolü,atomik kopya/readback;hata/iptalde temizleme |

HF19'dan 15 referans exact byte/hash ile provenance/hf19_golden altında arşivli.
Dört Golden policy JSON'u byte olarak aynı tutuldu. Kaynak/resource ve metadata
bağlaması metadata/GOLDEN_BINDING.json'da açık. Compatibility VERSIONINFO'daki
eski ürün metinleri yeni proje sürümü veya yeni codename değildir. .def internal
name pinned preloader.unsigned.dll oldu;üç public export/ordinal aynıdır,nihai
dosya adı preloader_l.dll'dir. RC yalnız production profilinin resource objesine
derlenir. Eski CMake dosyalarını greplemek yerine gerçek BuildSession komutları
ve profiline ait resource/objeler denetlenir. CMake developer aggregate'tir;
production dist için ikinci bir otorite değildir ve bu ortamda çalıştırılmadı.

FALCION destination profili LOCAL_REFACTOR_SOURCE_PACKAGE olarak açıkça seçildi:
tam kaynak/provenance/evidence tutulur;bu resmi flat release paketi onayı değildir.
Geçmiş iki evidence logundaki sekiz özel Windows kullanıcı yolu prefix'i
redakte edildi;OEM baytların geri kalanı korundu. Önce/sonra hash kaydı
metadata/REDACTIONS_RF31.json'da;immutable RF30 parent ZIP değişmedi.

## Doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz gerçek GCC C++20 compile/link | 58 production+2 test support,157 object,97 linked çıktı |
| Pozitif regresyon | 96/96 PASS;RF30 stdout/exit96/96 aynı |
| Golden Python gate QA | 61 beklenen sonuç;sentetik ve çalıştırılmayan PE fixture'ları |
| Profil/resource/dist QA | 62 beklenen sonuç;Windows/compiler/RC/host süreçleri taklit,Python orkestrasyon ve filesystem gerçek |
| Boundary/mimari/encoding/VS ortam/POSIX process | 30/13/16/13/7 beklenen sonuç |
| Gerçek GCC nm role izolasyonu | Production'da generic allocator/PTD test modeli yok;supportta var |
| Paket/source | Normal verifier PASS;2 bozuk paket beklenen ret;deterministik ZIP ve fresh extract/input eşleşmesi |
| Legacy GetConfig | FAIL_OBSERVED exit14;96 pozitif regresyondan ayrı |
| Gerçek Windows/MSVC production/Debug/native/CRT/oyun | NOT_RUN |

Golden QA, original HF19 finalizer ile aynı sentetik PE üzerinde exact byte
eşitliğini ve original ARYRA/MUSTAVEX/CORVEXON'un aynı fixture'ı kabulünü de
denetledi. Beş aşamanın her birindeki ret,dist temizliği,iptal,resource/tool
değişimi ve atomik yazım hataları test edildi. Bunlar mevcut Windows DLL'i
kabul edilmiş veya eski bütün native/CRT senaryoları taşınmış yapmaz.

