# Win32 thread API bağlantısı

`win32_effects.cpp`, mevcut `anadius-thread-source/threads.cpp` çekirdeğini SDK'nin yedi çağrı noktasına bağlayan yeni C++ entegrasyon katmanıdır. Özgün kaynak değildir. `native_api()` gerçek Win32 import adreslerini verir; testler bu adreslerin yalnız kimliğini kontrol eder, bunları çağırmaz.

| Özgün yordam | Kaynakta bağlanan çağrı |
|---|---|
| `0x18005e87c` | `GetModuleHandleExW(4,entry,&context.module)` |
| `0x18005e8e0` | `CreateThread`, başarısızlıkta `GetLastError`, `CloseHandle`, `FreeLibrary` |
| `0x18005e7a8` | Zorunlu caller tarafından sağlanan, yeniden derlenmiş thread wrapper girişi |
| `0x18005e818` | `CloseHandle`, `FreeLibraryAndExitThread` veya `ExitThread` |

`Api` alanları yerel SDK'nin tam fonksiyon tiplerini kullanır. DWORD/HMODULE çıktılarına ayrı yerel nesneler üzerinden erişilir; `uint32_t*` veya `uintptr_t*` native pointer tipi gibi dereference edilmez. Modül sonucu API FALSE döndürse bile geri kopyalanır. Thread ID yalnız başarılı HANDLE dönüşünde çekirdeğe aktarılır; başarısızlıkta çekirdeğin yerel ID'si zaten tüketilmez. Yedi API adresinden biri veya wrapper eksikse kurulum hata verir. Terminal API pointer'larının SDK `noreturn` sözleşmesi korunur; test karşılıkları exception ile kontrolü test sürücüsüne verir.

**Yaşam süresi:** constructor'ın `LPTHREAD_START_ROUTINE` parametresi uygulama payload fonksiyonu değil, yeniden derlenen `thread_entry_05e7a8` sarmalayıcısı olmalıdır. Bu sarmalayıcıya ait Effects bağını caller sağlar ve bekleyen, suspended veya çalışan bütün worker'lar gerçekten sonlanana kadar korur. Bu modül global bağ, PTD veya FLS icat etmez. `Context` aynen 40 bayttır ve `CreateThread` parametresi olarak doğrudan aktarılır. Heap/PTD/runtime/error/invoke yöntemleri soyut ve zorunlu kalır; bu katman tek başına thread runtime veya DLL değildir.

```powershell
& '.\outputs\anadius-thread-native-source\build.ps1'
```

367 sentetik kontrol: yüksek bitli pointer/handle/code değerleri, FALSE ve sıfır olmayan BOOL sonuçları, 64 bit stack aktarımı, eksik bağımlılık reddi, thread ID başarısızlık davranışı ve gerçek kurtarılmış thread çekirdeği ile wrapper→uygulama→çıkış akışı. Inline worker testte kontrollü fonksiyon çağrısıdır; `CreateThread`, module lookup/release ve thread çıkış API'leri yalnız tipli fixture'lardır. Özgün DLL/oyun yüklenmez veya çalıştırılmaz. Testlerin geçmesi özgün MSVC CRT/EH/SEH/FLS/CFG/XFG ya da çalışma zamanı eşdeğerliğini kanıtlamaz.

Adres/Evidence kimlikleri `evidence.json`, derleme ve test çıktısı `validation.json` içindedir. CRT runtime ve hata eşlemesi ayrı kaynak modülleridir; orijinal CRT heap/PTD/FLS sahipliği hâlâ ayrıca çözülmelidir.

RecoveredWin32Effects, aynı üretim biriminde beş runtime/error metodunu gerçek kurtarılmış runtime.cpp ve errors.cpp gövdelerine bağlar. Son 30 kontrol resolver/policy/runtime cache, iki başarılı thread akışı, hata eşleme ve invalid-entry yollarını birlikte sınar. Heap, PTD/FLS, invalid-parameter handler, uygulama çağrısı ve native provider etkileri zorunludur. Hata provider'ı, caller'ın diğer PTD metotları ile aynı TLS sahiplik alanını kullanmalıdır.

