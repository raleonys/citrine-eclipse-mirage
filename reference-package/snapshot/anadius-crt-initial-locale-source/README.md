# Başlangıç C-locale veri grafiği

Bu modül, değişmeyen `C:/Users/Skysect/Downloads/anadius64.dll` dosyasının SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` kimliğine bağlı, sınırlı bir başlangıç veri görüntüsüdür. **96 bölge, 116 DIR64 işaretçi kenarı ve 3.546 bayt** içerir. Yeni işlev gövdesi kurtarma sayısı **0**. Kanıtlar yerel PE dosyasından ve saklanan REA çıktısından türetilmiştir; yeni REA kanıt kimliği üretilmemiştir. Özgün DLL yüklenmedi, çalıştırılmadı veya değiştirilmedi.

| Kök | Tercihli VA | Dosya ofseti | Boyut | DIR64 alan |
|---|---|---|---:|---:|
| Locale | `0x1800a17e0` | `0x9fde0` | `0x160` | 11 |
| Lconv | `0x1800a1700` | `0x9fd00` | `0x98` | 18 |
| Time | `0x18008ad20` | `0x89520` | `0x2c0` | 87 |
| Kategori `L"C"` kimliği | `0x1800a1948` | `0x9ff48` | 4 | 0 |

`initial_locale.extraction.json`, her bölgenin kesin baytlarını (`raw_hex`), SHA-256 değerini, normalleştirilmiş baytlarını ve PE eşlemesini içerir. Her kenarda hem işaretçi alanının hem DIR64 relocation girişinin dosya ofseti, relocation blok sayfası ve ham 16-bit relocation sözcüğü bulunur. `verify.cjs`, bütün kenarları geri uygulayarak her bölgenin özgün baytlarıyla tamamen eşit olduğunu doğrular.

## Doğrudan gözlenen ayrıntılar

- Locale `+0x10` sayacı **1**; `+0x08` değeri **1**. `+0x18` u32 **0**, `+0x1c` u32 **1**. Time `+0x158` u32 **1**, `+0x15c` sayacı **0**.
- Locale kategori 0'ın ad alanı `+0x28` **NULL**. Kategori 1–5'in `+0x48/+0x68/+0x88/+0xa8/+0xc8` alanları aynı `L"C"` kimliğine gider. Bu, altı kategoriyi aynı sabit adla başlatan bir fixture ile aynı değildir.
- Locale `+0x158`, locale kökünün kendisine gider ve gerçek DIR64 relocation girdisine sahiptir. Kök boyutu `0x160` bu öz alanı içerir.
- Locale `+0x100/+0x108` sıfırdır. Dinamik ctype serbest bırakma şemasındaki `+0x108 - 0xfe` kuralı başlangıçta sahip olunan bir tahsis göstermez. `+0x00` sınıflandırma tablosu görünümüdür; `+0x110/+0x118` harf dönüştürme görünümüdür.
- Lconv `+0x50` değeri **`0x7f7f7f7f7f7f7f7f`** olup relocation veya işaretçi değildir. `+0x00` ASCII `"."`, `+0x58` UTF-16 `L"."` içerir; kalan işaretçiler boş dizelere gider.
- Boş ASCII `0x1800a4f19` ve UTF-16 `0x1800a4f1c` dizeleri dosyada saklanan baytlar değildir. `.data` bölümünün `VirtualSize=0x4a7c`, `SizeOfRawData=0x2800` alanları nedeniyle sanal sıfır doldurma bölgesindedirler. Bu iki yaprağın 1/2 sıfır baytı PE yükleme eşlemesinden türetilir; JSON bunları açıkça `virtual-zero-fill` diye işaretler.
- Time, 43 ASCII ve 44 UTF-16 işaretçi içerir. Her iki kodlamada kısa ve uzun `May` aynı adresi paylaşır; normalleştirme bu kimliği korur. Son UTF-16 yaprak `L"en-US"` değeridir. Tarih/saat biçimleri `MM/dd/yy`, `dddd, MMMM dd, yyyy`, `HH:mm:ss` olarak gözlenmiştir.

## Tabloların sınırı ve kanıtı

Saklanan `FUN_180062a80` (`0x180062a80`, `ev_50614aa9857141802f47ef16f2118e4167da2b050bd76658ee8a7a4107a6b417`) C-locale dalında `0x18008a220`, `0x18008a4a0`, `0x18008a620` görünümlerini doğrudan kurar. Aynı gövde dinamik dalda `0x180` WORD sınıflandırma ve iki adet `0x180` bayt harf eşleme tahsis eder; görünümleri `+0x100`, `+0x80`, `+0x80` konumlarına yerleştirir. `constructor.selected.pseudo.c` bu korunmuş gövdenin aynısıdır. Çıkarıcı tam saklanan pseudocode dosyasının SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb` kimliğini de doğrular.

