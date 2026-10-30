# Güncel görev ve kaynak yetkisi

Current owner branch decision: agents must never edit, commit, push or update
main directly, including through file-content APIs. Only the owner merges
reviewed PRs into main. Reuse the permanent bugfix branch for routine fixes,
dependency updates, documentation, tests and small maintenance; retain and
synchronize it after each merge without rewriting shared history. Dedicated
descriptively named branches are for explicitly scoped, sustained audits/projects,
not one or two small PRs. Temporary exceptions require an owner instruction.
Retire completed branches only after owner confirmation, work-preservation and
open-PR checks, recording their tip SHAs; never delete main/bugfix or active audit
branches. See the mandatory rules at the start of [AGENTS.md](../AGENTS.md) and
the [GitHub workflow](../docs/GITHUB_WORKFLOW.md).

Current owner release instruction: prepare draft candidates every Friday at
16:30 Europe/Istanbul when new commits exist. The owner publishes manually.
The weekly workflow runs the full source/portable/native MSVC/Production gates;
this is the explicit scheduled exception to the manual-only Windows CI rule.
Release series and revisions are external tag/package labels supplied through
manual start_tag and continued from draft state, never product-code defaults.
See [weekly draft releases](../docs/WEEKLY_RELEASE.md).

Current owner CI decision: keep the repository private and reduce automatic
hosted-runner use. Source QA and portable tests stay automatic on PRs/main;
Windows Native and Production/Golden run only on manual workflow dispatch.
Local build/acceptance scripts and product/vendor contracts remain unchanged.
Skipped Windows jobs do not count as native acceptance. See the
[review workflow](../docs/GITHUB_WORKFLOW.md).

Güncel sahip görevi: yeni yüklenen Asio kaynak arşivini byte eşitliğiyle vendorlamak.
Başlık sürümü1.38.2 değişmiyor; snapshot ZIP hash ve commit yorumuyla sabitlenir.
Diğer üç vendor ve ürün politikaları korunur. PR ayrı hazırlanır; birleştirme
sahibine aittir. [Kapsam](../docs/ASIO_UPDATE.md).

Güncel sahip kararı: kaynak arşivi byte eşitliğiyle GitHub başlangıcına alındı.
CI ve PR çalışma kuralları ayrı dal/PR olarak hazırlanır; birleştirme sahibine
aittir. Bu iş runtime/vendor davranışını değiştirme yetkisi vermez.
[Çalışma düzeni](../docs/GITHUB_WORKFLOW.md).

Güncel sahip kararı: mevcut gömülü C++ avatar yöntemi korunacak; son sağlanan
PNG yalnız profil avatarı yerine alınacak. Haricî dosya zorunluluğu, fallback,
CFG önceliği ve dosya yazma davranışı değişmez. [Kapsam](../docs/AVATAR_UPDATE.md).

Güncel devam: sahibin Windows/MSVC loglarındaki net_failure_contracts tahsis
enjeksiyonu kusurunu düzeltmek. Pin'li Asio'nun gerçek MSVC aligned CRT yolu
test executable'ında yakalanır; production/vendor baytları ve seçenekleri
değiştirilmez. Eski dört başarılı profil ile204/205 Native sonucu tarihsel
kanıttır; düzeltilmiş native testin kabulü yeniden istenir. Önceki kapsamlar
aşağıda korunur. [İnceleme](../docs/NATIVE_ALLOCATION_FIXTURE.md).

Güncel sahip talimatı: doğrulanan dört-TU üretim/referans ayrımını, DC-005–011
dar araç/test/ifade temizliğini ve crypt_observed Debug TLS kapısını birlikte
uygulamak. Kaynak API/test kapsamı ve bütün shutdown/native sahiplik düzeltmeleri
korunur. DC-012–019 tasarım adayları bu değişikliğe dahil değildir.
[Güncel kapsam](../docs/BUILD_CLEANUP.md). Aşağıdaki önceki görevler tarihsel
yetki/kapsam kayıtlarıdır; donor emekliliği onaylanmış değildir.

