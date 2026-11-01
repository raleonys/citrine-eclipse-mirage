# B4 — Callback ömrü ve kaynaklardan Runtime kurulumu

Bu paket, B2 `RuntimeOwner` ve B3 `SourceBackend` üzerine somut `CallbackRegistry`, `RegisteredServices`, `Runtime` ve `NativeExecution` sınıflarını ekler. Kontrollü kapanışta owner/provider nesneleri silindikten sonra eski exit callback'lerinin onlara erişmesini önleyen bağlantı kaynakta kurulmuştur. Bu, yeni bir bileşim politikasıdır; özgün DLL'den yeni native gövde kurtarıldığı anlamına gelmez.

## Kayıt ve kapanış

`CallbackRegistry`, her kabul edilen exit callback'i için derleyici CRT'sine ayrı bir `atexit` dispatch kaydı koyar. Böylece araya başka kodların koyduğu CRT callback'leriyle LIFO sırası korunur. Tek bir CRT kaydı altında bütün callback'leri topluca çalıştırmak bu sırayı bozardı. Kalıcı thunk, kısa kilit altında paylaşılan state ve aktif çağrı lease'i alır; kullanıcı callback'i kilit dışında çalışır.

`Runtime::close()` şu sırayı kullanır:

1. Zorunlu `Resources.stop_sources`, yeni uygulama, worker ve hook girişlerini durdurur. Önceden saklanmış application-entry pointer'ları da bu sözleşmeye dahildir. Bekleyen worker'ların tamamlanmasına izin vermelidir.
2. Registry yeni exit/terminate callback girişlerini ve kayıtlarını kapatır. Daha önce girmiş bir callback varsa owner korunur ve kapanış yeniden denenebilir.
3. B3 tracker, gerçek duplicate thread handle'larını bekler. Timeout veya wait/close hatasında temizlik başlamaz; kalan işler ve nesneler korunur.
4. Kayıtlı exit callback'leri bir kez çalışır. Mevcut kaynak sırası **library unload → online cleanup → hook cleanup** olarak kalır. Callback çağrılmadan önce kuyruktan çıkarılır; doğrudan sentetik bir callback exception'ı sonrasında kalan kuyruk tekrar denenebilir. Native `exit_entry` kendi exception sınırını korur.
5. Registry bağlantısı kaldırılır. Terminate handler hâlâ bizim dispatcher ise önceki handler geri yüklenir. Arada seri olarak başka bir handler kurulmuşsa o korunur.
6. B2 native Binding kapanır. Bu son adım Busy bildirirse owner canlı kalır; tamamlanmış callback temizliği tekrar çalıştırılmaz.

Kapanış sonrası saklanmış exit thunk'ı boş döner. Pasif veya geri dönen terminate handler yolu terminal politikaya gider; eski provider nesnelerine ulaşmaz. **Thunk'ın bulunduğu image bellekte kalmalıdır.** DLL unload, loader-lock içinde bekleme veya CRT'nin DLL callback kayıtlarını kaldırması bu paketle kanıtlanmış değildir.

Factory yapıcısı ve `publish()` tek başına callback kaydetmez. İlk uygulama başlatma yolunda kayıt yapılır. B2'nin henüz yayımlanmamış `Constructed` durumunda kapanması güvenlidir. Eksik sağlayıcılarla kurulum, global yayın yapılmadan reddedilir.

## Somut sağlayıcı seçimi

`Runtime(initial, seed, resources, permanent_errors, host_api)` bütün nesneleri ortak ömür içinde kurar. `InitialState` ve `RuntimeSeed` çağıranın açık girdisidir; özgün global ilk değerler tahmin edilmez. Aşağıdaki alanlar boş bırakılırsa gerçek somut kaynak adaptörleri seçilir:

| Resources alanı | Seçilen adaptör |
|---|---|
| `files` | Mevcut `file_recovered::NativeFactory` |
| `input` | Mevcut `startup_recovered::NativeInputPlatform` |
| `library_os` | Mevcut `libraries::NativeSystemCalls` |
| `execution` | Yeni `NativeExecution`: SDK SRW/QPF/QPC/Sleep sınırları |
| Tamamen boş `join_api` | B3 `native_join_api()` |

Kısmen doldurulmuş join tablosu sessizce tamamlanmaz; reddedilir. Diğer dokuz provider nesnesi zorunludur: configuration, console, threads, remaining Services, server, online, library effects, host ve hook cleanup. Ayrıca doğru admission/cancellation davranışı sağlayan `stop_sources` gerekir. Bu nedenle kaynakları birleştiren somut factory vardır; bütün alt sağlayıcıları kendiliğinden kuran tamamlanmış üretim factory'si henüz yoktur.

B3 altı `Services` metodunu yedi mevcut uygulama algoritmasına bağlıyordu. B4 iki kayıt metodunu da somutlaştırır: toplam **8/17 metodun bağlantısı**, **9 kalan politika**. Kalanlar log insert/finish, allocate/deallocate, thread error, guard return, thread cleanup, terminate log ve unexpected-return trap'tir. Diğer yönlendirme metotları yeni native kapanış sayılmaz.

