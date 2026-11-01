# REDFIELD RF32 — ayrı native test composition ve kabul kapsamı

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

RF32, M20.4'ün native composition sınırını refactor eder. Production DLL giriş
dosyasındaki test derleme koşulları, fault budget vetelemetry state kaldırıldı.
Production ve native DLL artık farklı giriş TU'larıyla aynı WindowsWorker
şablonunu compose eder. Runtime algoritmasının ikinci kopyası oluşturulmadı.
RF31'in262 application dosyasının261'i exactaynı;bir DLL entry refactor edildi,
bir ortak worker header'ı veüç test dosyası eklendi. Toplam266 application dosyası.
58 portable core kaynağının tamamı,mevcut96 regresyon gövdesi,7hook ve35
binding değişmedi. Yeni97'nci regresyon native gözlemcinin ABI/budget/concurrency
sözleşmesini sınar. Bu adım tüm codebase refactor'ının kapanışı değildir.

## Windows derleme girişleri

Tüm girişler scripts/build_orchestration altındadır;CMD ince invocation katmanıdır.
Mevcut derleme davranışı veprofil output kökleri korunur.

| Giriş | Çıktı ve kapsam |
|---|---|
| build_verify.cmd | build/verify/msvc_x64/preloader_l.dll;97 regresyon+ayrı legacy diagnostic;production/native DLL ve mevcut hostlar;dist'e yazmaz |
| build_production.cmd | /O2 /MT /GS,production entry veprofile-owned RC;public ABI smoke/linkaudit+beş Golden gate geçerse dist/preloader_l.dll |
| build_debug.cmd | Ayrı /MTd,debug symbols,objeler;dist/debug/preloader_l_debug.dll;current public ABI host |
| build_native_acceptance.cmd | Ayrı /MT,tests/native/dll_entry.cpp;dist/native_acceptance/preloader_l_native_test.dll;mevcut native host |
| build_crt_acceptance.cmd | Ayrı /MT /GS public/native entry vehostlar;dist/crt_acceptance;tam eski CRT black-box/parity açık |

surge.cfg DLL'in yanında olmalıdır. Production DllMain/SURGE/DllShutdown ve
ordinals1/2/3 aynı.Native yalnız RFTestStatus100/RFTestSetFaults101/RFTestTransport102
ek exportlarını taşır.Native entry ayrı source-role olarak kayıtlıdır;production
profile bu kaynağı reddeder.Nativetest artık production entry'ye macro vererek
oluşturulamaz.CMake developer aggregate de ayrı native entry'yi kullanır;bu
ortamda CMake/MSVC çalıştırılmadı.Production ve Debug include closure native test
başlığını almaz;shared core veMinHook her profilin kendi CRT/objeleriyle derlenir.

## Ortak worker ve sahiplik

src/windows/worker_runtime.hpp içinde tek WindowsWorker<Observation> vardır.
Production WorkerObservation alan tutmayan no-op policy'dir.Native gözlemci ve
fault bütçeleri yalnız tests/native/worker_observation.hpp içinde yaşar.
tests/native/dll_entry.cpp test ABI'sini compose eder.Production giriş yalnız
schedule/shutdown/export kabuğudur;PRELOADER_L_NATIVE_TEST koşulu yoktur.

WorkerOwner halen thread handle,module pin,stop/join veexplicit cleanup'un tek
sahibidir.Gözlemci fiziksel kaynak bırakmaz;sadece close/unpin denemesine izin
verir veya test için erteler.7/8/10 shutdown sonuçları veidempotent retry korunur.
SessionRetirement hook restorasyonu,service drain veconfig retirement sırasını
korur;başarısız restore durumunda worker module pin'ini bırakmaz.Konfigürasyon,
online/port veaccepted/active/frame yayın sırası/native packed ABI bitleri aynıdır.
Phase önce acquire edilir;counter'lar hâlâ ayrı atomiklerdir,koherent snapshot
garantisi eklenmedi.Tüm per-DLL sahipler constinit;loader lock altında config,
heap/service veya bekleme yapılmaz;yalnız schedule edilir.

## Tek Golden

RF31'in gerçek MSVC linkaudit → URSYNEX → ARYRA → MUSTAVEX → CORVEXON → FALCION
zinciri vetyped canlı outcome sonrası atomik production dist yolu korundu.
Pinned15 referans/dört policy,compatibility resource/export internal name,
static .fptable ve.data VS<=0x1800 sözleşmeleri aynı.FALCION kapsamı açık
LOCAL_REFACTOR_SOURCE_PACKAGE'tır;bu kaynak teslimi resmi flat release değildir.
Yalnız URSYNEX izinli baytları değiştirir;diğer gate'ler hash değişimini reddeder.
Hata/iptalde o denemenin dist'i temizlenir.Windows resource/layout/data kabulü
bu ortamda denenmedi;sentetik PE testleri gerçek DLL kabulü sayılmaz.

## Doğrulama

