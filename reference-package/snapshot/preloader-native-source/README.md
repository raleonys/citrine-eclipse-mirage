# Preloader için Windows ABI callback bağlayıcıları

`native_shims.hpp/.cpp`, mevcut `recovered_preloader::Core` çekirdeğine giden serbest Windows x64 fonksiyonlarını ekler. `Process32NextW`, opak msvcp140 `_Mtx_trylock`, **11 argümanlı** `CryptQueryObject` ve işlevsel uygulama DllMain çağrısını Core'a yönlendirir. Bu dosyalar yeni entegrasyon kodudur; özgün CRT veya orijinal prolog/epilog kaynaklarının kurtarılmış hali değildir. DLL export'u ve gerçek hook/Win32 Host kurulumu içermez.

## Kanıt ve adresler

Gömülü PE SHA-256: `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846`. Kapsayan REA kaydı: `ev_157c446e2c1bce1a022bed8f576a7cc535fa15c41a6478fc2ef751d2902f97a5`. `../preloader-analysis/ghidra-analysis.json` mevcut doğrudan Ghidra 12.1.4 analizidir; yeni REA decompile sonucu olarak sunulmaz. `evidence.json` dosyasında bu kaynağın hash'i ve 33 assembly işareti vardır.

| Özgün giriş | Kanıt | Bu katmandaki bağlayıcı |
|---|---|---|
| `0x180001000` | `0x180001024/1050/107c` üç detour adresini yükler; trampoline global'leri `0x1800072c8/72c0/72d0`; `0x1800010ad` sonuç 1 | `application_dll_main` → `Core::dll_main` |
| `0x180001229` | `0x18000123e` özgün çağrı; `0x180001251` PID alanını karşılaştırır; `0x180001256` parent PID alanını yazar; `0x180001259` özgün BOOL'u geri koyar | `process_next` → `Core::process_next` |
| `0x180001139` | `0x180001146/1162` once durumları; `0x18000116c/117b` özgün mutex fonksiyonu | `mutex_trylock` → `Core::mutex_trylock` |
| `0x180001264` | `0x180001513` seçilen yolu değiştirir; `0x18000153a` özgün pointer; `0x18000154a–1552` ilk dört register argümanı; `0x180001555–1565` stack'i eski haline getirip `JMP RAX` | `crypt_query` → `Core::crypt_query` |

CryptQuery decompiler imzasında yalnız dört argüman görünür. Tail jump özgün stack argümanlarını da taşır. Bağlayıcı, kurulu LLVM-MinGW `wincrypt.h:4755` prototipini `decltype(&::CryptQueryObject)` ile kullanır; Process32NextW için `tlhelp32.h:78` kullanılır. `static_assert` dört callback pointer'ının beklenen ABI türüne dönüştürülebildiğini doğrular. Bu kontrol Windows x64 tür uyuşmasını gösterir; özgün derleyici veya MSVC exception ABI eşitliği göstermez.

## Açık ömür ve hata sözleşmesi

Önce süreç ömrü boyunca yaşayan somut bir `Errors` politikası hazırlanır. `Binding(Core&, Errors&)` aynı anda yalnız bir Core'u **tüm thread'ler için** yayınlar. `thread_local`, örtülü Core veya başarı döndüren hata varsayılanı yoktur. Politika ilk kez kurulduktan sonra değiştirilemez; aynı nesneyi tekrar kurmak idempotent'tir. Core/Host/HookEngine bağlamı Binding'den ve bütün çağrılardan uzun yaşamalıdır. Original fonksiyon pointer'ları ve diğer dış durum, eşzamanlı callback'ler çalışırken değiştirilemez.

İkinci Binding, başka Errors nesnesi veya aktif çağrı varken `close()` denemesi zorunlu politikaya reddedilir. Reddedilen `close()` mevcut bağlamı korur. Çağrılar bittikten sonra close tekrar denenebilir; başarılı close tekrarları idempotent'tir. Kapanmış eski Binding'in sonradan yok edilmesi yeni Binding'i temizlemez. Yapıcılar ve close'u yöneten thread Core'u kapatmadan önce kendi callback girişlerini durdurmalı ve kullanıcı thread'lerini birleştirmelidir. Close hook kapatmaz ve trampoline serbest bırakmaz. Devam eden bir çağrıdan reentrant close bekleme yapmaz; CallsInFlight olarak reddedilir. Aynı Binding üzerinde close/destructor işlemlerini tek kontrol thread'i yürütmelidir.

Callback girişinde kısa bir süreç global kilidi altında aktif çağrı sayısı tutulur; bu kilit Core veya kullanıcı hata politikası çağrılırken tutulmaz. C++ istisnası callback ABI'sinden dışarı çıkmaz: lease temizlendikten sonra `Errors::escaped_exception(exception_ptr)` çağrılır. Bu politika `noexcept` ve `noreturn` olmak zorundadır. Üretim politikası süreci durdurmalı/trap etmelidir; API başarısına dönemez. Testlerde yalnız Binding API reddini incelemek için `reject` istisna fırlatır. Bir callback henüz hiçbir Errors kurulmadan çağrılırsa açık sözleşme ihlali `std::terminate` ile biter; Errors kurulduktan sonraki unbound callback zorunlu Errors üzerinden durdurulur. Gerçek MSVC/SEH/unwind davranışı henüz kurtarılmamıştır.

Core'a bağlanacak `Bindings` yapısı `with_detours(target_process, target_mutex, target_crypt)` ile hazırlanabilir. Hedef pointer'lar caller girdisidir. Bu fonksiyon gerçek import çözmez, msvcp140 yüklemez ve Win32 API çağırmaz. `application_dll_main` serbest fonksiyonu doğrudan SDK pointer'ıyla çağrılabilir; PE export/CRT giriş noktası olarak kurulmamıştır. MinHook motoruna gerçek bağlantı ve loader-lock güvenli tam DLL yaşam döngüsü ayrıca gereklidir.

## Derleme ve doğrulama

`build-and-test.ps1`, mevcut LLVM-MinGW clang 23.1.3 ile `-std=c++20 -O2 -Wall -Wextra -Werror` kullanarak **1 production nesnesi** derler. Statik bağlı kendi test EXE'miz **51 kontrol** ve **8 terminal hata alt süreç testi** geçirir. Komut: `& outputs\preloader-native-source\build-and-test.ps1`.

Test kapsamı: SDK fonksiyon pointer'ı üzerinden çağrılar, 11 CryptQuery argümanı ve output pointer'ları, yol değiştirme/değiştirmeme, birebir BOOL/int sonuçları, PID/parent PID, mutex aralığı/once, üç detour'un mock motora geçmesi, thread'ler arasında aynı Core, iki yapıcının yarışı, ikinci Binding/Errors reddi, aktif çağrı sırasında close reddi, kapanıştan sonra başka Core'a geçiş ve kapanmış eski nesnenin yeni bağlamı etkilememesi. Alt süreçler policy kurulmadan çağrı, dört unbound callback, escaped C++ exception, fatal rebind ve closed callback için beklenen terminal exit kodunu doğrular.

Yalnız sentetik belleğe sahip kendi test EXE'miz çalıştırıldı. Özgün veya yeniden derlenmiş DLL/oyun EXE'si yüklenmedi; gerçek hook, trampoline, CryptQueryObject, Win32 mutex veya modül API çağrısı yapılmadı. Bu test sonucu özgün DLL ile diferansiyel eşdeğerlik testi veya tam çalışır DLL kanıtı değildir. Test çıktısı `validation.txt`, nesne/test hashleri `build-record.json` içindedir.
