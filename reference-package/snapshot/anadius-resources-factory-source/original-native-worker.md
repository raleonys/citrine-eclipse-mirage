Kaynak güncellemesi: 2026-11-03. Derleme/test/link yapılmadı.

Attach image factory'sinin gerçek Worker.configure bağımlılığı için
configure_native_worker eklendi. Çağıran image sahibi aynı managed stream ve
açık version metnini verir. Fonksiyon gerçek NativeAcceptor/Iocp/Gate/Dispatcher/
AsyncAccept/SocketFactory/RetryCancel adaptörlerini, gözlenen084928 executor
move/cleanup gövdesini ve OriginalApplication lifecycle profilini seçer.
State/TLS/epoch ve concrete Information/Content/Licensing bağlarını mevcut
ApplicationWorker/Graph kaynakları kurar. Yapılandırma socket/IOCP açmaz,
WSAStartup/wait/worker start yapmaz; gerçek kaynak çağrıları sonraki run yolunda.

Adaptörler önce yerel Inputs içinde sahiplenilir; allocation/constructor hatası
gerçek Pending girdilerini değiştirmez. Boş hedefler tekrar kontrol edilir ve
gerçek shared owner/function-pointer/stream/version alanları commit edilir.
Mevcut sağlayıcı veya lifecycle callback'i sessizce değiştirilmez.

Önceki SourceOwned profili için bir üst-katman hatası düzeltildi: Graph::create
hâlâ dört callback'i koşulsuz istiyordu. Graph ve Policy artık aynı pasif
lifecycle_configuration_valid fonksiyonunu kullanır. Böylece seçilen profilin
Policy tarafından kabul edilip Graph tarafından reddedilmesi önlenir.

OriginalApplication profili own/null completion sahibini korur; gerçek
Application reap/worker-return ve Policy context-finish denetimlerini kullanır.
Harici unload callback'i veya zorunlu Stop kurulmaz. Worker döndükten sonra
aynı graph kimliği, Open IO, gerçek allocations/pending/active calls, accept
projeksiyonları, live session/job ve scheduler work/queue durumunu denetler.
Canlı takip edilen iş varsa false döner. Kaynak boşluğu üzerine gerçek
Profile/Graph/Runtime kapanışları devam eder; foreign/image unload kanıtı
üretilmiş sayılmaz. Cooperative Explicit/SourceOwned proof yolları korunur.

Kaynak senaryoları: mevcut Policy/Profile fixture'ında Stop ve dış proof
olmadan gerçek worker dönüşü, canlı IO/work için false ve kaynakların gerçekten
boşalması; gerçek own-pipe Graph/Worker/Runtime senaryosunda OriginalApplication
ile sıfır harici proof çağrısı; native configure'da gerçek paired provider
kimlikleri, executor, stream/version ve replacement reddi. Hiçbiri çalıştırılmadı.
Pipe fixture hâlâ kontrollü SDK sınırlarıdır; native AcceptEx testi iddia edilmez.

require_loader_attach_runtime henüz tanımlı değildir. Kalan iş gerçek image
köklerini (Domain, configuration, host/callback, Online, log/errors ve Resources)
aynı sahipte kurup bu configure gövdesini Worker settings'e bağlamak ve gerçek
ApplicationEntry'yi bir kez senkron çağırmaktır. Detach önceki turda kaynakta
tanımlandı. Donmuş build/link/ZIP yeni kaynakları kapsamaz.
