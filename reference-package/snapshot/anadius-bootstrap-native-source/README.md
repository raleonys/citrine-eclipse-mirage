# Bootstrap native giriş bağlantıları

Bu yeni C++ katmanı, mevcut `Bootstrap` gövdelerini gerçek yeniden derlenen fonksiyon adreslerine bağlar. Özgün dosya SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Özgün kaynak veya bağımsız replacement DLL değildir.

| Özgün VA | Bağlantı | Kanıt |
| --- | --- | --- |
| `0x180038870` | `BOOL WINAPI application_entry(HINSTANCE,DWORD,LPVOID)` → `Bootstrap::application_dll_main` | `ev_0a232e746d99beddba99fe362a4f132625939b77bd747e39ad749975c6f32a06` |
| `0x180038970` | Mevcut initialize gövdesi → `ThreadBridge` → mevcut `05e8e0` CRT core | `ev_77887ea2e7b744e62e52eaf81cb8a07abf2a4f50498001ba81b2fcf2496e8789` |
| `0x1800388a0` | `unsigned __stdcall no_argument_entry(void*)` | `ev_3022378b3b333555b2f1f63fa59a85ea675e756369871f825e3fd805143160eb` |
| `0x1800388d0` | `unsigned __stdcall boolean_entry(void*)` | `ev_220d092f92df6b8c1c7cbb91ffd4ee02e5b5af3f3f6fd15d599e710c8adf353e` |
| `0x180038950` | `void exit_entry()` | `ev_9e5bed8eb620590b20a44651d47fd66e0d4d00f7cc3caf952f79bc4a73f22049` |
| `0x180038890` | `noexcept terminate_entry()` | `ev_a73f817561c1dc663846bb56938e55cea83e98470e9af8c91186939f6bbbe144` |

`Resolver` thread/exit/terminate/uygulama girişlerinin derlenen adreslerini verir; tercih edilen image adreslerini yürütülebilir pointer olarak kullanmaz. Thread giriş tipi, kurulu MinGW `process.h` içindeki `_beginthreadex_proc_type` ile compile-time eşitlik kontrolünden geçer. Bu girişler `CreateThread` girişindeki CRT dış sarmalayıcısının yerine geçmez. Abstract `ThreadBridge`, zaten kurtarılan `BootstrapThreadBridge` üzerinden `begin_thread_05e8e0` işlevine bağlanır; CRT platform etkileri ve diğer bütün uygulama bağımlılıkları zorunlu kalır. `register_exit`/`install_terminate` uygulamasını üst katman sağlamalıdır; çözümleyici bunları CRT'ye kendisi kaydetmez.

`Binding` tek image için süreç genelinde açık bir Bootstrap bağlar. Girişler kısa kilit altında çağrı sayısını ve o kabulün hata politikasını yakalar. `close` çalışan giriş varken `Busy` ile reddedilir; kapanma beklemez. Bookkeeping, çağıranın `LastError` değerini korur; gerçek callback içindeki değişiklikler korunur. Hata politikası lease bırakıldıktan sonra başka bir binding kurulsa bile kabul edildiği binding'den alınır. Bootstrap ve bütün bağımlılıkları başarılı close'a kadar, Errors ise sonraki giriş denemeleri dahil gerekli süre boyunca yaşamalıdır. Hook/CRT kayıtları kapatılıp dış girişlerde quiescence sağlanmadan close/rebind yapılmamalıdır. Binding yönetimi tek kontrol thread'ine aittir; destructor sırasında Busy/lifecycle istisnası `noexcept` nedeniyle terminaldir.

Bu yeni sınır `noexcept` girişlerden kaçan C++ istisnalarını zorunlu `Errors::callback_exception` politikasına iletir; başarı kodu üretmez. İlk binding öncesindeki giriş için politika yoktur ve `std::terminate` çağrılır. Bu tercih özgün MSVC CRT/SEH/unwind davranışını kanıtlamaz. Null/geçersiz payload, geçersiz fonksiyon pointer'ı veya bağımlılık ömrü ihlali için bellek güvenliği kurtarma iddiası yoktur. Gerçek DLL entry export'u, loader lock, CRT başlangıç/çıkış sırası, atexit yaşam süresi ve asli PTD/FLS temizliği bu modülde uygulanmaz.

`build.ps1`, bir production object ve statik bağlı kendi test EXE'sini derler. 70 normal kontrol: enum/adres eşlemesi; initialize→üç CRT Context→yeniden derlenen payload girişleri; synchronous bool(true) sırası; 8/8/16-byte payload cleanup/delete; exit cleanup sırası; LastError; geçersiz enum; duplicate/rebinding; yalnız bize ait sentetik worker üzerinde Busy-close. Dört ayrı terminal süreç vakası beklenen 84/83/83/83 exit kodlarını doğrular. Testteki CRT Effects ve uygulama bağımlılıkları mock'tur. Özgün/yeniden oluşturulan DLL veya oyun çalıştırılmaz; native modül/hook/registry/network/provider işlemi yapılmaz. `validation.json` komutları ve sonuçları kaydeder.

Yaşam süresi ayrıntısı: active_calls()==0, henüz girişe ulaşmamış veya suspended worker bulunmadığını kanıtlamaz. close/rebind öncesinde bunlar dahil bütün gelecekteki giriş kaynakları dışarıdan durdurulmalı veya join edilmelidir.
