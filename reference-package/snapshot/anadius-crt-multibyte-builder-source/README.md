# Multibyte kayıt üreticisi: 0x18006f674

`multibyte_builder.cpp`, `anadius64.dll` içindeki **tek fonksiyonun normal akışını** okunabilir C++20 olarak yeniden kurar. Orijinal kaynak kod değildir. `build.ps1` yalnızca bu kaynağı ve sentetik sağlayıcılı test uygulamamızı derler/çalıştırır; orijinal DLL yüklenmez veya çalıştırılmaz.

## Kaynak ve adres kanıtı

- Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
- Fonksiyon: `0x18006f674`; retained Ghidra pseudocode `ev_82ac27f350ea82c6784171d9bc987418729f2f2712ec6c95b83fdb20a2058881`, assembly `ev_d4c04fc86da4968083b43e855ee7c4f116d7b038613a196501c9c0b65b7e59c5`. Tam sınırlı gövde `evidence.json` içindedir.
- `0x18006f69c`: gerçek `getSystemCP` çağrısı zorunlu etkidir. Sonuç sıfırsa `0x18006f904` üzerinden SBCS sıfırlama `06f050` çağrılır.
- `0x18006f6bd–6d0`: beş sabit DBCS kaydı aranır. `0x1800a20b0` bayrakları `01 02 04 08`; `0x1800a20c0` tablosu 5 × 48 bayt içerir. PE `.data` RVA `0xa1000`, ham dosya başlangıcı `0x9f600`; bayrak dosya ofseti `0xa06b0`, tablo `0xa06c0–0xa07af`. `retain-evidence.ps1` PE başlıklarından eşlemeyi yeniden hesaplar ve tam 240 baytın hex/SHA-256/satırlarını kaydeder.
- Yerleşik CP'ler 932, 936, 949, 950 ve 1361'dir. Altı WORD `record+0xc` konumuna kopyalanır. Dört bayrak grubu aralıkları ayrı ayrı OR edilir; byte `c` tablodaki `record+0x19+c` yerindedir; `+0x18` EOF hücresidir.
- `0x18006f6de`: `IsValidCodePage` yalnızca CP'nin alt 16 bitini alır; sonraki `GetCPInfo` tüm 32 biti alır. `65000` doğrudan başarısızdır. BOOL sıfır dışındaki tüm bit desenleri başarıdır.
- `0x18006f6f8–716`: UTF-8 için `+4` konumuna QWORD `65001`, `+0x220` konumuna NULL, `+0x18` DWORD ve `+0x1c` WORD sıfır yazılır. Ardından `+0xc` konumundan altı WORD sıfırlanır. Kalan karakter tablosu baytları **korunur**; bütün tabloyu sıfırlamak kaynakla eşdeğer olmaz.
- `0x18006f725–7ff`: dinamik CPInfo başarısızlığı yalnızca system-CP bayrağı sıfır dışındaysa SBCS'ye döner. Başarılı CPInfo `MaxCharSize == 2` olduğunda lead aralıklarına `4`, karakter `1–254` için trail `8` ekler. `0x18006f7a4` locale seçmeden önce kayıt CP'sini yeniden okur.
- Locale kimlikleri canlı getter'lardır: `0x18008bf70` ja-JP, `78` zh-CN, `80` ko-KR, `88` zh-TW. Yerleşik 1361 için NULL kalır.
- `0x18006f8fa`: başarıdaki son çağrı `06f0e8` case-tablosu üreticisidir. Fonksiyon tüm normal başarı yollarında `0`, hatada `0xffffffff` döndürür.

## API ve kapsam sınırı

`Effects` bellek erişimi, gerçek `getSystemCP`, system-CP bayrağı, `IsValidCodePage`, `GetCPInfo`, SBCS/case üreticileri ve locale kimliklerini zorunlu sağlar. `CPInfo`, komşu multibyte-tables modülü ile ortaktır. Bağımsız fixture bu bağımlılıkları sentetik olarak verir; ortak köprü gerçek kurtarılmış yardımcı kaynakları bağlayabilir.

Beş yerleşik satır özgün dosyanın sabit başlangıç değerleriyle `constexpr` tutulur. Özgün `.data` satırlarının çalışma sırasında değiştirilmesi bu modül sözleşmesi dışındadır. Başarılı CPInfo verisi, 12 bayt içinde sıfırla sonlanan lead çiftleri sağlamalıdır. Sonlandırıcı yoksa C++ gövdesi açık `std::out_of_range` kapsam sınırı verir; özgün fonksiyonun yığın dışı okumaları için davranış icat edilmez. Çağıranın `0x228` bayt kaydı geçerli olmalıdır. Native API gerçek davranışı, güvenlik cookie epilogu, SEH ve eşzamanlı erişim bu birim testlerinden kanıtlanmaz.

## Doğrulama

`build.ps1`: LLVM-MinGW clang++23, C++20, `-O2 -Wall -Wextra -Werror`, statik test uygulaması. Son kayıt `validation.json` içindedir. **885.150 kontrol**: beş yerleşik CP'nin 257 tip hücresi ve altı WORD'ü; 768 lead uç kombinasyonu, 0/255 ve ters/çoklu aralıklar; dinamik CP'nin tam 32 bit taşınması ve BOOL bit desenleri; sistem fallback'i; UTF-8 kısmi yazma/koruma sırası; locale seçiminde bellekten CP yeniden yükleme; sonlandırılmamış CPInfo için açık sınır. Kontrol sayısının büyük kısmı her sentetik bellek erişiminin sınır denetimidir, bağımsız semantik test sayısı değildir.
