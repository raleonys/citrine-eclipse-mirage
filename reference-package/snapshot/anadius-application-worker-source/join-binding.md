# Worker kapanış devamının controller stop/join bağlantısı

Sonraki kaynak aşaması: [prepared iş bildirimlerinin erken stop öncesinde tüketilmesi](../anadius-server-runtime-source/prepared-stop-drain.md).

Durum: **üretim kaynak bağlantısı eklendi; derlenmedi ve çalıştırılmadı**.
Değişiklikler backend `backend.hpp/backend.cpp`, Worker `worker.hpp/worker.cpp`
ve iki mevcut Worker test kaynağındadır. Yazarın statik incelemesi yapıldı.

Bulunan bağlantı sorunu: controller, worker'ın close epoch sinyalini vermeden
uzun native join beklemesine girerse aynı controller'ın uyandırması gereken
worker ilerleyemez. Yeni bağımsız `ApplicationJoinContinuation` arayüzü bu
hazırlık adımını SourceBackend'e bağlar. Mevcut ApplicationWorker virtual
slot'ları ve Resources alanları korunur; Worker'ın secondary base yerleşimi
değiştiğinden allocating/crosscasting/deleting tüketiciler yeniden derlenmelidir.

Seçili politika için controller çağrı sırası:

1. `stop_sources` admission/cancellation işlemini tamamlar; Worker stop istenir
   ve callback attachment pause şartı denetlenir.
2. `prepare_join`, yalnız güncel WaitingClose epoch'unu uyandırır. Worker hâlâ
   çalışıyor/bekliyor veya poisoned ise false döner.
3. False halinde TrackingThreads `stop_and_join(0)` yeni thread oluşturmayı
   kapatır ve yalnız fiziksel poll yapar. Sonuç true olsa bile bu turda kaynak
   routeları bırakılmaz, stopped/finish ilan edilmez; Runtime.close false döner.
4. Sonraki controller close turunda Worker Returned olduğunda normal native
   join, source lease drain ve Worker.finish_after_join kontrolleri sürer.
   Library/Online/hook cleanup ancak bundan sonra yapılır.

Worker kapatmayı aynı orijinal thread/fiber/PTD üzerinde sürdürür. Controller
sadece recheck sinyali verir; async/kernel/foreign drain kanıtı üretmez. Default
close politikası hâlâ kapalıdır. Seçilmemiş Worker ve eski interface'i kullanan
başka ApplicationWorker sağlayıcıları mevcut join yolunu korur. Kapanış süresi
boyunca Runtime/Worker/sağlayıcı sahiplerinin tutulması zorunluluğu devam eder.

Hazırlanan fakat yürütülmeyen kaynak senaryoları:

- Core route: false hazırlıkta gerçek configured wait argümanı0, finish yok;
  daha sonra true hazırlıkta configured timeout7, bağımsız native ticket
  sonucu ve tek finish; tamamlanmış çağrılar idempotent.
- `--join-continuation`: gerçek RuntimeOwner/SourceBackend üç Domain native
  worker başlatır. İlk external proof false döner. Stop lower yalnız bir
  terminal packet yayımlar ve WaitingClose'da geri gelir. Controller sadece
  Runtime.close çağrılarını tekrarlar; doğrudan retry_close çağırmaz.
  SourceBackend'in wake'i ile ikinci proof true, aynı worker'da Graph/TLS
  teardown, gerçek native/FLS join ve controller finish beklenir. Kaynak
  payload/Online/hook cleanup'ın tekrarlanmadığı ayrıca denetlenir.

Bu tam Runtime senaryosu mevcut inline mesaj Jobs yolunu kullanır; ayrı detached
NativeJobs çocuk fixture'ının yerine geçmez. Hiçbir süreç bu aşamada başlatılmadı.
Senaryonun kaynakta bulunması compiler/runtime PASS veya üretim DLL kanıtı değildir.

Seçili close policy için `stop_sources` lower'ı, admission durdurulduktan sonra
aynı bloke worker'ın Returned olmasını kendi içinde beklememelidir; yoksa
SourceBackend wake adımına ulaşamaz. Controller close retry'lerini seçilen
sonlu Worker bütçesi içinde sürdürmelidir. Üretime otomatik polling veya keyfi
foreign cleanup eklenmedi.

Sıradaki gerçek kaynak eksikliği: erken stop sırasında detached NativeJobs ready
receipt'i henüz tüketilmemişken orijinal worker'ın source erişimini nasıl açık
tutacağı. Bu aşamada tamamlanma varsayımı veya run_worker restart eklenmedi.
Production loader hook'ları ve CRT/image/foreign finality sözleşmeleri de açıktır.

Satır/hash kanıtı `join-binding.evidence.json`, inceleme
`join-binding-review.json` içindedir. Derleme/test/link/DLL çalıştırılmadı;
orijinal dosyalar, DLL/ZIP ve eski build kanıtları korunur. Toplam DLL yüzdesi
için güvenilir yeni bir ölçüm yoktur.
