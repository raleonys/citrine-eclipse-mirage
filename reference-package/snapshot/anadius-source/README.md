# anadius64.dll: okunabilir C++ başlangıç modülü

`bootstrap.cpp` ve `bootstrap.hpp`, uygulamanın DLL giriş ve iş parçacığı başlangıç akışının adresli, türlendirilmiş C++ yeniden yazımıdır. Pseudocode metnini derleyiciye vermek yerine gözlenen dalları ve sıralamayı açık C++ ile ifade eder. **Tam `anadius64.dll` kaynak kodu veya bağımsız DLL yerine geçmez.**

Kaynak örnek SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
Dayanak: üst klasördeki `anadius-all-functions.pseudo.c`, `anadius-all-functions.asm.txt`, `anadius-recovery-index.json`. Bunlar Ghidra 12.1.4 ile REA üzerinden alınmış önceki salt okunur kanıtlardır. Bu modül yazılırken özgün DLL yüklenmedi, çalıştırılmadı veya değiştirilmedi.

## Kaynak kapsamı

| Özgün VA | Yeni işlev | Kanıt (assembly evidence ID) |
|---|---|---|
| `0x180038870` | `Bootstrap::application_dll_main` | `ev_e22c193077e36533a0c4bdeab829161a9788ca97925356612bcaeadbc0285619` |
| `0x180038970` | `Bootstrap::initialize` | `ev_77887ea2e7b744e62e52eaf81cb8a07abf2a4f50498001ba81b2fcf2496e8789` |
| `0x1800388a0` | `Bootstrap::run_no_argument` | `ev_3022378b3b333555b2f1f63fa59a85ea675e756369871f825e3fd805143160eb` |
| `0x1800388d0` | `Bootstrap::run_boolean` | `ev_220d092f92df6b8c1c7cbb91ffd4ee02e5b5af3f3f6fd15d599e710c8adf353e` |
| `0x180038950` | `Bootstrap::on_exit` | `ev_9e5bed8eb620590b20a44651d47fd66e0d4d00f7cc3caf952f79bc4a73f22049` |
| `0x180038890` | `Bootstrap::on_terminate` | `ev_a73f817561c1dc663846bb56938e55cea83e98470e9af8c91186939f6bbbe144` |
| `0x18004a458` | `Bootstrap::detach` | `ev_218bf3eda319b5a7858921c077bd0fd00cadc3c2d550d8952be3eef945886923` |
| `0x18004a47c` | `cleanup_exit_waits` | `ev_b17cc6887bbf8d03f11ab86e0b2d89608cc3671d3da2122e64441f45bb292dc9` |

`start_detached` yeni, yerel bir yardımcıdır; `0x180038970` içindeki üç tekrarın ortak ifadesidir. Özgün dosyada bu isimde bağımsız işlev yoktur. Sınıf/arayüz isimleri ve semantik etiketler yeni seçilmiştir; PDB'den çıkarılmış özgün isimler değildir.

## Korunan akış

Giriş nedeni `1` ise başlatma yapılır ve normal dönüşte `true` verilir. Diğer nedenler yalnız `true` döndürür. Başlatma; çıkış işlevini kaydeder, terminate işlevini kurar, ortam/konsol hazırlığını, üç zincirli günlük eklemesini ve yapılandırma işlevini çağırır. Günlük literal adresleri `0x180083cd8`, `0x180086720`, `0x180086718` olarak korunur. Bu sayılar yeni programda dereference edilmez; günlük arayüzünde kanıt anahtarı olarak kullanılır ve gerçek entegrasyonda kurtarılmış verilerle eşlenmelidir.

Ardından `0x180029ea0` için 8 bayt payload/ilk thread başlatılır ve handle kapatılır. `0x18000e310` çağrısından sonra `0x180035b60` için ikinci thread başlatılır ve handle kapatılır. `0x180035850(true)` eşzamanlı çağrılır; 16 bayt payload ile aynı işlevin `false` çağrısı üçüncü threade aktarılır. Başlatıcı thread hiçbir worker sonucunu beklemez. Son `0x180017a30` tail jump (Ghidra `_guard_check_icall` etiketi) sözleşmede ayrı tutulur.

