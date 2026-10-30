Güncel kaynak optimizasyonu, dosya/fonksiyon tabloları ve ölçümler:
[kaynak optimizasyon incelemesi](docs/CODEBASE_OPTIMIZATION.md).
XML tahsisleri ve Release tanılama çağrıları iyileştirildi. Değişen kaynak için
native Windows/Golden ve oyun kabulü yeniden gereklidir; güncel yerel sonuçlar
[doğrulama kaydında](docs/evidence/optimization/VALIDATION.json).

Önceki kabul edilmiş DLL'in Windows sonuçları ve boyut karşılaştırması:
[tarihsel kabul ve boyut incelemesi](docs/DLL_SIZE_AND_NATIVE_ACCEPTANCE.md).

Güncel geniş kaynak denetimi: [tüm kaynak ve adaptör incelemesi](docs/BROAD_SOURCE_AUDIT.md).

Güncel kaynak düzeltmeleri ve test katkıları: [kaynak incelemesi](docs/SOURCE_REVIEW.md).

# SURGE

Son gönderilen arşivdeki SURGE avatarı birebir güncellendi.
[Avatar doğrulaması](docs/evidence/AVATAR_REFRESH.json).

Yeni referans DLL'in metadata alanları ve yeni SURGE avatarı uygulandı.
Native acceptance'taki namespace fixture ve Windows port sahipliği hataları
düzeltildi. [Kapsam ve doğrulama](docs/REFERENCE_METADATA_AVATAR.md).

Windows MinHook derleme kopyasındaki CRLF/LF hash uyuşmazlığı düzeltildi.
Vendor dosyaları ve sabit yama hash'leri korunur.
[Açıklama ve doğrulama](docs/HOOK_ENGINE_HARDENING.md).

Production derlemesi 64 C++ TU kullanır. Diğer 74 TU kaynakta, referans
testlerinde ve geliştirme DLL'inde korunur; bunlar genel olarak ölü kod değildir.
Uygulama gövdeleri ve başlıkları değiştirilmeden hedefler ayrıldı.
[Derleme sınırı ve dosya başına kullanım tablosu](docs/BUILD_PARTITION.md).

Orijinal genel dosya kopyalama yardımcısı, kimlik fallback’i ve zaman sorgusu
dökümden yeniden oluşturuldu; avatar tam gövdeyi mode2 ile çağırır.
[Kapsam ve testler](docs/RE_COPY_RECOVERY.md).

Ürün adları tüm config/runtime modüllerinde ortak tanımdan gelir: surge.cfg,
surge_override.cfg, %LOCALAPPDATA%/SURGE, SURGE.png ve preloader_l_online.dll.
Hata mesajları surge.cfg adını kullanır. Yellowfield kaynak geliştirme hattının
adıdır; tarihsel kayıtlar ve JSON şema kimlikleri korunur.
[Adlandırma düzeltmesi](docs/PRODUCT_IDENTITY_FIX.md).

Windows paket açılımı: güncel ZIP Asio'nun iki mantıksal bağlantısını birebir
hedef metni içeren normal dosyalarla taşır; symlink yetkisi gerekmez. Paketi
temiz klasöre aç. Vendor denetimi farklı yolları ve hash'leri açıkça raporlar.
Detay: [VENDORING.md](docs/VENDORING.md).

Anadius64 RE uygulama kaynaklarından düzenlenen Windows x64 C++20 projesi.
Kök düzeni Brownfield'deki kaynak/header/test/araç ayrımını izler. Brownfield
uygulama gövdeleri alınmamıştır. Son kullanıcı talimatıyla 11. hook ve gerekli hook
ömür yönetimi Bluefield’den uyarlanmıştır; diğer modüller RE kökenini korur.

```text
include/preloader_l/   uygulama başlıkları
src/                  config, core, runtime, net, platform, protocol, xml...
tests/                unit, native ve support fixture'ları
third_party/          kullanıcının dört aktif vendor arşivinden kaynaklar
scripts/              yerel build, bütünlük ve kaynak paketleme araçları
tools/                uygulama C API host'u, ayrı source host, CRT/PE araçları
resources/            kullanıcının SURGE görseli
exports/              uygulamanın sabit C export tablosu
metadata/             kaynak kökeni, kapsam ve gerçek doğrulama sonuçları
docs/                 geliştirme ve kapsam açıklamaları
governance/           güncel kullanıcı yetkisi ve kalıcı hedefler
```

`src/recovered`, `anadius-*-source`, tam preloader/version kaynak ağaçları,
özel CRT kurtarım paketleri veya geçmiş derleme executable'ları ürün ağacında yoktur.
Başlık include'ları yeni konumlara bağlandı; first-party namespace `preloader_l`.

