# BLUEFIELD / preloader_l — RE kaynaklarından farklar, FIX14

**Soru:** Kullanıcının verdiği tam reverse engineering paketinde kod bulunmasına rağmen farklı uygulanmış ne var; neden farklı; hangisi bağımlılık geçişi, kullanıcı isteği, derleme uyarlaması veya kendi uygulama kararı?

**Bu rapor özgün DLL binary/runtime eşdeğerlik raporu değildir.** Native test eksikliği bir kaynak farkının tespitini engellemez. Kaynak farkı otomatik hata/geri alma kararı değildir. FIX14 yalnız Asio upstreamini ve raporlama/QA metadatasını değiştirir; bütün src dosyaları FIX13 ile byte-identical.

## Kapsam ve kaynaklar

- RE arşivi: `0734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9`; 2951 dosyanın tamamı hash ile yeniden doğrulandı; 151 modül/payload grubu.
- Güncel src: **115 dosya** = 82 doğrudan archive-member karşılaştırması + 2 symbol/asset projection + 31 uygulama/entegrasyon dosyası.
- 82 doğrudan karşılaştırma: **50 normalize kaynak eşleşmesi, 26 kaynak farkı, 6 scoped extraction**. 26 farkın tamamında insan değerlendirme kaydı var; bu sayı26 bug demek değildir.
- 4247 lexical işlev konumu yardımcı envanterdir. İsim tokenı veya RVA bulunması aktif production binding kanıtı sayılmaz.
- Orijinal bütün paket dosya/hash envanteri: metadata/ORIGINAL_PACKAGE_FILES_FIX13.json. Yeni current crosswalk: docs/RE_SOURCE_CROSSWALK_FIX14.csv. Modüller: docs/RE_MODULE_COVERAGE_FIX14.csv. Her387 RE kaynak dosyası: docs/RE_PACKET_SOURCE_COVERAGE_FIX14.csv. Ham26 dosya diffi: docs/RE_DIRECT_SOURCE_DIFFS_FIX14.patch.
- Kapsam sınırı: dosyaların tümünün envanterlenmesi ve açık farkların incelenmesi, bütün olası semantik farkların matematiksel olarak bulunmuş olması değildir. İncelenmemiş/çözümlenmemiş alanlar örtülü PASS sayılmaz.

## Nasıl sınıflandırıldı?

| Sınıf | Yorum |
|---|---|
| UPSTREAM / UPSTREAM_PARTIAL | İstenen dependency ağacı/engine değişimi veya kısmi bağlama. Vendor iç farkları kendi uygulama patchi sayılmaz. |
| USER_REQUEST | Kullanıcının açık kimlik/avatar/path/log/failure-atomic taleplerine bağlı fark. Varsayılan karar korunması. |
| BUILD_PORTABILITY | Include/namespace, MSVC uyarısı, Linux QA veya açık tip uyarlaması. |
| APPLICATION / INTEGRATION / SOURCE_BRANCH | Kaynakta görülen kendi adapter/scheduling/error/policy seçimi; ayrı gerekçe ve etki gerekir. Dependency tarafından zorunlu olduğu varsayılmaz. |
| QA / PROVENANCE | Test oracle kapsamı, teşhis veya stale açıklama borcu; runtime kod farkından ayrı. |

Toplam **38 değerlendirme kaydı**: 15 belgelenerek korunan, 22 karar/inceleme bekleyen, 1 güncel crosswalk ile açıklığa kavuşturulan provenance kaydı. **Bunlar bug sayıları değildir.** Ayrıca aşağıda FIX13te kapanmış17 eski sapma var; yeniden açık borç sayılmaz.

## Öncelikli uygulama borçları

| Öncelik | Kayıt | Kaynakta görülen fark | Sonraki çalışma |
|---|---|---|
| P1 | D09 | Salt okuma için write ownership koşulu | Read/write/status yetki kapsamını ayıracak küçük değişiklik ve dış XML örneği |
| P1 | D08 | Locale65001/strict conversion/default-character/retry dalları bağlı değil | Kaynak encoding karar tablosu ve targeted adapter karşılaştırması |
| P1 | D07 | Status sharing/reparse/error/permissions dalları basitleştirilmiş | Caller etkisi ve Win32 status adapter kapsamı |
| P1 | D01/D12 | Main/worker/provider/extra-DLL phase farklı | Tek phase grafiği ve gereksinim kararı |
| P1 | D02/D03/D04 | Pool sayısı, pipelining ve terminal accept farklı | İstenen concurrency/retry sözleşmesi; Asio iç implementationı ile karıştırma |
| P1 | D14 | Process exit dalı session exceptionına dönüyor | Process/service/session hata karar tablosu |

P1, hemen körlemesine değiştirme emri değildir. Önce neden ve istenen davranış belirlenir. FIX14 bu tabloda hiçbir runtime borcunu otomatik geri almadı.

## Ayrıntılı değerlendirme kayıtları

### DEP01 — Asio snapshot güncellemesi

**Sınıf:** UPSTREAM · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** YES_FOR_VENDOR_TREE

**RE kaynağı:** `anadius-server-source/server.cpp:92` (`Session::frame_completed`)

**Güncel kaynak:** `src/transport/service.cpp:5` (`<asio/io_context.hpp>`); `third_party/asio/include/asio/version.hpp:21` (`ASIO_VERSION 103802`)

**Fark:** RE paketi Asio/server/execution davranışlarını çözümlüyor. Aktif backend gerçek upstream Asio; FIX14 kullanıcı arşivindeki 026dce snapshotını 8806a snapshotı yerine kullanır. Sürüm her ikisinde 1.38.2.

**Davranış/borç etkisi:** Kütüphane içindeki 240 değişen, 47 eklenen, 8 kaldırılan dosya kendi uygulama patchimiz sayılmaz.

**Dayanak:** Bu turdaki açık Asio güncelleme talebi; daha önce upstream Asio geçişi istendi.

**Önerilen sonraki işlem:** Koru; güncellemenin regresyon sonuçlarını değerlendir.

### DEP02 — MinHook upstream pini

**Sınıf:** UPSTREAM · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** YES_FOR_VENDOR_TREE

