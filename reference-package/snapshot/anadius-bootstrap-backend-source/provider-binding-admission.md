# SourceBackend kısmi kurulumunda kaynak kabulü

Runtime'ın `Prepared` ve SourceBackend'in `Selection` owner'ları, Resources nesnelerini yardımcı `narrow` sağlayıcıdan önce yıkacak sırada kurulmuş. `checked(...)` de Impl allocation'ından önce sıralı Selection'ı oluşturuyor. Bu sınırlı transfer incelemesinde bu sırayı değiştirecek bir açık bulunmadı.

Bir eksik, `bind_state` içindeki opaque çağrılardaydı: eski koruma yalnız Online factory/identity bölümünü kapsıyordu. Worker veya callback-attachment factory'si, hatta onlardan önceki provider binding çağrısı, henüz kurulum bitmeden `register_exit` ile route yayımlayabilir ya da `stop_and_join` ile dış stop işlemini başlatabilirdi. Bu, factory/binding işlemlerinin pasif kalması sözleşmesine aykırıdır; önceki kaynak bunu aynı biçimde denetlemiyordu.

`backend.cpp` artık tüm `bind_state` süresi boyunca tek bir source-admission reservation kullanıyor. Library phase, configuration state/path, Online, Worker ve attachment bağlamaları bu kapsamdadır. `alive()` kullanan backend kaynak metotları bu sırada geri girişi reddeder ve sticky poison kaydeder. Worker/boolean route seçimi de aynı denetimden geçer. `state()` ve tracking metadata okumaları kullanılabilir; opaque çağrı boyunca mutex tutulmaz. Bu, yeni bir thread synchronization modeli değildir; mevcut serialized controller sözleşmesi sürer.

Factory sonucu, reservation tekrar denetlenmeden önce backend tarafından sahiplenilir. Factory hatayı yutup owner döndürürse kurulum yine reddedilir; owner erken bırakılmaz. Worker'a yapılan her dependent binding çağrısından sonra ayrıca kontrol vardır. Scope guard exception halinde reservation'ı kaldırır, fakat poison'ı temizlemez. Reentry gözlenmeyen, yan etkisiz sentetik factory exception'ında mevcut unpublished cleanup yolu korunur. Bilinmeyen/poisoned owner'ın normal destructor'la geri alınabildiği iddia edilmez; mevcut terminal retention sözleşmesi uygulanır.

Public imzalar, Resources düzeni ve vtable değişmedi. Geçmişte bağlama sırasında `providers()` gibi korunan backend yöntemlerini çağıran custom factory'ler artık reddedilir; bunların ihtiyaç duyduğu referanslar construction girdilerinden sağlanmalıdır. Bu değişiklik, normal hazır provider çağrılarını veya özgün senkron başlangıç sırasını yeniden düzenlemez.

`backend_integration_tests.cpp` kaynaklarına şu beklentiler eklendi:

- Normal phase binding + Worker/attachment factory'leri pasif metadata okuyabilir, owner'lar benimsenir ve unpublished retirement sırası korunur.
- Owner döndürmeden exception atan iki pasif factory yolu reservation'ı bırakır; route yayımı veya source stop yapılmaz.
- Worker factory, attachment factory ve phase binding için üç izole negatif mod, `register_exit`/`stop_and_join`/`providers` retlerini yutup devam eder. Dış kurulumun yine reddedilmesi, registrar/stop sayaçlarının değişmemesi ve benimsenmiş sahiplerin tutulması beklenir. Negatif modlar destructor cleanup'ı uydurmadan `_Exit(181)` ile sonlanacak kaynaklardır.

**Bunlar derlenmedi veya çalıştırılmadı. PASS sonucu yoktur.** Statik inceleme yalnız yazara aittir. Bir üretim CPP, bir header açıklaması ve bir entegrasyon test CPP değişti. Son tam derlemeden farklı C/C++ dosya sayısı 73'tür. Yeni native gövde veya toplam DLL yüzdesi artışı sayılmadı.

Bu koruma external SDK etkilerini, başka thread'lerin sözleşme dışı çağrılarını, caller'ın sahipliği yok etmesini veya arbitrary cached entry kullanımını bütünüyle denetleyen bir sandbox değildir. Registry/remaining factory ve bütün diğer constructor prefix'leri için genel bir güvenlik ispatı verilmez. Loader hook tanımları, gerçek loader profili ve image/foreign/CRT finality hâlâ açıktır. Orijinal DLL'ler, eski build kanıtları ve ZIP değişmedi.

Sonraki hedef, genel constructor incelemesini uzatmak yerine, seçilen hook engine ile özgün senkron host/hook kurulumunun gerçek çağrı sırasını karşılaştırmaktır. Satır ve hash kanıtları: `provider-binding-admission.evidence.json`.
