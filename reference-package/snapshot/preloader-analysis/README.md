# version.dll içindeki gömülü preloader: statik analiz

Bu inceleme, `version-reconstruction-input.json` içindeki `embedded.bytes_hex` alanının **21.504 baytını** analiz eder. ZIP açılmadı; orijinal DLL değiştirilmedi veya çalıştırılmadı. Analiz kopyası ayrı bir Ghidra projesine alındı. Aşağıdaki adresler gömülü PE'nin tercih ettiği `0x180000000` tabanına aittir; dış `version.dll` adres alanıyla karıştırılmamalıdır.

## Kanıt kimliği ve kapsam

- Kaynak REA Evidence: `ev_157c446e2c1bce1a022bed8f576a7cc535fa15c41a6478fc2ef751d2902f97a5`.
- Dış dosyadaki başlangıç: VA `0x180006d80`, RVA `0x6d80`; uzunluk `0x5400`.
- Gömülü baytların SHA-256 özeti: `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846`.
- `LOCAL-PE-PRELOADER-1`: [pe-layout.json](pe-layout.json), bağımsız PE başlık/import/export/CodeView ayrıştırması.
- `LOCAL-GHIDRA-PRELOADER-1`: [ghidra-analysis.json](ghidra-analysis.json), Ghidra 12.1.4 headless; her fonksiyon için assembly, başvuru adresleri, sözde kod, çağıranlar ve çağrılanlar. Bu kayıt **REA tarafından üretilmiş Evidence değildir**.
- [all-functions.c](all-functions.c): 107 fonksiyonun Ghidra sözde kodu. Dosya uzantısı derlenebilir kaynak olduğu anlamına gelmez. Orijinal kaynak değildir; bilinmeyen türler ve decompiler uyarıları korunmuştur.
- [function-inventory.csv](function-inventory.csv): 107 fonksiyonun tam adres ve çağrı envanteri; bunların 16'sı thunk, 91'i thunk olmayan fonksiyondur. Derleme zamanı kitaplık yordamları bu sayıya dahildir.

REA için verilen yerel CLI yolu bulunamadı (`MODULE_NOT_FOUND`); paylaşılan REA MCP hedefi değiştirilmedi. Kurulu Ghidra doğrudan, bağımsız headless süreçte kullanıldı. İlk başlatma varsayılan AppData klasörünü oluşturamadı. Ghidra'nın belgelenmiş süreç ayarlarıyla yalnızca bu analizin ayar/önbellek klasörleri çalışma alanına yönlendirildi; kurulum ve kullanıcı yapılandırması düzenlenmedi. Ghidra işlem günlüğü [ghidra-headless.log](ghidra-headless.log) içindedir.

## PE yapısı

AMD64 PE32+ DLL; tercih edilen taban `0x180000000`, giriş RVA `0x3338`, görüntü boyutu `0xa000`. Export tablosunun DLL adı `preloader_l.dll`. Tek adlandırılmış export: ordinal **1**, `DllMain`, RVA **0x1000**, VA **0x180001000**. CodeView RSDS kaydı dosya ofseti `0x3bfc`'de `preloader_l.pdb` adını taşır; PDB dosyası mevcut değildir.

| Bölüm | RVA | Sanal boyut | Dosya ofseti | Ham boyut | Özellikler |
|---|---:|---:|---:|---:|---|
| .text | 0x1000 | 0x30eb | 0x400 | 0x3200 | R/X |
| .rdata | 0x5000 | 0x1388 | 0x3600 | 0x1400 | R |
| .data | 0x7000 | 0x900 | 0x4a00 | 0x400 | R/W |
| .pdata | 0x8000 | 0x390 | 0x4e00 | 0x400 | R |
| .reloc | 0x9000 | 0x44 | 0x5200 | 0x200 | R, discardable |

57 import, 6 kitaplık vardır. Tam isim/ordinal/IAT adresleri `pe-layout.json` içindedir:

| Kitaplık | Import sayısı | İlgili API örnekleri |
|---|---:|---|
| KERNEL32.dll | 39 | Process32NextW, VirtualProtect, VirtualAlloc, FlushInstructionCache, SuspendThread, GetThreadContext, SetThreadContext |
| CRYPT32.dll | 1 | CryptQueryObject |
| api-ms-win-core-synch-l1-2-0.dll | 2 | WaitOnAddress, WakeByAddressAll |
| msvcp140.dll | 2 | _Mtx_lock, _Mtx_trylock |
| VCRUNTIME140.dll | 5 | memcpy, memcmp, memset, __C_specific_handler, __std_type_info_destroy_list |
| api-ms-win-crt-runtime-l1-1-0.dll | 8 | _initterm, _initterm_e, _cexit, onexit/environment yordamları |