Bu şemaya dayanılarak tablo görüntüleri `0x18008a120 +0x300`, `0x18008a420 +0x180`, `0x18008a5a0 +0x180` aralıklarıyla sınırlanmıştır. Bunlar `[-128,255]` indeks alanının **şema tabanlı sınırlarıdır**; linker sembol boyutu veya bütün komşu nesnenin uzunluğu kanıtlanmış değildir. Başka veri keşfi için keyfi işaretçi taraması yapılmaz. Dize uzunlukları en fazla 256 kod birimi içinde ilk NUL'a kadar okunur; NUL dahil kesin uzunluk kaydedilir.

## C++ arayüzü

`initial_locale_data.hpp/.cpp`, C++20 `anadius::crt::initial_locale_data` ad alanını sağlar. `regions()` salt okunur `Region` dizisini, `relocations()` `{source_region, source_offset, target_region, target_offset}` girdilerini verir. Kök enum değerleri `Locale=0`, `Lconv=1`, `Time=2`, `Category=3` şeklindedir. Her bölgenin `bytes` alanında bütün işaretçi yuvaları sıfırdır. Önce her bölgeyi ayrı, ömrü yeterli depolamaya kopyalayıp sonra kenarları bu depolamanın adresleriyle uygulamak gerekir. Tablo kenarlarının hedef ofsetleri `0x100/0x80/0x80`, kendine kenarın hedefi `Locale+0` olur. `preferred_va` yalnız kanıt metaverisidir; canlı işaretçi olarak kullanılmaz.

Bellek tahsisi, relocation uygulaması, sahiplik ve locale/time referans yönetimi bu modülün dışında kalır. Geniş süreç/global ilk kullanıma hazırlama grafiğinin tamamı veya özgün DLL ABI eşitliği iddia edilmez.

## Yeniden üretim ve doğrulama

PowerShell'de çalışma klasöründen:

```powershell
node outputs/anadius-crt-initial-locale-source/extract.cjs
node outputs/anadius-crt-initial-locale-source/verify.cjs
& outputs/anadius-crt-initial-locale-source/build.ps1
```

Çıkarıcı yalnız hedef girdisini ve saklanan pseudocode'u okur; kendi klasöründeki JSON/C++/seçilmiş kanıt dosyalarını üretir. `--summary` yalnız kök özetini verir; isteğe bağlı ilk konumsal argüman aynı hash'e sahip DLL'in yoludur.

`extraction-validation.json` bütün ham/normalleştirilmiş hash'leri, işaretçi hedef sınırlarını, sıfır yuvaları, DIR64 sayfa/sözcük kanıtını ve tam geri dönüşü denetler. `build.ps1`, mevcut LLVM-MinGW ile üretim nesnesi ve bağımsız test EXE'si derler (`C++20 -O2 -Wall -Wextra -Werror`). Test, gerçek adres dereference etmeyen sentetik sayısal adreslerle 116 relocation'ı uygular; öz/kategori kimliklerini, sayaçları, `+0x50` scalarını, `May` ortak adreslerini, tablo görünümlerini ve relocation dışındaki bütün baytların korunmasını kontrol eder. Sonuç `validation.json` dosyasındadır. Test EXE'si yeni C++ verisini kullanır; özgün DLL'i çalıştırmaz.
