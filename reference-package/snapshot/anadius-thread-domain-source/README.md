# B6 — Sahip olunan başlangıç-locale thread domain'i

Bu paket gerçek `DomainStorage → make_recovered_threads → crt_glue::ThreadEffects → thread_entry_05e7a8` yolunu kurar ve kendi Win32 worker'larımızla sınar. Önceki B5 testindeki kontrollü alt thread sağlayıcısı bu yolun yerine kullanılmaz. Heap, native FLS/PTD, error yönlendirmesi, başlangıç locale verisi ve runtime modül referansları tek sahiplikte tutulur.

**Kapsam başlangıç locale'iyle sınırlıdır.** Bu, bütün özgün CRT/global locale/EH/Watson ABI'sinin veya tamamlanmış DLL'in elde edildiği anlamına gelmez. Yeni native gövde ve native kapanış **0**; mevcut algoritmalar yeniden kullanılır. Önceki 717 kaynak dosyasının byte'ları değişmez.

## Gerçek nesne grafiği

| Bileşen | Kurulan kaynak |
|---|---|
| Heap API | Mevcut typed `crt_heap_native::Calls` ve `crt_heap_native_bridge::Effects` |
| Heap/error döngüsü | Yalnız bir kez bağlanan ErrorProxy → mevcut `crt_glue::ErrorPtd` |
| Heap state | Açık cookie/new-mode/handler girdileri, mevcut `initialize_heap_0704cc` |
| Kilitler | Mevcut `crt_lock_native::Banks`; bu domain için ACRT 0, 4 ve 5 |
| Locale/MB veri sahibi | Mevcut `ImageState`, gerçek LONG/Interlocked sayaçları ve locale-ref kaynakları |
| PTD/FLS sahibi | Mevcut `crt_fls_native::Binding`; yalnız `binding.state()` kullanılır |
| Invalid/abort | Aynı FLS/PTD alanına bağlı InvalidPtd; seçilmiş rebuilt callback veya compiler abort politikası |
| Runtime | Yeni `NativeRuntime`, mevcut runtime resolver/cache/policy algoritmalarının typed SDK sınırı |
| Thread sağlayıcısı | B5 `make_recovered_threads` ile mevcut somut `ThreadEffects` ve launch zarfı |

`DomainStorage::create(seed, apis)` kopyalanmayan, taşınmayan bir ortak sahibi kurar. Seed içindeki security cookie, new-mode, rebuilt new/invalid handler, thread-policy cache ve exception identity/backing açık girdidir. Cookie ve encoded handler değerleri mevcut encode kaynağıyla tutarlı hazırlanır. Özgün tercih edilen VA'lar çağrılabilir pointer yapılmaz. Exception identity'nin canlı backing ömrü sahipte kalır; tam özgün exception-table içeriği/dispatch ABI'si ayrıca açık sınırdır.

FLS/heap/error constructor döngüsü bir bind-once proxy ile, hiçbir initializer veya worker çalışmadan önce çözülür. Domain kendi thread sağlayıcısını tutmaz. Sağlayıcıdaki shared lease domain'i canlı tutar; böylece güçlü sahiplik döngüsü oluşmaz.

## Kurulum, hata ve kapanış

1. `initialize()` gerçek process heap'i alır; bu ödünç heap için HeapDestroy yapılmaz. ACRT 0/4/5 native critical section nesneleri kurulur.
2. Mevcut `initialize_064e90`, aynı native FLS Binding state'iyle çalışır ve kontrol thread'inin gerçek PTD'sini kurar. Sadece FlsAlloc başarılı diye hazır sayılmaz.
3. Normal heap/kilit/FLS allocation/publication hatalarında bilinen kurulum ön eki geri alınır. Temiz Constructed durumunda initialize tekrar denenebilir. Source initializer cleanup'ı native slotu bırakamazsa ReleasePending durumu sahibini ve kilitlerini korur.
4. Hazır domain `make_threads()` ile gerçek B5 factory'sini çağırır. Her sağlayıcı bir domain lease'i taşır. Ready dışındaki durumlar yeni sağlayıcıyı reddeder.
5. B4 önce yeni girişleri durdurur; B3 bütün worker'ları ve onların gerçek FLS exit callback'lerini join eder; ardından B4 uygulama cleanup'ını çalıştırır. Bu sıralama değişmez.
6. **Runtime/provider nesneleri yok edildikten sonra** domain `close()` edilir. Yalnız Runtime.close çağrısı provider nesnesini yok etmez. Kalan lease varsa domain.close false döner ve FLS/PTD/kilitlere dokunmaz.
7. Source `uninitialize_064ecc` ilk native release'i dener. Windows FlsFree başarısız olsa bile kaynak FLS indeksini geçersiz yapar. Ayrı native indeks hâlâ geçerliyse domain tutulur; sonraki close yalnız gerçek native release'i tekrar dener. Gizli aynı-çağrı ikinci release denemesi yapılmaz.
8. FLS release tamamlandıktan sonra runtime modülleri bırakılır. Bir FreeLibrary hatasında kalan referanslar ve bağlı sahipler korunur. Son olarak kilitler ters sırada silinir ve Closed durumuna geçilir.

