# Karakter sınıflandırma sarmalayıcısı: 0x18006e210

`string_type.cpp`, `anadius64.dll` içindeki tek fonksiyonun normal kontrol akışını C++20 olarak yeniden kurar. Özgün kaynak değildir. Orijinal DLL yüklenmez veya çalıştırılmaz; test uygulaması yalnızca kurtarılmış kaynak ve sentetik sağlayıcılardan oluşur.

## Adres ve kanıt

- Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
- `0x18006e210`: pseudocode `ev_55d88e9cae0384ad87f9dd04be6ed1385a4141630d9ea934b9d1296fdde5d839`; assembly `ev_ff659bc38c0cecbd6049ae735ce486d23e30ad115dccbbb5d59e32f94f015a52`. Tam sınırlı gövdeler ve büyük kaynakların SHA-256 kimlikleri `evidence.json` içindedir. `retain-evidence.ps1` hash denetimiyle yeniden üretir.
- `0x18006e24d`: `058b04` locale güncellemesi her çağrıda çalışır. CP sıfırsa `0x18006e25c–260`, `locale_info+0xc` alanını okur; multibyte kaydını kullanmaz.
- `0x18006e263–283`: ilk `06ccc8` dönüşüm çağrısının bayrakları strict sıfırsa `1`, diğer tüm 32 bit değerlerde `9`; hedef ve kapasite sıfırdır. Ham kaynak adresi ve işaretli kaynak uzunluğu değiştirilmeden taşınır.
- `0x18006e288–2a9`: sorgu EAX değeri önce işaret genişletilir, ikiyle çarpılır; 64 bit sarılan toplama ile 16 bayt başlık eklenir. Taşan başlık toplamı sıfıra çevrilir. Negatif sorgu değerleri için tanımsız C++ işaretli taşması icat edilmez; `-1..-8` sıfır boyut yoluna, `-9` ve daha küçükleri çok büyük ham heap isteğine gider. Gerçek API'nin negatif dönüş yapması beklenmez; bu sözleşme ham makine kontrol akışını korur.
- `0x18006e2af–2e7`: toplam `<=1024` olduğunda 16'ya yuvarlanmış stack scratch kullanılır; sorgu `504` son stack, `505` ilk heap durumudur. Başlığın DWORD işareti stack için `0xcccc`, heap için `0xdddd` olur. Wide veri başlıktan 16 bayt sonradır.
- `0x18006e305–30d`: veri alanı tam `2*query` bayt sıfırlanır. `0x18006e312–329` ikinci dönüşümün bayrağı daima `1`, kapasitesi ilk sorgu değeridir. İkinci dönüşüm başarısızlığı native çağrıyı atlar.
- `0x18006e332–348`: `GetStringTypeW` IAT adresi `0x18007f3e0`; native uzunluğu ilk sorgudan değil ikinci EAX sonucundan alır. Output ham adresi aynı şekilde aktarılır. Dönüş değeri tam 32 bit BOOL olarak korunur.
- `0x18006e355–361`: cleanup, veriden 16 bayt önceki işareti yeniden okur. Yalnızca güncel işaret `0xdddd` ise başlık adresi `0668c0` ile serbest bırakılır. Önceden seçilen stack/heap dalının hatırlanması aynı davranış değildir.
- `0x18006e366–370`: locale context sahiplik baytı sıfır dışındaysa güncel `PTD+0x3a8` yeniden okunur ve yalnızca bit `2` temizlenir. Bu işlem normal hata çıkışlarında da allocation cleanup sonrasında uygulanır.

## API ve sınır

`Effects`, ham 64 bit adres alanı, locale constructor, malloc/free, scratch frame, sıfırlama, `06ccc8` ve native `GetStringTypeW` sağlar. Ortak köprü gerçek kurtarılmış locale/heap/dönüşüm gövdelerini bağlayabilir. Native API ve scratch alanı hâlâ çağırana ait bağımlılıklardır. `stack_allocate` marker başlığının adresini verir; ayrı çağrı çerçevesinin geri alınması çağıranın görevidir. Bu arayüz özgün alloca stack pointer'ını, probe veya unwind metadata'sını yeniden oluşturmaz.

Tüm normal dönüş yolları kaynaklaştırılmıştır. Güvenlik cookie kontrolü, `__chkstk`, SEH/nonlocal unwind ve bozuk belleğin gerçek işletim sistemi etkisi doğrulanmış değildir. Sağlayıcı istisnaları özgün SEH eşdeğeri sayılmaz. Testlerin sparse adres uzayı devasa istekleri yalnızca kaydeder; çok büyük host tahsisi veya gerçek stack tüketimi yapmaz. İşaret mutasyonu testleri cleanup yeniden okumasını kanıtlar; gerçek heap'e stack pointer serbest bırakan uygulama değildir.

## Doğrulama

`build.ps1`, LLVM-MinGW clang++23 ile C++20, `-O2 -Wall -Wextra -Werror` altında üretim nesnesini ve statik fixture uygulamasını derler. `validation.json` hash ve sonucu kaydeder. **70.165 kontrol**: sıfır/başarılı/negatif sorgu, 504/505 sınırı, yuvarlama ve 64 bit sarılma; explicit/locale CP, strict işaretli uçları; queried kapasite ile filled uzunluk ayrımı; tam BOOL dönüşleri; sorgu/fill/tahsis hataları; NULL'a sarılan veri adresi; sağlayıcı sonrası başlık yeniden okuması ve locale sahipliği/temizleme sırası. Bunlar kaynak kontrol akışına ait sentetik testlerdir; özgün DLL ile karşılaştırmalı native çalıştırma değildir.
