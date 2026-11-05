# Fiziksel child çıkışı ile concrete Worker kapanış retry'si

Durum: **senaryo kaynakları eklendi; derlenmedi ve çalıştırılmadı**.
Yalnız `graph_integration_tests.cpp` değişti. Üretim C++ kodunda bu aşamada ek
değişiklik gerekmedi; mevcut Worker/NativeJobs/Profile/Runtime/Stop sözleşmeleri
statik incelemede bu akışı destekliyor. Bu, runtime PASS veya DLL yüzdesi artışı değildir.

Yeni kaynak, gerçek concrete Worker'ın kendi PreparedMemory/Graph/TLS kurulumunu
ve aynı Domain NativeJobs çocuğunu kullanır. Controller Graph veya worker Parent
view'ı kurmaz/okumaz; yalnız lower Inputs, metadata snapshot'ları, atomic fixture
gözlemleri ve kendine ait proof HANDLE'ları kullanır. `SessionIoPrepared`, sonlu
prepared-stop ve Worker close-retry politikaları birlikte seçilir.

Çocuk source gövdesi ve hazır bildirimi döndükten sonra gerçek ExitThread'in
hemen önünde sahipli event ile bekler. Bu konum FLS callback'i içinde değildir.
Normal proof-join driver bu çocuğu controller bırakana kadar atlar; erken join
ile fiziksel gecikmeyi gizlemez. Source-job ve IO packet'leri bitebilirken
NativeJobs.pending1, private HANDLE/barrier ve borrowed backing korunmalıdır.

`--prepared-worker-session-delay` için yürütülmemiş beklentiler:

1. Worker WaitingClose epoch1 yayımlar. Child proof WAIT_TIMEOUT/STILL_ACTIVE;
   external proof ve provider destruction henüz yoktur. Controller finish/cancel
   girişimleri false döner.
2. Production `ApplicationJoinContinuation` arayüzü üzerinden prepare_join
   yalnız recheck'i uyandırır. Child hâlâ canlı olduğundan epoch2 gelir; bütün
   sahipler ve orijinal worker canlı kalır.
3. Controller child gate'ini açar, kendi proof HANDLE'ıyla gerçek çıkışı bekler
   ve epoch2'yi uyandırır. Orijinal worker physical reap, external proof,
   Graph/IO/TLS/provider retirement yapıp döner.
4. Controller gerçek original wrapper/FLS join'den sonra finish_after_join ile
   backing'i, en son native provider/Domain'i bırakır. Beklenen süreç exit0.

Concrete Information/Content/Licensing alias'ları kendi Online/auth/view
sahiplerini Graph retirement'a kadar tutar. Wrapper controller'da tutulmaz;
destructor actor'ının original thread olması ayrıca beklenir. Yeni private
Worker senaryosu Policy encapsulation'ını aşarak pointer okumaz; normal Policy
return doğrulaması üretim yolunda kalır. Mevcut direct Graph senaryosunun açık
Policy sayaç kontrolleri korunur.

`--prepared-worker-session-timeout`: child bırakılmaz; sonlu close bütçesi biter.
Original prosedür dönebilir, fakat Worker Poisoned/pending-retained ve borrowed
backing/child/provider sahipliği kalmalıdır. Controller prepare_join/finish
cleanup yetkisi kazanmaz; süreç203 ile unwind etmeden sonlanır.

Bu senaryo henüz çalıştırılmadı. Socket/CancelIoEx sonuçları ve external proof
kapalı fixture'ın sentetik seçimleridir; gerçek kernel cancellation veya foreign
kod finality'si kanıtlanmaz. Concrete Worker/controller seam aynı senaryoda,
tam RuntimeOwner/SourceBackend aggregate ise ayrı önceki kaynakta kapsanır;
ikisi tek detached-child aggregate senaryosu olarak doğrulanmış sayılmaz.
FreeLibraryAndExitThread veya gerçek FLS callback'i içindeki gecikme de kapsanmaz.

Yeni translation unit, export veya build-script değişikliği yoktur. Common
build kaydı Worker'ı zaten aynı static library'de listeler; bu yalnız statik
bağlantı envanteridir. Eski build/ZIP kanıtları yeniden etiketlenmedi. Açık
yeniden derleme talimatı olmadan compiler/test/link çalıştırılmayacak.

Sonraki üretim incelemesi kalan loader hook'ları ve CRT/image/foreign yaşam
süresi sözleşmelerine dönecek; kaynakta mevcut akışlar ile gerçek eksik
bağlantılar ayrılacak. Genel DLL yüzdesi için güvenilir yeni ölçüm yoktur.
Satır/hash kanıtı `worker-physical-close.evidence.json`, statik kaynak incelemesi
`worker-physical-close-review.json` içindedir.