Son devam turu native socket sahipliği ve IO retirement düzeltmesidir. Belgelenen
would-block sonucu exact handle ile retry edilir; belirsiz sonuçta kör reclose
yapılmaz, owner ve image lease korunur. Kapanış native kayıtlar ve callback drain
ile kabul edilir. [Güncel kapsam](../docs/SOCKET_RETIREMENT.md). Donor emekliliği
onaylanmadı; aşağıdaki eski açık bulgu kayıtları kendi tarihsel kapsamlarıyla kalır.

Son devam turu: hook cleanup iç döngüsünün tek denemeye çevrilmesi, gerçek DLL
worker/uygulama/binding üzerinde hata testleri, timer containment ve native
socket-close sahipliğinin karşılaştırılması. [Güncel karar](../docs/SHUTDOWN_RECOVERY_AUDIT.md).
Socket-close hata yolu çözülmedi; donor emekliliği BLOCKED kalır.

Güncel kapsam: doğrulanmış transport/ek DLL sahiplik hatalarını düzeltmek,
regresyon testlerini eklemek ve Brownfield/Bluefield kapanış incelemesini
sürdürmek. **Donor emekliliği onaylanmış değildir.** Seçilmiş branch incelemesi,
tüm kaynakların semantik kapanışı sayılmaz. Güncel karar ve kanıtlar:
[inceleme](../docs/DONOR_RECOVERY.md),
[doğrulama](../docs/evidence/recovery/VALIDATION.json),
[makine kaydı](../metadata/RECOVERY_REVIEW.json).
Aşağıdaki önceki görev açıklamaları tarihsel yetki kaydıdır; güncel durumu
`RUNTIME_CURRENT_STATUS.json` belirler.

Güncel görev: **SURGE-RE-COPY-20261029**. Kullanıcı eksik genel kopyalama
yardımcısının orijinal DLL’den çözülmesini istedi.048d20/04895c/0489f4 gövdeleri
pseudocode+assembly’den yeniden oluşturulur; avatar mode2 üzerinden tam gövdeye
bağlanır. Bu, filesystem kopyalama için sınırlı bir uyumluluk ekidir. Özel CRT
ithali, vendor değişikliği veya diğer hook’ların değiştirilmesi kapsamda değildir.
[Kurtarım ve doğrulama](../docs/RE_COPY_RECOVERY.md).

Önceki görev: **SURGE-IDENTITY-20261029**. Kullanıcı ürün kodunda kalan
eski adların tutarlı düzeltilmesini istedi. Config/veri/avatar/provider adları
ortak ürün tanımına bağlanır; hatalar surge.cfg der. Kaynak seviyesindeki
shutdown durum adları DllShutdownStatus / DLL_SHUTDOWN_* olur, değerler0..4
ve üç export/ordinal korunur. Köken yorumları, eski raporlar ve mevcut JSON
şema kimlikleri tarihsel kayıt olarak korunur.
[Adlandırma düzeltmesi](../docs/PRODUCT_IDENTITY_FIX.md).

Önceki görev: **YF-WINDOWS-IO-20261029**. Kullanıcı Windows filesystem farkını
düzeltmeyi, aktif başka kullanımı kalmayan Gulrak’ı tamamen kaldırmayı istedi.
RE standard filesystem/fstream adaptörü geri bağlanır; Gulrak vendor ağacı ve
CMake/API bağımlılıkları kaldırılır. Diğer dört vendor ve TinyXML2 500 korunur.
Asio Windows hedef tanımı düzeltmesi bu pakete dahildir. Kullanıcının export ve
log soruları açıklanır: son talimatla Brownfield üçlüsü Production’da kalır, surge.log yalnız Debug
yazar; eski Debug logunun Release’te değişmediği de kontrol edilir.
Önceki paketle oyun başlangıcı kullanıcı tarafından doğrulandı.
[Güncel kayıt](../docs/WINDOWS_IO_FIX.md).

Önceki görev: **YF-ASIO-WINDOWS-TARGET-20261029**. Kullanıcı production Asio
uyarılarının Brownfield’de çözülmüş olduğunu belirtti. Eksik _WIN32_WINNT ve
WINVER0x0601 build tanımları supplied Brownfield’den alınır; vendor patch veya
genel warning suppression yapılmaz. PE metadata profili değişmez. Kullanıcı
önceki paketle oyun başlangıcını doğruladı; güncel ayarla gerçek MSVC tekrarı
ayrı kanıt gerektirir. [Düzeltme](../docs/ASIO_WINDOWS_TARGET_FIX.md).