## Yerel Windows/MSVC derlemesi

Brownfield'den uyarlanan yerel derleme girişi:

```powershell
scripts\build_orchestration\build_unstable.cmd
```

Debug, Windows kaynak testleri, standart CRT ve yerel Golden için ayrı kayıtlı
komutlar da vardır: [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).
Geniş Brownfield aktarımı ve kalanlar
[metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json](metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json) içindedir.

Veya Visual Studio x64 geliştirici terminalinde:

```powershell
cmake -S . -B build/windows -A x64 -DBUILD_TESTING=ON
cmake --build build/windows --config Release
ctest --test-dir build/windows -C Release --output-on-failure
```

DLL hedefi `preloader_l`, dosyası `preloader.unsigned.dll`; public C arayüz
host'u `preloader_l_runtime_host` hedefidir. C++ kaynak API'leri ayrı
`preloader.source.dll` içindedir; `preloader_l_source_host` ona bağlanır. Yerel derleme Library/G0/yayın
işlemi başlatmaz. MSVC komutlarının bu Linux oturumunda çalıştırıldığı iddia edilmez.

LLVM-MinGW ile çapraz derleme:

```sh
export LLVM_MINGW_ROOT=/path/to/llvm-mingw
cmake --preset llvm-mingw-x64-vendors
cmake --build --preset vendors --parallel 3
```

Dört aktif vendor için uyumlu doğrulama araç seti LLVM-MinGW **20251118 / Clang 21.1.6**.
Clang 23'te PEGTL public API'si derlenmez; varsayılan CMake kontrolü testler kapalıyken
de bu uyumsuzluğu hata olarak bildirir. Ayrıntı: [docs/VENDORING.md](docs/VENDORING.md).

Platformdan bağımsız kaynak testleri:

```sh
cmake -S . -B build/portable -DPRELOADER_L_PORTABLE_CHECKS_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/portable --parallel 3
ctest --test-dir build/portable --output-on-failure
```

## Uygulama başlangıcı ve sınırı

DLL yüklenince, loader lock dışında çalışan worker tek `application::Application`
sahibini kurar. Tam PEGTL/hash config uygulaması, standart Windows filesystem/fstream IO, Asio
servisi, TinyXML2/crypto/router, event/achievement depolama, native MinHook ve
opsiyonel provider aynı ömür zincirine bağlıdır. Kullanıcı vendor byte'ları ve
TinyXML2'nin **500** varsayılanı değişmedi.

`SURGE()` import işaretidir; `DllShutdown(timeout)` IO/iş/hook/provider kapanışını bekler.
Yükleyen süreç DLL referansını bırakmadan önce kapanışı tamamlamalıdır.
Hazır olma/port yönetim export’ları kaldırıldı; araçlar mevcut EALsxPort/LSX
protokolünü gözleyerek gerçek başlangıcı doğrular.
Windows handle/worker hazırlığı dışında config/IO/hook işi DllMain'de çalışmaz.

`surge.cfg` ardından `surge_override.cfg`, RE'deki gibi executable klasöründen
okunur. Veri kökü `%LOCALAPPDATA%\SURGE`; ayrıntılı tanılama logu yalnız Debug'da vardır.
Tam runtime, kaynak uyarlamaları ve kanıt sınırları:
[RUNTIME_BINDING.md](docs/RUNTIME_BINDING.md),
[RUNTIME_CURRENT_STATUS.json](metadata/RUNTIME_CURRENT_STATUS.json).

Uygulama DLL'i Brownfield ile aynı üç C export’u açar: DllMain@1, SURGE@2, DllShutdown@3. Ayrıntı ve ordinal'ler
[PUBLIC_DLL_ABI.md](docs/PUBLIC_DLL_ABI.md) içindedir. Önceki 548-byte ABI
iddiası RE kanıtıyla doğrulanamadığı için geri çekilmiştir. **Gerçek
Windows/MSVC ve tam Golden kabulü bu ortamda yapılmadı; önceki startup-build-fix
paketiyle oyun açılışı kullanıcı tarafından doğrulandı.** Özgün consumer'ın
`anadius64!anadius` importuyla doğrudan ikili uyumluluk iddia edilmez.

## Bu oturumda doğrulananlar

- LLVM-MinGW ile Windows x64 DLL ve dinamik test host'u derlendi.
- Mevcut kaynak testlerine Golden/parser, paket/metadata, hedef koruma ve standart CRT testleri eklendi.
- Güncel parser/test sonuçları [docs/evidence/VDF_PARSER_STATUS.json](docs/evidence/VDF_PARSER_STATUS.json) içinde.
- DLL gerçekten yüklenerek calendar, SHA-1 ve config parser çalıştırıldı.
- Kaynak/başlık/test/vendor dosyaları ve tüm ürün ağacı ayrı manifestlerle denetlenir.

