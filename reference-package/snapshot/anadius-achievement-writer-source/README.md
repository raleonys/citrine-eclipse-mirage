# Achievement Watcher INI yazıcısı

`write_ini_01f9a0`, özgün01f9a0 normal akışının okunabilir C++ kaynağıdır. Girdi INI yolu boşsa hiçbir etki çağırmaz. Doluysa mevcut gerçek path kaynağıyla parent yolu bulur, zorunlu `Effects::create_directories_018e00` sınırını çağırır, sonra mevcut gerçek `write_binary_03c1d0` kaynağına byte dizisini verir. Dosya yazıcısının BOOL sonucu özgün çağıran gibi göz ardı edilir; stream/dizin exception'ları yayılır. Boş achievement map'i dolu yolda boş dosya yazdırır.

Kayıtlar `configuration::State::achievements` map anahtarı sırasıyla gezilir. INI bölüm adı map anahtarı değil, achievement `steam_id` string'idir. Her kaydın byte biçimi:

```text
[steam_id]\r\n
Achieved=field04\r\n
CurProgress=field08\r\n
MaxProgress=field00\r\n
UnlockTime=field70\r\n\r\n
```

CurProgress ve MaxProgress yalnız **signed field00 > 1** ise yazılır. Achieved boolean'a dönüştürülmez; field04 ve field08 signed32 ondalık, field70 unsigned64 ondalıktır. Bölüm adındaki NUL, köşeli parantez, CR/LF veya diğer byte'lar kaçırılmaz/kırpılmaz; string uzunluğu açıkça korunur. Aynı steam_id birden fazla kayıtta varsa bölümler yinelenir. XML üretimi bu yordamda yoktur.

`serialize_ini` test/bağlantı için ayrılmış mantıksal yardımcıdır; ayrı özgün yordam olarak sayılmaz. Decimal çıktı `std::to_chars` ile locale'den bağımsız üretilir. Özgün allocator/string kapasitesi, tahsis hatasındaki kısmi mutation, MSVC layout/exception/SEH veya bütün DLL eşitliği iddia edilmez.01e6e0 ve01e820 string yardımcılarının normal byte sonucu kullanılır; native allocation stratejileri yeniden oluşturulmaz.

`build.ps1` C++20/LLVM-MinGW ile üretim nesnesi ve kendi test programını derler. Testler kesin golden byte dizileri, signed/unsigned sınırlar, total koşulu, NUL ve delimiter koruma, map sırası, ignored initial-state failure ve exception sırasını kapsar. NativeFactory yalnız kendi oluşturulan `build/generated-fixtures/achievement-writer-generated.ini` dosyasında kullanılır. Directory effect kontrollüdür; gerçek kullanıcı CFG/token/achievement dosyaları okunmaz, özgün DLL/oyun çalıştırılmaz veya değiştirilmez. CMake tanımı verilir; doğrulanan derleme PowerShell yoludur.

Statik adresler, retained REA kimlikleri ve hash'ler `evidence.json` ve seçilmiş tam `evidence.asm.txt` / `evidence.pseudo.c.txt` pasajlarındadır. Derleme sonucu `validation.json` içindedir. CRT üretim kaynakları değiştirilmez.