Önceki görev: **YF-STARTUP-BUILD-20261029**. Kullanıcı beş build başlığını ve
cache JSON çıktısını düzeltti; ardından dist DLL ile oyunun hiç başlamadığını
bildirdi. Config isteğe bağlı kalır. Bluefield FIX19’da bulunan eksik native
registry wait Guard/Pause bağı kontrollü başarısız testle kanıtlanıp uyarlanır.
Yeni dosya-arama politikası, vendor değişikliği veya oyun PASS iddiası eklenmez.
Native logdaki bağımsız fixture NOMINMAX hatası da bu görevde düzeltilir.
[Bulgular ve sınır](../docs/STARTUP_BUILD_FIX.md).

Önceki görev: **YF-BLUEFIELD-ALIGN-20261029**. Kullanıcı Bluefield’in 11. hook
implementation’ını doğru referans olarak seçti; korumalı dosya adı okuma, UTF16
selector/fallback, original çağrı gözlemi, toplu hook aktivasyonu ve unload/thread
guard bu kapsamda uyarlanır. Önceki RE trap seçimi geçersizdir. Kullanıcı ayrıca
Compilation Timestamp dahil tüm derlenmiş metadata’nın Brownfield’e göre
kontrolünü istedi: MUSTAVEX/ARYRA/URSYNEX’in tüm beyan edilmiş alanları seçilir.
Timestamp0xCD268EB1, linker14.0, Rich-yok; checksum yeniden hesaplanır.
Önceki preserve-compiler talimatları tarihsel kalır. Beş export, statik CRYPT32,
PNG’nin PE resource olmaması, standard CRT ve immutable vendor kuralları korunur.
[Hook uyumu](../docs/HOOK_ALIGNMENT.md),
[metadata denetimi](../docs/REFERENCE_METADATA_AUDIT.md).

Önceki görev: **YF-PRODUCTION-20261029**. Kullanıcı son dosyanın dist içinde
üretilmesini ve 11. hook için Brownfield/Bluefield gibi statik CRYPT32
importunun korunmasını istedi. Önceki dinamik-only lookup kararı bu talimatla
geçersizdir; doğrudan SDK hedef referansı + crypt32.lib kullanılır. Callback/
selector ve beş export korunur. Yeni native logdaki ilk5 Golden PASS, son
FALCION Python cache FAIL düzeltilir. Alt yorumlayıcılara -B/PYTHONDONTWRITEBYTECODE
aktarılır; yalnız bilinen manifest-owned generated cache temizlenir. FALCION
gevşetilmez. Altı Golden kontrolü + final isimli DLL runtime çevrimleri sonrası
dist/preloader_l.dll, SHA256 ve receipt yazılır; yayın değildir. Önceki maddeler
tarihsel görev kayıtlarıdır. [Düzeltme](../docs/PRODUCTION_IMPORT_FIX.md).

Önceki görev: **YF-GOLDEN-AVATAR-20261029**. Kullanıcı dört eksik Golden hedefinin
tamamlanmasını istedi. Hedefler Yellowfield kaynak/export/import/CRT sözleşmesine
bağlanır; timestamp/linker/Rich/OS kimliği korunur, yalnız checksum kopyada
tamamlanır. Fixture sonuçları gerçek MSVC kabulü sayılmaz. PNG için açık yanıt:
“Yalnız PE resource olmasın; mevcut Anadius avatar verisinin yerine geçsin”.
Profil PNG ordinary read-only C++ veri olur; özgün ayrı fallback RE verisinden
korunur. VERSIONINFO değişmez. Önce eklenen COM known-folder bağımlılığı RE
ortam değişkeni okumasına döndürülür; OLE32 importu kaldırılır. Native C2589
hatası core tüketicilerine NOMINMAX yayılımıyla düzeltilir. Yeni ZIP yanında
birebir SHA256 yan dosyası teslim edilir. Aşağıdakiler önceki görev kayıtlarıdır.