False veya exception sonrası canlı kaynakları olan root'u stack unwind ile terk etmek güvenli cleanup sayılmaz. Root yeniden deneme için korunmalıdır. Canlı domain/runtime sahibinin örtülü yıkımı terminal sözleşme ihlalidir. Source PTD constructor'ının yayın sonrası bilinmeyen exception sonucu Poisoned kalır; henüz kanıtlanmamış lock/ref/constructor rollback'i uydurulmaz.

Kontrol metotları aynı, yeniden girilmeyen thread'e aittir ve loader lock dışında çalışır. Ham PTD pointer'ı, Cell32 ve phase/index görünümleri kontrol veya quiescent gözlemlerdir; domain kapanışından sonra geçerli değildir. B5 ham thread sağlayıcısı doğrudan kullanılırsa bütün pending/suspended işler ve FLS temizliği bitmeden yok edilemez; B3/B4 yolu bu sırayı sağlar.

## Başlangıç locale'i sınırı

`InitialLocale`, ImageState'in sahip olduğu relocate edilmiş initial locale ve multibyte nesnelerini seçer. ImageState'in raw initial-global verisini tam canlı CRT global'i diye sunmaz. Current-locale provider'ı bu sınırlı domain için açıkça initial locale'i döndürür. ACRT 4/5 kilitleri, mevcut locale-ref algoritmaları ve kayıtlı gerçek sayaçlar kullanılır.

Foreign/dynamic locale ve MB counter adresleri reddedilir; initial locale deep-free edilmez. Exception ve PTD alanlarındaki heap free çağrıları aynı heap kaynağına gider; bu bir keyfi adres doğrulama defteri değildir. Domain, dışarıdan değiştirilmiş/foreign PTD alanlarını desteklediğini iddia etmez.

Normal kaynak seed'i locale/MB sayaçlarında **1/0** tabanına sahiptir. Kontrol PTD'siyle **2/1**, üç ek canlı worker PTD'siyle **5/4** gözlenir. Worker join/FLS cleanup sonrasında **2/1**, son domain release sonrasında **1/0** geri gelir. Bunlar bu paketin gerçek own-worker testlerinde doğrulandı.

## Native runtime sınırı

`NativeRuntime` LoadLibraryExW/GetProcAddress/FreeLibrary/LastError çağrılarını ve RO/AppPolicy export'larını exact SDK function-pointer türleriyle bağlar. Mevcut kaynak module/procedure cache'lerini ve runtime policy hesabını yapmaya devam eder. AppPolicy output'u ayrı typed enum hücresi üzerinden, hata dönüşünde de bitleri korunarak taşınır.

Process flags yalnız mevcut kendi x64 process'imizin SDK PEB/ProcessParameters prefix'inden +8 offset'iyle okunur. Bu seçilmiş host sınırıdır; portable veya bütün Windows/private-ABI sürümleri için eşdeğerlik kanıtı değildir. İsimlerde embedded NUL ve geçersiz RO mode açıkça reddedilir. API ve bookkeeping aynı gerçek Win32 LastError alanını kullanmalıdır.

Her LoadLibrary referansı, OS çağrısından önce ayrılan ayrı ticket ile tutulur. Aynı HMODULE değerinin tekrar dönmesi referansları birleştirmez. Başarısız normal release referansı korur; kapanış tamamlanan ön eki tekrar bırakmadan devam eder. Worker'lar yükleme/resolution/RO yollarında çalışabilir; **son modül referansının bırakılması bütün lookup/RO/load/free çağrılarıyla seri olmalıdır**. Worker sırasında source `free_library` yalnız başka bir sahip olunan cache referansı kodu canlı tutarken ek duplicate referansı bırakabilir. Sahiplik kontrolü procedure/module lifetime lease'i yerine geçmez. Recovered missing-cache/concurrent-unload kenarı genel olarak kanıtlanmış değildir.

`close()` bütün worker/RO/FLS quiescence'ından sonra ve loader lock dışında çağrılır. Bilinmeyen native load/release exception sonucu poison olarak tutulur. Modüller cleanup ve son RoUninitialize tamamlanmadan boşaltılmaz. Hiçbir özgün DLL veya oyun API'si çalıştırılmaz; testteki gerçek modül yükleme yalnız Windows `kernel32.dll` referansıdır.

## Kanıt ve inceleme

Yeni decompilation yapılmadı. Aşağıdaki mevcut REA kanıtları ve kaynakları yeniden kullanılır:

