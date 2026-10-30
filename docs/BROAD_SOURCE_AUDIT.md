Sonraki kullanıcı Windows/Golden/native ve oyun kabulü ile DLL boyut ölçümleri: [güncel kabul kaydı](DLL_SIZE_AND_NATIVE_ACCEPTANCE.md). Aşağıdaki test ve bekleme bilgileri inceleme anının tarihsel kaydıdır.

# Tüm kaynak ve adaptör denetimi

Önceki inceleme yeni RE paketinin deltasına odaklanmıştı; beş düzeltme bütün codebase'in denetlendiği anlamına gelmiyordu. Bu tur kapsam, ürünün **277 kaynak/başlığının tamamına**, gerçek build ayrımına ve uygulama sahibinden Windows adaptörüne giden çağrı zincirlerine genişletildi. Yeni bulgular yalnız önceki beşliyle sınırlanmadı.

## Kapsam ve kanıtın anlamı

| Kaynak grubu | Dosya | Yapılan işlem |
|---|---:|---|
| Production C++ | 64 | Gerçek CMake nesneleri ve bağlantı sınırı; OS/adaptör, exception, sahiplik ve eşzamanlılık yolları |
| Korunan reference C++ | 74 | RE kaynak karşılaştırması, test/host sembol bağlantıları, production'a girmeme kontrolü; silinmedi |
| Başlıklar | 139 | Kaynak karşılığı, arayüz/layout değişiklikleri, doğrudan CPP kullanıcıları |
| Önceki dosya eşlemesi bulunanlar | 248 | Yeni RE girdisiyle yeniden token karşılaştırması:211eşleşme,37incelenmiş fark |
| Önceki dosya eşlemesinin dışındakiler | 29 | Uygulama bağlantıları, açık politikalar ve izinli donor/yeniden oluşturma adaptörleri |

Son iki satır ilk üç satırın başka bir sınıflandırmasıdır; toplamı artırmaz. Include yolları, yorum/boşluk ve kaydedilmiş namespace dönüşümleri token karşılaştırmasında ayrılır. Bu, compiler AST veya özgün DLL ile davranış eşdeğerliği ölçümü değildir.

[Dosya bazındaki CSV](evidence/BROAD_SOURCE_MATRIX.csv), [hash ve sembol bağlantıları içeren JSON](evidence/BROAD_SOURCE_INVENTORY.json) ve [doğrulama kaydı](evidence/BROAD_AUDIT_VALIDATION.json) birlikte okunmalıdır. JSON, gerçek138COFF nesnesi ve derlenmiş test/host nesnelerinin dış sembol bağlantılarını da içerir. Bir testin bir sembole bağlanması o fonksiyonun bütün dallarının çalıştığını göstermez. Inline, virtual, callback ve GetProcAddress yolları ayrıca değerlendirilmiştir; satır/dal kapsam yüzdesi uydurulmamıştır.

## Bu tur düzeltilenler

| Kimlik | Kaynak / sorun | Düzeltme ve regresyon kanıtı |
|---|---|---|
| B01 | Execution mutex `owner_48` alanı native kilitten önce okunuyordu; diğer thread'in yazımıyla C++ veri yarışı. | Layout korunarak atomic_ref erişimi. Aynı gerçek execution.cpp, düzeltmeden önce TSAN race/exit66; sonra temiz. Dört thread ile üç mutex tipinde120000kritik bölüm testi. |
| B02 | CommandLineToArgvW listesini dönüştürme/vektöre ekleme exception'ında LocalFree yoktu. | Exception yolunda da serbest bırakma. Kontrollü dönüştürme hatasında önce frees=0, sonra frees=1; normal/null yollar korunur. |
| B03 | DllShutdown döndükten sonra caller thread kimliği unload taramasından dışlanmaya devam ediyordu. | RAII her dönüşte kimliği sıfırlar. Gerçek worker/cleanup-retry DLL fixture'ı iki başarısız ve bir başarılı kapanışın ardından sıfırı denetler. Tam native unload ispatı olarak sunulmaz. |
| B04 | Boş XML child metni `const char*`→string_view dönüşümünde nullptr olabiliyordu. | Somut serializer adaptörü eksik metni invalid_argument ile reddeder. Gerçek XML reader/router ile boş Id içeren QueryPresence/AreChunksInstalled ve ardından geçerli istek sınanır. Kaynak modelin diğer serileştirme kuralları korunur. |
| B05 | ShowIGO queue oluşturma/enqueue exception'ında event mutex'i kilitli kalıyordu. | Başarılı kilit ediniminden sonra RAII unlock. İki ayrı hata noktasında tam bir unlock doğrulanır; eski gövde aynı testi başarısız bitirir. |
| B06 | Owned event initializer oluşturma başarısızlığında guard=-1 ve init kilidi kalabiliyordu. | Optional abort callback; owned runtime guard'ı sıfırlayıp kilidi bırakır. İlk seed oluşturma bad_alloc, aynı owner üzerinde retry, tek seed ve peer thread erişimi sınanır. Pure modelin default abort'u no-op; özgün private CRT EH mekanizması icat edilmez. |
| B07 | Stage/backup henüz yol döndürmeden write/close/copy hatasında handle veya geçici yol sahipliği kaybolabiliyordu. | Native backend edinimden önce kayıt ayırır, handle/yolu başarılı close/delete'a kadar saklar; recover_atomic retry eder. Gerçek geçici dosya/handle üzerinde kontrollü WriteFile/CloseHandle/DeleteFileW/CopyFileW hataları; hedefin korunması ve başarı sonrası tekrar yazım sınanır. Eski backend aynı testi geçemez. |
| B08 | Bilinmeyen entitlement alanının gerçek tanısı yerine dialog'da `UNKNOWN MESSAGE` yazıyordu. | Sayılı UTF8 tanı gerçek gövdede Unicode'a çevrilir; SURGE başlığı korunur. Gerçek Environment kodunda yalnız MessageBoxW yakalanır; eski gövde alan/değer kontrolünü geçemez. |
| B09 | IPv6 connect hook ::1'e yönlendiriyordu; somut server yalnız127.0.0.1dinliyordu. | Aynı seçilmiş portta exclusive IPv4 ve IPv6-only loopback listener. Önce ::1 testi Connection refused; sonra iki aile, IPv6 aday çakışması/IPv4 adayının bırakılması ve ortak oturum sınırı geçer. IPv6 hostta yoksa yalnız o kontroller açıkça skip edilir. |
| B10 | Extra worker son readiness bayrağını profile/factory hazır olmadan yayımlıyordu. | Worker tamamlanması private bayrakta; profile ve accepting yayımlandıktan sonra callback'in üçüncü kapısı açılır. Gerçek Application, Windows mutex ve worker'lar ile kontrollü duraklatma; eski gövde erken dönüşü yakalar, yeni gövde bekleyeni doğru anda bırakır. |

