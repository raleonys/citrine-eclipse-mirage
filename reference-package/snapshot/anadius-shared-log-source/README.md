# B20 — Ortak stream sahibi ve kısa log işlem kapısı

Altı somut yazıcı artık **aynı `shared_log::Stream` nesnesi** verildiğinde ortak bir kısa işlem kapısını kullanır: BootstrapLogger, ConfigurationLogging, LibraryEffects, OnlinePlatform, Host/HookCleanup ve Callback Policy. Gate, yalnız streambuf yazısını değil ostream sentry/format/flags/width/error-state ve endl işlemini de kapsar. Bu yeni compiler-C++ kaynak bileşimidir; özgün MSVC ostream/CRT ABI eşdeğerliği değildir.

**30 çekirdek + 39 entegrasyon = 69 yeni kontrol PASS.** Bağımsız reviewer üretim ve entegrasyon yazarlarından ayrıdır; 11 kaynak ve iki root-run EXE hash’i doğrulandı, reviewer test rerun yapmadı. Önceki 800 kaynağın 794’ü aynı; altı CPP değişikliği before/after hashleriyle kayıtlı. Beş yeni dosyayla toplam 805 kaynak.

## Sahiplik ve kullanım

`make_stream(shared_ptr<streambuf>)`, tek gerçek compiler ostream ve buffer sahibi yaratır. Altı writer’a bu AYNI shared nesne verilmeli; aynı buffer’ı kullanan ayrı ostream wrapper’ları yeterli değildir. `acquire(ostream&)` ile alınan aynı-thread, taşınamaz Lease güçlü stream/buffer sahibini tutar. Format, width/fill, exception maskesi ve error state Lease altında yapılandırılabilir; farklı writer’lar aynı kalıcı state’i görür.

BufferOwner **ilk virtual base**’dir. İlk nonvirtual base yeterli değildi: basic_ostream’un virtual basic_ios/ios_base yıkımı daha sonra çalıştığı için own erase-event testi buffer’ın erken öldüğünü yakaladı. Virtual base sırası düzeltildi; kapanış callback’i sırasında buffer yaşar ve ardından bırakılır. Son doğrulama kayıtları düzeltilmiş kodun hashleridir.

TLS managed scope, içinden başka bilinen adapter’a yeniden girişi mutex/private gate öncesinde reddeder; legacy adapter’a giriş de buna dahildir. BootstrapLogger sırası shared gate → mevcut private gate’tir. Tie veya dışarıdan değiştirilmiş rdbuf girişte reddedilir. Legacy stream’ler managed işlem dışında önceki yoldadır. Gate acquisition/release için ayrı WinLastError koruması kullanılır; body’nin gerçek LastError etkisi silinmez.

## İşlem sınırı ve hatalar

Kilit kısa source çıktı işlemlerini kapsar; bütün callback’i veya bütün log kaydını kapsamaz. Ayrı çağrılar arasında yazılar iç içe geçebilir. C-string, counted string_view, signed int32 ve uint16 ifadeleri, sayı formatı, NUL, width/fill ve endl aynen korunur; to_string veya buffering ile değiştirilmez.

ConfigurationLogging önce `Parsing ` prefixini kendi scope’unda yazar, gate’i bırakır, filename/encoding conversion çalışır, sonra counted suffix/endl scope’u alır. Query/fill hatası prefixi görünür bırakır. Conversion çalışırken başka gerçek LibraryEffects writer’ın tamamlandığı test edildi. Wait, dialog, original, hook engine, SDK ve diğer foreign provider çağrıları gate dışında kalır. Ostream’un zorunlu streambuf/facet kodu kilit altında çalışır; bu kodun başka writer’a reentry yapması veya başka thread’in yazmasını bekleyip/join etmesi desteklenmez.

Raw ostream/buffer erişimi, başka frontend, global locale/handler/FLS değişikliği ve Lease içinden bile tie/rdbuf değiştirme sözleşme dışıdır. Girişteki kontroller, scope içinde raw state değiştirmeyi güvenli yapmaz. Hata sonrası otomatik clear/retry/rollback yoktur: kısa write, committed bytes, badbit/failbit ve flush prefixi korunur. Streambuf içinden nested adapter reddi ostream tarafından badbit’e dönüştürülebilir; exception maskesi yoksa dışarıya exception çıkması zorunlu değildir.

## Test ve kanıt

Çekirdek testler sahiplik, destructor/erase callback ömrü, acquire/release LastError, stack-unowned/tie/rdbuf/reentry retleri, exception unwind, persistent format/NUL/error state ve altı own thread’in 600 işlem/flush’ını doğrular. Entegrasyon altı gerçek source adapter’ı 64 tur aynı Stream üzerinde paralel çalıştırır; Host+HookCleanup ve own controller thread’inde oluşturulan Policy’nin log metotları dahildir. Ayrı buffer instrumentation eşzamanlı buffer girişlerini algılar, fixture string’ini kendi kilidiyle korur.

B20 entegrasyonu **direct source logging** testidir; native callback admission veya genel callback concurrency testi değildir. B18/B19 actual source Runtime/callback/worker testleri ortak regresyonda korunur. Gerçek socket, hook, foreign DLL, UI veya kullanıcı dosya içeriği çalıştırılmaz/okunmaz.

`source-evidence.json`, retained Ghidra VA/Evidence ID/line/hash kayıtlarını bağlar: 006860/006880/007990/0088e0 biçimli çıktı, 00ad70 port, 00af90 signed integer, 019ad0 config ve 035260 library log prefixleri. Ortak gate yeni source tasarımıdır. REA 6.5.0 ve Ghidra 12.1.4 kullanılabilirliği binary_session ile doğrulandı; target açılmadı, yeni original byte/decompile sorgusu yapılmadı. Original veya rebuilt DLL/game değiştirilmedi/çalıştırılmadı; AV engelli version öğeleri yeniden denenmedi.

## Kalan sağlayıcı alanları

`provider-inventory.json`, altı writer’ın method/line/hash bilgisini ve fallback ayrımını içerir. Geçerli ve sabit admitted recovered FLS PTD varsa callback heap hatası PTD+0x20’ye gider; raw PTD setup/cleanup ortak fallback recursion’ını bypass eder. Bu, dış worker/PTD/FLS hatalarının genel güvenliği değildir. Linked compiler CRT errno/locale/invalid-handler ve process ANSI/OEM codepage ayrı alanlardır; recovered initial-locale graph bunların sabitliğini kanıtlamaz.

B18/B19 single-active callback/PTD scope, aynı Execution, explicit controller activation, stop/drain/join ve bağımsız external quiescence şartları korunur. Genel provider/CRT/EH/ABI/hooks/unload/PE ve tam DLL hâlâ açık. Yeni native gövde/kapanış **0**; 487 mapped / 491 eligible / 124 normal claim / 52 tarihsel kapanış değişmedi. Toplam DLL yüzdesi bu source bileşiminden türetilmez; tam DLL hazırlığı **BLOCKED**.

**B21:** callback sağlayıcılarının gerçek FLS/PTD identity ve compiler locale/codepage/handler sahiplik sözleşmelerini daraltmak. 7 online foreign çağrı, configuration/license/legacy document ve B10 tarihsel review borcu açık. Öncelik 1→2→3→5→4; genel CRT kuyruğu beklemede.

## B20 ortak doğrulama

**30 çekirdek + 39 entegrasyon = 69 kontrol PASS.** Ortak build 238 nesne (234 C++ / 4 C), 271 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 794 kaynak aynı; 6 önceki dosya açık hash kaydıyla güncellendi; toplam 805 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
