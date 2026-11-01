# QA ve kabul sınırları

Portable executable kapsamı: config/hash/parser, AES/PRNG, bilgi/profil, content,
entitlement, achievement/image/presence, XML, licensing/token/hash, path/calendar,
startup ve online model testleri. Runtime integration gerçek loopback TCP ile
parçalı/coalesced frame, profile, content/entitlement, offline auth, avatar yolu,
achievement XML/INI, restart reload, encrypted LSX ve 8 istemciyi çalıştırır.

Windows Native girişinde aynı app kaynakları MSVC ile derlenir. CTest sonrası
alt süreç fixture'ı yeni DLL'i LoadLibrary ile açar; connect port3216 redirect,
GetConfig/GetProfile, DllShutdown=0, environment restore ve modülün gerçekten
unload edildiğini sınar. Ayrı version-2 TEST_ONLY provider ile profile/auth/avatar
native parametre aktarımı sınanır. Gerçek oyun veya orijinal provider kullanılmaz.
CRT profilinde ayrıca warm-up sonrası 20 start/connect/stop döngüsünün debug CRT
allocation dengesi kontrol edilir. Bu, özel orijinal CRT eşdeğerliği iddiası değildir.

Source invariant katalogu sekiz alana ayrılır: graph, import/dependencies,
ABI/CRT, loader ownership, route coverage, operator launchers, license/vendor
pins, hygiene/default-asset. Bunların PASS'i statik source kabulüdür; native,
Golden veya önceki BROWNFIELD authoritative QA evreni gibi sunulmaz.

Paket verifier'ı receipt içindeki PASS metnine güvenmez; exact-byte tam member
manifesti, CRC, duplicate/path/symlink/quota ve aynı source kontrollerini tekrarlar.
İsteğe bağlı independently trusted --sha256 tüm ZIP kimliğini bağlar. Native ve
runtime kanıtları source digest'i ve gerçek binary hash'iyle ayrı kayıtlanır.

Final ZIP'ten tek temiz çıkarım + executable replay dış son QA kaydına bağlanır.
Sealed ACCEPTANCE.json dosyasında gelecekteki exact ZIP replay'i PENDING kalabilir;
aynı ZIP SHA'sını taşıyan son QA raporu bu sonucun dış kanıtıdır.

FIX02 için yeni portable selector testleri object türü, null, okunamayan path,
Unicode/surrogate, case-sensitive suffix, NUL sınırı ve prefix kurallarını sınar.
Windows DLL alt süreç fixture'ı gerçek CRYPT32 çağrılarını yükleme öncesi/sonrası
karşılaştırır: geçmeyen dosya/blob result ve LastError, null/suffix companion
seçimi, gerçek detour logu ve unload sonrası orijinal çağrı. Bu native fixture
bu ortamda çalıştırılmış sayılmaz. Source policy eksik gerçek target ve eksik
CRYPT32 link bağımlılığını da negatif mutasyonla reddeder.

FIX03 yeni portable stage-plan testi opsiyon kombinasyonlarını, original
slotların dolmasını, yalnız queue çağrılarını ve create/queue/module/symbol
hatalarını sınar. Native test ayrıca staging loglarının tek apply öncesinde
olduğunu, commit kaydını ve yeni startup kimliğini doğrular; Windows'ta henüz
çalıştırılmamıştır. Source policy per-hook direct enable ve eksik batch apply
mutasyonlarını reddeder. Kullanıcı logunda aşama aralığı görülmesi, belirli
Windows thread deadlock'unun gözlendiği veya düzeltildiği kanıtı değildir.

FIX04 startup compatibility testinde sekiz başlangıç senaryosu ve present-directory
hatası vardır: tam 14 profil alanı, altı settings alanı, game yanıtının bütün sabit
alanları, bağımsız game duration lookup, kaynak SDK, cache yolları, iki gerçek PNG
boyutu ve custom avatar yoluna dosya yazılmaması. Current sonuçlar source digest
ile evidence içinde bağlanır. Native fixture dört config senaryosunu provider
yokken ve altı senaryoyu sentetik provider ile çalıştıracak şekilde hazırlanmıştır;
bu ortamda native senaryolar NOT_RUN.