Bu PE'nin TLS dizini RVA ve boyutu sıfırdır. Headless günlüğündeki sistem kitaplığı yükleme iletileri hedef DLL'nin çalıştırıldığı anlamına gelmez; Ghidra ithal API sembollerini çözmek için bağımlılık dosyalarını statik okumuştur.

## Gerçek DllMain ve üç yönlendirme

Giriş zinciri statik çağrı kanıtıyla şöyledir:

`0x180003338 entry -> 0x180003210 CRT dispatcher -> 0x180001000 DllMain`.

`entry`, process attach için önce güvenlik çerezi yordamı `0x180003430`'u çağırır. `0x180003210` CRT başlatma/sonlandırmayı yönetir. İşlevsel DLL gövdesi `0x180001000`'dedir; giriş RVA `0x3338` değildir.

DllMain yalnızca neden `1` (process attach) olduğunda çalışır: `DisableThreadLibraryCalls`, üç hook oluşturma girişimi, ardından tüm hook'ları etkinleştirme. Oluşturma başarısında özgün fonksiyona dönüş trampoline adresi global alana kaydedilir. Hook oluşturma hataları bu gövdede DLL yüklemesini başarısız kılmaz; DllMain `1` döndürür.

| Hedef | IAT VA | Yerine geçen fonksiyon | Özgün trampoline globali | DllMain oluşturma çağrısı |
|---|---:|---:|---:|---:|
| Process32NextW | 0x180005028 | 0x180001229 | 0x1800072c8 | 0x180001033 |
| _Mtx_trylock | 0x1800051e8 | 0x180001139 | 0x1800072c0 | 0x18000105f |
| CryptQueryObject | 0x180005000 | 0x180001264 | 0x1800072d0 | 0x18000108b |

Bunlar yalnız IAT girişlerini değiştiren bir mekanizma değildir. Oluşturucu, IAT'den okunan **fonksiyon adreslerini** alır; patch uygulayıcı bu fonksiyonların kod başlangıcına jump yazar.

### Process32NextW: süreç üst kimliğinin değiştirilmesi

`0x180001229`, önce `0x1800072c8`'deki özgün fonksiyonu çağırır (`0x18000123e`). Başarı ve dolu çıktı yapısı koşuluyla `GetCurrentProcessId` (`0x18000124b`) sonucunu çıktı yapısının `+0x08` alanıyla karşılaştırır (`0x180001251`). Eşitse aynı kimliği `+0x20` alanına yazar (`0x180001256`).

Windows x64 `PROCESSENTRY32W` yerleşimiyle bu, mevcut süreç kaydının `th32ParentProcessID` alanını kendi süreç kimliğine çevirmektir. Genel bir süreç gizleme iddiası kurulamaz; gözlenen kod çıktı kaydındaki tek alanı koşullu değiştirir.

### _Mtx_trylock: ana modül içindeki mutex için bir kez _Mtx_lock

`0x180001139` her durumda özgün `_Mtx_trylock` trampoline'ine gider (`0x18000117b`). Öncesinde, mutex adresi ana görüntünün `[base, base + SizeOfImage)` aralığındaysa bir defalık yardımcı yol çağrılabilir.

Aralık yardımcısı `0x180001201`: `GetModuleHandleA(NULL)` çağrısı `0x180001207`; `e_lfanew` okuması `0x180001212`; NT header `+0x50` alanından `SizeOfImage` okuması `0x180001216`; son adresi hesaplama `0x18000121a`. Bu yordam RAX/RDX çiftiyle aralık döndürür. Ghidra'nın `void` olarak çıkardığı sözde kod imzası burada eksiktir; assembly belirleyicidir.

Bir defalık yürütücü `0x180003f8f`, `WaitOnAddress`/`WakeByAddressAll` kullanır ve callback'i `param_4 + 0x20`'den çağırır. `0x180005270` tablosunun `+0x20` girdisi `0x1800010b8`'dir; bu yordam `_Mtx_lock` IAT'sine (`0x1800051e0`) `0x1800010ca`'da tail jump yapar. İlgili durum globalleri `0x180007010`, `0x180007018` ve `0x18000701c`'dir. Bunun hangi üst seviye oyunsal durumu etkilediği mevcut statik kanıttan belirlenmemiştir.

### CryptQueryObject: sorgulanan dosya adının yönlendirilmesi

`0x180001264`, ilk argüman `1` olduğunda dosya yolu türünü işler (`0x18000127f`). UTF-16 girdiyi bayt gösterimine çeviren yardımcı döngülerden sonra yol sonundaki 15 baytı `preloader_l.dll` metniyle karşılaştırır (`0x1800014f8` string yüklemesi, `0x180001505` memcmp). Bu karşılaştırma son ek karşılaştırmasıdır; yol ayracı veya tam basename eşitliği ayrıca doğrulanmaz ve büyük/küçük harf normalizasyonu görünmez.