Bunlar gerçek Windows/MSVC, oyun veya Consumer kabul testi değildir.
İlk test çalıştırmasının iki sorunu ve son başarılı tekrarın ham kayıtları da
`metadata/` altında korunur. `BUILD_STATUS.json` önceki DLL temelini,
`INFRASTRUCTURE_STATUS.json` önceki altyapı aktarımının sonuçlarını kaydeder.
Vendor uyumluluk temeli `VENDORING_STATUS.json` ve önceki VDF build/test kimliği
`VDF_PARSER_STATUS.json` tarihsel kayıtlardır. Güncel bağlantılar
[RUNTIME_CURRENT_STATUS.json](metadata/RUNTIME_CURRENT_STATUS.json), arayüz
[PUBLIC_DLL_ABI.json](metadata/PUBLIC_DLL_ABI.json), bu kaynakla yapılan testler
[FIDELITY_FIX.json](docs/evidence/FIDELITY_FIX.json) içindedir.

Brownfield'in dört gerçek PE Golden aracı ve MSVC link denetleyicisi uyarlanmıştır.
Altı aşamalı zincir, ilk hatada durma ve özgün DLL'i koruma kurallarıyla bağlıdır.
Resource, üç C export ve dört Golden ürün hedefi artık tanımlıdır.
Gerçek MSVC final Golden kabulü yeni kaynakla tekrar çalıştırılmalıdır;
fixture testleri bunun yerine kullanılmaz.
[Golden akışı ve ürün hedefleri](docs/GOLDEN_PROTOCOLS.md).

Kaynak QA: `python -B scripts/run_source_qa.py`. Yeni kaynak ZIP'i **düz köklüdür**;
`CMakeLists.txt` doğrudan ZIP kökünde açılır. Paketleme manifest farkını otomatik kabul etmez.

Kaynak kapsamı: [docs/SOURCE_SCOPE.md](docs/SOURCE_SCOPE.md).
Güncel kaynak paketi: `python -B scripts/package_source.py --output ../source.zip`.
Güncel paket normal dosyalarla taşınır; Asio'nun iki kilitli hedef metni için
Windows symlink yetkisi gerekmez. Doğrulamalı açma için
`python -B scripts/extract_source.py ../source.zip ../source-clean`.
Güncel uygulama doğrulaması: [metadata/RUNTIME_CURRENT_STATUS.json](metadata/RUNTIME_CURRENT_STATUS.json).

Hook motorunun SAFE-FREEZE, transaction/rollback ve cleanup retry sözleşmesi ile
vendor commit kanıtları: [HOOK_ENGINE_HARDENING.md](docs/HOOK_ENGINE_HARDENING.md).

Kapanış taramasında başarısız resume/close sonrasında kaynak sahipliğini koruma ve
yeniden deneme sözleşmesi: [UNLOAD_RECOVERY.md](docs/UNLOAD_RECOVERY.md).

LSX açılış mesajındaki SDK kimliği sahibin verdiği sürüme sabitlenmiştir; eski
`SDK` CFG alanı uygulanmaz. Biçim ve güncelleme kuralı:
[SDK_IDENTITY.md](docs/SDK_IDENTITY.md).

Sunucu portu `3226–3236` arasında otomatik seçilir; eski `ServerPort` CFG alanı
uygulanmaz. Aralık doluysa başlangıç hata verir. Gerçek seçilen port `EALsxPort`
üzerinden bildirilir. Ayrıntılar: [AUTOMATIC_LISTENER.md](docs/AUTOMATIC_LISTENER.md).

## Ayrı kaynak karşılaştırmaları

28 Ekim 2026 incelemesi: [Bluefield — 42 konu](docs/COMPARISON_HOOK_REFERENCE.md) ve
[Reverse Engineering — 29 konu](docs/COMPARISON_RE.md). Kaynak gövdesi sadakati,
onaylı farklar ve eksik ürün bağlantıları ayrı işaretlenir. Bu tur yalnız rapor
ekler; uygulama değişikliği içermez. Kanıt/pinler: [COMPARISON_REVIEW.json](docs/evidence/COMPARISON_REVIEW.json).

Karşılaştırma tabloları önceki sabitlenmiş snapshot incelemesidir. Scalar-only
config sınıflandırma hatası ve bu turdaki gerçek bağlantılar
[Runtime değişiklik tablosunda](docs/RUNTIME_CONNECTIONS.md) açıklanır.

