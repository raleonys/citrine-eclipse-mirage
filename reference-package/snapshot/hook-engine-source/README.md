# Hook motoru: seçilen kaynak bağımlılığı ve C++ bağlayıcıları

Bu klasör gerçek MinHook motor kaynaklarını ve kurtarılan modüllere C++ bağlayıcılarını içerir. **Vendor kodu geri kazanılmış özgün kaynak değildir.** Resmî MinHook 1.3.4 commit `c3fcafdc10146beb5919319d0683e44e3c30d537`, açıkça seçilmiş uyumluluk bağımlılığıdır. Binary'nin kesin MinHook sürümü veya tam davranış eşitliği kanıtlanmadı.

Kaynak: [resmî MinHook repository](https://github.com/TsudaKageyu/minhook/tree/c3fcafdc10146beb5919319d0683e44e3c30d537). Dört C birimi `hook.c`, `buffer.c`, `trampoline.c`, `hde64.c` değiştirilmeden alındı; başlıklar ve tam `LICENSE.txt` dahil on bir dosyanın URL/boyut/SHA-256 kaydı `upstream-provenance.json` içindedir. Lisans bildirimleri kaynakta ve pakette korunur.

## Binary kanıtıyla ilişki

- `anadius-comparison.md/json`: anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için 21 fonksiyonun statik eşlemesi.
- `preloader-comparison.md/json`: gömülü preloader SHA-256 `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846` için 19 fonksiyonun ayrı adres alanındaki eşlemesi.
- 56 bayt hook kaydı, bayraklar, 64 bayt slotlar, 4 KiB bloklar, ±1 GiB arama, 5/7 bayt patch, 14/16 bayt mutlak dallar, hata kodları ve thread/IP yolları incelendi. Tek hedef disable'ın `Freeze(action=1)` kullanması her iki binary ve seçilen kaynakta aynı gözlenen ayrıntıdır.
- Her iki binary'deki HDE64 tablosunun 529 baytı resmî tabloyla aynıdır: SHA-256 `a35a7be4500f9100d6e014c666513cbb7ba52e4f7010eb230baefb638eac5033`. anadius VA `0x1800a1140`/raw `0x9f740`; preloader VA `0x180007020`/raw `0x4a20`.

Bu eşleşmeler incelenen yolların yapısal uyumu için kanıttır; bütün sürüm/derleme seçenekleri, eşzamanlılık ve opcode alanı için eşdeğerlik kanıtı değildir. Vendor'daki Remove/Queue/Apply/Status gibi ek API'ler özgün DLL'de aynı kapsamda kurtarılmış ilan edilmez.

## C++ bağlantıları

`api.hpp/cpp`: `NativeApi`, seçilmiş gerçek `MH_Initialize/CreateHook/EnableHook/DisableHook/Uninitialize` API'lerini çağırır. Gerçek uygulamada bellek koruması, patch ve thread işlemleri yapar. Bu incelemede derlendi/linklendi; metotları çağrılmadı.

`adapters.hpp/cpp`: `HostInitEffects` kurtarılmış host hazırlığının engine çağrılarını `Api`'ye yönlendirir; log/yol/OS sürüm/modül/simge sınırları çağırandan gelir. `CallbackResolver`, yeniden yazılmış callback'lere giden ABI uyumlu serbest wrapper adreslerini sağlamalıdır. `callback-bindings.json` içindeki tercih edilen özgün VA'lar çalıştırılacak callback değildir. Pointer çıktısı her normal dönüşte, başarısızlık dahil, integer original slotuna geri aktarılır; integer belleğe `void**` alias yapılmaz. Seçilen C API normal dönüş sözleşmesidir; C++ exception/SEH sırasında çıktı aktarımı eşdeğerliği ileri sürülmez. `CleanupEffects` mevcut `cleanup_hooks_0102b0` gövdesini disable/uninitialize API'lerine bağlar.

`preloader_engine.cpp`: `PreloaderEngine`, mevcut `Core` için initialize/create/enable sağlayıcısıdır. Core'un bir defalık başlangıcı, 0/1/-2 kabulü, create/enable hata davranışları ve üç hedef sırası değişmez. Seçilen NativeApi normal MinHook kodlarını üretir; -2 bir upstream başarı kodu olarak icat edilmez.

MinHook motor durumu linklenen görüntüye aittir. İki bağımsız DLL üretildiğinde **her DLL kendi statik motor kopyasını** linklemelidir. Ortak tek EXE'ye bağlanan kopyayı iki ayrı özgün motorun izolasyonu gibi kullanmak eşdeğer değildir. Adapter'lar otomatik cleanup/uninitialize yapmaz; özgün yaşam sırasını çağıran kurmalıdır.

## Derleme ve doğrulama

Çalışma alanı kökünden `outputs/source-workspace/build.ps1` bütün projeyi kurar. 46 C++ ve 4 C kaynak nesnesi vardır. `anadius_core_partial.a` ve `preloader_core_partial.a` kısmi kaynak arşivleridir; `hook_engine_compat.a` seçilen motor/NativeApi arşividir. C17/C++20, O2, Wall/Wextra/Werror kullanılır. CMake tanımı sağlanır, ayrıca çalıştırılmış sayılmaz.

- Adapter testi: **472 kontrol**. Gerçek host-init/path/cleanup ve preloader Core, fake Api üzerinden birlikte doğrulanır. NativeApi nesnesinin yalnız oluşturma/RTTI/yıkımı gerçek motorun linkini doğrular; hiçbir native hook metodu çağrılmaz.
- `build-fixture-tests.ps1`: **379 kontrol**. Dört gerçek upstream C birimi, yalnız test derlemesinde `testing/fake_win32.h` force-include edilerek yeniden derlenir. Bütün Win32 call site'ları `ReaTest*` işlevlerine gider; test C nesnelerinde `__imp_` kalırsa build durur. Test EXE'si native motor nesnelerine linklenmez.
- Gerçek HDE64, buffer/engine/trampoline algoritmaları sahip olunan veri tamponlarında çalışır. 5/7 bayt patch/restore, mutlak relay, RIP/CALL/Jcc/JMP taşınması, 80 hedefle kapasite büyümesi, fake thread/IP/hata ve modül yolları denetlenir. Üretilen komutlar yalnız okunur; çağrılmaz veya emüle edilmez. Fake VirtualAlloc, gerçek OS adres rezervasyonu/protection/arama başarısını kanıtlamaz.

Komutlar, nesne/executable hashleri ve sınırlar `fixture-validation.json`, `testing/README.md`, ortak `build-record.json` ve `common-validation.json` içindedir. Bağımsız kaynak/harness incelemesinde düzeltme gerektiren bulgu bulunmadı. Özgün/yeniden derlenen DLL veya oyun yüklenmedi, gerçek hook/thread suspend/protection/registry/socket işlemi yapılmadı.

Tam DLL için SDK shim'lerin gerçek app/CRT/ASIO/provider/global başlangıcı ve loader/shutdown bağları ve preloader'ın özgün gömülü makine kodunun kaldırılması hâlâ tamamlanmalıdır. Gerçek motor kaynağı eklenmesi bu işleri veya çalışma zamanı eşdeğerliğini kendiliğinden çözmez.

