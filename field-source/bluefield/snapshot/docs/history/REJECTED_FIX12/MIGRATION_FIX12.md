# Runtime geçiş kararı — FIX12

Yetki: kullanıcının 2026-10-26 tarihli runtime identity/filesystem/avatar/log/provider görev metni, SHA256 `52303a520eb3b1fac77e24ed21180bf85874e63b08a86b93734c62dcf93fe67c`. Parent FIX11 kaynak ZIP SHA256 `e2edb8fb46341135e7a31afa79e0b7580068623e2d7e8b97cca69a83e785ba67`. BROWNFIELD HF19 karşılaştırma ZIP SHA256 `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`. Bu aşama NON_RELEASE; yerel kod/test/build yetkilidir, canonical/release pointer değişmez.

| Yüzey | FIX11 | FIX12 | Karar / kanıt |
|---|---|---|---|
| Teknik proje/header | BLUEFIELD package/project ve include/bluefield | preloader_l; include/preloader_l | Kaynak/header yolu ve çağrı macro adı değişti; C ABI değişmedi |
| DLL / exports | preloader_l.dll; 1/2/3 | Aynı | Source/PE verifier; gerçek MSVC NOT_RUN |
| Görünür ad / numeric defaults | SURGE, source IDs | Aynı | 14 alanlı profile + settings/game regresyonları |
| App data / profile cache | iki farklı eski root | Tek LOCALAPPDATA/SURGE; cache/provider-v2 | Eski fallback, otomatik migration/delete yok |
| Config / override | module-relative optional | Aynı typed policy üzerinden | Config2 grameri, port, SDK değişmedi |
| Default avatar | donor PNG dizileri → bluefield.png/default.png | Tek resmi surge.png | Eski iki array kaldırıldı; AVATAR_FIX12.json origin hashleri korunur |
| Module surge.png | belirgin sözleşme yok | Salt okunur, exact resmi bytes ise seçilir | Başka dosya korunur; embedded resmi fallback |
| Özel avatar | cache ile geçersiz kılınabilir | Salt okunur, default/cache'den öncelikli | Online init seçimi restore eder; own QueryImage provider çağırmaz |
| last_avatar | profile/avatar cache override | Runtime schema alanı ayrılmış; aktif read/write yok | Kullanıcı cache dosyası silinmez |
| Diğer kullanıcı avatarı | ortak cache path | session nonce + bounded provider download | Own avatar override yok; PNG signature/IHDR/dimension denetimi, tam decoder doğrulaması iddiası yok |
| Profile cache | üç alan, doğrudan dosya | Aynı field format, yeni root, atomik save | ≤4096 byte, üç alan, uint64 IDs, geçerli UTF-8/control denetimi; invalid cache offline local profile |
| Provider | eski basename, EXE/test fallback | preloader_l_online.dll, yalnız module | version2 exports/slot mapping; yanlış sürüm modal UI açmadan offline |
| Disk diagnostics | içerikli kalıcı log | Production/Native/CRT compile-out | Debug/Diagnostic fixed enum, bounded 256KiB, process lock; hassas string API yok |
| Achievements | startup eksik INI üretir | İlk gerçek mutation XML+INI batch commit | Bellek/event rollback ve ikinci commit hata testleri |
| License / ticket | dış sistem isim/yol/format | Aynı dış sözleşme | Yeniden markalama yok; guard + atomic writes; gerçek dış consumer NOT_RUN |
| Tarihsel provenance / vendor | archive/function/address/hash kayıtları | Korunur | Upstream pinler ve Golden tool byte kimliği değiştirilmedi |

Tarihsel `anadius`, `BLUEFIELD`, `LSX emu` ibareleri IMPORTS/CONFIG_PROFILE_EVIDENCE, arşiv belgeler, araştırma kanıtları ve negatif fixture içinde kalabilir. `tests/fixtures/legacy_synthetic.cfg` ve Config2 differential corpus sabittir; ürün kimliği değildir. Eski provider basename yalnız ret testi ve tarihsel karşılaştırmada geçer. `resources/golden_version.rc` production Golden metadata sözleşmesidir, kör rename yapılmaz. Güncel `resources/version.rc` ProductName SURGE, OriginalFilename preloader_l.dll olarak ayarlandı. tools/golden upstream scriptleri aynıdır.

