Kaynak güncellemesi: 2026-11-03. Henüz derlenmedi/test edilmedi.

`require_loader_attach_runtime` için eksik Resources bağlantılarından sunucu
SDK yaprakları `select_native_worker_io` ile somut olarak seçiliyor:

| Sağlayıcı | Seçilen sınıf | Sahiplik |
| --- | --- | --- |
| Soket oluşturma/bind/listen/close | NativeAcceptorApi | Server ve Runtime aynı nesne |
| IOCP oluşturma/handle kapatma | NativeIocpConstructorApi | Server |
| Servis kilidini başlatma | NativeServiceGateApi | Server |
| IOCP bekleme/post/timer | NativeDispatcherApi | Server ve Runtime aynı nesne |
| AcceptEx/adres/context güncelleme | NativeAsyncAcceptApi | Runtime |
| Kabul soketi oluşturma | NativeSocketFactoryApi | Runtime |
| Kabul iptali | NativeRetryCancelApi | Runtime |

Varsayılan importlar gerçek SDK fonksiyonlarıdır. Seçici tüm importları ve boş
hedef alanları kontrol eder, yedi nesneyi oluşturur, sonra hedeflere birlikte
yerleştirir. Oluşturma hatası hedeflere kısmi kurulum bırakmaz. LastError korunur.
SDK tablolarının fonksiyonları seçim sırasında çağrılmaz; yalnız LastError
saklama/geri yükleme için GetLastError/SetLastError kullanılır.

Bu bağlantı mevcut `Worker::run_server` içindeki configure noktasında kullanılır.
Native sunucu çağrıları daha sonraki gerçek server kaynak akışında yapılır.
DllMain'e yeni Winsock/IOCP başlangıcı veya bir controller beklemesi eklenmez.
Bu durum senkron attach'in geri kalan loader sözleşmesini çözmüş sayılmaz.

Mevcut Resources entegrasyon senaryosunun kaynak kodu aynı seçiciyi kullanıyor.
Testteki soket ve terminal-paket SDK girişleri kontrollü shim'lerdir; yalnız
senaryonun sahip olduğu IOCP gerçek işletim sistemi nesnesidir. Normal üretim
varsayılanlarının yerine sahte soket sağlayıcıları konulmamıştır.

Sonraki kaynak güncellemesiyle sabit084928 executor davranışı ayrı
`select_recovered_socket_executor` seçicisine bağlandı. Seçili OwnedServer'ın
boş iptal motoru dalı sağlayıcı gerektirmiyor; canlı motor varsa gerçek sağlayıcı
eksikliği socket/owner etkilerinden önce reddediliyor. Bu, iptal kuyruğunu
genel olarak çözmüş sayılmaz. Completion-owner/lifecycle kararları ve harici
işlerin sonlanma kanıtı zorunlu kalır. SDK seçicisi bu kararları oluşturmaz veya
her zaman true dönen bir sağlayıcıyla doldurmaz.
İki üretim loader hook'u henüz tanımlanmamıştır; kaynak senaryoları çalıştırılmadı.
