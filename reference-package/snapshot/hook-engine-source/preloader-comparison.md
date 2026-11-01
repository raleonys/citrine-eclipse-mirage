# Gömülü preloader: MinHook kaynak karşılaştırması

Karşılaştırma sonucu **MinHook/HDE64 ailesiyle güçlü statik uyumdur**. Seçilen resmi MinHook 1.3.4 commit'i `c3fcafdc10146beb5919319d0683e44e3c30d537`, yeniden derleme için bir **uyumluluk bağımlılığıdır**; kayıp özgün kaynak veya kanıtlanmış özgün sürüm değildir. Yüklenmiş DLL, uygulanmış hook ya da çalışma zamanı eşdeğerliği gözlemi yoktur.

## Kimlik ve kanıt sınırı

- İncelenen PE: `version.dll` içine gömülü 21.504 bayt; SHA-256 `36e775ed97b7a41c4f0cb98b94ee8edb74fab5cfe17d5fa8cfed265228301846`.
- Adres tabanı `0x180000000`, **gömülü PE'nin ayrı adres alanıdır**; dış `version.dll` veya `anadius64.dll` adresleri değildir.
- Kök REA kaydı `ev_157c446e2c1bce1a022bed8f576a7cc535fa15c41a6478fc2ef751d2902f97a5`, gömülü baytların kökenini bağlar. Fonksiyon kanıtı `LOCAL-GHIDRA-PRELOADER-1`: `../preloader-analysis/ghidra-analysis.json`, Ghidra 12.1.4 doğrudan headless export. Bu yerel kayıt REA Evidence olarak sunulmaz.
- `all-functions.c` okunabilir arama görünümüdür; ABI/alan/argüman iddialarında JSON içindeki assembly esas alındı. JSON eşleme dosyası iki kaynağın SHA-256 kimliklerini korur.
- Upstream dosyalar ve lisans değişmeden `vendor/minhook` altında; dosya hashleri `upstream-provenance.json` içindedir.

## Fonksiyon eşlemesi

Aşağıdaki isimler özgün binary sembolleri değildir; resmi kaynakla karşılaştırmada kullanılan görev adlarıdır. [Resmi hook.c](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/hook.c) karşılıkları:

| Gömülü PE VA | Kaynak karşılığı | Gözlenen ilişki |
|---|---|---|
| `0x180002088` | `MH_Initialize` | Spinlock, özel heap, yinelenen başlatma |
| `0x180001ec4` | `MH_CreateHook` | Adres/tekrar denetimi, trampoline, kayıt, yedek |
| `0x180001930` | `AddHookEntry` | İlk kapasite32, iki kat büyüme |
| `0x180001a98` | `EnableHook` | Tek hedef veya tümü; durum hataları |
| `0x18000207c` | `MH_EnableHook` | Önceki yordamın enable=1 sarmalayıcısı |
| `0x1800019d4` | `EnableAllHooksLL` | İlk farklı kayıttan itibaren uygulama |
| `0x180001b78` | `EnableHookLL` | Koruma, patch/yedek, cache temizleme |
| `0x180001cdc` | `EnumerateThreads` + `Freeze` | Geçerli süreç, diğer thread'ler |
| `0x1800020e4` | `ProcessThreadIPs` + IP yardımcıları | Eski/yeni talimat sınırları |
| `0x180002280` | `Unfreeze` | Sıfır olmayan ID'ler, resume, listeyi bırakma |
| `0x180001c9c` | `EnterSpinLock` | İlk32 bekleme Sleep(0), sonra Sleep(1) |

Kayıt adımı `0x38`; hedef `+0`, relay `+8`, trampoline `+0x10`, yedek `+0x18[8]`; `+0x20` bitleri patchAbove/isEnabled/queueEnable; nIP `+0x24` düşük4 bit; old/new sınırlar `+0x28[8]`/`+0x30[8]`. Assembly `0x180001b2b` öncesinde Freeze eylemi sabit1'dir: tek hedef disable için de upstream aynı davranır. Başarısız suspend edilen ID sıfırlanır; tümünü enable/disable yolu eylemi değiştirir. Patch5/7 bayt, E9 ve EB F9, VirtualProtect→FlushInstructionCache sırası uyumludur. Bunlar seçili kaynakla davranış karşılaştırmasıdır; tam binary eşitliği değildir.

