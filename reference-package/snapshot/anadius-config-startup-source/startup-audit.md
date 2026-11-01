# Config2 başlangıç bağlantısı: statik denetim

Bu denetim mevcut hash'e bağlı REA assembly/pseudocode ile `configuration.cpp`, `bootstrap.cpp` ve `startup.cpp` kaynaklarının normal kontrol akışını karşılaştırır. Yeni Ghidra oturumu veya çalışma zamanı gözlemi üretilmedi. Özgün DLL/oyun çalıştırılmadı; gerçek kullanıcı CFG/token dosyası okunmadı. Bu dosya ve eşlik eden JSON dışında kaynak değiştirilmedi.

Hedef SHA256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

## Kanıt kimlikleri

| Giriş | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
| `0x18001c870` yapılandırma başlangıcı | `ev_42beaf13c42bb51716fcf407bd74587506fbb6acf17bfba9e16c31bdd31f8fba` | `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e` |
| `0x180038970` uygulama başlangıcı | `ev_77887ea2e7b744e62e52eaf81cb8a07abf2a4f50498001ba81b2fcf2496e8789` | `ev_e66705fdaa9bdd88fb50be13d22e354939026d931f472329e0f14e17c9fb4fbd` |
| `0x180019910` ortam/argüman başlangıcı | `ev_78e7337670570fa2c7c212620f726bd602b090f213f6248c453917219f5023fc` | `ev_90684472bf3c11d29dce5129d1c8a0e460e645004eaaef332b7623a91a4f03f3` |
| `0x180038760` konsol başlangıcı | `ev_25fbfd8671a513b4750ab57ccab47ce6ee66118511fe48e1c7c1744914a56238` | `ev_443e0c3c2732489164806801392df22b75420bef29996038536b50f1d9fab851` |
| `0x180018480` argüman filtresi | `ev_10f34c57de95a8ee129e5c91b2dde9842b1f85959f5e9e17d8ba4d9dac40ff3f` | `ev_691d99dfac41c8e1f0484feff33584d6291e41fff99d694fff05e78775aefb04` |

## Uygulama giriş sırası

| Çağrı noktası | Davranış |
|---|---|
| `0x18003897f` | `atexit(038950)`; dönüş değeri kullanılmaz |
| `0x18003898b` | `set_terminate(038890)` |
| `0x180038990` | `019910`: CRT ortamı veya argv → filtre → OriginEmuCmdArgs yazımı |
| `0x180038995` | `038760`: paylaşılan bayraklara göre konsol |
| `0x1800389a8 / 0389b7 / 0389c6` | üç log insert; önceki insert'in stream sonucu sonrakine aktarılır |
| `0x1800389ce` | log finish |
| `0x1800389d4` | `01c870` yapılandırma başlangıcı; tamamen dönmeden worker tahsisi yok |
| `0x1800389e3 / 038a1d` | ilk payload tahsisi ve CRT thread başlatma |
| `0x180038a4b / 038a61` | ilk thread detach; başarılı yoldan sonra host hazırlığı |
| `0x180038aa5 / 038ad3` | ikinci worker başlatma/detach |
| `0x180038aeb` | boolean worker'ın senkron `true` çağrısı |
| `0x180038b33 / 038b59` | üçüncü boolean payload `false` worker/detach |
| `0x180038b67` | guard tail jump |

Bu sıralama, yeni yapılandırma adapter'ının `BootstrapDependencies::load_configuration_01c870` metoduna bağlanmasını destekler. Diğer bootstrap yükümlülüklerini boş başarıya çevirmez. Thread creation başarısızlığında CRT tablo indeksi6, sıfır id veya başarısız detach durumunda indeks1 throw yolu vardır; bu hata politikaları mevcut zorunlu bootstrap bağımlılığına aittir.

## 01c870 içindeki sıra