`NativeExecution` SDK fonksiyon pointer türlerini ve caller SRW storage adresini korur. QPF/QPC OS hatasında `std::system_error` yeni adaptör politikasıdır; özgün MSVC exception ABI eşdeğerliği değildir. Thread-error tablo indeksi alt sağlayıcıya aynen gider. Canlı bir SRW lock kopyalanmamalı; seed geçerli başlangıç storage'ı içermelidir. Mevcut `execution.cpp` içindeki kilit öncesi owner alanları ve paylaşılan frequency cache için genel eşzamanlı C++ davranış eşdeğerliği ayrıca kanıtlanmalıdır.

## Korunan native kanıt

Bu pakette yeniden decompile edilen native gövde yoktur. `anadius-all-functions.pseudo.c`, asm ve `anadius-rea-evidence.json` içindeki önceki kanıtlar kullanılır:

| Native adres | Korunan gözlem ve Evidence ID |
|---|---|
| `0x180038970` | Uygulama initialize ve callback kayıt yolu; `ev_e66705fdaa9bdd88fb50be13d22e354939026d931f472329e0f14e17c9fb4fbd` |
| `0x180038950` | Exit cleanup sırası; `ev_cfa69b2b4a15d114d9202339fcf5fbbd77988ffeb6ed350e33b89f550b3d9c65` |
| `0x180038890` | Terminate callback; `ev_fa9e792ca1dc6003fa08b9f989066ed017af372bde3045f31207ef552c7e855b` |
| `0x180035ab0` | Library unload; `ev_9ed3371723edd843422711d1fec614635252352442f0f10feeb036b1f5ef7cc7` |
| `0x1800366f0` | Online cleanup; `ev_a10737dd11c06584a1d8bfb25cfe4eb94945bdc1d2b23aca89b5fddc0066d5f4` |
| `0x1800102b0` | Hook cleanup; `ev_00e35447d2de5559a4451e4788fcc72993ba5950a8158522aaa1efb615b7037c` |

Yeni registry/factory ömür politikaları bu adreslerde gözlenmiş özgün kod gibi sunulmaz. Native adres haritalama ve normal gövde sayıları artırılmaz; yeni adres son ekli adaptörler `composition-only.json` içinde ayrı tutulur.

## Doğrulama ve sınırlar

Çekirdek hızlı test **96**, bağımsız yazılan factory entegrasyon testi **265** kontrol geçti. Altı terminal alt süreç yalnız kendi test EXE'mizdir. Entegrasyon, gerçek B2/B3 kaynak akışlarını sentetik OS/harici sağlayıcılarla kullanır; gerçek kullanıcı CFG/token/achievement içeriği okunmaz. Beş varsayılan adaptörün seçimi kurulum düzeyinde sınanır; bu test gerçek kullanıcı dosyası veya ek DLL yüklemez.

Ortak build'in yetkili grupları `bootstrap_callbacks` ve `bootstrap_factory`'dir. Nihai build sonucu aşağıdaki ortak doğrulama kaydına ayrıca eklenir. Hızlı sonuçlar tek başına bütün projenin geçtiği anlamına gelmez.

Bağımsız mimari inceleme dört üretim dosyası için belirtilen koşullarda engel bulmadı. İnceleyen rapor dosyasını yazmadan kullanım sınırına ulaştı; `architecture-review.json` ve `.md`, tamamlanmış inceleme mesajlarından ana ajan tarafından derlendi ve bu kaynak açıkça belirtildi.

Doğal CRT exit, worker quiescence sağlamaz. Kanıt kapsamı loader lock dışında açıkça `Runtime::close()` çağrılan kontrollü kapanıştır. Kayıtların değiştirilmesi ve terminate handler sahipliği tek kontrol thread'inde serileştirilmelidir; doğal process exit ile kayıt değişimi yarıştırılmaz. Özgün initialize `atexit` hatasını görmezden geldiği için başarısız kayıt sonrasında hayali bir exit cleanup eklenmez; bu hata durumunda eksiksiz kaynak temizliği kanıtlanmış değildir.

Derleyici CRT'si image'ı bir kez başlatır; gelecekteki PE girişi yalnız uygulama katmanını çağırmalıdır. Kurtarılan dış CRT ikinci kez çağrılmaz. Bu paket `DllMain` kurmaz ve DLL yüklemez. Native concurrency, foreign ABI, gerçek alt sağlayıcılar ve image unload/loader-lock kapıları açıktır. Orijinaller değiştirilmez veya çalıştırılmaz; engelli version girdisi ve tarihsel üretilmiş version DLL yeniden okunmaz.

Önceki 703 kaynak dosyası korunur; yedi yeni kaynak/test dosyası eklenir. Yeni native gövde ve kapanış **0**. 487 native adres haritalaması, 491 karşılaştırılabilir derlenmiş etiket ve 52 tarihsel kapanış korunur. Bunlar bütün DLL'in davranışsal tamamlanma yüzdesi değildir.

Öncelik sırası **1 → 2 → 3 → 5 → 4** olarak korunuyor. Sonraki paket B5: gerçek configuration/console/thread bağlama yolları ve kalan Services politikaları. Genel CRT leaf kuyruğu ile numeric `0033c0` beklemede.

## B4 ortak doğrulama

**96 çekirdek + 265 entegrasyon = 361 kontrol PASS.** Ortak build 215 nesne (211 C++ / 4 C), 239 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 703 kaynak aynı; toplam 710 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
