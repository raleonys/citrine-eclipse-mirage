# Kalan küçük ACRT callback gövdeleri

Bu modül SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan değişmeyen DLL'den **4 yeni gövdeyi** C++20 olarak yeniden kurar. Özgün hedef yüklenmedi, çalıştırılmadı veya değiştirilmedi; kullanıcının commandline verisine erişilmez.

| VA | C++ işlevi | Davranış |
|---|---|---|
| `0x1800704e8` | `uninitialize_heap_0704e8` | Mevcut paylaşılan heap state hücresini sıfırla, AL=true |
| `0x1800644f0` | `uninitialize_vcrt_0644f0` | Mevcut gerçek VCRT uninitialize gövdesine terminating=0 ile yönlendir, sonucu ilet |
| `0x180006500` | `success_006500` | Yan etki olmadan AL=true |
| `0x18006f9bc` | `initialize_command_lines_06f9bc` | GetCommandLineA sonucu→A5A00, sonra GetCommandLineW sonucu→A5A08, AL=true |

`0704e8` özgün 11 baytı `4883252055030000b001c3`, dosya ofseti `0x6f8e8`: `AND qword[A5A10],0; MOV AL,1; RET`. HeapDestroy, serbest bırakma veya başka callback yoktur. Kaynak aynı `crt_heap_recovered::State.heap_0a5a10` hücresini kullanır; yeni heap sahibi oluşturmaz. Kaynak normal değer davranışını kurar; özgün instruction flag/RMW fault davranışı veya üst RAX bitleri ABI eşitliği iddia edilmez.

`0644f0` özgün 7 baytı `33c9e9b190feff`, dosya ofseti `0x638f0`: `XOR ECX,ECX; JMP04d5a8`. Kaynak gerçek `vcrt_ptd_recovered::uninitialize_04d5a8` gövdesini aynı VCRT State ve Effects ile çağırır ve onun bool sonucunu döndürür. Ayrı PTD/kilit alanı veya yeni başarı kararı oluşturmaz. Başlangıç çift tablosunda initializer'ı NULL olan pair6'nın uninitializer'ıdır: normal ters walker bunu çağırır; failure rollback initializer NULL olduğu için atlar.

`006500` birden çok pair konumunda aynı başarı leaf'idir; dört yeni gövde sayısında yalnız bir kez sayılır. Sözleşme yalnız düşük AL bitinin true olmasıdır; özgün `MOV AL,1` tüm RAX'ı 1 yapmaz.

## Paylaşılan commandline alanları

`CommandLines`, **mevcut** `crt_argv_recovered::State` nesnesine ve ayrı wide borrowed span alanına referans tutar. Dar sonuç doğrudan `argv.command_line` içine yazılır; ara/shadow dar hücre yoktur. W getter'ı çalışırken A sonucu zaten görünürdür. Getter etkisi A alanını değiştirirse W sonucu yazıldıktan sonra A yeniden yazılmaz. NULL sonuçlarda da yeni bir başarı testi eklenmez.

`CommandLineEffects` iki mandatory getter'ı enjekte eder. Sağlayıcı sınırları bilinen borrowed `span<const char>` / `span<const char16_t>` görünümleri verir. Boş span NULL'ı modeller; nonempty span downstream tüketiciler için NUL dahil geçerli sınır içermelidir. Metadata yeni kaynak temsilidir; native Windows ABI, OS pointer taraması veya buffer sahipliği iddia edilmez. Yayınlanan depolama getter/argv tüketimi boyunca canlı kalır; provider/user kodu senkronizasyonu sağlar. Modül varsayılan OS API çağrısı veya kullanıcı commandline acquisition sağlamaz. Test getter'ları yalnız kendi typed dizilerini kullanır.

## Kanıt ve doğrulama

`collect-evidence.cjs`, özgün dosya ve saklanan REA hash'lerini doğrular; yerel bounded audit'teki dosya ofsetleri/ham baytları tekrar PE ile karşılaştırır. `evidence.json` retained `006500/06f9bc/04d5a8` assembly/pseudocode EvidenceID'lerini, PE import isimlerini, image zero-fill global hücrelerini ve pair relocation kayıtlarını ayırır. Bu worker REA sunucusuna dokunmadı.

Root tarafından yönetilen yeni REA byte okumaları `../source-workspace/pair-leaves.snapshot.json` içinde ayrıca korunmuştur. `0704e8` için `ev_26b0e54af8729b48978b321830da453af248a6390a76dabde0a737eb5e348d5e`, `0644f0` için `ev_396089764fe78d606badcf63a55ffd8db3f53083ffba29d057c662c397dcb735` tam/eksiksiz bayt sonuçları yerel PE ile eşittir. Byte kaydı, eksik retained decompiler function kaydını kendiliğinden oluşturmaz.

```powershell
node outputs/anadius-crt-pair-leaves-source/collect-evidence.cjs
& outputs/anadius-crt-pair-leaves-source/build.ps1
```

Mevcut LLVM-MinGW ile `C++20 -O2 -Wall -Wextra -Werror` üretim nesnesi ve test EXE'si derlenir; değişmeyen ortak kısmi arşivden gerçek VCRT ve argv algoritmaları bağlanır. **140 kontrol** heap hücresinin diğer alanlara dokunmadan sıfırlanmasını; VCRT free başarı/başarısızlık ve index değişiminden sonra gerçek core sırasını; 9 NULL/boş string/nonempty A/W kombinasyonunu; yayın sırasını ve provider etkilerini denetler. Son fixture, yayınlanan aynı argv state'ini gerçek `configure_0638e8` ve `parse_0636c0` üzerinden geçirerek sahip olduğu `own.exe alpha "two words"` verisini ayrıştırır. Kaynak/arşiv hash'leri ve sonuç `validation.json` içindedir. Tam CRT lifecycle, native ABI veya bütün DLL eşitliği kanıtlanmamıştır.