**RE kaynağı:** `hook-engine-source/upstream-provenance.json:1` (`<file>`)

**Güncel kaynak:** `third_party/minhook/include/MinHook.h:1` (`<file>`)

**Fark:** BROWNFIELD ile aynı upstream MinHook 1.3.4 snapshotı kullanılıyor; FIX14 bunu değiştirmedi.

**Davranış/borç etkisi:** Vendor gövde farkları upstream kaynaklıdır; bizim install/queue politikamız ayrı D05 kaydıdır.

**Dayanak:** Kullanıcının MinHook/TinyXML2 upstream eşitleme talebi.

**Önerilen sonraki işlem:** Koru; vendor hashleri ayrı doğrulansın.

### DEP03 — TinyXML2 upstream pini

**Sınıf:** UPSTREAM · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** YES_FOR_VENDOR_TREE

**RE kaynağı:** `anadius-xml-reader-source/reader.cpp:1` (`<file>`)

**Güncel kaynak:** `third_party/tinyxml2/tinyxml2.h:1` (`<file>`)

**Fark:** Upstream 11.0.0 master snapshotı kullanılıyor; vendor patchi yok. Paketin vendor derinlik değeri100, yeni upstream500; uygulama adapterı başarılı ağaçta100 denetler.

**Davranış/borç etkisi:** Vendor güncellemesi hata değildir. Parse sonrasındaki adapter davranışı D15 ile ayrıca değerlendirilir.

**Dayanak:** Kullanıcının TinyXML2 upstream eşitleme talebi.

**Önerilen sonraki işlem:** Koru; adapter ile vendoru aynı bulgu sayma.

### DEP04 — PEGTL üzerinde VDF/Config2 grameri

**Sınıf:** UPSTREAM · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** YES_FOR_ENGINE_NOT_GRAMMAR_POLICY

**RE kaynağı:** `anadius-parser-source/parser.cpp:19` (`class Parser`); `anadius-parser-source/parser.cpp:49` (`block_comment_body`)

**Güncel kaynak:** `src/configuration/parser.cpp:23` (`namespace vdf`); `src/configuration/parser.cpp:125` (`peg::parse<vdf::Grammar,Action,Control>`)

**Fark:** Pakette elle yazılmış çözümlenmiş parser, upstream PEGTL4 snapshotı ve VDF grammar/action/control ile ifade edilmiş. VDF ayrı bir alternatif backend değildir.

**Davranış/borç etkisi:** Yorumlar, iç içe objeler, duplicate subtree replacement, input rewind/action nonrollback ve tek UTF8 BOM pakette vardır; bunları uydurma özellik diye geri almak hatalı olur.

**Dayanak:** Kullanıcının PEGTL/VDF kaynağını çözme ve bağlama talebi.

**Önerilen sonraki işlem:** Koru; syntax/action farkı iddiasını paketteki karşı örnekle kanıtla. 8766-case differential corpus bağımsız binary oracle değildir.

### DEP05 — gulrak/filesystem kapsamı kısmi

**Sınıf:** UPSTREAM_PARTIAL · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** PARTIAL_CHOICE_NOT_MANDATORY

**RE kaynağı:** `anadius-path-source/paths.cpp:79` (`join_016b00`)

**Güncel kaynak:** `src/paths/module_relative.cpp:16` (`ghc::filesystem::path`); `src/application/providers.cpp:15` (`std::filesystem::status`)

**Fark:** gulrak1.5.17 yalnız module-sibling path construction için bağlı. std::filesystem IO/status ve çözümlenmiş Windows lexical path kodu da aktif.

**Davranış/borç etkisi:** Filesystem geçişinin tamamlandığı söylenemez; bu kısmi kapsam otomatik olarak bug da değildir.

**Dayanak:** Kullanıcının gulrak/filesystem inceleme talebi; exact Windows backend tercihi ayrıca değerlendirilmelidir.

**Önerilen sonraki işlem:** Path construction, status, encoding ve IO için ayrı sahiplik/kapsam kararı çıkar.

### U01 — Kimlik, namespace ve görünür profil

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-config-source/configuration.hpp:10` (`namespace`); `anadius-online-source/online.cpp:52` (`Online library version mismatch`)

**Güncel kaynak:** `src/configuration/defaults.cpp:8` (`state.username="SURGE"`); `src/runtime/path_kind.hpp:5` (`provider_filename`)

**Fark:** Teknik preloader_l ve görünür SURGE kimliği; include/namespace taşımaları. Numeric kullanıcı/persona, SDK ve diğer defaults kaynak ctor projeksiyonundan korunuyor.

**Davranış/borç etkisi:** Marka değişikliği kaynak sapmasıdır ama istenmiştir; namespace/include farkı işlev hatası değildir.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c

**Önerilen sonraki işlem:** Koru; kaynak constructor projeksiyonu ve isimleri birbirine karıştırma.

### U02 — Veri kökü ve config/provider konumu

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-config-source/configuration.cpp:255` (`state.user_directory`); `anadius-config-application-source/application.cpp:145` (`anadius_override.cfg`)

**Güncel kaynak:** `src/runtime/runtime_paths.cpp:22` (`data_(local_/"SURGE")`); `src/configuration/apply.cpp:145` (`deps.runtime_path`)

**Fark:** Eski kök yerine LOCALAPPDATA/SURGE; config ve provider DLL dizinine bağlanır. RuntimePaths tek yol sahibidir.

**Davranış/borç etkisi:** EXE ile DLL farklı dizindeyse config arama konumu değişir. Bu dosya sözleşmesi açıkça kaydedilmiştir.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c

**Önerilen sonraki işlem:** Koru; üç farklı filesystem işlevini tek bir backend iddiasıyla açıklama.