İki trampoline, callback normal dönünce `0x18004a47c` temizliğini ve sırasıyla 8/16 boyutlu delete çağrısını yapar; `0` döndürür. Boolean trampoline'deki son null kontrolü callback dereference işleminden sonradır; null payload güvenli hale getirilmedi. Başlatıcıdaki allocation-null dalları korunur; özgün allocator normalde hata durumunda exception atar. Callback exception atarsa bu kaynak kendi başına yeni bir cleanup/RAII politikası eklemez. Özgün MSVC EH/SEH, terminate ve unwind ABI eşdeğerliği henüz doğrulanmış değildir.

Çıkış callback'i `0x180035ab0`, `0x1800366f0`, `0x1800102b0` sırasını korur. Terminate callback'i `0x180038920` yolunu çağırır; bu yol gözlenen log işlemlerinden sonra `abort` eder. Beklenmedik geri dönüşte `0x18003889b` üzerindeki `INT3` ayrı arayüz çağrısı olarak temsil edilir.

## CRT ve thread sınırlarının doğru yorumu

`0x18004b0bc` `_malloc_base` ve new-handler döngüsü içerir; sonuç bulunamazsa `0x180047f80` üzerinden allocation exception yoluna gider. Ghidra'nın `void` dönüş tipi eksiktir: çağrı yerleri `RAX` sonucunu kullanır. Burada `allocate_04b0bc(size_t)` bir pointer döndürür ve exception atabilir. `0x18004b0f8`, çağrı yerlerinde `EDX=8`/`16` ile kullanılır; gövdesi `0x1800668c0` free yoluna atlar ve size değerini kullanmaz.

`0x18005e8e0` **CRT thread wrapper'ıdır**. Çağıranın stack-size/flags değerleri bu başlangıç akışında sıfırdır; güvenlik nitelikleri null'dır. `0x18005e87c` `_calloc_base(1,0x28)` ile ayrı CRT context oluşturur: giriş işlevi `+0`, kullanıcı payload'u `+8`, thread handle alanı `+0x10`, module ref `+0x18`, CRT/COM flag'i `+0x20`. `GetModuleHandleExW(4, entry, &context.module)` çağrısıyla girişin modülüne referans alınır; dönüş sonucu kontrol edilmez. Aynı yardımcıda `free(nullptr)` çağrısı da gözlenir.

`CreateThread`, doğrudan uygulama trampoline'ini değil `0x18005e7a8` CRT girişini alır. Bu giriş context'i thread-local CRT verisinde `+0x3c0` alanına kurar, gözlenen CRT/COM başlangıç yolunu yürütür, gerçek callback'i çağırır ve `0x18005e9bc`/`common_end_thread` (`0x18005e818`) ile çıkar. Son yol gerektiğinde handle kapatır, COM kapanışını yapar ve `FreeLibraryAndExitThread`/`ExitThread` kullanır. Bu CRT ABI'sinin tam uygulaması bu modülde yoktur; `begin_thread_05e8e0` zorunlu arayüz sınırıdır. Ham `CreateThread` veya sıradan `std::thread` ile değiştirilmesi eşdeğerlik olarak kabul edilmez.

Thread oluşturma başarısızsa wrapper, `GetLastError` değerini `0x18005d698` ile CRT hata durumuna dönüştürür, context içindeki dolu handle/module alanlarını kapatır ve CRT context'i serbest bırakır. Caller payload'u bundan ayrıdır; başlatıcıdaki hata yollarına fazladan payload delete eklenmedi. Başarılı oluşturma sonrası thread ID dışarı yazılır.

`0x18004a458`, sadece handle üzerinde `CloseHandle` yapar. Assembly'deki `NEG EAX; SBB EAX,EAX; NOT EAX; AND EAX,4` dizisi **başarıda 0, başarısızlıkta 4** üretir. Thread beklemez ve yapıyı sıfırlamaz. Ghidra'nın `byte` dönüşü yerine tüm `EAX` sonucu dikkate alındı.

Başlatıcı null handle görünce `_Thrd_error(6)`; sıfır thread ID veya başarısız detach görünce `_Thrd_error(1)` çağırır. `0x18004a648` içindeki iki veri tablosunun (`0x180087db8`, `0x180087df0`) özgün PE baytları salt okunur doğrulanmıştır:

| Tablo indeksi | Hata sayısı | Mesaj VA | Mesaj |
|---|---:|---|---|
| 1 | 22 | `0x180087888` | `invalid argument` |
| 6 | 11 | `0x180087b98` | `resource unavailable try again` |

Bunlar callback return kodları veya doğrudan Win32 hata numaraları değildir. MSVC C++ exception sınıfı, error category vftable ve throw metadata'nın birebir ABI üretimi açık iştir. Testlerde ayrı `ThreadError` kullanılması yalnız dalın seçtiği tablo indeksini sınar.

## Thread çıkış kayıtlarının temizliği

`cleanup_exit_waits` önce mevcut thread ID'yi alır, `0x1800a4010` üzerindeki critical section'a girer ve `0x1800a42b0` listesini tarar. Blokta 20 adet, `0x28` baytlık kayıt; sayaç `+0x320`, sonraki blok `+0x328` konumundadır. Alan yerleşimleri `static_assert` ile denetlenir; okunmayan alanların anlamı bilinmiyor.

Mutex'i dolu ve thread ID'si eşleşen her kayıtta varsa ready işaretçisi `1` yapılır; `0x18004a6b4` mutex bırakma ve `0x18004ae28` condition notify-all çağrılır, yalnız mutex alanı sıfırlanır ve sayaç azaltılır. İlk yardımcı, mutex sayacını `+0x4c` konumunda azaltıp sıfır olduğunda owner `+0x48` alanını `-1` yapar ve `+0x10` SRW lock'u bırakır. İkincisi `condition+8` üzerinde `WakeAllConditionVariable` çağırır. Bu iki yardımcı işletim sistemi/CRT sözleşmesi olarak dışarıda tutuldu. Fonksiyon sonunda critical section'dan çıkılır.

## Derleme ve doğrulama

PowerShell: `./build.ps1`. Gerekirse `-ToolchainRoot` ile taşınabilir LLVM-MinGW kökünü belirtin. Alternatif olarak `CMakeLists.txt` kullanılabilir. Varsayılan script aynı çalışma alanındaki mevcut Clang 23.1.3'ü kullanır; kurulum veya indirme yapmaz.

`bootstrap.cpp` uyarılar hata sayılarak x64 COFF object olarak derlendi. Yalnız mock bağımlılık kullanan test executable'ı derlendi ve çalıştırıldı:

```text
PASS: 191 mock contract assertions; original DLL was not loaded or executed.
```

Kontroller tüm başlangıç sırasını, payload alanlarını, trampoline sırasını, üç threadin dokuz hata dalını, null/throw allocation durumlarını, exit/terminate akışını ve bağlı bloklarda thread çıkış temizliğini kapsar. DLL yükleme, gerçek thread oluşturma, gerçek worker, ortam/yapılandırma değişikliği yapılmaz. Bunlar kaynak modülünün sözleşme testleridir; özgün binary ile çalışma zamanı veya tam davranış eşdeğerliği kanıtı değildir.

## Tam DLL için kalan iş

`BootstrapDependencies` ve `ExitWaitDependencies` içindeki tüm soyut arayüzlere gerçek, kanıtla yeniden kurulmuş alt sistemler gerekir. Ortam, konsol, günlük, yapılandırma, `0x18000e310`, üç worker ve exit callback alt sistemleri bu dosyada gövde olarak kurtarılmamıştır. Başvurular: `../anadius-thread-source` CRT thread/context gövdelerini ve yeni `BootstrapThreadBridge` bağlantısını; `../anadius-host-init-source` 00e310 host hazırlığını; `../anadius-online-source` 035b60/0366f0 online worker/temizlemeyi içerir. Diğer kurtarılmış uygulama modülleri ve kalan sınırlar `../KOD-REHBERI.md` içinde listelenir. Kaynak içinde boş başarı döndüren üretim implementation veya özgün DLL'ye delegasyon yoktur. ABI adaptörleri, gerçek global veri yerleşimi, CRT/EH/SEH, tüm exportlar, diğer uygulama ve kütüphane işlevleri tamamlanmadan bağımsız DLL yeniden derlemesi yapılamaz. Proje bu eksikleri gizleyerek sahte bir DLL üretmez; yalnız gerçekten yeniden yazılan modülü ve mock testini derler.
