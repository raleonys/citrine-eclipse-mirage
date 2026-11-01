# B5 — Configuration, compiler console ve CRT thread başlangıcı

Bu paket B4 Runtime'a verilebilen üç somut sağlayıcı yolunu ekler. Yeni native fonksiyon gövdesi veya native kapanış sayılmaz; mevcut kaynak algoritmaları yeniden kullanılır ve yeniden derlenen CRT için açık adaptör politikaları kurulur. Önceki 710 kaynak dosyası değiştirilmez.

## Configuration bağlantıları

`Configuration`, mevcut 21 metotlu `configuration::Dependencies` arayüzünün yedi metodunu bağlar:

| Metot | Gerçek kaynak veya sınır |
|---|---|
| `parent_path_019820` | Mevcut recovered path algoritması |
| `join_016b00` | Mevcut recovered path algoritması |
| `filesystem_widen_010540` | Mevcut `filesystem_widen_006d70`, kod sayfası seçimi ve iki geçişli conversion kaynağı |
| `filename` | Mevcut `filename_00e120` |
| `crt_is_space` | Seçilen derleyici CRT'sinin `isspace` fonksiyonu |
| `crt_tolower` | Seçilen derleyici CRT'sinin `tolower` fonksiyonu; aşağıdaki sınırla |
| `set_environment_variable` | Gerçek `SetEnvironmentVariableA` SDK çağrısı |

Diğer **14 configuration politikası** sahip olunan alt sağlayıcıya kalır. Executable/known-folder yolları, UTF8 widen, narrow/resolve, log, license, dizin/PNG/enumeration/remove ve achievement persistence için sessiz başarı dönüşleri eklenmez. B4'ün gerçek FilePass/parser/hash yolu kullanılır; eski `load_document` adaptörü o yolda atlanmaya devam eder.

Recovered uygulama bazı karakterleri signed-char olarak `tolower` sınırına taşır. Compiler C `tolower` için EOF dışındaki negatif değerler tanımsız davranışa yol açabileceğinden yeni adaptör bunları `domain_error` ile reddeder. Bu, özgün MSVC multibyte/locale davranışını çözdüğümüz anlamına gelmez. `isspace` unsigned-byte girdisini aynen alır. İlk kod sayfası, conversion flag'leri, uzunluk ve conversion hataları mevcut kaynak algoritmasından gelir. Dar path için UTF8 veya ANSI seçimi tahmin edilmez; alt politika açık kalır.

## Somut compiler console

`CompilerConsole`, console arayüzünün bütün metotlarını uygular. Win32 AttachConsole/AllocConsole/GetConsoleWindow/MoveWindow çağrılarını ve derleyici CRT `freopen_s` fonksiyonunu seçer. Testlerde bu API'ler enjekte edilir; gerçek console açılmaz ve standart akışlar yeniden yönlendirilmez.

Kaynak console akışı, aynı stream kimliğini iki ayrı argüman olarak geçirir. Özgün 0x58 baytlık MSVC `FILE` nesnesi compiler `FILE` diye yorumlanmaz. Yeni adaptör, sabit adresli üç `FILE*` slotunu opaque token olarak sunar; `freopen_s` geçerli ayrı bir `FILE*` output hücresi ve kaynak stream pointer'ını alır. Hiçbir `FILE` kaydı kopyalanmaz. Bu, yeniden derlenen CRT için seçilen bir uyumluluk politikasıdır; özgün alias, nesne yerleşimi ve hata yan etkileriyle native eşdeğerlik kanıtı değildir.

`noConsole`, başarısız attach/showConsole, allocation sonucu görmezden gelme, pencere operandları ve stdin/stdout/stderr reopen sırası mevcut `setup_console_038760` kaynağında kalır. Stream token'ları nesne taşınmadığı sürece kararlıdır. Başarısız reopen hücreyi null yaparsa sonraki tekrar açıkça reddedilir. Process genelindeki stream değişiklikleri için çağıranın seri kullanım politikası gerekir; otomatik geri yükleme eklenmez.

`bind_configuration_and_console(Resources, ...)` config'i yeni somut kaynak adaptörüyle sarar. Console alanı boşsa `CompilerConsole` seçer; açıkça seçilmiş console sağlayıcısını korur. Kurulum callback kaydetmez, worker başlatmaz veya kullanıcı dosyası okumaz.

## Global route kullanmayan CRT thread başlangıcı

`NativeThreads`, her SDK CreateThread çağrısı için `{provider, Context}` içeren ayrı bir launch zarfı oluşturur. Win32 thread başlangıcı bu zarfı alır, içeriğini kopyalar ve **recovered `thread_entry_05e7a8` çağrılmadan önce zarfı siler**. Böylece `ExitThread` C++ stack destructor'larını atladığında bir RAII zarfı sızmaz. Provider'a ulaşmak için per-image global pointer kullanılmaz.

Application callback'i doğrudan SDK thread girişi yapılmaz. Akış mevcut kaynak üzerinden `05e7a8 → application callback → 05e9bc/05e818 → exit` şeklindedir. Diğer thread metotları alt kaynağa gider. Başarılı CreateThread dönüşünden sonra Launch veya Context yeniden okunmaz; worker bunları o anda çoktan tüketmiş olabilir. DWORD ID yalnız başarılı dönüşte kopyalanır. Full-width stack size, flags ve LastError korunur. Launch allocation başarısızlığı yeni adaptör politikası olarak ERROR_NOT_ENOUGH_MEMORY döndürür.

