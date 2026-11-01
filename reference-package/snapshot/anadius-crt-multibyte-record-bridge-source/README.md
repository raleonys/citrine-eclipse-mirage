# Gerçek multibyte kayıt ve tablo üretimi bağlantısı

`RecordNative`, önceki multibyte köprüsündeki `06f674` sağlayıcı arayüzünü **gerçek kurtarılmış kaynak gövdesine** bağlar. Bu gövdenin `06f050` SBCS ve `06f0e8` case çağrıları da gerçek kaynaklara gider. Artık bu bileşimde sentetik bir record builder kullanılmaz. Tam DLL/özgün C++ kaynağı veya native davranış eşdeğerliği iddiası değildir.

| Adres | Kaynakta bağlanan davranış |
|---|---|
| `0x18006f674` | CP seçimi; beş gömülü DBCS kaydı; dinamik CPInfo; UTF-8 kısmi yazımı; SBCS/case çağrıları |
| `0x18006f050` | Altı WORD ve başlık alanlarını sıfırlama; canlı başlangıç kaydından 257/256 byte kopyalama |
| `0x18006f0e8` | ASCII fallback veya CPInfo/ctype/map üzerinden case ve sınıflandırma bitlerini üretme |
| `0x18006efd0` | Önceki gerçek SystemCP gövdesi; aynı canlı `0a59d8` bayrağına yazma |
| `0x18006f2d0 / 06f5f8` | Önceki gerçek setter/initializer; aynı PTD, heap, locale ve argv kaynak zinciri |

Beş sabit CP kaydı (932/936/949/950/1361) orijinal hash doğrulanarak PE `.data` bölümünden okundu: 240 byte, dosya `0xa06c0`, SHA-256 `bdb03f7a145976f2c290206ebd8efd878e013c5bb058b02cfc9c01b1a1fa3970`. Bu tabloların çalışma sırasında değiştirilmesi kapsam dışıdır; getter ile verilen locale pointer hücreleri canlı okunur. Adresli tam pseudo/assembly ve ham sabit çıkarma kanıtı komşu builder/tables modüllerindedir.

Yeni bileşim kurulumu: önce `RecordNative`, sonra onu native arayüz olarak kullanan önceki `multibyte_bridge::Effects` oluşturulur; ardından `bind_system_cp(effects)` ve normal başlatma çağrılır. Bağlanmamış kullanım açık `logic_error` verir. Farklı bir support nesnesine yeniden bağlama reddedilir; bu kaynak bileşiminin yaşam süresi kuralıdır, özgün DLL davranışı değildir. Binding bellek/global alanlarının tutarlılığını otomatik kanıtlamaz. Bütün borrowed nesneler, aynı heap/PTD/global adres alanı, initial MB kimliği ve locale isim depoları çağrılar boyunca geçerli ve eşzamanlanmış olmalıdır. Üç köprüdeki başlangıç MB kimlikleri aynı kaydı göstermelidir.

Native CP/GetCPInfo/IsValidCodePage, Heap/FLS/lock/atomik etkiler hâlâ sağlayıcıdır. Özellikle `06e210` GetStringType ve `06cc30` LCMapStringA alt CRT gövdeleri bu aşamada kurtarılmadı. Testler bunları açıkça tanımlanmış çıktı hücreleriyle verir. Case üreticisi helper dönüşlerini yok sayar; sıfır dönüşte bile gerekli çıktılar tanımlıysa devam eder. Özgün başlatılmamış stack hücresi tüketilecekse uydurma sıfır yerine açık reconstruction-boundary istisnası gelir. Bu istisna özgün hata/SEH davranışı değildir. Başarılı CPInfo içindeki lead çiftleri 12 byte içinde sonlanmalıdır; aksi yol da açık kapsam sınırıdır.

```powershell
& '.\outputs\anadius-crt-multibyte-record-bridge-source\build.ps1'
```

Strict C++20 bağımsız derleme **12.032 kontrol PASS**. Gerçek kaynak bileşiminde 932 başlık/WORD/lead/case alanları, 932→UTF8→1251→dinamik DBCS→SBCS geçişleri, UTF8 sırasında eski type bitlerinin korunması, 513 yayın byte'ı, dinamik ref/free dengesi, başarısız init/malloc/validation, yok sayılan helper hataları ve tanımsız çıktı sınırı sınandı. Doğrudan builder `-3` çağrısının CPInfo hatasında SBCS fallback yapması; çözümlenmiş pozitif CP çağrısının system bayrağını sıfırlayıp hata vermesi de gerçek SystemCP ile doğrulandı. Önceki argv/find/conversion/locale/deep-free/onexit testleri bu bileşimde korunur.

Native/lower-wrapper fixture'ları yalnız kontrollü ASCII harf dönüşümleri üretir; gerçek Windows CP tabloları oldukları ileri sürülmez. Başlangıç kaydı testte sentetik CP1252/lead tablosudur; gerçek image global başlangıcı kurulmuş sayılmaz. Test teardown'u özgün DLL kapanışı değildir. Native API, gerçek dosya arama, hook, özgün/yeniden derlenen DLL veya oyun çalıştırılmaz. Orijinal girdiler değiştirilmez. `validation.json` bağımsız, `common-validation.json` ortak derlemeyi; `review-tables.json` kaynak/test incelemesini kaydeder. ABI, loader, security-cookie/SEH ve eşzamanlı native davranış eşdeğerliği açık kalır.