Güncel düzeltme: **YF-MSVC-LOOKUP-20261029**. Son beş kullanıcı logunda
Compile/Debug/CRT PASS, CRT6/6 doğrulandı. Native derlemesindeki
`unexpected()` C2668 çakışması test fixture namespace nitelemesiyle düzeltilir.
On iki çağrı açıkça bağlanır; senaryolar atılmaz. MSVC dışı derleyicide aynı
global bildirim derleme regresyonu olarak tutulur. Uygulama/vendor/resource
ve beş export değişmez. Golden BLOCKED/exit3 korunur.


Güncel düzeltme: **YF-MSVC-PROFILES-20261028**. Kullanıcının beş native profil
logundaki iki gerçek uyumsuzluk düzeltilir: ek MSVC DLL manifesti ve test-only
`__int128`. Ürün resource allowlisti gevşetilmez; `/MANIFEST:NO` tüm DLL
profillerine uygulanır. Test oracle standart64-bit olur, mevcut testler korunur.
CRT yalnız seçili test hedeflerini derler. Golden hedefleri eksikse sonuç
BLOCKED/exit3 kalır; hedef değerleri uydurulmaz veya fixture
politikasından etkinleştirilmez. Uygulama, vendor, resource byte ve exportlar
değişmez. Yeni kaynak için native MSVC tekrarı henüz yapılmamıştır.


Güncel düzeltme: **YF-WIN-ZIP-20261028**. Kullanıcının Windows/MSVC configure
aşamasındaki Asio1607/1607 vendor doğrulama hatası ele alınır. Özgün vendor
hashleri değiştirilmeden iki kilitli Asio aliasının ZIP taşıması normal dosya
hedef metni olarak yapılır; Windows symlink yetkisi önkoşulu kaldırılır.
Yalnız birebir kilitli iki descriptor kabul edilir. Uyuşmazlık gerçek dosya
yollarını/hashleri yazar. Uygulama C++ kodu, exportlar ve TinyXML2=500 korunur.


Güncel isim düzeltmesi: **YF-DLL-NAMES-20261028**. Kullanıcının talimatıyla
readiness ve port exportları `DllWaitReady` ve `DllServerPort` olarak adlandırılır.
İmzalar, davranışlar ve ordinal4/5 korunur; tüm çağıran araçlar güncellenir.
Takip talimatıyla yinelenen `YellowfieldShutdown` ve ordinal6 kaldırılır;
dışarıya tek `DllShutdown` kalır. Kapanış sırası ve mevcut DllShutdown sonuç
kodları korunur. Toplam beş export vardır. Önceki kaynak ZIPleri korunur.


Güncel devam görevi: **YF-PUBLIC-ABI-20261028**. Kullanıcı açık kalan işleri
gördükten sonra “devam edelim” dedi. Uygulama DLL'inin altı C export'u sabitlenir,
C++ kaynak API'leri ayrı geliştirme DLL'ine taşınır; gerçek başlangıç, socket,
shutdown ve unload aracı eklenir. SURGE marker'ı RE'deki RET-only davranışın
isim uyarlamasıdır; DllShutdown mevcut gerçek cleanup'a bağlanır. Önceki
548-byte ABI iddiası kanıt bulunamadığı için geri çekilir. Yeni export ordinal4–6
Yellowfield'e aittir; özgün consumer importları veya EXE değiştirilmez.
Kapsam ve sınırlar: [PUBLIC_DLL_ABI.md](../docs/PUBLIC_DLL_ABI.md).

Güncel görev: **YF-RUNTIME-BIND-20261028**. Kullanıcının “o bağlantıların
saglanmis olmasi gerekiyordu” düzeltmesi, önceki ürün hedefindeki eksik somut
bağlantıları tamamlamayı gerektirir. Asio/ghc gerçek uygulama yolları, mevcut tam
config uygulama yolu, kaynak constructor alanları, XML/crypto/router/persistence,
hook/provider ve DLL başlatma/kapatma sahipliği bu görevde bağlanır. Önceki
report-only kısıtı yalnız karşılaştırma turuna aitti; bu uygulama görevinin
engeli değildir. Vendor byte'ları ve TinyXML2=500 korunur. Bluefield uygulama
kodu alınmaz. Detay ve davranış farkları: [RUNTIME_BINDING.md](../docs/RUNTIME_BINDING.md).

