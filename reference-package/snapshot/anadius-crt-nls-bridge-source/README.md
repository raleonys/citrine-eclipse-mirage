# NLS sınıflandırma ve harf dönüşümü kaynak bağlantısı

Bu köprü önceki record bileşimindeki `06e210` ve `06cc30` sağlayıcılarını gerçek kaynak gövdelerine bağlar. `06cc30` içindeki `06c8fc` dönüşüm/tampon algoritması ve `063168` bounded-count yardımcısı da kaynakta çalışır. Dönüşümler mevcut gerçek `06ccc8` / `06cd58` API politikasını, gerçek LocaleUpdate ve malloc/free kaynaklarını kullanır.

| Özgün adres | Kurtarılan davranış |
|---|---|
| `0x18006e210` | Locale CP seçimi, iki MB→wide çağrısı, geçici payload sıfırlama, native GetStringTypeW, marker ve sahip olunan PTD bayrağı temizliği |
| `0x18006cc30` | LocaleUpdate dış sarmalayıcısı; pseudocode void gösterse de assembly EAX sonucunu korur |
| `0x18006c8fc` | Giriş uzunluğu, wide query/fill, LCMAP_SORTKEY veya ikinci wide tampon ve geri dönüşüm; ikinci/ilk marker sırasıyla bırakma |
| `0x180063168` | Bounded byte sayımı; giriş NUL değilse `source[count]` konumunu da okur |

Wide sayısı 504 için 1008 payload+16 marker byte toplamı1024 stack yolundadır; 505 için1026 byte gerçek kurtarılmış malloc/free akışına gider. Stack marker `CCCC`, heap marker `DDDD`; karar cleanup sırasında canlı marker yeniden okunarak verilir. Bunlar native stack ABI kurulumu değildir. Stack alanları çağıranın zorunlu scratch sağlayıcısındadır. GetStringType payload'ı sıfırlar; LCMap wide tamponları aynı şekilde sıfırlanmaz. Sorgu uzunluğu ve doldurma dönüşü aynı değişken sanılmaz; her gövde assembly'de kullanılan sayıyı korur.

`Effects` eski `multibyte_record_bridge::NativeEffects` arayüzünü uygular. Native GetStringTypeW çıktısı `WordOutput`, dar/sort-key çıktısı `ByteOutput` hücrelerine taşınır. Raw adres olarak kullanılan çıktı kimlikleri bu yeni adapter'ın token'larıdır; gerçek LPWORD/LPSTR veya özgün ABI layout'u oldukları ileri sürülmez. Native provider yazdığı hücreleri açıkça tanımlar. Başarısız çağrıda uydurma sıfır üretilmez; eski case kaynağı tanımsız hücre tüketirse açık kapsam sınırı korunur.

Case üreticisinin mantıksal giriş span'i256 byte'tır. `__strncnt`'nin fazladan endbyte okuması `read_input_tail(end_address)` zorunlu sağlayıcısına gider. Sağlayıcı bilinen komşu byte verebilir veya bilinmeyen okunmayı reddedebilir. Testte0 ve255 için aynı normal sonuç doğrulandı. Bu, özgün komşu stack byte'ının kurtarıldığı, adresin hatasız okunabildiği veya fault footprint'inin eşit olduğu iddiası değildir. Native dönüşümlerin explicit count'u bu endbyte'ı tüketmez; giriş payload'ı kopyalanıp değiştirilmez.

`Invocation` yeni adapter'ın geçici input kaydını, çıktı token/span bağlarını ve scratch çerçevesini yönetir. Bu metadata cleanup'ı özgün SEH/unwind değildir; kaynak gövdelerinde olmayan heap/PTD exception cleanup'ı eklenmez. Bağımlılıklar, aynı canlı heap/global/PTD adres alanı, borrowed memory ve scratch sağlayıcıları çağrı süresince geçerli ve eşzamanlanmış olmalıdır. Testler ardışık frame bırakmayı doğrular; native callback üzerinden iç içe Invocation ve concurrency ayrıca sınanmadı.

```powershell
& '.\outputs\anadius-crt-nls-bridge-source\build.ps1'
```

Strict C++20 bağımsız derleme **27.665 kontrol PASS**. Önceki record/SBCS/case/locale/PTD/argv/heap/deep-free/onexit testleri gerçek NLS gövdeleri üzerinde çalışır. Ek testler504/505 stack/heap sınırını, iki map tamponunun gerçek heap ile bırakılmasını, UTF8 API flag süzmesini, NUL nedeniyle uzunluk kısalmasını, doğrudan sort-key çıktısını, malloc ve native-map hatalarını, tanımlı çıktıya rağmen native BOOL0 sonucunun case tarafından yok sayılmasını, endbyte0/255 ve metadata/frame bırakmayı kapsar. Kontrol sayısı bağımsız semantik test veya DLL tamamlanma yüzdesi değildir.

`0671d4` LCMapStringEx/LCMapStringW çözümleme gövdesi ve native GetStringTypeW, dönüşüm, CP, scratch/Heap/FLS/lock/atomik uçları zorunlu sağlayıcıdır. Test backend'i byte→UTF16 eşleme ve ASCII case dönüşümleri kullanır; gerçek Windows codepage/locale tabloları değildir. Native locale API, DLL/oyun veya hook çalıştırılmaz. Girdiler değiştirilmez. Tam DLL, orijinal ABI, security cookie, stack probe, native hata/SEH ve concurrency eşdeğerliği açık kalır. Tam seçilmiş kanıtlar iki komşu kaynak modülündedir; `review-string-type.json` yalnız güncel kaynak/test incelemesini kaydeder.