### U03 — Config olmadan başlangıç ve varsayılanlar

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-config-application-source/application.cpp:23` (`read_config2`)

**Güncel kaynak:** `src/application/application.cpp:84` (`const bool absent`); `src/configuration/defaults.cpp:4` (`State initial_state`)

**Fark:** Eksik config boş overlay olarak işlenir; mevcut bozuk/unreadable config hata kalır. initial_state bağımsız constructor projection sağlar.

**Davranış/borç etkisi:** Paketteki modüler API caller-seeded State ister; paketlenmiş uygulama bu durumu kurar. SDK alanının varlığı referans EXE kaynaklı olduğu anlamına gelmez.

**Dayanak:** Sohbette config olmadan başlangıcın düzeltilmesi ve çalışan profilin korunması talebi.

**Önerilen sonraki işlem:** Koru; defaultları rastgele değiştirme.

### U04 — Provider adı ve güvenilir yükleme dizini

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-online-source/online.cpp:37` (`anadius64online.dll`)

**Güncel kaynak:** `src/profiles/online_session.cpp:55` (`LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32`)

**Fark:** Yeni ad preloader_l_online.dll; yalnız module dizini, LoadLibraryExW restricted search flags, eski isim fallbacki yok.

**Davranış/borç etkisi:** Provider dependency arama davranışı LoadLibraryW ile birebir aynı değildir; güvenilir dizin isteğinin uygulamasıdır.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c

**Önerilen sonraki işlem:** Koru; provider factory hatası ve phase wait ayrı bulgulardır.

### U05 — Production log compile-out ve hassas veri gizliliği

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-messages-source/messages.cpp:125` (`log_received`)

**Güncel kaynak:** `src/application/logging.cpp:4` (`#if PRELOADER_L_DISK_DIAGNOSTICS`); `src/application/logging.hpp:7` (`enum class LogEvent`)

**Fark:** Production/Native/CRT disk logu derlemede kapalı; Debug/Diagnostic yalnız typed event adı yazar.

**Davranış/borç etkisi:** Protocol payload ve kimlik loglama kaynakta bulunsa bile bu talimatla kapatılmıştır. Tanılama ayrıntılarının fazla azalması Q02 kaydıdır.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c; sonraki mesajla seçilmiş boyut/adet kotaları reddedildi.

**Önerilen sonraki işlem:** Koru; log kotası veya CFG switchi yeniden icat etme.

### U06 — Resmî avatar ve cache override engeli

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-content-handlers-source/content_handlers.cpp:147` (`bool query_image`); `anadius-online-source/online.cpp:72` (`state.avatar_0a3a90 = state.last_avatar`)

**Güncel kaynak:** `src/handlers/content.cpp:159` (`s.online_0a3c3b!=0 && !own`); `src/application/avatars.cpp:6` (`AvatarStore::resolve`)

**Fark:** Resmî embedded SURGE fallback, mevcut module sidecar önceliği, explicit custom avatar salt okunur. Eski last_avatar kendi/özel avatarı değiştirmez.

**Davranış/borç etkisi:** Own-avatar provider indirmesi ve last_avatar kopyalama dalı da atlanıyor. Görünür seçim talimata dayanır; provider yan etkileri konusundaki kapsam D16 ile ayrıca işaretlenmiştir.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c

**Önerilen sonraki işlem:** İstenen avatar seçimini koru; yan-etki atlama kapsamını ayrı değerlendir.

### U07 — Atomik dosya/state/event persistence

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-content-handlers-source/content_handlers.cpp:113` (`bool grant_achievement`); `anadius-file-source/native_io.cpp:41` (`class NativeOutput`)

**Güncel kaynak:** `src/handlers/content.cpp:119` (`auto previous=`); `src/files/atomic_io.cpp:73` (`void atomic_batch`); `src/files/native_io.cpp:49` (`class NativeOutput`)

**Fark:** Kaynak direkt truncate/write, sonra diğer dosya ve event işlemlerini yapar. Aday staging/replace, preimage rollback, prepared event ve memory rollback kullanır.

**Davranış/borç etkisi:** Hata sırası, open/write/close zamanlaması ve bellek maliyeti değişir. Sırf farklı diye geri alınmaz; kullanıcı failure-atomic istemiştir.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c §3/§9 failure-atomic gereksinimi.

**Önerilen sonraki işlem:** Koru; READ guard yetki genişletmesini D09 ile ayır; crash-proof transaction iddiası yapma.

### U08 — Başlangıçta eski avatarları silmeme ve lazy fallback

**Sınıf:** USER_REQUEST · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-config-source/configuration.cpp:272` (`deps.remove_04948c`)

**Güncel kaynak:** `src/configuration/state.cpp:256` (`no cache enumeration/deletion`); `src/application/avatars.cpp:14` (`atomic_write`)

**Fark:** Kaynak _AVATAR_ içeren dosyaları enumerate/delete eder ve iki asset yazar. Aday eski dosyaları silmez, fallbacki ihtiyaçta create-only üretir.

**Davranış/borç etkisi:** Dosya oluşturma zamanı değişir; mevcut kullanıcı dosyasını koruma isteğidir. Achievement INI geciktirmesi bu gerekçeyle tekrar eklenmez.

**Dayanak:** Kullanıcının SURGE migration talimatı; SHA256 52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c

**Önerilen sonraki işlem:** Koru; U06 ile bağlı ama rejected INI delay ile aynı karar değil.

### P01 — Platform ve derleyici uyarlamaları

**Sınıf:** BUILD_PORTABILITY · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-achievement-runtime-source/utc.cpp:10` (`gmtime_05fffc`); `anadius-achievement-storage-source/storage.cpp:17` (`HostCrtNumbers::scan_decimal`); `anadius-hash-source/hash.cpp:46` (`std::fill`)

**Güncel kaynak:** `src/achievements/utc.cpp:12` (`#ifdef _WIN32`); `src/achievements/storage.cpp:20` (`sscanf_s`); `src/hashing/hash.cpp:46` (`std::uint8_t{0}`)

**Fark:** Windows gmtime64 korunup Linux gmtime_r eklenmiş; MSVC numeric sscanf_s branchi ve açık uint8_t zero kullanımı var.

**Davranış/borç etkisi:** Derlenebilirlik/platform desteği için küçük farklar; hash algoritması veya achievement formatı değişikliği değil.

**Dayanak:** C++20/MSVC uyumluluğu, uyarıları düzeltme ve portable QA yetkisi.

**Önerilen sonraki işlem:** Koru; platform farklı hata dallarını kaynak algoritma bozulması olarak sayma.