## Bellek ve trampoline

[Resmi buffer.c](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/buffer.c) karşılıkları: allocate `0x1800022fc`, free `0x180002320`, blok bulma/tahsis `0x180002388`, boş initialize `0x180002598`, executable sorgusu `0x18000259c`. Gözlenen 4KiB blok, 64 bayt slot, ±1GiB arama, allocation-granularity hizalama ve VirtualQuery/VirtualAlloc dizisi uyumludur. Blokta next/free/usedCount alanları `+0/+8/+0x10`; boşalan blok VirtualFree MEM_RELEASE ile bırakılır. Executable sorgusu VirtualQuery sonucunu ayrıca doğrulamaz; bu sınırda seçili kaynak da aynı kontrol biçimini kullanır.

[Resmi trampoline.c](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/trampoline.c) karşılıkları: kurucu `0x1800025d0`, padding `0x180002938`. Binary'deki 14 bayt mutlak jump, 16 bayt mutlak call/Jcc, RIP-relative displacement düzeltmesi, iç dallarda uzunluk koruma, dış LOOP reddi, 50 bayt trampoline sınırı ve en fazla8 talimat sınırı uyumludur. Padding tek değer tekrarıyla 00/90/CC kabul eder; küçük prologda üst5 bayt sıcak patch bölgesi denetlenir. Sonuna detour relay yazılır. Özgün derleyici, optimizasyonlar ve tüm geçersiz bellek girdileri karşılaştırılmadı.

## HDE tablosunun birebir bayt karşılaştırması

`0x180002974`, `0x180007020` tablosunu kullanan HDE64 uyumlu decoder'dır. [Resmi table64.h](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/src/hde/table64.h) initializer'ının **529 baytı**, gömülü PE dosya ofseti `0x4a20`'deki 529 baytla birebir aynıdır. İki SHA-256 da `a35a7be4500f9100d6e014c666513cbb7ba52e4f7010eb230baefb638eac5033`. Karşılaştırmada `version-reconstruction-input.json` içindeki gömülü baytlar bellekte çözüldü; önce tam PE hash'i doğrulandı. İşlem hedefi yüklemez, yazmaz veya çalıştırmaz. Tablo eşitliği tüm decoder talimatlarının ya da MinHook sürümünün eşitliğini kanıtlamaz.

## Durum değerleri ve açık kalanlar

[Resmi MinHook.h](https://github.com/TsudaKageyu/minhook/blob/c3fcafdc10146beb5919319d0683e44e3c30d537/include/MinHook.h) ile gözlenen değerler uyuşur: 0 başarılı; 1 önceden başlatılmış; 2 başlatılmamış; 3 kayıtlı; 4 kayıtsız; 5 zaten etkin; 6 zaten devre dışı; 7 executable değil; 8 desteklenmeyen prolog; 9 tahsis/thread-listesi hatası; 10 koruma değişimi hatası. Preloader başlatıcı sarmalayıcısının kabul ettiği `-2` kendi sentinel yoludur; MinHook başarı kodu olarak yeniden adlandırılmamalıdır.

Bu bounded incelemede initialize/create/enable yolunda eyleme geçirilebilir bir uyuşmazlık bulunmadı. Kaynakta bulunup retained preloader envanterinde ayrı gövdesi belirlenmeyen uninitialize/remove/queue/API-name yardımcıları için kurtarım iddiası yoktur. Özgün sürüm, derleme seçenekleri, compiler/CRT ABI'si, gerçek thread context başarısızlıkları ve gerçek hedef prologlarıyla runtime eşdeğerlik açıktır. C++ adapter'ın seçilen bağımlılığı çağırması bu açıkları kapatmaz; sahte başarı döndüren motorun yerine somut fakat sürüm eşitliği kanıtlanmamış bir sağlayıcı koyar.