Brownfield resource metadata da bağlıdır: `resources/preloader_version.rc`
birebir aktarılmış, sabit VERSIONINFO profili korunmuştur. Son talimatla
SURGE PNG yalnız profil avatar verisinin yerine geçer; PE resource değildir.
[Resource sözleşmesi](metadata/RESOURCE_METADATA_CONTRACT.json).

Önceki C arayüzü snapshot doğrulaması: Release Wine173/173, Debug etkilenen testler6/6,
Linux18/18. Temiz ZIPten BUILD_TESTING=OFF derleme ve gerçek public host geçti.
Açık kabul işleri: [REMAINING_WORK.md](docs/REMAINING_WORK.md).

Tarihsel isim/kapatma düzeltmesinde beş export vardı. Son kullanıcı talimatıyla
DllWaitReady/DllServerPort kaldırıldı; güncel Production Brownfield ile aynı üç export’u açar.
Güncel kayıt: [DLL_NAME_CHANGE.json](docs/evidence/DLL_NAME_CHANGE.json).

Beş kullanıcı MSVC profil logundaki manifest/test derleme sorunlarının düzeltmesi:
[MSVC_PROFILE_FIX.md](docs/MSVC_PROFILE_FIX.md). Golden hedefleri eksikse sonuç
BLOCKED/exit3 kalır; düzeltilmiş kaynakla native MSVC profil tekrarı gereklidir.

Son kullanıcı loglarında Compile/Debug runtime ve CRT6/6 PASS doğrulandı.
Kalan Native C2668 ad çakışması ve test-only düzeltme:
[MSVC_NATIVE_LOOKUP_FIX.md](docs/MSVC_NATIVE_LOOKUP_FIX.md).

Güncel düzeltme: [avatar, OLE32, Native C2589 ve Golden hedefleri](docs/GOLDEN_AVATAR_FIX.md).
Kaynak paketleyici ZIP yanında otomatik `.zip.sha256` üretir.

Son production düzeltmesi: [Python cache, statik CRYPT32 ve dist](docs/PRODUCTION_IMPORT_FIX.md).
`build_production.cmd` tüm Golden + son runtime kontrolleri geçince
`dist/preloader_l.dll`, `.dll.sha256` ve `LOCAL_DIST_RECEIPT.json` üretir.
Ham linker dosyası `build/.../preloader.unsigned.dll` olarak korunur.

Güncel düzeltme: [Bluefield hook uyumu](docs/HOOK_ALIGNMENT.md) ve
[tüm Brownfield metadata alanlarının denetimi](docs/REFERENCE_METADATA_AUDIT.md).
Final DLL timestamp’i **0xCD268EB1**, linker sürümü **14.0**, Rich header **yok**
olmalıdır; bunlar Brownfield’in sabit profil değerleridir, derleme saati değildir.
Ham `preloader.unsigned.dll` ile Golden sonrası `dist/preloader_l.dll` ayrıdır.
Güncel kanıt: [BLUEFIELD_ALIGNMENT_STATUS.json](docs/evidence/HOOK_ALIGNMENT_STATUS.json).

Son düzeltme: [başlangıç registry kilidi ve beş build girişi](docs/STARTUP_BUILD_FIX.md).
Oyun açılmaması kullanıcı tarafından bildirildi; yeni kaynak için oyun tekrarı
henüz yapılmadı. Güncel kanıt: [STARTUP_BUILD_FIX_STATUS.json](docs/evidence/STARTUP_BUILD_FIX_STATUS.json).

Güncel build düzeltmesi: [Brownfield Asio Windows hedef tanımları](docs/ASIO_WINDOWS_TARGET_FIX.md).
Önceki startup-build-fix paketinden oyun açılışı kullanıcı tarafından doğrulandı.

Son Windows IO düzeltmesi: [WINDOWS_IO_FIX.md](docs/WINDOWS_IO_FIX.md). Gulrak
üründen kaldırıldı; dört kalan vendorun kaynak byte’ları korunur. Production’ın
üç C export’u korunur; surge.log yalnız Debug’da yazılır.

Son export düzeltmesi: Production yalnız DllMain@1, SURGE@2, DllShutdown@3
açar. DllWaitReady/DllServerPort kodu ve ordinal4/5 kaldırıldı; runtime host
gerçek LSX greeting/profile ve EALsxPort üzerinden başlangıcı doğrular.

Final dist import DLL sırası kullanıcı talimatıyla Anadius’a eşitlenir:
CRYPT32 → WS2_32 → KERNEL32 → USER32 → SHELL32 → MSWSOCK. URSYNEX ayrı
kopyadaki descriptor kayıtlarını permüte eder; ham linker çıktısı korunur.
