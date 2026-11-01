# CRT thread başlatma ve çıkış çekirdeği

`threads.cpp`, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için kayıtlı REA/Ghidra assembly ve pseudocode kanıtından yeniden yazıldı. Özgün kaynak veya tam MSVC ABI'si değildir. `evidence.json` adresleri ve Evidence kimliklerini içerir.

| Özgün VA | Kaynak yordamı | Korunan davranış |
|---|---|---|
| `0x18005e87c` | `make_context_05e87c` | calloc(1,40), gözlenen free(nullptr), entry/argument, GetModuleHandleExW(4,entry), yok sayılan sonuç |
| `0x18005e8e0` | `begin_thread_05e8e0` | Null entry/error-slot/invalid-handler, CreateThread sarmalayıcısı, hata eşleme ve normal sahiplik aktarımı |
| `0x18005e7a8` | `thread_entry_05e7a8` | PTD+0x3c0, policy2 runtime başlangıcı, uygulama callback'i, sonucu thread çıkışına aktarma |
| `0x18005e818` | `common_end_thread_05e818` | Runtime temizliği, handle/module için 0 ve -1 sentinel'ları, FreeLibraryAndExitThread veya ExitThread |
| `0x18005e9bc` | `end_thread_05e9bc` | Geri dönmeyen çıkış çağrısı; özgün son INT3 normalde ulaşılamaz |

40 baytlık context'te `entry` +0, `argument` +8, `handle` +16, `module` +24 ve runtime bayrağı +32 adreslerindedir. Başarılı CreateThread dönüşündeki handle context'e yazılmaz. Başarıdan sonra context okunmaz; worker o anda bitmiş olabilir. Başarısızlıkta yalnız CRT context'i temizlenir, uygulamanın verdiği payload bu yordam tarafından serbest bırakılmaz. Hata temizliği her sıfır olmayan handle/module değerini kullanır; çıkış yolu ise ayrıca -1'i dışlar. Bu farklılık korunur.

`BootstrapThreadBridge`, önceki `BootstrapDependencies::begin_thread_05e8e0` sınırını bu gövdeye bağlayan **yeni entegrasyon** sınıfıdır. Caller, kendi kaynak trampolinlerinin kimliğini `bind_application_entry` ile sağlar. Sayısal özgün binary adresleri çalıştırılacak callback olarak kullanılmaz. Diğer bootstrap bağımlılıkları soyut kalır.

Derleme:

```powershell
& '.\outputs\anadius-thread-source\build.ps1'
```

655 sentetik kontrol; başarısız tahsis/başlatma, ID'nin yalnız başarıda yazılması, modül lookup başarısızlığının yok sayılması, -1/-2 ve yüksek bitli handle'lar, runtime policy/sonuç kombinasyonları, exception akışı, CreateThread dönmeden context'in yok olması ve gerçek kurtarılmış bootstrap ile iki katmanlı payload aktarımını kapsar. Testte callback'ler ve çıkış API'leri kontrollü arayüzlerdir. OS thread, DLL, oyun veya provider başlatılmaz.

Devam modülleri: `anadius-crt-error-source` hata alanlarını ve 45 OS hata girdisini; `anadius-crt-runtime-source` runtime/policy/resolver gövdelerini kurtarır. `anadius-thread-native-source` SDK çağrı noktalarını bunlara, `anadius-bootstrap-native-source` uygulama giriş adreslerini bu çekirdeğe bağlar. Bu kaynak bileşimleri ayrıca sentetik test edilir; bu klasörün 655 kontrolü tarihsel çekirdek grubudur.

Tamamlanmayan sınırlar: özgün CRT heap/PTD/FLS yaşam süresi, gerçek loader/PEB/provider ve cookie/cache başlangıcı, CRT kayıt ve wrapper bağının bütün pending/suspended worker'lar boyunca korunması, CFG/XFG, EH/SEH ve exception geçiş ABI'si. Worker exception'ı için gözlenmeyen otomatik cleanup eklenmez. ExitThread/FreeLibraryAndExitThread `[[noreturn]]` sözleşmesidir; bunların geri dönmesi desteklenen girdi alanında değildir. Context'in çıkışta explicit free edilmesi veya PTD slotunun temizlenmesi bu beş yordamda gözlenmediğinden burada icat edilmez; CRT yaşam süresi ayrı sınırdır. Test başarısı çalışma zamanı eşdeğerliği değildir.
