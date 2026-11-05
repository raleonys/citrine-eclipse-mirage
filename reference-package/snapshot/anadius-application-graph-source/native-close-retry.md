# Bilinen kapanış ertelemesinde Graph retry düzeltmesi

Sonraki statik denetimde, aşağıdaki eski erken-stop195 beklentisinin Profile
live-job guard'ından dolayı ulaşılamadığı saptandı; güncel kaynak düzeltmesi
[prepared stop drain](../anadius-server-runtime-source/prepared-stop-drain.md) kaydındadır.

Sonraki kaynak aşaması: [aynı worker üzerinde açık kapanış devamı](../anadius-application-worker-source/close-continuation.md).
Aşağıdaki Worker sınırı bu belgenin önceki snapshot'ını anlatır.

Durum: **üretim kaynağı düzeltildi; derlenmedi ve çalıştırılmadı**.
Değişen dosyalar: `graph.cpp`, owned-server `server.hpp/server.cpp` ve
`graph_integration_tests.cpp`. Yalnız yazarın statik kaynak incelemesi yapıldı.

Bulunan hata: `OwnedServerBackend::quiesce`, alt Runtime normal olarak false
döndürünce sahipleri ve Quiescing durumunu koruyor, fakat generic logic_error
atıyordu. `Graph::Server::invoke` her istisnada Graph'ı kalıcı Poisoned yapıyordu.
Sonuçta bilinen erteleme sonrası aynı worker üzerinde ikinci pending-socket
destruction çağrısı yapılamıyordu.

Düzeltme: normal false dönüş için ayrı HOST `QuiescenceDeferred` sinyali eklendi.
Graph yalnız `destroy_pending_socket_024db0` çağrısında; backend Quiescing,
attestation false ve her iki socket durumu hâlâ canlıysa bu sinyali koruyarak
server çağrı rezervasyonunu bırakır. Diğer hatalar kalıcı poison olmaya devam
eder. İstisna türü tek başına cleanup izni değildir. Yeni export, virtual-slot
veya recovered server_start sırası değişikliği yoktur.

Eklenen kaynak senaryoları:

| Mod | Yürütülmemiş beklenti |
|---|---|
| `--prepared-native-late-close` | 70 native mesajdan sonra sahipli original-thread registry kaydı Policy close'u erteler. Graph Drained ve NativeJobs Closed korunur. Yalnız bu kayıt çözülür, pending/acceptor/context teardown aynı worker'da tekrar denenir; job/04a47c/HANDLE kapanışı tekrarlanmaz. Beklenen exit0. |
| `--prepared-native-early-stop` | Kendi IOCP'sinde ready receipt önüne açık stop konur. Gövde bitse de receipt tüketilmemiştir. Job/closure/control/work/handle sahipliği tutulur; attestation çağrılmaz, süreç195 ile unwind etmeden biter. |
| `--prepared-native-unknown-external` | Opaque external proof aynı exception türünü atar. Alt katmanlar poison olur; Graph bu durumu bilinen false dönüş saymaz. Süreç196; retry yok. |

Önemli kalan sınır: `application_worker::Worker::run` bütün istisnaları yakalayıp
kendi attempt'ini Poisoned yapmaya devam ediyor. Bu dosya değiştirilmedi.
Dolayısıyla düzeltme Graph/sunucu sınırındaki **açık, aynı-worker retry** yoluna
aittir; uçtan uca uygulama Worker retry'sinin tamamlandığı iddia edilmiyor.
Erken stop sonrası receipt tüketimi veya otomatik iptal/cleanup da eklenmedi.

Sonraki üretim hedefi üst Worker'ın orijinal actor/PTD ve sağlayıcı sahipliğini
koruyan açık kapanış devam mekanizmasıdır. Controller thread'inde worker-affine
destruction veya server_start'ı baştan çalıştırmak çözüm olarak kullanılmayacak.
Loader hook'ları ve CRT/image/foreign yaşam süresi sözleşmeleri ayrıca açıktır.

Satır/hash kanıtı `native-close-retry.evidence.json`, kaynak incelemesi
`native-close-retry-review.json` içindedir. Derleme, proje testi, link veya DLL
çalıştırılmadı; orijinal dosyalar, mevcut DLL/ZIP ve eski derleme kanıtları
değişmedi. Kaynak düzeltmesinden güvenilir toplam DLL yüzdesi hesaplanamaz.
