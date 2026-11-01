# anadius64.dll başlangıç girdisi ve konsol katmanı

`startup.cpp`, özgün DLL'deki altı işlevin gözlenen kararlarını ve dış çağrı sırasını C++20 olarak yeniden kurar. Kaynak, özgün kaynak kod değildir ve tek başına anadius64.dll üretmez. `windows_platform.cpp` gerçek Win32 girdisi/konsol API'leri için derlenmiş adapter içerir; bu incelemede adapter çalıştırılmadı. Test executable'ına bu dosya linklenmez.

Özgün DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Kaynak karşılaştırması üst dizindeki `anadius-all-functions.asm.txt` ve `anadius-all-functions.pseudo.c` saklanan kanıtları kullanır. Her adresin Evidence kimliği ve manuel denetim sonucu `evidence.json` içindedir.

| Özgün VA | Kaynak işlev | Korunan davranış |
|---|---|---|
| `0x180018480` | `filter_arguments_018480` | Yalnız tanınan seçenekler dönen metne eklenir; dil seçeneği sonraki token'ı tüketir. |
| `0x180019910` | `setup_environment_019910` | Dolu CRT environment değeri literal boşlukla bölünür; boş/yoksa argv okunur; süzülen metin Win32 environment'a yazılır. |
| `0x180038760` | `setup_console_038760` | noConsole/AttachConsole/showConsole dalları ve çift stream çağrıları korunur. |
| `0x180039720` | `read_arguments_039720` | CommandLineToArgvW sonucunda executable adı atlanır; kalan değerler dönüştürülür; normal dönüşte LocalFree çağrılır. |
| `0x180039950` | `get_environment_039950` | CRT getenv null dönerse boş string, aksi halde NUL'a kadar değer kopyası döner. |
| `0x1800453d0` | `narrow_argument_0453d0` | Boş girdi kısa devresi, codepage65001/flags0 ile iki dönüşüm ve ikinci sonucun yok sayılması korunur. |

`-emuGameLang` ancak sonraki token varsa tanınır. Bu durumda iki bitişik bayrak da 1 yapılır (`0x180018540`: word `0x101`). `INSTALLED_LANGUAGE` girdisi oluşturulur; sonraki token boşsa mevcut değer korunur, boş değilse değiştirilir. Token'ın başka bir seçenek gibi görünmesi tüketilmesini değiştirmez. Dil seçeneği ve değeri dönen metne katılır; boş değer sonuna boşluk bırakır. `-showLSX`, `-showConsole`, `-noConsole` tam ve büyük/küçük harfe duyarlı karşılaştırılır. Diğer girdiler aktarılmaz. Kaynak, bayrakların başlangıç değerlerini sıfırlamaz; entegrasyon `StateReferences` ile gerçek paylaşılan state'i bağlamalıdır.

`OriginEmuCmdArgs` boş değilse `0x18003cf00` delimiter `" "`, limit0 ile çağrılır. Rekonstrüksiyon ardışık/başlangıç/son boşluklardan boş token üretir; tab ayırmaz ve tırnak yorumlamaz. Ortam değişkeni boş veya yoksa GetCommandLineW/CommandLineToArgvW yolu kullanılır. İlk argv executable kabul edilip atlanır. SetEnvironmentVariableA sonucu kullanılmaz. Özgün DLL kendi statik CRT getenv cache'ini okur; adapter'ın `std::getenv` kullanan başka CRT'si aynı cache değildir. Process environment ve CRT cache'in tüm yaşam döngüsü eşdeğerliği kanıtlanmadı.

`0x1800453d0` uzunluğu 32 bite indirip UTF-8 sizing çağrısı yapar. Başarı/hata için GetLastError kontrolü eklenmez. İlk sonuç kadar sıfırla doldurulmuş çıktı ayrılır; ikinci dönüşüm sıfır dönse de ilk uzunluk korunur. Bu yardımcının flags0 davranışı, kitaplık yükleme modülündeki flags0x80 ve hata kontrolü yapan `0x180048b30` yardımcısından farklıdır.

Konsol akışında noConsole sıfır değilse API çağrısı yapılmaz. AttachConsole(parent) başarılıysa yalnız stderr yeniden açılır. Başarısızsa showConsole sıfırken dönülür; showConsole etkinse AllocConsole sonucundan bağımsız GetConsoleWindow, MoveWindow(100,100,800,500,true), stdin/stdout/stderr yolları çalışır. Her stream index'i **iki defa** sorulur: ilk pointer stream argümanı, ikinci pointer output-pointer argümanı olarak `0x18005f024` sınırına verilir. Bu iki çağrı birleştirilmedi.

`0x180052238` özgün CRT FILE kaydını `0x1800a15f0 + index*0x58` ile bulur. `0x18005f024`, argümanları aktararak `0x18005ef2c` çağrısına beşinci parametre0x80 ekler. İkinci stream pointer'ının output pointer olarak kullanılması assembly'de gözlenir; pointer rolü düzeltilmedi veya MinGW `freopen_s` ile değiştirilmedi. `ConsolePlatform` bunu zorunlu, opak `uintptr_t` sözleşmesi olarak tutar. `NativeConsolePlatform` bu iki CRT metodu tamamlanmadan abstract kalır.

`./build.ps1` mevcut LLVM-MinGW kökünü kullanır; kurulum/indirme yapmaz. Çekirdek ve Windows adapter Clang23.1.3 ile C++20, -O2, -Wall, -Wextra, -Werror seçenekleriyle ayrı nesnelere derlenir. Yalnız sentetik input/konsol kullanan test executable'ı çalıştırılır. **155 kontrol geçti.** Gerçek environment değişikliği, UI, konsol açılması, orijinal DLL/EXE veya yeniden derlenmiş DLL çalıştırılması yapılmadı.

Manuel denetimde altı işlevin yukarıdaki kararları için maddi fark bulunmadı. Testler diferansiyel özgün kod testleri değildir. MSVC string/vector/map yerleşimi, SSO, heap büyümesi, CRT cache ömrü, tam EH/SEH cleanup tabloları, argüman kopyalama/geçici nesne allocator ayrıntıları ve gerçek global ABI bağları tamamlanmadı. Derlenebilir modül, tüm DLL'nin davranış eşdeğerliği olarak sunulmaz.
