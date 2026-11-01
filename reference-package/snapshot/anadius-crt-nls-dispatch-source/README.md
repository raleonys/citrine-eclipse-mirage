# NLS API çözümleme ve LCMap gönderimi

Bu modül `anadius64.dll` içindeki iki gövdeyi statik Ghidra assembly ve pseudocode kanıtlarından C++20 olarak yeniden kurar. Özgün dosya SHA256 değeri `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` ile doğrulanmıştır. Tam seçilmiş gövdeler, kanıt kimlikleri ve salt okunur PE byte eşlemeleri `selected-evidence.json` içindedir.

| Özgün adres | Kaynak davranışı ve önemli kanıt |
|---|---|
| `0x1800671d4` | `0x180067208` procedure ID `0x11`, `0x18006720d` resolver çağrısı. Ex bulunursa dokuz argüman aynen aktarılır; `0x180067271` çağrısının EAX sonucu korunur. Bulunmazsa `0x18006727e` locale-name→LCID gövdesini flags0 ile çağırır, ardından `0x1800672a5` LCMapStringW çağrısına gider. |
| `0x1800672c0` | `0x1800672dd` procedure ID `0x13`; resolver `0x1800672e9`. Bulunan fonksiyon locale adı ve flags alır. Yoksa `0x18006730d` yalnız adı alan `07310c` fallback'ini çağırır. Her iki yolda tam DWORD EAX sonucu korunur. |

Retained pseudocode iki sarmalayıcıyı `void` gösterir. Assembly çağrıdan sonraki epiloglarda EAX'ı değiştirmediği için kaynak `0671d4` için `int32_t`, `0672c0` için `uint32_t` döndürür. BOOL'a daraltma, LCID0 için erken dönüş veya native Ex çağrısı0 döndürünce W fallback'i eklenmez. İşaretli source/destination count değerleri değiştirilmez. W yolu version, reserved ve sort-handle argümanlarını özgün gövde gibi aktarmaz.

Her resolver çağrısının iki DWORD modül adayı tam sırayla `{3,16}` değerlerindedir. `LCMapStringEx` için `0x18008b9e0..0x18008b9e8` dosya `0x8a1e0..0x8a1e8`, `LocaleNameToLCID` için `0x18008ba18..0x18008ba20` dosya `0x8a218..0x8a220` aralıkları `03 00 00 00 10 00 00 00` içerir. Bitiş adresleri aynı zamanda ilgili NUL sonlu ANSI API adlarının başlangıcıdır. `.rdata` RVA `0x7f000`, raw offset `0x7d800`, image base `0x180000000` ile eşlenmiştir.

```powershell
& '.\outputs\anadius-crt-nls-dispatch-source\build.ps1'
```

Strict `-std=c++20 -O2 -Wall -Wextra -Werror` bağımsız derleme ve **12.581 kontrol PASS**. Üç map yolu, iki locale yolu, argüman ve çağrı sırası, NULL adres token'ları, işaretli count uçları, tüm EAX bitleri, tam LCID değerleri ve0 native sonuçları test edilir. Nonzero çözülmüş adres token'ı doğrudan aktarılır; çözülmüş fonksiyonun güvenilirliği resolver sınırındadır. Her çağrı resolver'a gider; cache davranışı bu gövdelerde yeniden icat edilmez. Testteki C++ exception ile callback'ten dönüş yapılamaması yalnız ek cleanup/fallback eklenmediğini sınar; özgün Windows SEH kanıtı değildir.

`Effects` bu modülde resolver `0669b0`, gerçek API çağrıları ve `07310c` için zorunlu bağımlılıklar tanımlar. Bağımsız fixture bunları sentetik endpoint'lerle sağlar. Üst bileşim gerçek kurtarılmış resolver ve locale fallback kaynağını bağlayabilir; burada boş veya varsayılan başarı sağlayıcısı yoktur. Assembly'deki CFG guard/hash kurulumu typed callback sınırında temsil edilir; özgün CFG güvenliği, native NLS semantiği, ABI/stack, hata durumu ve concurrency eşdeğerliği doğrulanmış değildir. DLL, oyun veya gerçek locale API çalıştırılmaz; özgün girdiler değiştirilmez. Kontrol sayısı bağımsız semantik test sayısı veya DLL tamamlanma yüzdesi değildir.
