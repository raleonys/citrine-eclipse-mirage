# REDFIELD RF31 — Golden altyapısı refactor

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

RF31, M20.3 kapsamındaki Golden/resource/link ve production dist yolunu mevcut
ortak derleme motoruna bağlar. RF30'daki 262 uygulama dosyasının tamamı byte
olarak aynıdır. Çalışan runtime iskeletine yeni oyun özelliği eklenmedi.
Golden algoritmaları tek bounded PE okuyucusu, her kaynak kökü için yeniden
yüklenen policy ve mevcut BuildSession sahipliği üzerinden uyarlandı.
Bu çalışma bütün codebase refactor'ının veya gerçek Windows kabulünün kapanışı değildir.

## Windows derleme girişleri

Tüm girişler scripts/build_orchestration altındadır. CMD dosyasını çalıştırın;
Python asıl motoru kullanır ve child exit kodu aynen korunur.

| Giriş | Çıktı ve kapsam |
|---|---|
| build_verify.cmd | build/verify/msvc_x64/preloader_l.dll; developer aggregate,96 regresyon ve ayrı legacy diagnostic;dist'e yazmaz |
| build_production.cmd | build/production/preloader.unsigned.dll; /O2 /MT /GS,RC/public ABI smoke/link audit ve beş Golden aşaması;hepsi geçerse dist/preloader_l.dll |
| build_debug.cmd | Ayrı /MTd,symbol,objeler;dist/debug/preloader_l_debug.dll;mevcut host kapsamı |
| build_native_acceptance.cmd | Ayrı /MT/native entry;dist/native_acceptance/preloader_l_native_test.dll;mevcut native host |
| build_crt_acceptance.cmd | Ayrı /MT /GS public/native DLL ve mevcut CRT host kapsamı;dist/crt_acceptance |

Production artık sırf eski GOLDEN_PENDING engeli nedeniyle durmaz. Gerçek gate
başarısızlığı derlemeyi durdurur ve o denemenin production dist çıktısını temizler.
Bu ortamda Windows DLL derlenmedi;yerel MSVC'nin layout/resource/.data koşullarını
sağladığı henüz kanıtlanmadı. Önceki build'in gerçek DLL PASS kaydı yeni DLL'in
kabulü sayılmaz. surge.cfg çalıştırılan DLL'in yanında bulunmalıdır.

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

## Profil ve kalan refactor

RF30'da surge.cfg örneğinin eksik dokuz User alanı düzeltildi;gerçek örnek
dosyayı okuyan regresyon bu tur da geçiyor. GetConfig normal34 servis kontrolü
geçer;legacy exit14 son unknown entity rejection beklentisi ile parser politikası
arasındaki farktır. Entity politikası veya diagnostic PASS olarak değiştirilmedi.
GetProfile index selector ve SDK handshake/session crypto incelemesi açıktır.
Oyundaki gerçek profil kök nedeni kanıtlanmadı;RFCL-005 OPEN kalır.

4 önceki source review kapalı,16 modül açık. Yeni mandatory import-refactor
kuralında 294 maintained code dosyası ve4 vendor integration PENDING,QUALIFIED0.
Bu dört eski review yeni kural için otomatik qualification değildir.
verify_source.cmd --require-refactor-complete bu nedenle beklenen exit1 verir;
normal local build pending kayıtlarla çalışabilir. M20.3'ün kaynak entegrasyonu
uygulandı;M20 kapanmadı. Sırada M20.4 Debug/native/CRT coverage vecomposition,
ardından B02 shutdown/bellek,B03 service/TCP/ASIO,B04 XML/zaman,B05 response,
B06 hooks/DLL,B07 son kaynak kapanışı var. Gerçek Windows layout/data kabulü
ve bütün semantik qualification ayrıca takip edilir.

GPL-3.0-only,preloader_l,surge.cfg,vendor pinleri,7 hook,35 binding veHF19/CURRENT
korunur. Governance normative mirror2026-10-21.1. Uygulama C++ değişmediğinden
sanitizerlar bu tur tekrarlanmadı. Bu kaynak teslimi NOT_RELEASE'tir.