| Native adres | Evidence ID |
|---|---|
| `0x1800704cc` heap init | `ev_b6c4736f154dc16ac5e368c7b8d569dca1c7238dd8704175a00636d5f9c23649` |
| `0x180064e90` PTD init | `ev_9e333e33a4960b2fa42317907dca8150dff6c829e4d2a8ac893f096a103d7ce7` |
| `0x180064ecc` PTD uninit | `ev_548f2fca0b7a721cc982f98265afacc70e59e6f00fb0dfe28f5871f282ad6b5a` |
| `0x1800648fc` PTD construct | `ev_5b39f111b222558bd8e52132523a2e361a941261e916883af37f3e602c09927b` |
| `0x1800649ec` PTD destroy | `ev_23ece12ba558d0057c31265d9b8efd21dd1a29b0ade68bd61f6fd47b3fd82e96` |
| `0x1800649cc` FLS destroy | `ev_2f68beb438ece936d2c426ccfea03da3a96c7696419ca3d29a00b4d7b88fdcb8` |
| `0x18006c0d0` runtime policy | `ev_c2ea8dc17525b2b3a8dd4f72a9ed92b2a47441b788109d3fb92c138cfdb3ef95` |

Bağımsız inceleme PASS; altı üretim dosyasının hash'leri, önceki 717 kaynağın değişmediği ve yakalanmış test sonuçları doğrulandı. İnceleme, yeni kurulum kodunda SDK BOOL sonucunun yanlışlıkla yalnız alt baytla test edilmesini buldu. Tam sıfır-dışı SDK başarı kontrolü düzeltildi; **BOOL=0x100** regresyon testi gerçek kurulan üç critical section'ın da doğru kapanmasını doğruladı. Native runtime son-referans concurrency sözleşmesi de inceleme bulgusu olarak açık tutuldu.

## Doğrulama ve kalan işler

Hızlı testlerde **168 çekirdek + 101 entegrasyon = 269 kontrol PASS**. Çekirdek test gerçek heap/FLS/CS failure-prefix/release retry, typed runtime/AppPolicy sınırları, duplicate module ownership ve dört kendi terminal alt sürecini kapsar. Entegrasyon gerçek `make_recovered_threads` yolundan üç eşzamanlı kendi worker'ımızda PTD/errno ayrımını ve FLS temizliğini; tam B3/B4 yolunda da üç suspended worker'ın seri resume/join, timeout retry ve tek seferlik uygulama cleanup'ını kapsar.

Worker testlerinde thread-policy seed **1 sentetik açık girdidir**; varsayılan native process/OS policy sorgusunun veya bütün RO yollarının eşzamanlı çalıştırıldığı iddia edilmez. Modül pin sınırı own EXE için kontrollü FALSE'tur; buna rağmen gerçek recovered ThreadEffects, heap, FLS ve kilit grafiği kullanılır. Kod image'ının FLS cleanup boyunca bellekte kalması şarttır. Kaynak error fallback alanları ve genel OOM/EH eşzamanlılığı ayrıca kanıtlanmış değildir.

Yetkili ortak build grupları `thread_domain` ve `thread_domain_integration`'dır. Nihai ortak build sonucu belgenin sonuna eklenir. Yeni üç C++ üretim nesnesi ve dokuz kaynak/test dosyası vardır: toplam **220 nesne (216 C++ / 4 C), 726 kaynak**, 243 normal test grubu ve 19 framework terminal senaryosu beklenir; son doğrulama kaydı bu gerçek sonuçlara bağlanır.

Yeni dosyalar composition-only olarak sayılır. Native mapped adres **487**, karşılaştırılabilir compiled tag **491**, tarihsel kapanış **52** olarak korunur. Services politikası **9**, configuration politikası **14** açık kalır. Tam mutable/global/native CRT, EH/Watson, server/online/host/hook sağlayıcıları, image unload/loader-lock ve PE girişi tamamlanmış değildir. Tam DLL için güvenilir toplam yüzde verilmez.

Sonraki B7 paketi, mevcut kaynakların desteklediği temel Services allocation/cleanup/error bağlantılarını bu domain'e bağlamak ve kalan configuration OS/persistence yollarını seçmektir. Genel CRT leaf kuyruğu ve numeric `0033c0` beklemededir; öncelik **1 → 2 → 3 → 5 → 4**. Orijinaller değiştirilmez/çalıştırılmaz; gerçek kullanıcı CFG/token/achievement içeriği okunmaz. Engelli version girdisi ve tarihsel üretilmiş version DLL yeniden denenmez.

## B6 ortak doğrulama

**168 çekirdek + 101 entegrasyon = 269 kontrol PASS.** Ortak build 220 nesne (216 C++ / 4 C), 243 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 717 kaynak aynı; toplam 726 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