B01/B02/B05/B06/B04, kaynak modelini C++/somut ürün içinde çalıştırmanın exception/eşzamanlılık sınırlarıyla ilgilidir; özgün makine kodunun aynı hatayı taşıdığı iddia edilmez. B03/B07/B08/B09/B10 sahip olduğumuz uygulama/adaptör birleşimindeki açıklardır. B09/B10 önceden onaylanmış IPv6 ve başlangıç hazır olma davranışlarını tamamlar; yeni CFG, export, port veya SDK politikası getirmez.

## İnceleme alanları

| Alan | Denetlenen sınır / kullanılan test aileleri |
|---|---|
| Application startup/shutdown | Worker join, readiness/profile yayını, iptal, sahiplik, timeout/retry; application_runtime, startup_publication, cleanup_retry, public lifecycle |
| Environment OS bağlantıları | Unicode/path, optional main/override CFG, avatar, dizin, log, gerçek dialog; configuration_platform, environment_dialog, application_runtime |
| Execution/argv/flags | Pre-lock owner erişimi, mutex tipleri/layout, argüman cleanup, QPC'nin callback öncesi yayını; execution, execution_concurrency, startup + TSAN |
| Config/VDF/hash/order | PEGTL action/rewind/partial state, iki Config2 girişinin kökü ve sıra, kaldırılan SDK/port override'ları; parser, vdf_parser_regressions, configuration, config_application/hash/order |
| File/path/status/license/token | Stream false/throw/close, path kökü, standard filesystem ve mevcut kaynak kuralları; file_io, path, status, license_file_integration, licensing, token, windows_effects |
| Tam copy helper | Mod/identity fallback, zaman karşılaştırması, ters close ve fatal sınırı; file_copy, file_copy_windows_integration |
| Atomik persistence | Edinim/yol devri, stage/backup başarısızlığı, reverse rollback ve retry sahipliği; atomic_persistence, atomic_staging, achievement_runtime/writer |
| Hook kurulumu/rollback/unload | Queued apply, SAFE-FREEZE build-copy pinleri, incomplete rollback, trampoline drain ve handle retry; hook_transaction, hook_freeze, unload_guard, hook_engine_fixtures |
| Callback binding/lease/locks | Aynı canlı state/original slots, common Guard/Pause, LastError, close/drain; production_callback, native_callback, host_init, hook_adapter_integration |
| Connect | Exact loopback, tam const adres kopyası, dar attempts kilidi, readiness; production_network_hook, network_hook + gerçek listener testleri |
| Hook11 | 32768UTF16 sınırı, protected read, suffix/null/fallback, SDK sonucu/LastError; crypt_path_probe + callback testleri |
| Registry/process/folder | A/W ABI sınırları, handle/map kimliği, registry wait sırasında Pause; registry, host_callback, production_callback |
| Aktif transport/messages | Asenkron greeting, iki worker, oturum FIFO, limit/deadline, cancellation buffer ömrü; server, handler_pool, network_policy, session_flow, listener_port |
| Event queue | Seed/guard retry, mutex exception, reserve/commit/retry ve allocation/persistence sırası; event_queue/integration, misc_handlers, session_flow |
| Achievement | Map rollback, XML/INI batch, event'in kalıcılıktan sonra yayını; content_handlers, achievement_storage/runtime/writer, atomic_persistence |
| Handler/router | Komut kayıtları, XML null text, stable attribute kopyaları; handlers, misc_handlers, dispatch_integration, messages |
| TinyXML bridge/printer | Document/wrapper ömrü, ErrorStr, attribute sırası/escaping, backend node kimliği; xml_reader, xml_printer, dispatch_integration |
| Codec/hash/encoding | AES/padding/offset ve mevcut hata domain'i, SHA1, UTF8 ve filesystem codepage ayrımı; crypto, hash, encoding + gerçek encrypted roundtrip |
| Provider/extra library | Vtable slotları ve mevcut fixture, yükleme/sahiplik, worker bağımlılıkları; online, libraries, application_runtime. Gerçek harici provider ABI'sinin tümü doğrulanmadı. |
| Zaman | Wrap/signed arithmetic ve UTC/timestamp parsing; calendar, timestamp, utc |
| Sabit kimlikler | SDK10.6.2.31/wire10,6,2,31, product path ve CFG override olmaması; configuration, application_runtime, source requirement kontrolleri |
| Reference transport/XML | Tüm68CPP + başlıklar kaynak karşılaştırmasında ve var olan model/integration fixture'larında; production dışı oldukları linker-map ile ayrıca denetlenir |
| Reference composition | Diğer6CPP + başlıklar, test/source API bağlantıları korunur; private CRT veya yeni controller admission ürün yoluna alınmaz |

