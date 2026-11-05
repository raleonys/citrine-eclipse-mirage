Kaynak güncellemesi: 2026-11-03. Derleme/test/link yapılmadı.

`LifecycleMode::Explicit` mevcut varsayılandır ve dört callback'i de zorunlu
tutar. Yeni `SourceOwned` modu açıkça seçilir; callback listesi sessizce
değiştirilmez. Bu modda harici owner seçicisi reddedilir. Geçerli own/null
completion sahibi korunur; yabancı scheduler reddedilir. Gerçek Stop receipt
kararı Runtime'da bu noktadan önce uygulanmaya devam eder.

`Application::after_completion` zaten reap ve hazırlanmış sonuç teslimini
yapıyor. `Application::worker_returned` canlı job'ları, Policy ise gerçek
worker-context finish ve tek dönüşü denetliyor. SourceOwned için zorunlu
etkisiz after/returned fonksiyonları kurulmaz; istenen gözlemciler opsiyoneldir.

Harici drain kanıtı iki modda da zorunludur. SourceOwned, callback'ten önce
kısa bir Parent Operation altında IO/accept projeksiyonları, loans, live
session/job'lar ve scheduler iş/kuyruk durumunun boşluğunu kontrol eder.
Yerel canlılık varsa kaynaklar tutulur ve proof çağrılmaz. Boş yerel durum
tek başına kernel veya foreign/code/CRT sonlanması değildir.

Gerçek harici callback Parent Operation bırakıldıktan sonra çağrılır. Aynı
yerel kontrol dönüşte tekrar yapılır; opaque callback true dönse bile yeni
IO/iş, tutulmuş Parent veya bozuk kaynak durumu başarılı drain sayılamaz.
Callback aynı denemede ikinci kez çağrılmaz. Graph/Profile/Runtime'ın mevcut
kapanış, gerçek packet/receipt ve provider-lease denetimleri korunur.

`resources_factory::select_source_worker_lifecycle`, boş owner seçimi ve
önceden verilmiş gerçek harici proof ile bu modu seçer. Stop kurmaz; harici
kanıtı, zamanlamayı veya image lifetime'ı üretmez. Lifecycle HOST by-value
yerleşimine mode eklendiğinden Options/Graph/Worker tüketicileri sonraki açık
derleme talebinde yeniden derlenmelidir.

Kaynak senaryoları: gerçek Policy/Profile köklerinde own/null sahibin
korunması, observer olmadan source reap/worker dönüşü, canlı IO/iş varken
proof'un çağrılmaması, boş sayaçların false proof'u geçersiz kılamaması ve
true proof'un sonradan oluşturduğu IO'nun tutulması. Senaryolar çalıştırılmadı.
İki production loader hook'u ve gerçek per-image dış yaşam süresi hâlâ açık.