FIX05 iki ek Application entegrasyon grubu taşır: anonim gerçek CFG ve HF19'den
uyarlanan ownership/nesting/comment vakaları. Ayrıca byte-identical Golden tool
öz testi sentetik PE'lerde çalışır. Toplam 25 C++ grup + bir tool öz testi,
HF19 evreninin veya beş aşamalı yeni native DLL Golden kabulünün yerine geçmez.
Namespace ve unreachable-abort mutasyonları kaynak policy negatiflerine eklendi.

FIX06 adds real five-stage orchestration and the inherited link-command checker
selftest to portable/CTest tool groups. Total: 25 C++ + 3 tool groups. Synthetic
pipeline cases run actual gate programs and reject failures in every stage,
read-only mutation, link-policy errors, source drift, PDB/sensitive material and
unsafe publication. Source-policy mutations additionally bind production.def,
Golden link flags, driver invocation and stage order. These results accept build
infrastructure only. Windows/MSVC/NMake and real-DLL Golden acceptance remain
NOT_RUN until their Windows receipts exist; game startup remains OPEN.

## FIX07 güncel durum

Kullanıcı son sürümde surge.cfg olmadan başlangıç ve profilin çalıştığını bildirdi.
Önceki no-config OPEN bulgusu USER_CONFIRMED düzeyinde kapanır; exact DLL SHA ve
Golden receipt sağlanmadı. FIX07 varsayılan adı SURGE olur, sayısal ID'ler korunur.
Eski CFG'nin exact etkisi ve düzeltmenin 14 alanlı profile eşitliği yeni entegrasyon
testindedir. Kapsam 26 C++ + 3 tool grubudur. FIX07 native/Windows/DLL Golden
NOT_RUN; eski CFG ile tüketicide anadius görülmesinin exact nedeni açık kalır.
Ayrıntı PROFILE_CONFIG_FIX07.md içindedir.

## FIX08 dependency notu

FIX07 kullanıcı tarafından çalışır olarak doğrulandı. FIX08 exact HF19 pristine
MinHook/TinyXML2 pinlerine geçer. Asio/IOCP ve PEGTL/VDF özgün kanıtları ile
BLUEFIELD backend seçimleri ayrı şeylerdir; DEPENDENCIES_FIX08.md güncel açıklamadır.
Önceki XML depth=100 vendor uyarlaması owned reader'a taşınır. Derin bozuk XML
hata sırası/footprint ve Windows native eşdeğerliği açık sınırdır.

## FIX09 Asio

Aktif gerçek TCP yolu Asio'ya taşındı; Windows IOCP source guard'ı vardır. Yeni
Asio integration grubu ile toplam 27 C++ + 3 tool grubu bulunur. Frame/profile/
crypto/persistence regresyonları aynı uygulama kodunu kullanır. Allocation failure,
large-write, FIFO, cap ve stop/lifetime testleri gerçek loopback üzerinden çalışır.
Portable sonuç native IOCP veya gerçek DLL Golden sonucu değildir. PEGTL ve
filesystem sonraki ayrı kapsamdır. ASIO_TRANSPORT_FIX09.md güncel tasarımdır.

## FIX10 Config2/PEGTL

Yeni configuration_test_pegtl_differential grubu sealed FIX09 çıktısıyla 8766
byte girdinin tam ağaç/action/error/BOM durumunu karşılaştırır. Corpus hash'i
source policy'ye bağlıdır; mevcut 115 parser ve 53 profile/config assertion'ı
ayrıca kalır. Toplam 28 C++ + 3 tool = 31 executable/tool grubu vardır.
Bunlar Windows/native/gerçek Golden DLL veya özgün runtime eşdeğerlik kabulü
değildir. Native fixture aynı FIX10 build marker'ını ister.

## FIX11

33 groups = 30 C++ + 3 tools. New path unit (2137 Linux checks) and real Unicode
config/cwd integration (47 checks); existing full profile and frozen PEGTL corpus
remain. Asio OOM test separately proves owned Session recovery and observes
ready=false for terminal arbitrary dispatch/rearm faults. A negative-control
translation unit restores the old catch loop; its outcome is recorded separately.
Windows path cases, MSVC/Native/CRT, actual DLL Golden/game and LSan are not inferred.
