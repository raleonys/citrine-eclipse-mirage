# CRT cookie ve WinAPI cache yaşam süresi

`cache.cpp`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan `anadius64.dll` için saklanan REA/Ghidra assembly kanıtından yeniden yazılmış dokuz gövde içerir. Bir üretim C++ nesnesi ve bir sentetik test grubu derlenir. Özgün C++ kaynak veya tam DLL değildir.

| Özgün VA | Yeniden yazılan davranış |
|---|---|
| `0x18004c134` | Varsayılan cookie ise FILETIME, thread/process ID, performans sayacı ve stack hücresi adresini XOR eder; 48 bite maskeler; varsayılan değerle çakışırsa bir artırır; tüm normal yollarda complement yazar |
| `0x180067538` | `A53A0` konumundan itibaren **31** yordam hücresine ham cookie yazar; dönüşte yalnız AL'yi 1 yapar, üst cookie bitleri RAX'te kalır |
| `0x18006755c` | CL sıfırsa `A52F0..A5398` aralığındaki **21** modül hücresini sırasıyla işler; 0 ve -1 için FreeLibrary çağırmaz; tüm sıfır olmayan hücreleri çağrıdan sonra sıfırlar; CL sıfır değilse hiçbir hücreye dokunmaz |
| `0x1800644b0` | Cookie'yi bir kez okuyup beş yardımcıya sırayla aktarır; AL=1 |
| `0x18005c7e4` | `A4F00` hücresini cookie ile başlatır |
| `0x1800634a8` | `A5178` hücresini cookie ile başlatır |
| `0x18005ea40` | `A4F20/A4F28/A4F30/A4F38` hücrelerini cookie ile başlatır |
| `0x1800705f0` | `A5A18` hücresini cookie ile başlatır |
| `0x18005cd94` | `A4F10` hücresini cookie ile başlatır |

Cookie ile NULL kodlanınca ham cookie elde edilir. Bu nedenle yordam ve global başlangıçları sıfır yazmaya dönüştürülmez. Global hücrelerin daha yüksek seviyedeki anlamı bu modülde adlarından tahmin edilmez.

`EntropyEffects` gerçek Win32 entropy işlemlerini zorunlu bağımlılık olarak bırakır. Stack adresi özgün `LEA RBP-10` kaynağını temsil eden opak bir provider değeridir; testin veya tercih edilen binary adresinin eşit olduğu varsayılmaz. FILETIME hücresi çağrıdan önce sıfırdır. QueryPerformanceCounter BOOL sonucu assembly'de yok sayılır: başarısız çağrı bir değer yazmışsa aynı aritmetik devam eder. Hiç değer yazılmadıysa özgün yordam tanımlanmamış stack verisi okur; yeniden yazım bu alanı `UninitializedCounterDomain` ile açıkça sınırlar. Bu exception özgün uygulamanın exception davranışı olarak sunulmaz.

`anadius-crt-runtime-source/State` 21 modül ve 31 yordam hücresine genişletildi. Tüm 21 modül adı, hedef SHA-256 doğrulandıktan sonra `.rdata` içindeki `0x18008b3a0` pointer tablosundan sınırlı okunmuştur. Son ad slot 20'de `user32`'dir. `module-names.json` pointer/ad adreslerini, raw offsetlerini ve NUL dahil UTF-16 byte hash'lerini tutar. Bu yerel okuma yeni REA Evidence değildir; kod gövdelerinin özgün kanıt kimlikleri `evidence.json` içindedir.

State constructor'ının verdiği cookie ile kodlanmış NULL cache kurması yeni kaynak bileşimidir; `067538` ile aynı 31 hücrelik gövdeye dayanır. Cookie'nin önceden seçilmesi gerekir. `initialize_thunks_067538` ayrıca çağrılabilir kurtarılmış yordamdır; constructor otomatik olarak cookie entropy veya CRT startup tablosu çalıştırmaz. Initialization/uninitialization resolver ile dışarıdan senkronize edilmelidir; özgün kilit mekanizması uydurulmaz. Shutdown FreeLibrary'nin dönüş bool'unu yok sayar; gerçek API çağrısı bu modülde uygulanmaz.

```powershell
& '.\outputs\anadius-crt-cache-source\build.ps1'
& '.\outputs\anadius-crt-runtime-source\build.ps1'
```

**5.555 kontrol PASS:** entropy sırası, ID sıfır genişletme, 48 bit maske, çakışma, önceden seçilmiş cookie, başarısız fakat değer yazan QPC, yazılmamış QPC alanı, 64 rotasyon, 31 hücrenin tamamı, 21 modül slotu ve FreeLibrary öncesi/sonrası gözlenebilir durum, 0/-1, nonzero termination flag, dönüş status'unun yok sayılması, son slot 20/30 ile gerçek resolver kaynak bileşimi ve sekiz global hücre. Mevcut runtime grubu genişleme sonrası **3.688 kontrol PASS**.

Testler yalnız bizim C++ sentetik programımızı ve zorunlu dependency taklitlerini çalıştırır. Özgün DLL/EXE, gerçek modül yükleme/FreeLibrary, thread/process API'leri veya provider yürütülmez. CRT genel başlangıç/çıkış bileşimi, native entropy adaptörü, global hücrelerin diğer modüllerle aynı depoya bağlanması, loader/ABI/EH/SEH/CFG/XFG eşdeğerliği tamamlanmış değildir. Test sayısı tam DLL davranış eşitliği kanıtı değildir.
