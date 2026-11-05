# Configuration hata yolunda sıralı sahiplik — kaynak aşaması

Configuration constructor/factory/installer zincirinde, kısmi kurulum başarısız olduğunda `Inputs::lifetime` sahibinin document/license capture'larından sonra bırakılmasını açıkça sağlayan bir yerel owner eklendi. Önceki kaynak bu ayrı by-value parametrelerin yıkım sırasına güveniyordu. Bu, statik incelemede bulunan bir **sıra garantisi eksikliğidir**; mevcut derleyicide çalıştırılarak gözlenmiş UAF iddiası değildir.

`factory.cpp` içindeki `PendingConfiguration`, Inputs'i ilk, document/license sahiplerini son alanlar olarak alır. Altı public giriş noktası bu owner'ı herhangi bir allocation veya occupied-slot validation öncesinde oluşturur. Alanların taşınmasının exception üretmemesi için Inputs ve Paths move işlemlerine compile-time koşullar eklendi. Facade allocation başarısızsa yerel owner bütün girdileri tutar; facade constructor'ı içine girilmişse aynı düzen bu kez Impl allocation'ını korur. Impl de yalnız bu tek owner referansını alır; normal kurulmuş üyelerin yıkım sırası korunur. Public imzalar, sınıf yerleşimi ve vtable değiştirilmedi.

| Hata noktası | Tutulan sahiplik |
|---|---|
| Factory facade allocation | Henüz move edilmemiş PendingConfiguration; document/license önce, Inputs backing sonra bırakılır. |
| Constructor Impl allocation | Constructor'ın kendi PendingConfiguration'ı aynı sırayı sağlar. |
| Impl validation veya Persistence allocation | Kurulmuş Impl üyeleri document → license → persistence → Inputs sırasıyla bırakılır; backing Inputs içindedir. |
| Installer'da dolu configuration alanı | Validation'dan önce benimsenmiş owner rejected girdileri bırakır; mevcut Resources sağlayıcısı değiştirilmez. |
| Başarılı kurulum | Mevcut pasif construction ve attached retirement akışı devam eder; installer yalnız tamamen kurulmuş sonucu yayımlar. |

Gerekçe C++ dil kurallarıyla sınırlıdır: parametrelerin birbirlerine göre kurulma sırası belirli değildir ve parametreler kendi kurulma sıralarının tersinde yıkılır. Bu yüzden public parametre listesinin yazılış sırası backing ömrünü garanti etmez. [Function call kuralları](https://eel.is/c++draft/expr.call#7), [new-expression sıralaması](https://eel.is/c++draft/expr.new#25) ve [üye kurulum/yıkım sırası](https://eel.is/c++draft/class.base.init#15) bu kaynak çıkarımını destekler. Yeni aggregate'ın entry sonrasındaki ilk adoption işlemleri nonthrowing olmalıdır. Çağıranın daha fonksiyona girilmeden yaptığı argument construction hataları bu değişikliğin kapsamı dışındadır.

`factory_tests.cpp` içine yalnız sentetik test executable'ında etkili bir allocation-failure seçicisi eklendi. Girdiler tamamen hazırlandıktan sonra tek bir allocation bad_alloc üretir ve seçici kendini kapatır. Altı giriş yolu için facade/Impl/Persistence allocation prefix'lerinde toplam 16 planlı senaryo; iki installer için de occupied-slot rejection senaryoları eklendi. Dışarıda yaşayan gözlem nesnesi, backing'in document/license destructor'ları boyunca canlı kaldığını denetler; test bilinçli olarak bırakılmış backing belleğini dereference etmez. Kaynak işinin çağrılmaması ve başarısız sonuçların yayımlanmaması da beklenir.

**Derleme, proje testi, link veya hedef çalıştırması yapılmadı.** Yeni static_assert ve allocation senaryoları henüz compiler/runtime doğrulaması almadı. Allocation sayıları bu seçilen test kaynaklarının beklentisidir; farklı optimizasyon/stdlib davranışı görülürse fixture ayrıca değerlendirilmelidir. Bağımsız ajan incelemesi yapılmadı.

Bu aşamada bir üretim CPP, bir public header açıklaması ve bir test CPP değişti. Son tam derlemeden farklı kaynak/header sayısı 72 oldu. Özgün senkron startup kararı korunur; iki loader hook tanımı ve loader bağlamı/rollback/finality hâlâ açıktır. Yeni native gövde veya toplam DLL yüzdesi artışı sayılmadı. Orijinal DLL'ler, tarihsel build kanıtları ve ZIP değiştirilmedi. Satır/hash kanıtları `construction-ownership.evidence.json` içindedir.
