# preloader_l.dll çekirdeğinin C++ yeniden yazımı

`preloader_core.hpp` ve `preloader_core.cpp`, statik kanıtla kurtarılmış işlevsel çekirdeği okunabilir, türleri belirtilmiş C++20 koduna çevirir. **Nesne dosyası derlenmiştir; eksiksiz veya davranışı birebir aynı bağımsız DLL olduğu iddia edilmez.** Hook motoru ve gerçek sistem API bağlayıcıları zorunlu arayüzlerdir; seçilen motor kaynağı ve PreloaderEngine uygulaması sonradan ../hook-engine-source içinde eklenmiştir; başarı döndüren sahte üretim implementasyonu yoktur.

Kanıt kaynağı: gömülü 21.504 baytlık PE, SHA-256 `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846`; önceki `preloader-analysis/ghidra-analysis.json` ve `pe-layout.json`. REA kaynak kaydı `ev_157c446e2c1bce1a022bed8f576a7cc535fa15c41a6478fc2ef751d2902f97a5`; yeni C++ kodu REA tarafından üretilmiş Evidence değildir.

## Kurtarılan kaynak kapsamı

| Kod | Binary adresi | Korunan ayrıntı |
|---|---|---|
| `Core::dll_main` | 0x180001000 | Attach dışındaki nedenlerde 1; thread bildirimlerini kapatma; sıralı üç hook girişimi; oluşturma hatalarını göz ardı etme; tümünü enable etme |
| `Core::process_next` | 0x180001229 | Özgün BOOL sonucunu koruma; yalnız geçerli kaydın PID'si mevcut PID olduğunda parent PID alanına yazma |
| `Core::mutex_trylock` | 0x180001139 | Ana modülün yarı açık adres aralığı; ilk uygun mutex için bir defa `_Mtx_lock`; ardından özgün `_Mtx_trylock` |
| Ana görüntü aralığı | 0x180001201 | `GetModuleHandleA(NULL)`, `e_lfanew`, `SizeOfImage`; RAX/RDX çift dönüşünü açık iki alanla modelleme |
| `Core::crypt_query` | 0x180001264 | WinCrypt prototipindeki 11 argümanın tamamını geçirme; dosya dışı object türünü değiştirmeme |
| Yol dönüşümü | 0x180001299–0x1800015fd | UTF-16 birim sayısıyla ilk tahsis; WTF-8 üretimi; kapasite büyümesi; bozuk surrogate için dönüşüm bittikten sonra trap |
| `Once::call` | 0x180003f8f | Durumlar 0/1/2/3, waiter biti4, CAS, bekleme, callback, XCHG, wake |
| Engine başlatma | 0x1800016d3 / 0x180001727 | Bir defalık çağrı; 0, 1 ve assembly'deki sentinele eşit literal -2 sonucunu kabul etme; diğer sonuçlarda trap |

Yol eşleştirmesi son 15 UTF-8 baytta, büyük/küçük harfe duyarlı olarak yapılır. `xpreloader_l.dll` de eşleşir; yol ayıracı denetimi eklenmez. Dosya türü 1 ve `NULL` yol, ikame DLL adına gider; boş yol gidermez. Geçerli surrogate çifti dört bayt üretir. Eşleşmeyen ad değiştirilmez. Bozuk UTF-16 değiştirme karakterine dönüştürülmez: hedef kodun `UD2` yolu korunur. Üretim `Host::invalid_instruction` uygulaması `UD2` üretmelidir; test uygulaması yalnız o dala ulaşıldığını görmek için istisna fırlatır.

Once başlatıcıları farklı davranır: engine, poisoned durumda force kullanmaz ve trap eder. Görüntü aralığı force kullanır ama callback poison bayrağını görünce trap eder. Mutex callback'i force kullanır ve poison bayrağını göz ardı eder. Tamamlanan state0 anında döner. Çalışan state1 için waiter biti4 kurulup 32 bit karşılaştırmalı sonsuz bekleme yapılır. Callback dönüşündeki state ile XCHG sonrası waiter varsa uyandırılır. Gözlenen normal yolda otomatik C++ exception-to-poison işlemi bulunmadığından böyle bir davranış eklenmemiştir; mock callback istisnası Running durumunu bırakır.

## Eksik üretim sözleşmeleri

`Host` gerçek Win32 heap, bekleme, mutex ve modül işlemlerini sağlamalıdır. `HookEngine` gerçek initialize/create/enable davranışını sağlamalıdır. `Bindings` içindeki hedefler gerçek import fonksiyonları; detour adresleri aynı `Core` örneğine giden ABI uyumlu serbest fonksiyonlar olmalıdır. Bu bağlayıcılar ve CRT/DllMain export glue'u bu kapsamda tamamlanmamıştır. `Core::dll_main` doğrudan işletim sistemi yükleyicisine bağlı bir export değildir.

