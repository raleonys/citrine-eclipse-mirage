# NLS kaynak zincirinin gerçek Windows API bağlantısı

Bu yeni C++ adapter, önceki kurtarılmış NLS/dispatch/locale/record kaynaklarını Windows SDK çağrılarına bağlar. Yeni çıkarılmış özgün fonksiyon gövdesi sayısı **0**; katkı, mevcut algoritmaların native API sınırının uygulanması ve sınanmasıdır. Tam bağımsız DLL değildir.

`Backend`, CP/conversion/GetStringTypeW çağrılarını `crt_nls_native::Calls` nesnesine; scratch, input-tail ve exchange işlemlerini zorunlu `ContextEffects` sağlayıcısına yönlendirir. Dışta `crt_nls_dispatch_bridge::Effects`, ayrı `crt_nls_dispatch_native::Calls` ile kullanılmalıdır. Eski 0671d4 slotuna doğrudan başvurulursa açık hata üretilir; yapay başarı değeri yoktur. Bağımlılıklar ve ham adreslerin canlı bellekleri bütün çağrı boyunca geçerli olmalıdır.

## Doğrulama

`build.ps1` LLVM-MinGW/C++20 ile kendi test EXE'sini derler ve çalıştırır. **2.114 kontrol PASS**. Karşılıklı kaynak incelemesi `review-native-bridge.json` dosyasındadır; ana derleme sonuçları `common-validation.json` dosyasında tutulur.

- Sentetik UTF-8 A/Ä girdisinin sınıflandırılması, küçük/büyük harfe dönüşümü ve sort-key çıktısı gerçek SDK çağrılarıyla sınandı. Hatalı UTF-8 başarısızlığı tanımlı çıktı sayılmıyor.
- Gerçek sistem modülü çözümleyicisi üzerinden modern Ex yolu ve testte export yokluğu uygulanarak eski W/statik LCID yolu sınandı. Paylaşılan kurtarılmış resolver cache'inin tekrar sorguyu önlediği kontrol edildi.
- 504/505 karakter sınırında kurtarılmış marker/stack/heap kararları ve heap başarısızlığında errno12/temizlik sınandı.
- 0x228 baytlık sahipli kayıtta CP932, UTF-8 ve SBCS üretimi gerçek CPInfo/type/map çağrılarıyla sınandı; UTF-8 dalındaki önceki lead-byte bilgisinin korunması gözlendi.

Testteki loader yalnız sabit sistem modüllerini yükler ve sahip olduğu referansları bırakır. Üretim için genel loader/global yaşam döngüsü yerine geçmez. PTD/FLS, heap, lock/atomic, scratch ve input-tail/global depoları hâlâ kontrollü sağlayıcılardır. Diğer bağlı kaynakların tüm algoritmalarının bu grupta çalıştığı iddia edilmez. Native adapter'ların kendi staging alanları çevresindeki LastError koruması testlidir; bütün CRT zincirinin özgün LastError/ABI/SEH davranışının eşdeğerliği kanıtlanmış değildir.

Özgün DLL/oyun yüklenmedi veya çalıştırılmadı; girdi dosyaları değiştirilmedi. Orijinal ve yeniden derlenen DLL arasında runtime karşılaştırması yapılmadı.

## Adresli dayanaklar

Önceki statik çıkarımlar: `06e210` GetStringTypeW, `06cc30/06c8fc/063168` map/sayım, `06ccc8/06cd58` conversion API politikası, `0671d4/0672c0` modern/eski API seçimi, `0669b0` resolver, `07310c/0736d4` locale fallback ve `06f674/06f050/06f0e8` record/case üretimi. Bunlar tercih edilen image tabanında `0x180` önekiyle tam VA'lardır; bu adresler callback olarak yürütülmez. Hash bağlı delil bağlantıları `evidence.json` içindedir.

SDK sözleşmeleri: [GetStringTypeW](https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-getstringtypew), [WideCharToMultiByte](https://learn.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte), [LCMapStringEx](https://learn.microsoft.com/en-us/windows/win32/api/winnls/nf-winnls-lcmapstringex). SDK tipleri ile byte/WORD/UTF-16 kapasite ayrımı, ayrı native modüllerde uygulanır.