Eşleşmede ikinci argümanı `EAAntiCheat.GameServiceLauncher.dll` yapar (`0x18000150c`). Kaynak yol `NULL` olduğunda da aynı ikame stringine gider (`0x1800012d0`). İkame UTF-16 stringi `0x1800052d0`, eşleştirilen ASCII string `0x1800052c0` adresindedir. Sonunda özgün `CryptQueryObject` trampoline'ine `0x180001565`'te tail jump yapılır. Kalan stack argümanları bu çıkışta korunur; Ghidra'nın gösterdiği yalnız dört parametreli prototip API'nin tam imzası değildir.

**Gözlem:** farklı bir dosyaya kriptografik nesne sorgusu yönlendirilir. **Çıkarım:** bu, preloader hakkında yapılacak imza/sertifika incelemesinin başka DLL'ye yöneltilmesini amaçlıyor olabilir. Bu statik inceleme imza doğrulamasının başarıya ulaştığını veya belirli bir anti-cheat korumasının atlatıldığını gözlemlemez.

## Hook altyapısının önemli çağrıları

| Adres | Kanıtlanan görev / çağrılar |
|---|---|
| 0x1800016d3 | Bir defalık altyapı başlatma; callback `0x180005318 + 0x20 -> 0x180001727 -> 0x180002088` |
| 0x180002088 | Özel heap oluşturma (`HeapCreate`) ve durum hazırlama |
| 0x180001756 | Hook oluşturucuyu çağırma; hata/başarı ve trampoline sonucunu sarmalama |
| 0x180001ec4 | Yürütülebilir adres denetimi, tekrar hook denetimi, trampoline oluşturma, eski 5/7 baytı saklama |
| 0x18000259c | `VirtualQuery` ile yürütülebilir bellek uygunluğu denetimi |
| 0x1800022fc / 0x180002388 | Yakın adres alanında trampoline bloğu ayırma; `VirtualQuery`, `GetSystemInfo`, `VirtualAlloc` |
| 0x1800025d0 | Başlangıç talimatlarını kopyalama/yeniden konumlama; RIP-relative, CALL/JMP/koşullu dal ayrımları |
| 0x180002974 | x86-64 talimat uzunluğu/alan çözümleyicisi |
| 0x18000207c | Tümünü etkinleştirme: `0x180001a98(0,1)` |
| 0x180001a98 / 0x1800019d4 | Tekil/tüm hook durumunu değiştirme; thread dondurma, patch, thread devam ettirme |
| 0x180001cdc | Geçerli sürecin diğer thread'lerini listeleme ve `SuspendThread`; `0x1800020e4` ile context düzeltme |
| 0x1800020e4 | `GetThreadContext`/`SetThreadContext` ile talimat işaretçisini trampoline karşılığına taşıma |
| 0x180001b78 | Kod patch'ini uygulama/geri alma; `VirtualProtect`, kod yazımı, `FlushInstructionCache` |
| 0x180002280 | Dondurulan thread'lerde `ResumeThread` ve geçici listeyi serbest bırakma |

Patch uygulayıcıda `0x180001bf0`'da `E9` yakın jump opcode'u yazılır. Gerekli sıcak patch düzeninde `0x180001c0b`'de `EB F9` kısa jump yazılır. Yordam önce korumayı `0x40`'a çevirir (`0x180001bd8`), eski korumayı geri yükler (`0x180001c44`) ve instruction cache'i temizler (`0x180001c59`). Bu mekanizmanın yürütülmesi incelenmemiştir; burada yalnız statik kod davranışı tarif edilir.

## Kaynağa dönüşüm sınırları

107 tanımlanmış fonksiyon için decompiler metni üretildi; açık `decompile failed` sonucu yoktur. Bu, tüm orijinal fonksiyonların bulunduğunu, türlerin doğru olduğunu veya yeniden derlenebilir bir proje elde edildiğini göstermez. İndirekt çağrılar, özel ABI ile dönen değerler, bilinmeyen global yapı türleri, CRT/enstrümantasyon ve kaynak adları ayrı sorunlardır. Trampoline kodunun başka bir kitaplığın kaynak sürümüyle birebir eşleşmesi doğrulanmadı. Tam C++ kaynak iddiasında bulunulmuyor.

Özellikle `0x180001201`'in iki register ile dönüşü, CryptQueryObject'un tam prototipi ve bir defalık callback yapılarının türleri doğrulanmadan sözde kodu C++ olarak derlemek davranışı bozabilir. Kanıt, dış yükleyicinin gömülü DLL'yi nasıl kullandığıyla birleştirilmeye uygundur; üst seviye işlev adları ve paket/oyun sürümü ilişkisi ayrıca bilinmemektedir.

Sonuçların tamamı statiktir. Kaynak binary çalıştırılmadı; ağ bağlantısı, gerçek thread patch'i veya gerçek sertifika sorgusu yapılmadı. Analiz kopyasının son SHA-256 değeri başlangıçla eşleşir.
