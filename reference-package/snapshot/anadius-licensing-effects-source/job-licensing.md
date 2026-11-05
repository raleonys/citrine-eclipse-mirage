# Licensing çocuk iş kapsamı

Durum: **yalnız kaynak kodu; derleme/test/link/DLL çalıştırılmadı**.
Bu aşamada Core/Handlers ve Graph bağlantısı değiştirildi; mevcut lisans
algoritmaları yeniden çözülmüş veya native ABI tamamlanmış sayılmıyor.
Son statik inceleme ve dosya hash'leri `job-licensing-review.json` içindedir.

`Core::with_job` ve `Handlers::with_job` aynı canlı Parent Operation'ın
JobPermit'ini, senkron POD checkpoint'i ve çağıranın eylemini kullanır.
Eylem referansla çağrılır; sahip olunan callable transferi yoktur. Void/değer
sonuçları ayrı ele alınır, referans sonuçlar kabul edilmez. Eylem, facade,
checkpoint context'i, Runtime/Online, authentication hücresi ve bütün alt
sağlayıcılar çağrı sonuna kadar yaşamalıdır. Permit kopyası Operation'ı uzatmaz.

İki katman kendi stack descriptor'ünü ilk callback öncesinde yayımlar.
Her metot busy/checking/binding rezervasyonunu opaque çağrıdan önce kurar;
Parent/proof/checkpoint/SDK/dosya çağrıları metadata kilitleri dışında kalır.
Core.check → Handler.check yalnız metadata/permit kontrolüdür; yeniden
Core.admit çağrısı yapmaz. Handler'ın son kapsam kontrolü, Core kapsamı
kapandıktan sonra kendi iznini kullanır. Normal metotlar kapsam dışında,
authentication binding, checkpoint bind/clear ve destruction her zaman asıl
worker/fiber gerektirir. Yeniden giriş yutulsa da poisoned durum korunur.

Bilinen `no_response_02bcd0_04d228` kontrol sonucu yalnız eylem bölgesinde
yakalanır. İki kapsamın son kontrolleri ve descriptor kaldırmaları başarıyla
bittikten sonra dışarı aktarılır. Admission/son kontrol hatası aynı C++ istisna
türünü kullansa bile geçerli kontrol sonucu sayılmaz; poisoned olur ve çocuk
kapsam sınırında logic_error'a dönüşür. Opaque SDK/XML hataları da poisoned
kalır. Başarısız son kontrol ikinci kez denenmez.

Core normal kapanışta GuardedFiles'ın known durumda ve bekleyen ham dosya
sahibinin sıfır olmasını gerektirir. Belirsiz open/read sonuçlarında ham
sahiplik korunur; tahmini close/delete veya tekrar çalıştırma yapılmaz.
Dosya callback'i stack descriptor yerine yaşayan Impl'ı kullanır.

Graph License adapter'ı canlı Graph JobCall'dan aynı Parent/permit'i alıp
somut Handlers'ı senkron çağrı boyunca shared olarak tutar. Üç normal handler
ve bağımsız unknown-message diagnostic bu bağlantıyı kullanır. Diagnostic
Graph sınırında control=false kalır. Varsayılan asıl-worker CheckedCalls
rotası korunur; çocuk rotasında legacy/custom fallback seçilmez.

Mevcut kaynak rotaları: auth `02e3b0` → Core owned auth → Online slot30;
request `0320c0` → status `01ea20`, hash `042d20`, token `02afa0`;
invalidate `030c50`; ticket `03c1d0`; UTC `041b60`; Wine `045010`;
unknown-message diagnostic `07bd70`. Bunlar önceki kaynak gövdeleridir;
bu aşamada yeni REA/adres veya orijinal çalışma zamanı kanıtı üretilmedi.

`effects_tests.cpp` içinde gerçek Domain/NativeThreads/prepared iki Parent
altyapısıyla alt katman senaryoları yazıldı: sentetik bellekte SHA1/AES,
primary/cache/invalidate/block ve sonrasında sağlıklı auth; kendi test EXE'si
Online slot30'u; kontrollü UTC, ticket/NUL ve UI; yanlış Parent/actor,
expired/fresh izin, void/değer sonuç, scope dışı erişim ve yeniden giriş.
Tek seferlik aynı-marker checkpoint hataları Core başlangıç/son/admission
aşamalarında ve iki katmanın ayrı kapanışlarında tarif edilir. İki yeni izole
süreç modu open/read belirsizliğinde ham input'un kapatılmadığını/yok
edilmediğini denetleyecek şekilde yazıldı. **Hiçbiri çalıştırılmadı.**

Sınır: bu, tam prepared Graph/Profile çocuk entegrasyon testi değildir.
Parent izni tek başına borrowed Runtime/Online ve lower-code ömrünü veya dış
global erişimlerin serialize edildiğini kanıtlamaz. Profile'ın exact-job
JobLogic seçimi, request/response/closure/021770 zinciri, native Jobs ve fiziksel
thread/FLS çıkış bağlantıları, uçtan uca fixture ve iki üretim loader hook'u
açıktır. Güncel kaynak derlenmediğinden üretim DLL'inin hazır olduğu veya
toplam tamamlanma yüzdesinin yükseldiği iddia edilmez. Önceki ZIP/derleme
kanıtları tarihsel snapshot'larına aittir.