Ek düzeltme: “brownfieldde resources kismindaki metadata alinmamis alinacakti”.
Brownfield `resources/preloader_version.rc` VERSIONINFO dosyası birebir alınır;
sabit profile SURGE adı/sürümü uydurulmaz. Bu eski RCDATA kararı son kullanıcı düzeltmesiyle kaldırılmıştır; aşağıdaki güncel avatar sözleşmesi geçerlidir.
Diğer PE header/ABI kararları bu resource aktarımıyla otomatik seçilmez.

Sonraki düzeltme: “src ve include icinde hala garip alt klasor altinda klasorler var.”
Brownfield'in tek modül katmanlı düzeni uygulanır. 177 dosya taşınmış, 261 kaynak
gövdesi include satırları dışında byte-identical doğrulanmıştır. Yollar ve
kanıtlar [LAYOUT_MIGRATION.json](../docs/evidence/LAYOUT_MIGRATION.json) içindedir.

Önceki görev: **YF-COMPARE-20261028**. Kullanıcı Bluefield ve RE source için
iki ayrı fark/eksik/sapma tablosu istedi ve “devam edelim” dedi. Bu tur
yalnız inceleme, kaynak kanıtı, dokümantasyon ve paketleme içerir; uygulama
kodu veya davranışı değişmez. Bluefield kararları otomatik ürün talimatı sayılmaz.
Tablolar: [Bluefield](../docs/COMPARISON_HOOK_REFERENCE.md), [RE](../docs/COMPARISON_RE.md).

Önceki uygulama görevi: **YF-VDF-PEGTL-20261028**. Kullanıcı “pegtl vdf grameri vardi”
düzeltmesinden sonra RE'deki mevcut grameri PEGTL'ye bağlama açıklamasına
**“devam edelim”** dedi. Bu, yalnız mevcut Config2/VDF kuralları ve action
semantiğini koruyarak parser backend'ini PEGTL'ye geçirme yetkisidir.
Bluefield karşılaştırma kaynağı kalır; uygulama kodu alınmaz.
Ayrıntı: [VDF_PARSER.md](../docs/VDF_PARSER.md).

Önceki **YF-VENDOR-20261028** ve **YF-VENDOR-CONTINUE-20261028** görevlerinde beş
orijinal vendor arşivi kuruldu ve uyumlu LLVM-MinGW 20251118 / Clang 21.1.6
ile doğrulandı. Vendor byte'ları, sürümleri ve TinyXML2'nin **500** varsayılanı
korunur; 100 patch'i uygulanmaz. Yeni storage/transport politikası seçilmez.

Önceki görev: **YF-INFRA-20261028**, Brownfield altyapısının Yellowfield'e uyarlanması.
Kullanıcı, eksik aktarım listesine yanıt olarak **“evet devam edelim”** dedi.
Bu talimat aşağıdaki altyapı çalışmasının yetkisidir; aynı işlem için yeniden
onay istenmez. Eski başlangıç paketinin report-only evresi bu görevi engellemez.

## Kapsam

Dokümantasyon sistemi, Golden araçları/zinciri, root düzeni, metadata sahipliği,
resource kontrolü, scriptler, yerel build/test orchestration, uyumlu testler,
komutların hata/durum görünürlüğü ve kaynak paketleme.
Hedeflerin özgün kaydı [INTEGRATION_REQUIREMENTS.json](../metadata/INTEGRATION_REQUIREMENTS.json),
güncel aktarım kaydı [BROWNFIELD_TRANSFER_MATRIX_V2.json](../metadata/INFRASTRUCTURE_TRANSFER_MATRIX.json).

Altyapı görevinde uygulama `.cpp`/başlık semantiği değiştirilmedi. Yeni CRT kontrol host'u
bir araçtır; compiler standart CRT'sini sınar, özel kurtarılmış CRT sağlamaz.
Brownfield'in uygulama ABI'sini isteyen altı eski host aynen alınmaz. Yellowfield'in
mevcut source host'u ve CTest hedefleri gerçek arayüzleriyle kullanılır.

