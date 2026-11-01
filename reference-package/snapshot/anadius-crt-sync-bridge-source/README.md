# Gerçek kilit/Interlocked/FLS kaynak bağlantısı

Yeni `VcrtLockCalls`, VCRT API arayüzünün yalnız iki kritik-bölüm çağrısını typed native bankalara bağlar; loader/FLS/TLS yöntemleri zorunlu ve soyut kalır. Yeni `PtdSyncEffects`, mevcut kurtarılmış ACRT lock/unlock kaynaklarına ve sahipli `Cell32` atomik hücrelerine bağlanır; global/locale/resource yöntemleri zorunlu ve soyuttur. Yeni çıkarılmış özgün fonksiyon gövdesi **0**; bu aşama mevcut kaynaklar için native sağlayıcı bağlantısıdır.

`AtomicCells::resolve` yalnız bilinen, yaşayan, doğru hizalı `LONG` hücresini döndürmelidir. Rastgele byte/global adresi `LONG*` yapılmaz. `Banks` nesnesindeki gerçek SDK `CRITICAL_SECTION` nesneleri, kurtarılmış LockRef kimliklerine karşılık gelir; SDK nesnesi 40 bayt olsa da yardımcı initialized alanları nedeniyle yeni C++ bankanın eleman stride/layout'u özgün image bankası ABI'si olarak sunulmaz. Orijinal VA'lar çalıştırılan pointer'lar değildir.

## Doğrulama

`build.ps1` C++20/O2/Wall/Wextra/Werror ile kendi EXE'sini derler: **120 kontrol PASS**. Bağımsız inceleme `review-native-sync.json`, ortak doğrulama `common-validation.json` dosyasındadır. CMake tanımı ayrıca çalıştırılmış sayılmaz.

- Gerçek `vcrt_bridge::LockEffects`, ACRT encoded `0669b0` ve VCRT plain `0509bc` resolver kaynaklarını kullanır. Sabit sistem modüllerinden `InitializeCriticalSectionEx` çözülür. Export yokluğu testte enjekte edilerek her iki kurtarılmış fallback'in gerçek `InitializeCriticalSectionAndSpinCount` çağrısı yapması doğrulanır. ACRT14/VCRT1 ve spin4000 kaynak kararları korunur; cache'ler kaynakta kullanılır.
- Modern ve fallback yollarında dörder worker eşzamanlı PTD edinir. Barrier'lar worker'ların PTD'lerini aynı anda canlı tutar: main dahil beş heap bloğu, beş multibyte referansı ve beş locale referansı gözlenir. Her worker farklı PTD/errno kullanır; LastError korunur. Worker'lar bırakılıp join edildiğinde gerçek FLS callback'leri sonrası yalnız main kaydı ve birer referans kalır; main kapanışında hepsi sıfırlanır.
- Referans yordamları gerçek ACRT4/5 kritik bölümlerinden ve sahipli Windows Interlocked hücrelerinden geçer. Heap gözlem haritası testin ayrı mutex'iyle korunur; check sayacı atomiktir. Observation pointer, global/locale kimlikleri, binding ve native-bank initialized bayrakları worker ömürlerinde değişmez. Kapanış, tüm worker ve callback'ler tamamlandıktan sonra FLS→VCRT→ACRT sırasındadır.
- Başlatma hatası testinde ilk dört ACRT kilidi gerçekten kurulur, beşinci native Spin çağrısı testte FALSE döndürür. Gerçek kaynak rollback'i bu dört kilidi bırakır; sayaç ve initialized bayrakları sıfırdır. Ayrı VCRT ilk-call başarısızlığı da temiz kalır.

Fixture loader yalnız sabit Windows sistem modüllerini yükler ve sahip olduğu referansları bırakır. Üretimde genel loader/global yaşam döngüsü değildir. Locale kimliği ve basit referans yöntemleri kontrollü fixture'lardır; tam locale ağacı, global değişiklikler, eşzamanlı shutdown, arbitrary lock ownership misuse ve genel DLL concurrency eşdeğerliği gösterilmez. Bu test, önceki yalnız sıralı worker testine ek **dört eşzamanlı worker** kanıtıdır; her program için concurrency doğruluğu anlamına gelmez. MSVC ABI, SEH/unwind ve orijinalle runtime eşdeğerliği açıktır.

Orijinal veya yeniden derlenen DLL/oyun yüklenmez, çalıştırılmaz veya değiştirilmez. Worker'lar yalnız bu test programına aittir.

## Adresli dayanaklar

`0x1800625b0/062614/0625f8/06264c` ACRT lifecycle/lock; `0x18005093c/050984` VCRT lifecycle; `0x18006707c` ACRT API seçimi; `0x180050c38` VCRT API seçimi; resolver `0x1800669b0/0509bc`; PTD-data `0x1800647d4/06489c/0648fc/0649ec/0649cc`. Hash bağlı statik referanslar `evidence.json` içindedir.

SDK sözleşmeleri: [InitializeCriticalSectionEx](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionex) ve [InterlockedExchangeAdd](https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedexchangeadd). Canlı typed nesne, hizalama ve quiescent delete sınırları bu yeni adapter'larda açık tutulur.