SDK başarısız create sonucu worker başlatılmadığını garanti eder; o yolda zarf serbest bırakılır. Enjekte edilmiş create'in bir worker başlatıp başlatmadığı bilinmeden C++ exception atması durumunda zarf korunur. B3 tracker bu sonucu poisoned sayar ve provider ömrünü bırakamaz; hayali rollback eklenmez. Gerçek Win32 callback ABI'sinden C++ exception çıkamaz; worker exception'ı terminal sınıra alınır.

`make_recovered_threads(ThreadDomain)` mevcut somut `crt_glue::ThreadEffects` sınıfını seçer. Shared pointer ile verilen domain ömrü, bağlı runtime/error/PTD/heap/invalid sağlayıcılarından ve worker FLS temizliğinden uzun olmalıdır. Domain'in gerçek başlangıç state'i, FLS binding'i ve bütün provider'larının aynı CRT alanını kullanması çağıranın açık girdisidir. **Bu paket tamamlanmış varsayılan ThreadDomain üretmez.** Yeni factory'nin kaynak/type bağlantısı derlenir; bu paketteki worker testleri kontrollü alt etkiler kullanır. Mevcut CRT-glue testlerinin kapsamı, bu yeni factory'nin gerçek üretim domain'iyle çalıştırıldığı gibi sunulmaz.

`NativeThreads` doğrudan kullanıldığında çağıran bütün worker'ları, askıda/bekleyen işler ve FLS kapanışı dahil, nesneyi silmeden önce join etmelidir. B4 Runtime içinde bu garanti B3 `TrackingThreads` kapanış sözleşmesiyle sağlanır. Console/encoding/thread wrapper bağlantısı; foreign ABI, genel Execution eşzamanlılığı, image unload veya loader-lock güvenliği kanıtı değildir.

## Korunan native kanıt

Yeni decompilation yapılmadı. Önceki `anadius-all-functions.pseudo.c`, asm ve REA kanıtları tekrar kullanılır:

| Adres | Evidence ID |
|---|---|
| `0x180019820` parent path | `ev_99245712414cad2dc0f40f8e24c135ff69b46245d499ebddd8a2fcc0dfd29995` |
| `0x180016b00` path join | `ev_b91a74cb8fab9bce0f5c77c83454c94d7877d70d76cb86a6cb00b109b717ec8b` |
| `0x18000e120` filename | `ev_10862633c6e4104753810f9acde38e7bb612799eadefb180ec6aca524a9a7ab3` |
| `0x180006d70` filesystem widen | `ev_ddea68d70083a15fe46d222afc8aab9746f91c7339b12ef8b6ebddb7afb88461` |
| `0x180038760` console orchestration | `ev_443e0c3c2732489164806801392df22b75420bef29996038536b50f1d9fab851` |
| `0x18005e7a8` recovered CRT thread entry | `ev_5e71c9771b4ee41eabb165b637fe2731b56aa25a4607e88aa0466f7c42c11285` |
| `0x18005e8e0` begin-thread source | `ev_06d02d032bc19f80b5602d3d24b5ce1d432c5a742d8014e2c0f45f08f084deea` |

Yeni production dosyaları `composition-only.json` içinde ayrı tutulur. Adaptörün adres son ekleri native kaynak haritalamasını artırmaz. 487 mapped adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 tarihsel native kapanış korunur.

## Test kapsamı ve kalan işler

Hızlı testler **53 çekirdek + 110 entegrasyon = 163 kontrol PASS**. Çekirdek test kendi Win32 worker'ını ve kendi terminal alt sürecini kullanır. Entegrasyon gerçek B2/B3/B4/config/parser kaynaklarını ve üç kendi worker'ını kullanır. Worker'lar join sırasında sırayla başlatılır; genel paralel runtime eşdeğerliği iddiası yoktur. Test sayacı yalnız resume öncesinde veya join sonrasında ana thread'den okunur/değiştirilir. CFG bellek içi fixture'dır; console, encoding, network/foreign ve storage yan etkileri kontrollüdür.

Ortak build grupları `bootstrap_providers` ve `bootstrap_providers_integration`'dır. Nihai ortak build doğrulaması bu belgenin sonuna eklenir. Üretim nesnesi sayısı iki artar; yedi yeni kaynak/test dosyası eklenir. Önceki bütün kaynak byte'ları aynı kalmalıdır.

Kalan Services politikası **9** olarak duruyor; B5 bu sayıyı azaltmadı. Configuration tarafında yedi bağlanan metot ve 14 açık politika vardır. Gerçek ortak ThreadDomain, server/online/host/hook alanları, foreign ABI, Execution concurrency, image unload ve PE girişi açıktır. Tam DLL için güvenilir toplam yüzde ölçülmez; yeni native gövde/kapanış **0**.

Sonraki B6 paketi, tek gerçek ThreadDomain'in sahipliği ve mevcut kaynaklardan desteklenen kalan temel Services/config politikalarıdır. Öncelik **1 → 2 → 3 → 5 → 4**; genel CRT leaf kuyruğu ve numeric `0033c0` beklemededir. Orijinaller çalıştırılmaz/değiştirilmez; gerçek kullanıcı CFG/token/achievement içeriği veya engelli version girdileri okunmaz.

## B5 ortak doğrulama

**53 çekirdek + 110 entegrasyon = 163 kontrol PASS.** Ortak build 217 nesne (213 C++ / 4 C), 241 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 710 kaynak aynı; toplam 717 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
