# CRT heap, PTD/FLS ve thread kaynak bileşimi

`crt_glue.cpp` yeni bir entegrasyon katmanıdır. Özgün DLL'den ayrı bir fonksiyon daha kurtarıldığı anlamına gelmez; önceki kaynak gövdelerini birbirine bağlar. Bir C++ üretim nesnesi derlenir.

- `PtdEffects`: altı SDK FLS/LastError çağrı noktası; gerçek kurtarılmış calloc/free ve PTD construct/destroy gövdeleri. Caller yeniden derlenen bir `PFLS_CALLBACK_FUNCTION` sağlar.
- `ErrorPtd`: gerçek optional PTD edinme gövdesini hata hücrelerine bağlar.
- `HeapErrors`: gerçek OS hata tablosu ve errno hücresini heap hata yollarına bağlar.
- `InvalidPtd`: invalid-parameter scoped context'ini state-index0 raw PTD edinmesine bağlar. İlk saved-error pointer'ına yazmaz; bu yardımcının özgün bounded gövdesi argümanı kullanmaz. Başka indeks bağlanmış değildir ve açıkça reddedilir.
- `ThreadEffects`: Context calloc/free, gerçek invalid-parameter gövdesi, PTD+0x3c0 tipli slot ve yeniden derlenen unsigned-stdcall uygulama çağrısı. Mevcut Win32/runtime/error katmanından kalan bütün thread Effects metotları tamamlanır.

Heap'ten dönen sıfırlanmış, doğru hizalı yeterli storage üzerinde `std::construct_at`, yeni C++ Ptd/Context nesne yaşam süresini başlatır. Böylece byte buffer'dan `Context*&` uydurulmaz; PTD'nin son alanı gerçek `Context*` nesnesidir. İki kayıt trivial-destructible'dır; başarılı serbest bırakma storage ömrünü bitirir. Başarısız heap free için erken C++ destructor çalıştırılıp nesne ömrü bitirilmez. Bu yeni yaşam süresi köprüsü özgün C++ sınıf tanımlarının kurtarıldığı iddiası değildir.

`native_fls_api()` gerçek SDK import adreslerini bağlar. Test yalnız adres kimliklerini kontrol eder; bütün çağrılar tipli fixture'lardır. Heap, runtime loader/PEB/provider, locale/refcount/atomik/lock, invalid-handler/Watson ve abort etkileri zorunlu kalır. Gerçek FLS ve CRT wrapper girişlerinin callback bağını caller sağlar; bunlar özgün image adresi olmamalıdır. State/Effects/Identities ve callback bağı bütün pending/suspended/çalışan worker'lar, FLS destructor'ları ve shutdown işlemi bitene kadar yaşamalıdır. FLS slotunu kullanırken serbest bırakma, DLL unload ve eşzamanlı shutdown burada otomatik çözülmez.

```powershell
& '.\outputs\anadius-crt-glue-source\build.ps1'
```

202 birleşik kontrol: gerçek kurtarılmış heap→Context→CreateThread wrapper→PTD→runtime→uygulama→çıkış→FLS destructor→Context/PTD free akışı; CreateThread dönmeden context'in temizlenmesi; tekrar worker ve cache kullanımı; başlatma hatasında OS hata eşleme; returning invalid-handler; heap-free hatası; heap tahsisi sırasında errno/PTD özyinelemesini -1 FLS sentinel'ın kesmesi. SDK thread/FLS/module çağrıları ve locale/runtime provider'ları fixture'dır; raw storage yalnız testin kendi operator-new heap'inden gelir. FLS çıkışı fixture tarafından açıkça simüle edilir; test işletim sisteminin FLS davranışının doğrulaması değildir.

Tam bağımsız DLL, native concurrency, CRT başlangıç/global cache, SEH/unwind, CFG/XFG ve loader/export ABI eşitliği hâlâ açık. Bu kaynak modüllerinin birlikte çalışması özgün DLL'lerin tam çalışma zamanı eşdeğerliği anlamına gelmez. `evidence.json` bağlı gövdelerin kanıt dosyalarını, `review-*.json` bağımsız inceleme kapsamını gösterir. Standalone doğrulama tarihsel; son teslim doğrulaması `common-validation.json` ve `../source-workspace/build-record.json` içindedir.
