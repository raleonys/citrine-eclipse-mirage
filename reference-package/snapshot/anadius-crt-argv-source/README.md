# CRT dar argv yapılandırması

`argv.cpp`, hash'i `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan `anadius64.dll` içindeki iki gövdenin bağımsız C++20 yeniden yazımıdır:

| Özgün adres | Yeni kaynak | Davranış |
| --- | --- | --- |
| `0x180063888` | `allocate_buffer_063888` | İşaretçi/dizi boyutlarının sıkı taşma sınırları, `calloc(toplam,1)`, ardından `free(NULL)` |
| `0x1800638e8` | `configure_0638e8` | Mod seçimi, modül yolu/komut satırı seçimi, gerçek ayrıştırıcıyla sayım ve yazım, argv sahipliğinin yayımlanması |

`0x18006ecd4` yalnızca `0x18006e5bc` adresine atlayan bir thunk'tır. Bu modülde ek bir algoritma olarak sayılmaz. Mod 2, gerçek wildcard genişletmesi sağlayan zorunlu `expand_06ecd4` etkisini çağırır. Wildcard çekirdeği sonraki `anadius-crt-wildcard-source` içinde gerçek search/append/qsort kaynaklarına bağlandı; bu modülün arayüzü ona `ExpansionPolicy` üzerinden bağlanabilir. Native find/conversion/locale ve recalloc sağlayıcıları açık kalır.

Kaynak, komşu `anadius-crt-argv-parser-source/parse.cpp` gövdesini iki kez kullanır: önce gereken işaretçi/karakter sayısını bulur, sonra tek bir tahsis içindeki `char*[]` ve karakter dizisini doldurur. Yerleştirmeli dizi kurucuları bu tahsiste gerçek C++ dizi ömürlerini başlatır; ek tahsis yapmaz. Mod 1'de tahsisin sahipliği yayımlanan argv'ye geçer. Mod 2'de genişletme sonucu ilk ayrıştırma tahsisinden bağımsız yaşamalıdır; başarıda yalnızca ilk tahsis serbest bırakılır, başarısızlıkta kısmi genişletme sonucu önce bırakılır.

`State`, özgün `A5180` modül yolu tamponunu, `A59E0` program adını, `A59E8` argc DWORD'unu, `A59F0` argv işaretçisini ve `A5A00` komut satırı işaretçisini yeni C++ depolamada temsil eder. Tercih edilen sanal adresler hiçbir zaman çalıştırılmaz veya özgün dosyaya bağlanmaz. Sıfır varsayılanları yeni kaynak seçimidir; özgün PE başlangıç baytlarına ilişkin bir iddia değildir.

Assembly, decompiler'ın iki yanıltıcı noktasını düzeltiyor. `0x18005d724`, PTD `+0x20` errno hücresine erişir; Ghidra'nın `__doserrno` etiketi kullanılmaz. Mod 1, `0x1800639dc` ve `0x1800639e3` adreslerinde önce argv'yi, sonra argc'yi yazar. Mod 2, tüm sonlandırılmış genişletme işaretçilerini saydıktan sonra `0x180063a37/0x180063a43` adreslerinde argc ve argv'yi yayımlar. Modül yolu işlevinin dönüş değeri yok sayılır; boş/NULL komut satırı bu tampona düşer.

Güvenli kaynak arayüzü, tahsis sağlayıcısından hizalı ve yeterli boyda canlı depolama; genişletme sağlayıcısından sınırlı ve NULL ile sonlanan canlı işaretçi dizisi ister. Hatalı sağlayıcı, sınırsız/sonlandırılmamış giriş veya özgün `DIV 0` yolu açıkça desteklenmeyen alan olarak hata verir. Bunlar özgün DLL'in hata dönüşleri değildir. Sağlayıcı istisnalarında özgün assembly'de kanıtlanmayan RAII/SEH temizliği eklenmez. Tekrar yapılandırma önce yayımlanmış argv'yi kendiliğinden bırakmaz; bu davranış da korunur.

`build.ps1`, Clang C++20 ile `-O2 -Wall -Wextra -Werror` kullanır ve gerçek parser bağımlılığıyla fixture EXE'sini derler. **6.777 kontrol geçti:** mod 0/yanlış mod, errno ve `free(NULL)` sıralaması, tahsis başarısızlığı, tırnak/ters çizgi/DBCS ayrıştırması, modül yolu fallback'i, tek tahsis düzeni, zorunlu wildcard çağrısı, kısmi başarısızlık temizliği ve katı 64 bit boyut sınırları. Bütün yollar sentetik sağlayıcılar ve fixture'a ait bellek ile sınanır. Özgün DLL, yeniden üretilmiş DLL veya oyun yürütülmez; gerçek modül yolu, locale, TLS/FLS veya süreç API'si çağrılmaz.

`evidence.json` adresleri ve kanıt kimliklerini; `selected-assembly.txt` ile `selected-pseudo.c.txt` sınırlı gövdeleri; `validation.json` derleyici, kaynak ve fixture hash'lerini içerir. `collect-evidence.ps1`, özgün dosyayı ve tutulmuş iki kanıt dosyasını SHA-256 ile yeniden doğrular, yalnızca bu klasörde kanıt çıktısı yazar. Tam DLL veya birebir ABI eşdeğerliği tamamlanmış değildir.


Sonraki heap aşamasında `../anadius-crt-recalloc-bridge-source` gerçek malloc/realloc/msize/recalloc gövdelerini wildcard ve onexit akışlarına aynı heap state üzerinden bağladı;5.483 ortak kontrol geçti. Bu klasörün eski bağımsız testi/validation kaydı kendi sağlayıcısını kullanmayı sürdürür. Yeni bileşimde recalloc artık kopyala/free compatibility fixture değildir; alt Heap API sağlayıcıları sentetiktir ve gerçek Windows/SEH/ABI eşdeğerliği açık kalır.