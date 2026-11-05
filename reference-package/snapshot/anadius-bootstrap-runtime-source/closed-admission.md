# Kapanmış CRT callback rotasının kilitsiz dönüşü — kaynak aşaması

`callbacks.cpp` içindeki gerçek kaynak eksikliği düzeltildi: elle kapatılmış Runtime'dan sonra CRT'de kalan `exit_dispatch` adresi, callback çalıştıramayacağı halde Hub mutex'ini alıyordu. Yeni `constinit std::atomic<bool>` kabul bayrağı false okunduğunda yönlendirici Hub'a erişmeden döner. `terminate_dispatch` aynı durumda mevcut abort politikasını Hub'a girmeden uygular; abort'un kendisi için kilitsizlik iddiası yoktur.

Bayrak yalnız mevcut Hub sahibi etkinleştirilince açılır, aynı sahibin pause/finish işlemiyle kapanır. Yayımlanmamış başka bir registry'nin kapanışı canlı sahibin kabulünü kesmez. True okumuş çağrı eski mutex altındaki state denetimlerini ve sayılan lease'i kullanır. Bu bayrak bir join/barrier değildir. Elle callback drain işlemi bayraktan bağımsızdır; pending callback'ler tüketilebilir.

İki üretim dosyası (`callbacks.cpp`, `callbacks.hpp`) ve bir senaryo kaynağı (`callbacks_tests.cpp`) değişti. Yeni senaryo etkin olmayan registry kapanışının canlı callback'i engellememesini, pause sonrasında elle drain'in çalışmasını ve owner yıkımından sonraki thunk çağrılarının LastError'ı koruyarak cleanup'ı tekrarlamamasını bekler. **Derleme, proje testi, link veya DLL çalıştırması yapılmadı. PASS sonucu yoktur.** İnceleme yazarın statik kaynak denetimidir.

Loader incelemesi hâlâ iki eksik üretim hook tanımına ulaşıyor: `require_loader_attach_runtime` ve `require_loader_detach_quiescence`. Bunlar native fonksiyon sayacı değildir. Mevcut Runtime/Domain kontrolü loader lock dışındadır. Senkron ana yolun gerçek LoadLibrary/console/hook işlemleri, kısmi attach hatası, dış sağlayıcıların kökeni ve image/CRT ömrü çözülmeden success stub eklenmedi. Başlangıç zamanı değiştirilmedi.

[Microsoft DllMain sözleşmesi](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain) loader altında yapılan işi sınırlar ve detach sırasında process termination ile FreeLibrary/başarısız load durumunu ayırır. [Microsoft DLL uygulama kuralları](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices) DllMain içinden LoadLibrary, User32 ve thread eşzamanlaması için kısıtlar belirtir. Bu belgeler kaynak tasarımının sınırıdır; hedef DLL'in davranış ölçümü değildir.

Bayrak false gözlemlemeyen, zaten ilerlemekte olan dispatcher hâlâ mutex kullanabilir. Kapanmamış süreç sonlanması çözülmüş değildir. CRT'de kalan thunk adresinin kodu yürütülene kadar image mapped kalmalıdır; bu düzeltme FreeLibrary izni vermez ve FLS/FreeLibraryAndExitThread veya gerçek CRT yıkım sırasını kanıtlamaz.

Kanıt ve güncel kaynak hash/adres satırları: `closed-admission.evidence.json`. Orijinal DLL'ler, son build kanıtları ve ZIP değiştirilmedi. Toplam DLL yüzdesi için ölçülmüş bir artış iddia edilmiyor.
