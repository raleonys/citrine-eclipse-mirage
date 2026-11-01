# Multibyte başlatma ve codepage kaynak bağlantısı

Bu yeni C++ köprüsü, kurtarılan beş normal akış gövdesini mevcut locale/PTD/heap/errno/invalid ve argv kaynaklarına bağlar. Tam DLL üretmez. `06f674` karakter kaydı ve tablo üreticisi zorunlu, henüz kurtarılmamış bir sağlayıcıdır; testteki deterministik üretici onun yerine yazılmış üretim kodu değildir.

| Özgün adres | Kurtarılan davranış / yeni bağlantı |
|---|---|
| `0x18006efd0` | Her çağrıda gerçek LocaleUpdate; -2 OEMCP, -3 ACP, -4 **locale_info+0xC**; sahip olunan PTD bit2 temizliği |
| `0x18006ee00` | Kilit5 altında 257 type ve 256 case baytı yayımlama, global eski referansı bırakma, PTD kaydını global hücreye taşıma ve artırma |
| `0x18006532c` | Locale-changed DWORD'ünü 1 ile atomik değiştirme; önceki değeri döndürme |
| `0x18006f2d0` | Gerçek thread yenileme, CP seçimi, 0x228 tahsis/snapshot, zorunlu builder, referans değişimi, maskeye bağlı yayın; yalnız tam EAX=FFFFFFFF hata |
| `0x18006f5f8` | Case/current/type global kurma, **raw required PTD**, ACP setter; setter hatasına rağmen initialized=1; yalnız AL=1 sonuç sözleşmesi |

`Effects`, aynı canlı locale global hücrelerini kullanır; özellikle `current_multibyte_slot_0a59d0()` kopya bir pointer cache'i döndürmez. Malloc/free gerçek kurtarılmış heap gövdelerine, errno ve invalid gerçek kaynak gövdelerine, thread yenileme `06f540` gövdesine bağlanır. `Policy`, önceki sayaç fixture'ı yerine gerçek initializer'ı argv configure akışında çağırır. Modül yolu ve wildcard davranışı mevcut kaynak sınıfından gelir.

Çağıran taraf `Dependencies` içindeki nesneleri, canlı adres alanını, paylaşılan globals, başlangıç kimliklerini ve tablo depolarını aynı ömür/eşzamanlama alanında tutmalıdır. `identities.initial_multibyte` ile locale globals içindeki başlangıç MB kimliği aynı kaydı belirtmelidir. `Memory` hem global hücreleri hem PTD ve tahsisleri kapsamalıdır. CP API'leri, atomik exchange, lock/XADD, FLS ve diğer native uçlar zorunlu sağlayıcılardır; köprü bunların native implementasyonunu kurmaz. Global başlangıç görüntüsü, native concurrency, SEH/unwind ve tam ABI eşdeğerliği kanıtlanmış değildir.

```powershell
& '.\outputs\anadius-crt-multibyte-bridge-source\build.ps1'
```

Strict C++20 bağımsız derleme: **10.377 kontrol PASS**. Testler gerçek control/support/locale/thread/PTD/heap kaynaklarını birlikte kullanır. İlk/idempotent başlatma, raw PTD LastError etkisi, -2/-3/-4 CP, iki dinamik kayıt değişimi, 513 tablo baytı, global/thread referans dengesi, positive builder-status=7, malloc/builder hatası, dönen invalid handler, başarısız başlatmanın initialized davranışı ve başlangıç pair yayını sınandı. Owned dinamik kayıtlar gerçek thread/publish/free akışıyla bırakıldı. Bu test teardown'u özgün DLL shutdown gövdesi değildir.

Native GetACP/GetOEMCP, gerçek 06f674, Heap API, lock/atomik ve cached FLS uçları testte kontrollü sağlayıcılardır. Fixture tabloları CP uyumluluğu iddiası taşımaz. Eski argv/find/conversion/onexit/deep-locale test akışları da bu yeni kaynak bileşimi üzerinde çalışır. Gerçek dosya arama, modül yolu API'si veya özgün/yeniden derlenmiş DLL/oyun çalıştırılmaz; orijinal girdiler değiştirilmez.

Adresli tam retained pseudo/assembly ve Evidence ID'leri komşu support/control modüllerindedir. `evidence.json` kaynak bağlantılarını kaydeder. `validation.json` bağımsız derlemeyi, `common-validation.json` ortak derlemeyi, `review-support.json` kaynak incelemesini belgeler. Kaynak incelemesinden sonra eklenen dokümanlar ve kapsam açıklaması ayrı metadata incelemesiyle kaydedilir. Kaynak arayüzleri yeni C++ tasarımıdır, özgün MSVC sınıf/ABI kaynakları değildir.