### P02 — Native DLL helper ve include macro koruması

**Sınıf:** BUILD_PORTABILITY · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-libraries-source/native_system.cpp:7` (`NativeSystemCalls::get_error_mode`)

**Güncel kaynak:** `src/windows/library_calls.cpp:3` (`#ifndef WIN32_LEAN_AND_MEAN`)

**Fark:** Direct original library helper yalnız include/namespace/SPDX ve #ifndef macro guardıyla adapte.

**Davranış/borç etkisi:** Normalizasyon gövde farkı diye işaretlese de yükleme algoritması değişmiyor.

**Dayanak:** Derleme/kimlik uyarlaması.

**Önerilen sonraki işlem:** Koru; raw diff sayısını bug sayısı yapma.

### D01 — Bootstrap ve ek DLL phase sırası

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-source/bootstrap.cpp:25` (`void Bootstrap::initialize`); `anadius-libraries-source/libraries.cpp:78` (`if (!from_main)`)

**Güncel kaynak:** `src/windows/dll_entry.cpp:64` (`unsigned __stdcall runtime_worker`); `src/windows/dll_entry.cpp:55` (`for(const auto* list:`)

**Fark:** Kaynak detached server/provider işçileri ve synchronous from-main sonra waited worker yüklemesi kullanır. Aday prepare→hooks→activate→her iki ek listeyi tek workerda yükler.

**Davranış/borç etkisi:** ExtraDllsFromMain adının ima ettiği thread/phase adayda korunmuyor. Asio veya MinHook upstream bunu zorunlu kılmaz.

**Dayanak:** Deterministik lifecycle istenmişti; tam phase dönüşümü ayrıca kaynak kararıdır.

**Önerilen sonraki işlem:** Kaynak phase grafiği ile istenen loader-lock disiplinini birlikte değerlendir; otomatik geri alma yok.

### D02 — IO/handler thread düzeni

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-server-source/server.cpp:96` (`scheduling_.launch_detached`)

**Güncel kaynak:** `src/transport/service.cpp:33` (`std::thread io_thread`); `src/transport/service.cpp:146` (`asio::thread_pool>(2)`)

**Fark:** Aday bir IO thread ve iki handler workerı sabitler; kaynak frame başına launch_detached modeli taşır.

**Davranış/borç etkisi:** Thread sayısı ve execution modeli kendi entegrasyon tercihidir; upstream Asio kullanmak bunları seçtirmez.

**Dayanak:** Asio geçişi istendi; bu worker sayılarının ayrıca açık kullanıcı seçimi yok.

**Önerilen sonraki işlem:** Kapasite, ownership ve kaynak sırasını incele; uygun karar verilmeksizin değiştirme.

### D03 — Session okuma/işleme/yazma ardışıklığı

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-server-source/server.cpp:92` (`Session::frame_completed`); `anadius-server-source/server.cpp:60` (`Session::start_025720`)

**Güncel kaynak:** `src/transport/service.cpp:81` (`void compute(`); `src/transport/service.cpp:66` (`self->outgoing.clear();self->consume();`)

**Fark:** Kaynak frame işçisini başlatır, girdiyi tüketip okumayı hemen yeniden kurar; greeting sync-send ve response queue kullanır. Aday response tamamlanana kadar aynı sessionda yeni isteği işlemez; greeting async_write yolundan geçer.

**Davranış/borç etkisi:** Pipelining/backpressure, gecikme ve response completion sırası değişir. Bu kaynakta görülen kesin farktır; bug hükmü henüz verilmedi.

**Dayanak:** FIFO/cipher güvenliği için kendi tasarımı; Asio şartı değil.

**Önerilen sonraki işlem:** Çoklu frame ve uzun response senaryolarıyla iki sözleşmeyi karşılaştır.

### D04 — Accept hatasında servis sonlandırma

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-server-source/server.cpp:134` (`void accept_completion_025720`)

**Güncel kaynak:** `src/transport/service.cpp:203` (`if(error&&!stopping.load())`)

**Fark:** Kaynak error logundan sonra async_acceptı yeniden kurar. Aday accept errorında listenerı kapatıp stopping=true/ready=false yapar.

**Davranış/borç etkisi:** Geçici accept hatası adayda kalıcı unavailable duruma dönüşebilir; upstream Asio bunu zorunlu kılmaz.

**Dayanak:** Önceki fail-visible servis kararı; retry sözleşmesi kaynakla farklı.

**Önerilen sonraki işlem:** Hata türüne göre retry/terminal ayrımını incele; bu görevde retry eklenmedi.

### D05 — Hookları tek batch ile etkinleştirme

**Sınıf:** INTEGRATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-host-init-source/host_init.cpp:21` (`effects.enable_hook_046a10`)

**Güncel kaynak:** `src/hooks/setup.cpp:25` (`effects.queue_hook_enable`); `src/windows/hooks.cpp:128` (`MH_ApplyQueued()`)

**Fark:** Kaynak create sonrası tek tek enable eder. Aday slotlar bağlandıktan sonra tek queued batch uygular.

**Davranış/borç etkisi:** Activation ve rollback zamanlaması farklı. MinHook API buna izin verir ama zorunlu kılmaz.

**Dayanak:** Mevcut geliştirme sözleşmesinde tek batch ve güvenli binding disiplini var; keyfî kota benzeri geri alma adayı değildir.

**Önerilen sonraki işlem:** Koru/değiştir kararını lifecycle gerekçesiyle değerlendir; vendor patchi diye etiketleme.

### D06 — Ek global kilitler ve attempt counter senkronizasyonu

**Sınıf:** INTEGRATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-network-hook-source/connect.cpp:32` (`++state.attempts_0a3888`); `anadius-native-source/native_callbacks.cpp:20` (`template<class F>`)

**Güncel kaynak:** `src/windows/hook_callbacks.cpp:28` (`serialization`); `src/windows/hooks.cpp:66` (`increment_attempts`); `src/application/application.cpp:158` (`std::scoped_lock lock`)

**Fark:** Callback serialization recursive mutexi, mutexli attempts artışı ve application-wide request state locku eklenmiş.

**Davranış/borç etkisi:** Race/lifetime koruması amaçlanır; reentrancy/ordering/concurrency kaynakla farklı.

**Dayanak:** Açık resource ownership/RAII isteği; kilit kapsamı kendi tasarım tercihi.

**Önerilen sonraki işlem:** Kilit grafiğini ve callback recursionını ayrı incele; blanket revert yok.

### D07 — Dosya status dalları basitleştirilmiş

**Sınıf:** SOURCE_BRANCH_NOT_BOUND · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-status-source/status.cpp:3` (`Status status_01ea20`); `anadius-status-source/status.cpp:19` (`StatsResult UsedStats::query`)

**Güncel kaynak:** `src/application/providers.cpp:14` (`status_recovered::Status file_status`); `src/profiles/online_session.cpp:82` (`OnlineSession::status_01ea20`)

**Fark:** Aday std::filesystem::status→kind2/3/9, permissions0 döndürür. Pakette errors2/3/53/123 mapping, readonly permissions, sharing32 FindFirst fallback ve reparse attribute query dalları var.

**Davranış/borç etkisi:** Yok/erişilemez/sharing/reparse ayrımı farklı olabilir. Bunlar mevcut kaynak dalları; native binary belirsizliği gerekçesiyle yok sayılmaz.

**Dayanak:** std::filesystem/gulrak geçişi bu dalları otomatik zorunlu kaldırmaz.

**Önerilen sonraki işlem:** Hangi status alanlarının callerda kullanıldığını belirle; bilinen dallar için adapter kararı çıkar.

### D08 — Encoding kod sayfası ve hata bayrakları

**Sınıf:** SOURCE_BRANCH_NOT_BOUND · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-encoding-source/encoding.cpp:7` (`filesystem_codepage_048ac0`); `anadius-encoding-source/encoding.cpp:11` (`convert_narrow_048ae8`); `anadius-encoding-source/encoding.cpp:15` (`convert_wide_048b30`)

**Güncel kaynak:** `src/application/environment.cpp:29` (`UINT cp=AreFileApisANSI()`); `src/application/environment.cpp:30` (`MultiByteToWideChar(cp,0`); `src/application/environment.cpp:39` (`std::string path_utf8`)

**Fark:** Pakette CRT locale65001 önceliği, narrow flags8, wide default-character kontrolü ve ERROR_INVALID_FLAGS retry var. Aday ACP/OEM seçer, narrow flags0 kullanır; path_utf8 std::filesystem üzerinden gider.

**Davranış/borç etkisi:** Özellikle UTF8 locale ve bozuk byte girişleri farklı kabul/ret veya farklı dosya adına dönüşebilir.

**Dayanak:** Bu bir upstream zorunluluğu değil; el altında bulunan kaynak yolları tam bağlanmamış.

**Önerilen sonraki işlem:** Önce locale/ANSI/OEM/default-character karar tablosu çıkar; sonra gerekçeli düzeltme değerlendir.

### D09 — Salt okuma işlemlerine require_write denetimi

**Sınıf:** UNSUPPORTED_POLICY_EXTENSION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-file-source/native_io.cpp:50` (`NativeFactory::open_input_039f70`); `anadius-online-source/online.cpp:4` (`exists_035b00`)

**Güncel kaynak:** `src/files/native_io.cpp:59` (`supported(mode,share);if(owner_)owner_->require_write`); `src/profiles/online_session.cpp:82` (`OnlineSession::status_01ea20`); `src/profiles/online_session.cpp:101` (`OnlineSession::read_binary_03b3f0`)

**Fark:** Yetkisiz yazmayı engellemek için eklenen ownership/reparse policy bazı read/status yollarında da çağrılıyor. Paketin input factorysinde bu kontrol yok.

**Davranış/borç etkisi:** Owner bağlı factoryde SURGE dışındaki regular read hedefi dosya açılmadan reddedilebilir; ör. haricî achievement XML. Özel avatar resolver farklı yoldan döndüğü için bütün custom avatarlar bozuk denemez.

**Dayanak:** Kullanıcı yetkisiz yazmama istedi; bunun bütün okumalara genişletilmesi için açık dayanak bulunmadı.

**Önerilen sonraki işlem:** Read/write/status yetkilerini ayıracak en küçük değişikliği planla; bu turda uygulanmadı.

### D10 — Path çözümleme adapterının farklı API kullanması

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-path-source/paths.cpp:43` (`parent_path_019820`); `anadius-path-source/paths.cpp:54` (`append_016b50`)

**Güncel kaynak:** `src/application/providers.cpp:23` (`ConfigurationEffects::resolve_path_0192d0`); `src/application/providers.hpp:32` (`Path parent_path_019820`)

**Fark:** Çözümlenmiş Windows lexical algoritmaları kaynakta mevcut ve bazı yollarda kullanılıyor. Effects parent/join/resolve yollarının bir kısmı std::filesystem parent_path/operator/lexically_normal ile karşılanıyor.

**Davranış/borç etkisi:** UNC, drive-relative ve özel prefix/.. spelling uç durumlarında kaynakla aynı olduğu varsayılamaz.

**Dayanak:** gulrakın kısmi bağlanması ve yeni adapter tercihi.

**Önerilen sonraki işlem:** Path kategorisi başına hangi algoritmanın kullanıldığını sabitle; sadece namespace farkı diye kapatma.

### D11 — Provider factory null sonucu hata yerine return

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-online-source/online.cpp:63` (`if (!state.object_0a3a88)`)

**Güncel kaynak:** `src/profiles/online.cpp:63` (`provider factory rejected`)

**Fark:** Paket null factory sonucunda explicit OutOfEvidenceInput atar. Aday loglayıp load_online fonksiyonundan döner.

**Davranış/borç etkisi:** Provider hata sinyali ve üst workerın tamamlanma yolu değişir. Bu normal valid profile yolunda fark var iddiası değildir.

**Dayanak:** Eksik/uyumsuz provider deterministik ele alınsın isteği var; tam null politikası ayrıca seçilmiş.

**Önerilen sonraki işlem:** Bu durum için istenen hata/yerel profile geçiş sözleşmesini açıklaştır.

### D12 — Source wait_steps yerine readiness/port kontrolü

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-online-source/online.cpp:113` (`wait_steps_036880`); `anadius-network-hook-source/connect.cpp:23` (`wait_steps_036880`)

**Güncel kaynak:** `src/profiles/online_session.cpp:97` (`OnlineSession::wait_steps_036880`); `src/windows/hooks.cpp:65` (`void wait_steps_036880`)

**Fark:** Provider count argümanını kullanmayıp port nonzero kontrolü yapar; connect service condition variable bekler. Kaynak Execution step/ready senkronizasyonu doğrudan bağlanmamış.

**Davranış/borç etkisi:** Bekleme ve başlangıç hata sırası farklı; Asio kullanımının zorunlu sonucu değil.

**Dayanak:** Yeni servis ownership/prepare-activate düzeniyle ilişkili.

**Önerilen sonraki işlem:** Phase bağımlılıklarını D01 ile birlikte karşılaştır; rastgele sleep ekleme.

### D13 — Conservative unload ve fault-domain korumaları

**Sınıf:** INTEGRATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-source/bootstrap.cpp:69` (`void Bootstrap::on_exit`); `preloader-source/preloader_core.cpp:142` (`crypt_query`)

**Güncel kaynak:** `src/windows/unload_guard.cpp:21` (`while(scanned<64ULL*1024*1024)`); `src/windows/original_effects.cpp:26` (`ReadProcessMemory`); `src/windows/dll_entry.cpp:99` (`for(;;)Sleep(1000)`)

**Fark:** Thread/stack taraması, self-pin, DllShutdown timeouts, okunamayan pointerı forward etme ve cleanup failureda unload reddi yeni native owner kararlarıdır.

**Davranış/borç etkisi:** False positive unloadu geciktirebilir. Stack tarama64MiB sınırı kullanıcı dosyası/read quota değildir; güvenlik taramasının boundudur.

**Dayanak:** Güvenli shutdown/unload ve ağır işin DllMain dışına alınması isteği; ayrıntılı algoritmalar kendi seçimi.

**Önerilen sonraki işlem:** Güvenlik gerekçesiyle ayrı değerlendir; input/cache kota bulgularıyla karıştırma.

### D14 — Kaynak exit/error etkisinin SessionFailure ile karşılanması

**Sınıf:** APPLICATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P1 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-license-source/license.cpp:53` (`exit_05cdc0`)

**Güncel kaynak:** `src/application/providers.hpp:96` (`void exit_05cdc0`); `src/application/application.cpp:162` (`catch(const PersistenceRecoveryRequired&)`)

**Fark:** License Effects exit_05cdc0 gerçek process exit yerine SessionFailure atar. NoResponse ve mesaj hataları da owned exception arayüzleri üzerinden taşınır.

**Davranış/borç etkisi:** Kaynağın processi sonlandıran dalı candidate session/worker failure kapsamına dönüşebilir. Çıktı formatı değişti demek değildir.

**Dayanak:** Host prosesini/lifecycleı koruma entegrasyonu; dependency gereği değil.

**Önerilen sonraki işlem:** Hangi hata process/service/session düzeyinde olmalı karar tablosu çıkar. Bu turda handler değiştirilmedi.

### D15 — XML derinlik denetimi parse sonrasına taşınmış

**Sınıf:** UPSTREAM_ADAPTER_TRADEOFF · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-xml-reader-source/reader.cpp:29` (`int parse(`); `anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.h:112` (`100`)

**Güncel kaynak:** `src/xml/reader.cpp:11` (`SourceDepthPolicy`); `src/xml/reader.cpp:62` (`if(source_depth_error_)return`)

**Fark:** Upstream pristine kalsın diye original100 limiti successful tree visitorı ile uygulanıyor. Derinlik hatasında özel adapter diagnostic literalı üretiliyor.

**Davranış/borç etkisi:** Bozuk/derin dokümanda hata önceliği, allocation miktarı ve diagnostic string farklı olabilir; 100 sayısı uydurma değildir.

**Dayanak:** TinyXML2 upstream güncellemesi istenmiş; adapter tradeoff ayrıca belgelenmeli.

**Önerilen sonraki işlem:** Upstream kodunu körlemesine geri alma; adapterın malformed/depth sözleşmesini değerlendir.

### D16 — Avatar seçim isteğinin provider yan etkilerine uzanması

**Sınıf:** APPLICATION_SCOPE_REVIEW · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-content-handlers-source/content_handlers.cpp:156` (`if(s.online_0a3c3b!=0)`); `anadius-content-handlers-source/content_handlers.cpp:162` (`s.saved_avatar_size_0a1130`)

**Güncel kaynak:** `src/handlers/content.cpp:159` (`s.online_0a3c3b!=0 && !own`); `src/application/application.cpp:135` (`state.avatar=selected_avatar`)

**Fark:** Kendi avatarını provider/cachein değiştirmemesi istenmişti. Uygulama bunun için own-avatar download/copy dalını tamamen atlıyor ve initialize_online sonrası seçili avatarı geri koyuyor.

**Davranış/borç etkisi:** İstenen görünür sonuç korunur; indirme/cache yan etkilerinin de kaldırılması daha geniş uygulama kararıdır.

**Dayanak:** Avatar seçimi kullanıcı talebidir; bütün provider side effectlerini kaldırma ayrı onaylı requirement olarak gösterilemez.

**Önerilen sonraki işlem:** Görünür seçim ile cache/download side effect sözleşmesini ayrı değerlendir.

### D17 — getenv yerine sahipli _dupenv_s kopyası

**Sınıf:** BUILD_ADAPTER_DIFFERENCE · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P3 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-startup-source/windows_platform.cpp:9` (`std::getenv`)

**Güncel kaynak:** `src/windows/startup_platform.cpp:17` (`::_dupenv_s`)

**Fark:** C4996 ve borrowed pointer lifetime için static CRT environment kaynağından owned copy alınır. Allocation/errno hatası system_error üretebilir.

**Davranış/borç etkisi:** Normal string aynı kaynaktan gelir; OOM/error dalı ve pointer lifetime farklıdır.

**Dayanak:** Uyarıları düzeltme talebiyle uyumlu adaptation; runtime failure farkı görünür olmalı.

**Önerilen sonraki işlem:** Koru/değiştir kararını warning/lifetime gerekçesiyle ele al; kaynak algoritma kaybı diye sayma.

### D18 — Çift configuration orchestration yolu

**Sınıf:** INTEGRATION_DECISION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-config-source/configuration.cpp:240` (`void initialize_configuration`)

**Güncel kaynak:** `src/configuration/state.cpp:237` (`void initialize_configuration`); `src/application/application.cpp:105` (`void initialize(){`)

**Fark:** Imported configuration initialization fonksiyonu paket içinde duruyor; aktif Application kendi apply/default paths/storage/env adımlarını tekrar compose eder.

**Davranış/borç etkisi:** İki orchestration gelecekte farklılaşabilir. Fonksiyon dosyada var diye aktif üretim bindingi var sayılmaz.

**Dayanak:** Bağımsız uygulama composition gereği; duplicate akışın kapsamı borçtur.

**Önerilen sonraki işlem:** Aktif call graphı ve ortak adımların tek ownershipini değerlendir; sırf TU azaltmak için birleştirme yapma.

### D19 — CRT/TLS/container/thread alt modellerinin host ile karşılanması

**Sınıf:** HOST_RUNTIME_SUBSTITUTION · **Durum:** KEEP_DOCUMENTED · **Öncelik:** P3 · **Dependency zorunluluğu:** HOST_LIBRARY_IMPLEMENTATION

**RE kaynağı:** `anadius-source/bootstrap.cpp:55` (`cleanup_threads_04a47c`)

**Güncel kaynak:** `src/application/events.hpp:20` (`std::deque<std::string>`); `src/application/events.hpp:12` (`std::recursive_mutex`)

**Fark:** Paketin std/CRT/thread/allocator modellerinin tamamı production içine kopyalanmaz; compiler CRT ve standard containers/upstream scheduler kullanılır.

**Davranış/borç etkisi:** Her unimported CRT dosyası eksik oyun özelliği değildir. Uygulama düzeyindeki exception/order kararları D01–D18de ayrı kaydedilir.

**Dayanak:** C++20 derlenebilir DLL, RAII ve upstream reuse hedefi.

**Önerilen sonraki işlem:** Host replacement olarak sınıflandır; eksik application feature sayısını şişirme.

### Q01 — Kümülatif metadata açıklamaları güncel davranışla karışmış

**Sınıf:** PROVENANCE_DEBT · **Durum:** DOCUMENTED_CURRENT_CROSSWALK · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** Üretim davranışı değil; mevcut metadata/QA kaynakları.

**Güncel kaynak:** `metadata/IMPORTS.json:1736` (`two exact PNG`); `metadata/ASIO_TRANSPORT_FIX09.json:15` (`maximum_clients`); `metadata/CONFIG2_PEGTL_FIX10.json:28` (`size/nesting guards`)

**Fark:** Bazı adaptation metinleri eski davranışı anlatıyor: original PNG iddiası, historical32 clients ve config guards. Hashler güncel olsa bile açıklama güncel implementation kanıtı değildir.

**Davranış/borç etkisi:** Yanlış bakım/denetim kararına yol açar. FIX14 yeni current assessment alanları ve kaynak crosswalkı ekler; tarihsel kayıtları değiştirmez.

**Dayanak:** Doğru ve izlenebilir raporlama gereği.

**Önerilen sonraki işlem:** Güncel otorite olarak bu raporu ve ASIO_UPSTREAM_FIX14ü kullan; geçmiş stage JSONlarını current contract sayma.

### Q02 — Test oracle bağımsızlığı ve aşırı kaba diagnostic

**Sınıf:** QA_DIAGNOSTIC_DEBT · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** Üretim davranışı değil; mevcut metadata/QA kaynakları.

**Güncel kaynak:** `tests/unit/configuration/test_pegtl_differential.cpp:1` (`<file>`); `src/application/application.cpp:52` (`void log_bad_xml`); `src/windows/dll_entry.cpp:89` (`catch(const std::exception& error)`)

**Fark:** Portable suite ve frozen FIX09 corpus regresyon kanıtıdır. Bazı testler intentional app değişikliklerine göre uyarlanmıştır. Fixed events privacyyi korurken exception detayını/phase ayrımını da silebiliyor.

**Davranış/borç etkisi:** PASS sayısı kaynak seçiminin doğruluğunu ispatlamaz; hata teşhisinde bilgi eksilir. Bu bir native binary eşdeğerlik beklentisi değildir.

**Dayanak:** Hassas veri loglamama istenmiştir; bütün nonsecret error class/phase bilgisini silmek zorunlu değildir.

**Önerilen sonraki işlem:** Source-derived unchanged expectations, app-specific tests ve synthetic fixtureları ayrı raporla; nonsecret event sınıflarını değerlendirmeye al.

### D20 — Config hata metni EXE dizinini söylüyor

**Sınıf:** DIAGNOSTIC_CONTRADICTION · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** `anadius-license-source/literals.hpp:26` (`same folder as the game exe`)

**Güncel kaynak:** `src/licensing/literals.hpp:26` (`same folder as the game exe`); `src/runtime/runtime_paths.cpp:30` (`module_sibling(m,"surge.cfg")`)

**Fark:** Aktif config DLL dizininden çözülüyor, fakat yeniden adlandırılmış bir kaynak hata metni hâlâ configi game EXE yanına koymayı söylüyor.

**Davranış/borç etkisi:** DLL ve EXE farklı dizindeyse kullanıcıya yanlış konum tarif edilir. Bu kaynakta görülen diagnostic tutarsızlığıdır; dependency kaynaklı değil.

**Dayanak:** Config yerleşimi ve kimlik sözleşmesine bağlı metin uyarlaması eksik kalmış.

**Önerilen sonraki işlem:** Bir sonraki hedefli düzeltmede metni gerçek RuntimePaths sözleşmesine bağla; bu turda runtime literal değiştirilmedi.

### D21 — Staging ve unload iç algoritma sınırları

**Sınıf:** INTERNAL_POLICY_BOUND · **Durum:** REVIEW_NO_AUTOMATIC_REVERT · **Öncelik:** P2 · **Dependency zorunluluğu:** NO

**RE kaynağı:** Üretim davranışı değil; mevcut metadata/QA kaynakları.

**Güncel kaynak:** `src/files/atomic_io.cpp:38` (`attempt<100`); `src/windows/dll_entry.cpp:22` (`buffer.size()>=32768`); `src/windows/hooks.cpp:148` (`quiesce(std::chrono::seconds(5))`); `src/windows/unload_guard.cpp:21` (`scanned<64ULL*1024*1024`)

**Fark:** Yeni ownership uygulamasında temporary collision için100 retry, module-path buffer için32768 sınırı, destructor quiesce için5 saniye ve stack taramasında64MiB bound mevcut. Worker quiesce100ms/retry10ms ve failureda1000ms bekleme de kendi algoritma değerleridir.

**Davranış/borç etkisi:** Bu sayılar kullanıcı dosyası/cache boyut/adet kotası değildir. Yine de upstream zorunluluğu veya RE paketinden aynen alınmış değer diye gösterilemez; gerekçe ve failure davranışı belgelenmelidir.

**Dayanak:** Safe unload/failure-atomic amacı kullanıcı talebidir; bütün iç bound değerleri kullanıcının tek tek seçimi değildir.

**Önerilen sonraki işlem:** Ürün kabulünü daraltan sınırla iç algoritma bütçesini ayır; kapsam/gerekçe incelemesi yapılmadan hepsini kaldırma.

## FIX13te zaten kapanmış kaynak sapmaları

Bu17 kayıt FIX14te yeniden uygulanmadı; kaynak aynı kaldığı için düzeltmeler korunur. Eski davranışı tekrar ekleyen bir rapor kararı yoktur.

| Kayıt | Eski sapma | Kaynaktaki düzeltme |
|---|---|---|
| R01 | Invented assets/<hash>, cache/provider-v2 and avatar nonce/session dirs | Plain SURGE/surge.png, remid_cookie, last_profile, last_avatar and original _AVATAR_UserId_Width spelling |
| R02 | Existing module/default image required official byte seal, otherwise ignored/error | Existing file/sidecar is read-only and accepted without hash gate; embedded official bytes only missing-file fallback |
| R03 | 4KiB/raw-field/uint64/UTF8/control acceptance filter on profile cache | Source raw-byte reader; original split/application branch decides fields |
| R04 | 128-char/ASCII identifier restriction and Width normalization | No invented component/length/width rule; source filename text and OS path behavior |
| R05 | Missing achievement INI creation deferred until grant | Original startup status/missing-INI write branch restored |
| R06 | 256KiB log cutoff; 64 avatar files/8MiB each/32MiB total, PNG/IHDR gating and destroy cleanup | No replacement quota/cutoff/nonce/delete/PNG policy; direct provider path. Production disk logging stays compiled out |
| R07 | 1MiB config / 128 nesting application pre-parser, present already in FIX03 | No secondary scanner; original Config2 parser sees the full bytes |
| R08 | 1MiB frame receive and Connection::process guard, present already in FIX03 | Source pending.max_size() boundary; duplicate message guard removed |
| R09 | 32 live-client rejection present in FIX03 and carried into Asio rewrite | No invented client count cap |
| R10 | 64MiB general read/output/preimage caps | No custom file-size caps; natural stream/string representation/allocation limits remain |
| R11 | FIX12 NativeInput threw on short read | Source returns gcount and keeps caller zero-filled requested tail |
| R12 | Game.KeyForLicense side effect stored pending until final/online profile | Runs at source Config2 Game application call point before later User keys |
| R13 | Extra DLL failure aborted startup; changed loader/error-mode/unload order | Original libraries.cpp + NativeSystemCalls used: dialog-and-continue, LoadLibraryW, process mode, forward unload |
| R14 | Provider suppressed thread error mode instead of source process mode | Imported source suppress_errors_0450a0 and SetErrorMode restoration |
| R15 | Crypt object reader stopped after 32768 code units | Read-until-terminator with natural address-overflow/readability bounds; no invented string cutoff |
| R16 | Application bypassed imported WindowsTimeSource with chrono-only wrapper | Imported WindowsTimeSource used for protocol time |
| R17 | FIX12 version-mismatch dialog was silently suppressed by native adapter | Original Win32 MessageBoxA restored; negative version case requires manual dialog acknowledgement in native operator flow |

## Hangi farklar özellikle hata sayılmadı?

- VDF yorum, nested object ve duplicate replacement desteği RE parser kaynağında mevcut. Bunların varlığına bakarak yeni özellik uyduruldu denmez.
- XML100 derinlik değeri paket vendorında mevcut; sonradan seçilmiş config128 nesting sınırıyla karıştırılmaz.
- C++ namespace/include taşıması ve Linux QA branchleri, kendi başına işlev kaybı değildir.
- Asio/MinHook/TinyXML2/PEGTL/gulrak upstream dosyaları ile bizim adapter/policy katmanları ayrı tutuldu.
- Doğrudan alınmayan CRT/container/allocator dosyaları otomatik eksik uygulama özelliği sayılmadı.
- Source-derived sabitler için native DLL çalıştırma şartı öne sürülmedi; kaynak karşılaştırması bulguları kayda alındı.

## Yapılmayan değişiklikler ve teslimat yorumu

Bu aşamada uygulama C++/header kaynaklarının hiçbiri değiştirilmedi. Yeni byte/istemci/dosya/cache sınırı, config anahtarı, fallback veya retry kuralı eklenmedi. BROWNFIELD application kodu kopyalanmadı; diğer FIELD projeleri veya canonical/release pointer değiştirilmedi. Asio snapshotı kullanıcı talebiyle güncellendi.

36 portable test grubunun sonucu paket doğrulama kaydında ayrıca verilir. Test PASS, burada belgelenmiş uygulama kararlarını otomatik doğru kılmaz. Windows/MSVC çalıştırması bu ortamda NOT_RUN; bu durum kaynak farkı raporunun konusu veya kapanış gerekçesi değildir.
