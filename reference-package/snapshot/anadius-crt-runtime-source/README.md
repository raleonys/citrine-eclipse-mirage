# CRT runtime ve dinamik yordam çözümleme çekirdeği

`runtime.cpp`, `anadius64.dll` SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için saklanan REA/Ghidra assembly kanıtından yeniden yazıldı. Bir üretim C++ nesnesi ve bağımsız sentetik test grubu derlenir. Özgün C++ kaynak, tüm CRT veya tam DLL değildir.

| Özgün VA | Kaynak yordamı | Doğrulanan davranış |
|---|---|---|
| `0x1800669b0` | `resolve_0669b0` | Cookie ile kodlanmış yordam cache'i, modül cache'i, `LoadLibraryExW` bayrakları, aday taraması, negatif cache ve yarışta yeni handle'ı bırakma |
| `0x180066bc4` | `query_thread_policy_066bc4` | Slot 26, `AppPolicyGetThreadInitializationType`, token -6, bulunamazsa düşük 32 bit `0xc0000225` |
| `0x18006c0b8` | `process_parameters_flag_06c0b8` | PEB→ProcessParameters→Flags(+8) alanının bit 31'i; alanın daha ileri anlamsal adı varsayılmaz |
| `0x18006c0d0` | `runtime_policy_06c0d0` | Cache 0 iken flags sorgusu, bit31 sıfırsa AppPolicy; sonuç değeri 1 ise 2, diğer durumda 1; status yok sayılır; XCHG ile yayınlama |
| `0x180067320` | `initialize_runtime_067320` | Slot 23, `RoInitialize` için aynen mode/HRESULT; yordam bulunamazsa EAX zaten 0 olduğundan dönüş 0 |
| `0x18006736c` | `uninitialize_runtime_06736c` | Slot 24, `RoUninitialize` varsa çağırma, yoksa geri dönme |

`RoInitialize` için decompiler `void` gösterir; `0x180067342–0x180067369` assembly akışı ve CRT thread çağırıcısının EAX testi dönüş sözleşmesini belirler. Yordamın bulunamaması thread çekirdeğinde runtime bayrağının kurulmasına yol açabilir; bu davranış başarı tahminiyle değiştirilmez. Gerçek yordamın `S_FALSE` (1) dönüşünde önceki thread çekirdeği bayrağı kurmaz; yalnız 0 dönüşünde kurar.

`0x18008ba34` ve `0x18008ba48` adaylarının her biri modül 9'u (`api-ms-win-core-winrt-l1-1-0`, isim VA `0x18008b680`); `0x18008ba90` ise modül 19'u (`api-ms-win-appmodel-runtime-l1-1-2`, isim VA `0x18008b890`) içerir. İsimler, SHA-256 eşleşmesi kontrol edilmiş DLL'nin `.rdata` bölümünden sınırlı yerel okumayla doğrulandı; bu okuma yeni REA Evidence kaydı değildir.

Resolver ayrıntıları korunur: `0x800` ile yükleme başarısız ve hata 87 olduğunda, yalnız ilk yedi UTF-16 karakteri `api-ms-` veya `ext-ms-` olmayan isimde bayrak 0 ile tekrar denenir. `0x1800646a0` assembly'sindeki `wcsncmp` doğrudan UTF-16 değerlerini karşılaştırır; büyük/küçük harf dönüşümü yapmaz. Bulunan modülde export yoksa sonraki aday denenmez. Yükleme cache'ine XCHG öncesinde başka bir değer yazılmışsa yeni yerel handle FreeLibrary'ye verilir ve GetProcAddress aynı yerel handle ile devam eder. Bu gözlenen yarış davranışı düzeltilmiş bir algoritmaya dönüştürülmez. Gerçek yarış veya Win32 çağrısı çalıştırılmamıştır; test aynı ara durumları bağımlılık callback'leriyle kurar.

`State` yeni C++ durum nesnesidir. Constructor açıkça verilen, önceden seçilmiş cookie için 31 yordam hücresini kodlanmış NULL olarak başlatır. Bu yeni kaynak bileşimi `0x180067538` gövdesiyle doğrulanır; cookie başlatmasını kendiliğinden gerçekleştirmez. Modül tablosunun 21 adı SHA-256 eşleşmesi kontrol edilmiş sınırlı `.rdata` okumasıyla tamamlandı; ayrıntıları `../anadius-crt-cache-source/module-names.json` içerir. Resolver aralık içindeki indeksleri ve immutable, gömülü NUL içermeyen adları destekler. Atomikler özgün MOV/XCHG hücrelerini C++ veri yarışı üretmeden temsil eder; özgün bellek yerleşimi veya makine kodu eşitliği kurulmaz.

`Effects` gerçek loader, GetProcAddress, PEB okuması ve yordam çağrısını zorunlu soyut bağımlılık olarak bırakır. Başarılı native provider taklidi yoktur. `ThreadRuntimeBridge`, daha önce kurtarılan CRT thread çekirdeğinin üç runtime bağımlılığını bu gövdelere bağlayan yeni kaynak bileşimidir; diğer heap/PTD/thread/exit işlemleri soyuttur. Özgün sayısal binary adresleri çalıştırılacak function pointer olarak kullanılmaz.

Derleme:

```powershell
& '.\outputs\anadius-crt-runtime-source\build.ps1'
```

3.688 sentetik kontrol PASS: 64 cookie rotasyonu ve sınır bitleri, resolver pozitif/negatif cache, API-set ve normal isim fallback'i, case-sensitive prefix, eksik exportta aday taramasının durması, araya giren cache yazımı, policy status/sonuç/flag kombinasyonları, iç içe policy hesapları ve kurtarılmış CRT thread giriş–runtime–çıkış bileşimi. Testte loader ve prosedür çağrıları kontrollü C++ arayüzleridir; `threads.cpp` ve `bootstrap.cpp` test yürütülebilirine kaynak olarak bağlanır. Girdi DLL/oyun, gerçek modül yükleme, PEB erişimi, OS worker thread veya provider çalıştırılmaz.

`evidence.json` altı gövdenin özgün REA kimliklerini, sınırlı `.rdata` okumasını ve saklanan kanıt dosyalarının hash'lerini içerir. `assembly-evidence.txt` bu altı gövdenin saklanan assembly kesitidir. Prefix yardımcısının kanıt kimlikleri ayrıca aynı JSON'da bulunur; yardımcının genel gövdesi bu modüle yeniden yazılmamıştır.

Açık sınırlar: tam CRT cache/kilit/başlatma–çıkış yaşam süresi, gerçek loader/PEB adapter'i, CFG/XFG çağrı doğrulaması, EH/SEH ve özgün compiler/ABI. AppPolicy wrapper'ının burada kullanılan dönüşü düşük 32 bit ve caller'ın status'u yok saydığı yol ile sınırlıdır. Test sayısı tam DLL veya çalışma zamanı eşdeğerliği kanıtı değildir.