Provider ABI incelemesi: `version_check(): int32 == 2`, `create_online(const wchar_t* cookie): void*`, object ilk alanı function table. x64 slotlar 0 destroy, 1 status, 2 username, 3 persona, 4 user-id, 5 avatar(object,wchar-path,char-owner,int32-size)→uint8, 6 auth(object,char-client)→char*. Borrowed cookie pointer instance boyunca saklanır. Object destroy provider unload'dan önce gelir; optional provider yok/exports eksik/sürüm yanlış/factory null ise offline. Version etiketi ve dosya ismi keyfi bir DLL'nin ABI/memory güvenliğini kanıtlamaz; imzalı, bilinen sağlayıcı ve native test gerekir. Synthetic fixture gerçek provider değildir.

Resmi avatar: kullanıcıdan gelen Library surge.png, 208×208 RGB, 3653 byte, SHA256 `606c08b47732a77439776af8e8ab9e5132a31ecceb9fb3e7e21b8f487c50f12b`. Yeniden tasarım/resize/encode yok. Donor default_user_png 7488 byte ve default_avatar_png 3882 byte parent byte dizileriyle ayrı ayrı hash doğrulandı; runtime'dan çıkarıldı. Orijinal görsellerin tüm sürümlerdeki görsel anlamını veya bilinmeyen cache geçmişini hash tek başına kanıtlamaz.

| HF19 karşılaştırması | HF19 kaynak bulgusu | FIX12 durumu |
|---|---|---|
| Binary/exports | exports/preloader_l.def, preloader.unsigned.dll staging, ordinal 1/3/2 | Public ordinal sözleşmesi aynı; staging ve araç zinciri ayrı kanıt |
| Config | process_configuration.hpp DLL-adjacent base/override; module_relative_filesystem.hpp bounded path join | Aynı module otoritesi; gulrak salt path işlemi, genel OS I/O replacement değil |
| Config2 | HF19 section-policy parser | Anadius kökenli PEGTL byte grameri korunur; HF19 application parser kopyalanmadı |
| No-config/profile | bağımsız normalization/handler uygulaması; AvatarId boş | Source numeric IDs ve 14 alan korunur; portable source karşılaştırması native eşdeğerlik değildir |
| Avatar | image_route.cpp Avatar veya module surge.png; bağlı image provider yok | Resmi sealed sidecar/embedded lazy default ve explicit custom; other-user provider ayrı |
| Diagnostics | src/debug/debug_diagnostics.cpp in-memory stats; Debug hedefleri ayrı | Production disk compile-out, Debug/Diagnostic sabit bounded dosya sözleşmesi |
| Cache | HF19 bu provider-v2 disk cache uygulamasını göstermiyor | FIX12 kendi kökenini açıkça sahiplenir; sessiz HF19 eşdeğerlik iddiası yok |
| Achievement | achievement_watcher.cpp callback sink, varsayılan g_sink=nullptr | XML + INI dosya persistence; ayrı dış entegrasyon, sink ile aynı sayılmaz |
| Native shutdown | HF19 kendi runtime/ownership yolu ve tests | FIX12 mevcut Service drain + DllShutdown/FreeLibrary sözleşmesi; native NOT_RUN |

Achievement Watcher uygulaması için `SURGE/achievement_watcher` kökü manuel custom watched folder olarak tanımlanmalıdır; AppId/achievements.ini düzeni ve Achieved/CurProgress/MaxProgress/UnlockTime alanları korunur. Bu kök standart otomatik discovery listesinde var diye kabul edilmedi. Dış uygulama ayarı otomatik düzenlenmez. Kaynaklar: https://github.com/xan105/Achievement-Watcher ve https://github.com/xan105/Achievement-Watcher/wiki/Compatibility . Dış uygulama ile gerçek bildirim testi NOT_RUN.

Sınırlar: In-process ordinary file rollback vardır; crash-atomic çok dosyalı işlem/durable journal/cross-process transaction izolasyonu yoktur. Reparse/path kontrolü ile sahiplik sınırlandırılır; hostile concurrent filesystem değişimine karşı tam sandbox değildir. Gerçek provider kendi içinde hangi ek dosyaları açar bilinmiyor; bu I/O owner sınırı raporda Unknown. Native Windows IOCP, MSVC warnings, gerçek Golden DLL metadata ve oyun kabulü henüz çalıştırılmış değildir.
