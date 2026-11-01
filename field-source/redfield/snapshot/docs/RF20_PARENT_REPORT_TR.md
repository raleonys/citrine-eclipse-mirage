# REDFIELD RF20 — Grant TCP commit ve event sırası

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

RF19 üzerine **GrantAchievement LocalService/TCP bağlantısı** tamamlandı. Yanıt ve completion event byte'larını tutan hazırlık nesnesi artık TCP Peer'a taşınır. Tam, başarılı yerel yanıt yazma callback'i state'i commit eder; ardından varsa completion event aynı bağlantıda, sonraki request işlenmeden önce gönderilir. **QueryAchievements current committed snapshot'ı okur.**35 LocalService route; önceki context/config/Auth-only dispatcher'larda34 route ve Grant no-response davranışı korunur.

## Sahiplik ve failure sınırı

FrameObserver yeni prepare callback'i ile fixed output buffer veya unique_ptr<FrameDelivery> döndürür. Exactly-one callback ve mutually-exclusive output admission kontrolleri vardır. Owned response boş olamaz; response ve event ayrı ayrı16 KiB kapasiteden küçük olmalıdır. RF19 preparation iki mesajı aynı candidate'tan validate/measure/admit/render eder; Peer aynı nesneyle event slot'unu ilk yazmadan önce rezerve eder. Ek sınırsız event kuyruğu ya da ikinci wire kopyası oluşturulmaz.128 bağlantı sınırı,4096 byte write chunk, FIFO/backpressure ve half-close davranışı korunur.

Commit öncesi kısmi yazı, socket hatası, timeout, shutdown veya owner invalidation committed state'i yayınlamaz. Cancel/close sırasında hazırlık nesnesi pending callback bitene kadar tutulur; buffer async işlemce ödünç alınırken silinmez. Reap sonrasında RAII rollback rezervasyonu bırakır. Full response callback success ise commit bir kez çağrılır. Failed commit yanıtın kısmen/tamamen yazılmış olmasını geri alamaz; event gönderilmeden bağlantı kapanır.

**Sınır yerel TCP yazma completion'ıdır; remote application ACK değildir.** Cancel callback'ten önce olursa uzak uç byte görmüş olabilir ve yerel state yine rollback olabilir. Commit sonrası event iptali/hatası ise state'i geri almaz. Network atomicity, exactly-once, remote tüketim, durable retry veya recovery iddiası yoktur. Event yalnız Grant isteyen bağlantıya gider; watcher/subscription/fanout uygulanmadı. Her response/event kendi send fazı timeout'una tabidir.

responses yalnız yanıtları, events ayrı event'leri, bytes_written ikisinin byte'larını sayar. committed_deliveries başarılı owned delivery commit sayısıdır. Caller-supplied FrameDelivery, immutable byte views ve noexcept commit sözleşmesini karşılamalıdır. Eski byte-only callback behavior korunur.

## LocalService ve state

AchievementSession, ServiceSession start sonrası ilk ConfigSnapshot'tan initialize edilir. Grant timestamp mevcut current_utc_timestamp helper ile YYYY-MM-DDTHH:MM:SS biçimindedir; yeni clock backend yoktur. Aynı servis bir bekleyen Grant rezervasyonu tutar; rakip Grant busy ise peer-local rejection olur. Query eski committed snapshot'ı görebilir, unpublished candidate görünmez. configuration() ilk immutable config'i tutar; achievements() current committed pin verir. Diğer config-backed route'lar ilk config'le çalışmayı sürdürür.

Shutdown sırası port restore → TCP pending completion drain → AchievementSession ve Auth8 close → ServiceSession stop'tur. Restore başarısızsa sahiplik korunur ve stop tekrar denenebilir. Commit edilmiş snapshot pin'i owner close sonrası yaşayabilir. Restart/reload/persistence eklenmedi. Partial, completion, repeat/no-op, missing selector, invalid numeric ve malformed selector politikası RF19'dan korunur; event yalnız count0→nonzero geçişinde üretilir.

## Taze kapsam

