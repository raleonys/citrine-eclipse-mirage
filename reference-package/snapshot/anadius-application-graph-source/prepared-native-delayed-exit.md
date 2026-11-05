# Bildirim kabulü ile fiziksel çıkış arasındaki kaynak senaryosu

Sonraki aşama: [Graph/sunucu kapanış retry düzeltmesi](native-close-retry.md).
Aşağıdaki inceleme kendi önceki kaynak hash'ine aittir.

Durum: **kaynak eklendi; derlenmedi, test/link/DLL çalıştırılmadı**.
Değişiklik yalnız `graph_integration_tests.cpp` içindedir. Üretim C++ kodu ve
orijinal dosyalar değişmedi. Bu aşama toplam DLL yüzdesinde ilerleme sayılmaz.

Yeni `--prepared-native-delayed-exit` modu, mevcut gerçek Domain/PreparedMemory/
NativeThreads/Graph/Profile/NativeJobs senaryosunu kullanır. İlk çocuk, hazır
bildirimini yayımlayıp native iş gövdesinden döndükten sonra, gerçek `ExitThread`
çağrısının hemen önünde iki sahipli event ile bekletilir. Bu konum FLS callback'i
içinde değildir; gerçek FLS çıkışı hâlâ daha sonra gerçekleşecektir.

Asıl worker bildirimi tükettiğinde kaynak beklentileri şunlardır:

- Çocuk proof HANDLE'ı `WAIT_TIMEOUT`, exit code `STILL_ACTIVE` verir.
- Kaynak job tamamlanmıştır; NativeJobs satırı hâlâ kapasite1'i tutar.
- Bir sonraki Parent dışı kontrolde iş sayacı tam bir kez azalmıştır.
- Gerçek `quiescent_prepared` false döner; private HANDLE ve barrier kapanmaz,
  iş sayacı yeniden azaltılmaz, sağlayıcı Bound kalır.

Ardından event açılır. Mevcut proof join ve NativeJobs fiziksel reaping yolları
çalışacak şekilde bırakılır; 70 mesajın FIFO, tekil yanıt, kapasite reuse ve
HANDLE kapanış beklentileri korunur. Controller, iki event'i ve trace sahibini
asıl worker ile çocukların gerçek fiziksel çıkışından sonra bırakır. Beklemeler
sınırlıdır; bilinmeyen sonuçta canlı sahipler unwind edilmeden süreç sonlandırılır.

Bu ifadeler yürütülmüş sonuçlar değildir. Bu aşamada yalnız yazarın statik kaynak
incelemesi yapıldı; önceki bağımsız inceleme eski kaynak hash'ine aittir. Yeni
inceleme ve adreslenebilir satır/hash kanıtları `prepared-native-delayed-exit-review.json`
ile `prepared-native-delayed-exit.evidence.json` içindedir. Senaryo tek kontrollü
zamanlamayı kapsar; bütün yarışlar veya production ABI/finality kanıtı değildir.

Sıradaki kaynak hedefi erken stop/detach ve daha sonraki sağlayıcının kapanışı
ertelemesi durumunda retry akışıdır. Üretim loader hook'ları ve CRT/image/foreign
ömür sözleşmeleri ayrıca açıktır. Derleme ve çalışma zamanı doğrulaması açık
yeniden derleme talimatını bekler; mevcut DLL/ZIP ve eski build kanıtları korunur.