## Kalıcı hedefler

- First-party namespace `preloader_l`, görünen kimlik SURGE, kullanıcı görseli birebir.
- Reverse Engineering temel fonksiyon kaynağı; Bluefield yalnız 11. hook ve gerekli hook lifecycle için açıkça seçilmiş donor, diğer modüller karşılaştırma kapsamındadır. Frozen girişler değişmez.
- 11-hook MinHook hedefi; runtime patch/rollback/quiescence kabulü ayrıca gerçek native kanıt ister.
- Dört vendor aktiftir: Asio, MinHook, PEGTL ve TinyXML2. Kullanıcı düzeltmesiyle
  kullanılmayan Gulrak kaldırılmıştır. Sürüm/byte pinleri değişmez; kopyalama,
  aktif runtime kabulü sayılmaz. Güncel durum metadata/RUNTIME_CURRENT_STATUS.json.
- Standart CRT; özel CRT için otomatik geri dönüş yok.
- `%LOCALAPPDATA%\SURGE`, Debug-only `surge.log`; runtime adapterında RE'nin
  executable-relative main→override sırası `surge.cfg`/`surge_override.cfg`
  adlarıyla kullanılır. Mevcut RE Config2/VDF grameri PEGTL ile bağlıdır.
- Opsiyonel `preloader_l_online.dll` mevcut RE version2/factory/virtual-slot
  sözleşmesine bağlıdır. Gerçek harici provider/native uyumluluk kabulü açık kalır.
- Library senkronizasyonu, yayın ve resmî release kapsam dışıdır.

## Kanıt sınırı

Kaynak QA, sentetik parser testleri, çapraz DLL linki, Wine çalıştırması,
gerçek Windows/MSVC ve oyun/Consumer kabulü farklıdır. Birinin PASS sonucu
diğerine aktarılmaz. Golden ürün değerleri belirsizken araçlar dosyayı değiştirmeden başarısız çıkar.
Şimdi Brownfield metadata değerleri açık kullanıcı talimatıyla seçilmiştir;
üç public export ve diğer kullanıcı istisnaları korunur.

Ek export talimatı: DllMain@1, SURGE@2, DllShutdown@3 ordinal düzeni Brownfield
ile aynı kalacak. Test/yönetim için eklenen DllWaitReady/DllServerPort kaldırılır;
araçlar mevcut EALsxPort/LSX akışına bağlanır. Import edilen DLL’lerin descriptor
sıralaması ayrı bir konudur. Sonraki açık kullanıcı talimatıyla yalnız finalized
kopyada CRYPT32, WS2_32, KERNEL32, USER32, SHELL32, MSWSOCK sırası seçilmiştir;
raw compiler çıktısı değiştirilmez ve read-only Golden kontrolleri gerekir.

Son SURGE-FIDELITY-FIX-20261029 talimatı dört denetim bulgusunun tamamını giderir:
Bluefield dar network sayacı ve ortak callback Guard kapsamı, RE type2 event
mutex sözleşmesi, Debug tanılama içeriği ve güncel durum işaretçileri. Standart
CRT, Debug-only dosya logu, vendor byte'ları ve Golden politikaları korunur.
Tarihsel runtime raporları yeniden yazılmaz; yeni durum ayrı kaydedilir.

Teslim sürümü, kod adı ve revizyonu yalnız dış arşiv adı ve yanındaki checksum
dosyasında kullanılır. Kaynak, araçlar, build başlıkları, receipt ve paket içi
metadata/dokümanlara taşınmaz. Paketleme aracı çıktı yolunu çağırandan alır.

## Whole-source review continuation

The owner requires review across all application and retained model sources, not
only the latest input delta or an arbitrary defect count. Review mapped source
differences, owned adapters, native boundary call chains, ownership/exception and
concurrency failure paths, and suitable test contributions. Adopt proven fixes and
record concrete pre/post evidence and remaining native/domain limits. Preserve
existing product policies and frozen evidence; do not import unrelated private
CRT or controller restrictions. Current review: docs/BROAD_SOURCE_AUDIT.md.