Üç yeni gerçek loopback regression: generic delivery full-write commit/FIFO/event/admission/partial-cancel/failed-commit; LocalService Grant partial→query→complete→event→query→repeat/no-op ve half-close; gerçek RF19 ticket ile kısmi iptal rollback/retry, commit sonrası event iptali, owner close, kapasite ve busy rejection. İptal sınırları poll_for(0ms) ile completion bazında kontrol edilir; remote ACK iddiasına dönüştürülmez. Önceki67 regression da taze çalıştırılır.

İlk exploratory run'da bir yeni selector fixture Request id içermediği için envelope validation'da reddedildi; fixture'a geçerli id eklenerek amaçlanan selector senaryosu ayrıştırıldı. Bu başarısız denemeden PASS taşınmaz; nihai girdilerle tüm profiller yeniden çalıştırılır.

Normal çalışma dizinindeki ilk tam deneme70 testten sonra linked-output execute permission guard tarafından reddedildi (test_tcp_content mode0644). Receipt üretilmedi; çıktı izinleri düzeltilerek PASS alınmadı. Nedeni NOT_ESTABLISHED olarak bırakıldı. Kabul profilleri ayrı çıkarımda baştan derlenir; bu denemeden PASS devralınmaz.

## Reuse ve kaynak

1114 byte-identical destination +24 derived file (14 TU,10 header) değişmedi. RF20 yeni core TU eklemez; mevcut project-owned tcp_ingress/LocalService rolü genişletildi. Özgün selector/wire recipe gövdeleri ve vendor pins/licenses aynıdır. Üç production export ve SURGE single-return korunur; hooks eklenmedi. GPL-3.0-only, reuse provenance korunur; clean-room veya donor parity iddiası yoktur.

Exact RF19 parent SHA-256: `dace3c99f0c9d9d11660c2274e9e0573766646f88ae6356c2aa09cca6da91068`. Actual HF19 ZIP: `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`. Whole-file/recipe byte'ları actual ZIP üyeleriyle yeniden karşılaştırılır. Governance2026-10-19.3/v5 yeniden okundu; historical RF00 dört arşiv envanteri yeni whole-Library incelemesi değildir. Resmî HF19/CURRENT değişmez.

## Doğrulama

| Kontrol | Taze sonuç |
|---|---|
| GCC C++20 normal |70/70 PASS|
| ASan, leak kapalı |70/70 PASS|
| UBSan |70/70 PASS|
| Bağımsız temiz çıkarım |70/70 PASS|
| Her profil 125 object/71 linked output guard |PASS|
| Özgün GetConfig diagnostic |Her profil exit14 / FAIL_OBSERVED|
| Kaynak bütünlüğü negatifleri |8/8 beklenen reddetme|
| Compiler/output negatifleri |4/4 beklenen reddetme|
| Actual HF19 adoption/recipe |1114/1114 +24/24 PASS|
| LeakSanitizer |BLOCKED|
| Windows/MSVC/native/IOCP, CMake/CMD, SDK/oyun |NOT_RUN|

Build-input SHA-256: `e039fd6bb2bd7fc31045f279b501e85dc56139d1caa5b0cf91884b5c3ffe83a5`. Dört profil aynı compile/header/reference/recipe/test/tool girdi listesine bağlıdır. ZIP CRC/manifest/SHA, deterministic repackage ve final çıkarımdaki compile input'ların clean receipt ile eşleşmesi ayrıca doğrulanır.

## Açık alanlar

GetConfig RF03-QA-001 exit14 ayrı FAIL_OBSERVED; PASS sayılmaz. Kabul edilen ASan leak kapalı; LSan gerçek probe sonucu ayrıca kaydedilir. Windows/MSVC/native/IOCP, CMake/CMD ve SDK/oyun NOT_RUN. Watcher/fanout/persistence/retry, SDK greeting/session crypto/connect redirect tamamlanmadı. Yerel Grant ve query akışı SDK/game compatibility kanıtı değildir. Önceki dış output bozulması root cause NOT_ESTABLISHED; guards korunur. NOT_RELEASE.