| Kontrol | Sonuç ve sınır |
|---|---|
| Temiz gerçek GCC C++20 | 58production+2support,158object,98linked çıktı |
| Pozitif regresyon | 97/97 PASS;önceki96 stdout/exit exactaynı |
| Worker entry derleme/closure | Eski/yeni production/native girişleri gerçek GCC ile derlendi;Win32 bildirimleri model |
| Worker davranış eşlemesi | 16karşılaştırma/32process yürütümü;aynı senaryolarda kaynak cleanup/status/frame davranışı eşleşti |
| Worker QA toplamı | 21kontrol;phase/cancellation yarışında izinli sonuçlar korunur,tek zamanlama dayatılmaz |
| Yeni gözlemci regresyonu | Packed status/transport ABI,masks,independent close/unpin budget,8thread üzerinde exact consume sayıları |
| ASan+UBSan | Yeni gözlemci regresyonu gerçek GCC ile PASS;full Windows/core sanitizer çalışması değil |
| LeakSanitizer | Ortamın ptrace/proc erişim kısıtı nedeniyle UNAVAILABLE;PASS sayılmadı |
| Golden/profil QA | 61/64 beklenen sonuç;PE sentetik,Windows/MSVC/RC/host süreçleri taklit |
| Boundary/mimari/encoding/VS ortam/POSIX process | 35/13/16/13/7 beklenen sonuç |
| GCC nm izolasyonu | Production core'da generic allocator/PTD test modeli yok;supportta var |
| Paket/source | Normal verifier PASS;2bozuk paket beklenen ret;deterministik repack/fresh extract/input binding |
| Legacy GetConfig | FAIL_OBSERVED exit14;97pozitif regresyondan ayrı |
| Gerçek Windows/MSVC/Debug/native/CRT/oyun/CMake | NOT_RUN |

Worker eşlemesinde gerçek C++ girişleri,WorkerOwner,LocalService/ASIO veConfig2
çalıştı.Win32 thread/module/environment veWindowsHooks sınırı modeldir;gerçek
DLL load,MinHook veyaWindows CRT kabulü değildir.Normal/missing/invalidconfig/
invalidpath/close-unpin retry/retained cleanup/early stop senaryoları iki girişte;
native observer fault vebir gerçek loopback TCP çerçevesi ayrıca sınandı.Önceki
portable core objeleri exactcurrent source/header/vendor veparent receipt hash
kontrolünden sonra bu eşleme için yeniden kullanıldı;giriş/model objeleri taze.
Bundan bağımsız temiz current build bütün97 regresyonu taze derledi veçalıştırdı.

## Kabul kapsamı ve kalan refactor

İki canonical HF19 betiğinden aynı29 native/CRT senaryosu çıkarıldı;dört eski
Debug host,iki shutdown stress senaryosunun ilave4'er round'u veCRT shipping
GetConfig black-box açığı tek tek kaydedildi.9host/CMake/export referansı exact
HF19 ZIP'inden arşivlendi.Mevcut portable/native testlerle source-hash bağlı
eşleme metadata/ACCEPTANCE_COVERAGE.json vedocs/ACCEPTANCE_COVERAGE_TR.md içinde.
Envanterin tamamlanması native parity tamamlandı demek değildir;29satırın tamamı
PARTIAL/PENDING.Dört eski Debug diagnostic API/host henüz taşınmadı.Mevcut
Debug public ABI+/MTd+symbols'tur;eski diagnostic prototype ile aynı kapsamda değil.
Eski custom PTD/FLS modeli production'a dönmeyecek;tests/support model testleri
supported /MT CRT'nin gerçekWindows lifetime kabulü yerine kullanılmayacak.

4önceki source review kapalı,16modül açık.M20 NEEDS_CHANGE kalır.298maintained
code+4vendor integration yeni mandatory import qualification PENDING,QUALIFIED0.
Yeni dosya sayısı yeniqualification değildir;eski dört review de otomatik onay
değildir.Normal local build pending kayıtlarla çalışabilir;--require-refactor-complete
beklenen pending reddi verir.Native composition ayrıldı;debug composition,
tam native/CRT senaryo/stress/shipping-blackbox vegerçekWindows kabulü açık.
Ardından B02shutdown/bellek,B03service/TCP/ASIO,B04XML/zaman,B05response,
B06hooks/DLL,B07sonkaynak kapanışı var.

RF30 örnek surge.cfg User düzeltmesi vemevcut GetProfile regresyonu exactkorundu.
GetConfig normal34servis kontrolü geçer;legacy14 unknown-entity rejection beklenti
farkıdır,parser politikası değiştirilmedi.Oyundaki profil kök nedeni kanıtlanmadı;
RFCL-005 OPEN.GetProfile selector,SDK handshake/sessioncrypto ve7deferredroute
ilgili source/runtime incelemelerinde açıkça izlenir.Yeni oyun özelliği eklenmedi.
GPL-3.0-only,preloader_l,surge.cfg,vendor pinleri,3publicexport,7hook,35binding ve
HF19/CURRENT korundu.Governance mirror2026-10-21.1.Teslim NOT_RELEASE'tir.