## Elenen şüpheler ve açık sınırlar

TinyXML2 başarılı parse için ErrorStr'ın nullptr olabileceği şüphesi elendi: vendored implementation boş C string döndürüyor; mevcut başarı testi empty() denetliyor. Gereksiz reader değişikliği yapılmadı. Soyut boundary'lerdeki bilinçli throw'lar otomatik olarak eksik implementasyon sayılmadı; örneğin LicenseFiles hashing/token işini sahiplenir, output-only achievement reader istek okumaz, native greeting senkron yolu kullanmaz.

Execution'ın QPC cache'i Application constructor'da callback yayımından önce hazırlanır. Event seed/guard'ları da session yayımından önce oluşturulur. Bu gerçek ürün sırası, bütün bağımsız C++ model kullanıcılarının concurrent cold initialization'ının ispatı değildir. Başlangıç dışındaki soyut model kullanımları kendi sözleşmesiyle sınanmalıdır.

Kalan doğrulama sınırları: yeni kaynak için native MSVC/Golden ve gerçek oyun/consumer tekrar koşulmadı; yabancı provider ABI'si, özgün private CRT/global-layout/EH, her Windows/filesystem hata kodu ve bütün OS sürümleri eşdeğer ilan edilmez. Atomik çoklu dosya güncellemesi power-loss/cross-process transaction değildir. Arbitrary/unaligned yabancı pointer domain'i, mevcut typed SDK sözleşmesinin ötesinde kabul edilmiş sayılmaz.

## Korunanlar ve yeniden çalıştırma

64production/74reference ayrımı, üç C export/ordinal, static Crypt32, dört vendorun byte pinleri, version metadata profili, profile/fallback avatar byte'ları ve Release log yasağı korunur. Production build bağımsız olarak tests/source API kapalı derlenir; raw compiler DLL incelemesi final Golden/MSVC kabulü yerine konmaz. C++ source arayüzlerine optional virtual yöntemler eklendiğinden geliştirme tüketicileri tüm kaynaklarla yeniden derlenmelidir.

Normal native acceptance launcher tüm testleri toplar. Ayrı Release/Debug CTest çalıştırmaları, source QA, gerçek linker-map sınırı ve production-only runtime host üç load/request/stop/unload çevrimi doğrulama kaydındadır. İlave dört bağımsız test target'ı execution concurrency, native atomic staging, Environment dialog ve startup publication alanlarını kapsar; mevcut testlere de hata yolları eklendi.

Bu rapor tamamlanan kapsamı ve bulunan hataları kaydeder. Hata sayısı, başka hata bulunamayacağına dair bir kapanış ölçütü değildir. Sonraki değişikliklerde aynı dosya/çağrı alanları ve native tüketici senaryoları yeniden değerlendirilmelidir.

Güncel sonuç: tam Release193/193, etkilenen Debug17/17, kaynak QA16/16.
ZIP temiz klasöre açılarak QA16/16 ve tests/source API kapalı sıfırdan
production derlemesi geçti; bu DLL de üç gerçek public-ABI runtime çevrimini
tamamladı. Ayrıntılı sınıflandırma, eski kodda başarısız olan regresyonlar ve
platform sınırları doğrulama JSON/XML/log dosyalarındadır.