| Çağrı/karar noktası | Etki |
|---|---|
| `0x18001c8ae / 01c8bd` | executable path alma ve aynı global path'e atama |
| `0x18001c8d1` | `019ad0(false)` base geçişi |
| `0x18001c8d8` | `019ad0(true)` override geçişi |
| `0x18001c90f / 01c947 / 01c95c` | filesystem codepage ile LSX emu / anadius; LocalAppData |
| `0x18001c98a / 01c9bd / 01c9d1` | LocalAppData/anadius/LSX emu yolu ve state ataması |
| `0x18001ca39 / 01ca43` | create_directories; hata değeri varsa throw helper yolu |
| `0x18001cac8 / 01cb68 / 01cc09` | remid_cookie / last_profile / last_avatar state yolları |
| `0x18001cc2c / 01cc34` | avatar boşluk testi; doluysa kullanıcı varsayılanı oluşturma atlanır |
| `0x18001ccba` | avatar boşsa user_directory/anadius.png ataması |
| `0x18001cd5e` | default_avatar = user_directory/default.png, her iki avatar dalında |
| `0x18001cda0` | başlangıçtaki boş-avatar koşulu doğruysa embedded user PNG yazımı |
| `0x18001cdba` | embedded default PNG yazımı, her iki dalda |
| `0x18001cdfc / 01ce03` | directory iterator kurma; başarısızlık throw |
| `0x18001d170 / 01d26e / 01d287` | filename'de case-sensitive `_AVATAR_` arama; eşleşirse remove; iterator increment |
| `0x18001d38a / 01d3ff / 01d421` | önce INSTALLED_VERSION lookup, sonra AVAILABLE_VERSION lookup/atama |
| `0x18001d4f0..01d4fe` | SDK byte dizisinde `.` karakterlerini `,` yapma |
| `0x18001d500` | achievement storage update `01eb00` |
| `0x18001d523` | SetEnvironmentVariableA(ContentId) |
| `0x18001d592 / 01d5af` | INSTALLED_LANGUAGE lookup ve SetEnvironmentVariableA(EAGameLocale) |

Base/override'daki aynı configuration state nesnesi post aşamada kullanılmalıdır. Override başarısızsa post aşamaya ve ilk worker'a geçilmez; base'in o ana kadarki değişiklikleri geri alınmaz. Dizin oluşturma, PNG yazımı, filename dönüşümü, remove ve achievement persistence sınırlarında yayılan bir exception sonraki işlemleri durdurur. İki environment API'sinin normal BOOL sonuçları kullanılmaz; ilkinde false dönmesi ikincisini engellemez. Kaynak sağlayıcısı exception yayarsa bu ayrı kontrollü kaynak sözleşmesidir, Win32 BOOL davranışının eşdeğeri değildir.

AVAILABLE_VERSION atamasında C++20 RHS-before-LHS kuralı, özgün INSTALLED_VERSION lookup'unun önce olmasını sağlar. Avatar koşulu base ve override sonrası güncel avatar değerine göre değerlendirilir; sabit başlangıç varsayımı yapılmamalıdır. Achievement storage state'i değiştirebilir; son ContentId/INSTALLED_LANGUAGE environment değerleri bu çağrıdan sonra okunur.

## Paylaşılan argüman ve konsol durumu

`018540` tek WORD `0x101` ile language_from_arguments `A3AD0` ve registry_spoof `A3AD1` byte'larını birlikte1 yapar. `-emuGameLang` eşleşmesinde language map girdisi aynı game_attributes nesnesidir; boş argüman eski değeri silmez. `018653 / 018689 / 0186cc` showLSX/showConsole/noConsole byte'larıdır. Adapter ayrı configuration state veya ayrı map kopyası üretirse sonraki CFG Language/Spoof guard'ı yanlış olur.

`019910` dolu CRT OriginEmuCmdArgs değerini literal space ile böler; bu dalda argv ve tırnak parser'ı kullanılmaz. Boş değerde argv yolu seçilir. `038764..03876b` noConsole dalı hiçbir konsol etkisi çağırmadan döner. Attach başarılıysa yalnız stderr reopen; attach başarısız ve showConsole sıfırsa return; showConsole açıkken AllocConsole sonucu daha sonraki MoveWindow/reopen çağrılarını kapılamaz. FILE/reopen ABI'si hâlâ zorunlu dış sınırdır.

## Tamamlanma ve açık sınırlar

- Yanıtlandı: normal giriş/iki-pass/post-configuration/ilk-worker sırası, paylaşılan flag/map gereksinimi, avatar koşulları, erken hata ve ignored BOOL kararları.
- Kısmen yanıtlandı: mevcut kaynak arayüzleri üzerinden kontrollü hata sırası; native tahsis ve STL/SEH unwind eşitliği yoktur.
- Açık: özgün native OS/persistence/logger/provider gerçekleştirimleri, tam global/cookie/loader/CRT/FILE ABI ve bütün DLL runtime eşdeğerliği.
- Ek sınır: mevcut `directory_entries` sözleşmesi bir vector döndürür. Özgün iterator remove ile iç içe ilerler. Sağlanan normal listing sırası korunabilir; iterator'ın bazı remove'lerden sonra hata vermesi, dış OS mutation/concurrency ve tam native iterator fault zamanlaması bu toplu arayüzle kanıtlanmış değildir.

Eşlik eden `config-startup-audit.json` retained listing ve incelenen kaynak hash'lerini, sınırlı arama kapsamını ve makinece okunabilir çağrı tablosunu taşır. Yeni source-level `ConfigurationPass` kompozisyon arayüzü özgün binary virtual ABI'si olarak sunulmamalıdır.