`_Mtx_lock`/`_Mtx_trylock` için mutex türü `void*` olarak opak tutulur; bellek yerleşimi uydurulmaz. `CryptQueryObject` prototipi [Microsoft'un resmi API tanımı](https://learn.microsoft.com/en-us/windows/win32/api/wincrypt/nf-wincrypt-cryptqueryobject) ve kurulu WinCrypt başlığından alınır. Üretimde aynı mutex ABI'sini sunan msvcp140 sürümü ayrıca belirlenmelidir.

Gerçek sistem API bağlayıcısı olmadığı için bu çekirdek tek başına çalışır preloader DLL'si üretmez. `Tail JMP` yerine C++ çağrıları, farklı prolog/epilog, vtable tabanlı bağımlılık enjeksiyonu ve farklı CRT; stack incelemesi veya ABI dışı register bağımlılığı açısından aynı binary değildir. Kaynak hata yollarında normal bellek adreslerinin okunabildiği varsayılır; erişilemez işaretçiler için ek doğrulama eklenmemiştir. Windows heap'in gerçek hata/LastError ayrıntıları, asenkron SEH, debugger müdahalesi, özel unwind ve orijinal derleyicinin kaynak dili doğrulanmamıştır.

## Hook motorunun birincil kaynak karşılaştırması

Karşılaştırma için [resmi MinHook v1.3.4 commit'i](https://github.com/TsudaKageyu/minhook/commit/c3fcafdc10146beb5919319d0683e44e3c30d537) seçildi; bu, binary'nin sürüm kimliği değildir.

[Resmi hook.c](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/hook.c) ile yapısal eşleşmeler: `0x180001ec4` CreateHook akışı; `0x180002088` initialize; `0x180001b78` enable/disable patch; `0x180001cdc` thread dondurma; `0x180002280` devam ettirme. Hedef/trampoline/backup ve bayrak alanları, 0x38 baytlık kayıt adımı, 5/7 bayt prolog yedeği, E9/EB F9 yazımı ve cache flush sırası uyumludur. Başarısız suspend edilen thread'in ID'sinin sıfırlanması da uyumludur. Bunlar MinHook ailesi için güçlü kanıt oluşturur; tüm kaynak/derleme seçenekleri veya kesin sürüm eşitliği kanıtlanmaz.

[Resmi trampoline.c](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/trampoline.c) ve [trampoline.h](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/trampoline.h), `0x1800025d0`'daki 14 bayt mutlak jump, 16 bayt mutlak call/koşullu jump ve RIP-relative taşıma şablonlarıyla uyumludur. [MinHook.h](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/include/MinHook.h) durum değerleri gözlenen 0..10 sonuçlarıyla uyumludur. Sonraki aşamada `../hook-engine-source` içine bu resmî sabit commit kaynağı, lisans/provenance ve `PreloaderEngine` adapter'i eklendi. Bu seçilmiş uyumluluk bağımlılığıdır; kurtarılmış özgün motor kaynağı veya kesin sürüm eşitliği olarak sunulmaz. Önceki 152 kontrol yalnız bu çekirdeğin tarihsel mock doğrulamasıdır; yeni ortak build motor bağlantısını ayrıca test eder. Kaynak alınacaksa [resmi lisansın tamamı](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/LICENSE.txt) korunmalıdır.

## Derleme ve doğrulama

`build-and-test.ps1`, belirtilen yerel LLVM-MinGW ile çekirdek nesnesini ve yalnız mock kullanan test EXE'sini üretir. `-ToolchainRoot` ile başka kurulu toolchain seçilebilir. Kullanılan derleyici clang 23.1.3, hedef `x86_64-w64-windows-gnu`; seçenekler `-std=c++20 -O2 -Wall -Wextra -Werror -static`.

Sonuç: **152 kontrol başarılı**. Testler tüm CryptQuery argümanlarını, Unicode/null/son ek/hata yollarını, tahsis boyutlarını, PID alanını, mutex aralığını, başlatma hatalarını, poison durumlarını ve sekiz thread'in aynı once üzerinde yarışmasını kapsar. Bunlar yeniden yazılan kodun testleridir; orijinal binary ile diferansiyel eşdeğerlik testi değildir. Test sırasında özgün DLL veya gömülü PE yüklenmedi, gerçek hook kurulmadı, sertifika API'si çağrılmadı. Derlenmiş EXE yalnız test harness'idir; oyun klasörüne konulacak DLL değildir.


