# 16 ACRT pair için typed binding denetimi

Salt okunur kaynak/API, retained REA ve iki eksik leaf için yerel PE + LLVM static disassembly incelemesidir. Yeni üretim kodu veya REA oturumu yoktur. Özgün DLL çalıştırılmadı/değiştirilmedi. Hedef SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`; JSON kaynak hash'lerini, tabloyu ve kanıt ayrımını kaydeder.

Bu bir binding ledger'ıdır; production dispatcher değildir. “Mevcut/eksik” ifadeleri JSON hash'leri alınan inceleme snapshot'ını anlatır. Root sonraki bounded aşamada 0704e8/0644f0/006500/06f9bc için dört pair-leaf ve 067890/0678cc/073258 için üç lowio gövdesi uygulamaya karar verdi. Lowio ensure0732a8, inherited067698 ve std-handles067788 alt provider'ları bu aşamada mandatory kalacak; yeni kaynaklar ayrıca bağımsız incelenecek.

## Callback eşlemesi

| Pair | Initialize | Uninitialize | Mevcut core / gerekli binding |
|---:|---|---|---|
| 0 | 064468 | null | locale_startup::initialize_locale_064468(shared locale slot/identity/thread Effects) |
| 1 | 0644b0 | null | cache::initialize_encoded_globals_0644b0(shared EncodedGlobals,cookie) |
| 2 | 067538 | 06755c | cache thunk init/uninit; **aynı runtime::State**; uninit low byte argument'ı alır |
| 3 | 006500 | 006500 | AL1 no-op; uninit return ignored |
| 4 | 0625b0 | 062614 | locks::initialize_acrt / uninitialize_acrt; aynı locks::State/Effects/native Banks |
| 5 | 0704cc | 0704e8 | heap initialize mevcut; leaf uninit aynı heap.heap_0a5a10=0, AL1 |
| 6 | null | 0644f0 | leaf thunk **VCRT uninitialize04d5a8(...,0)**; aynı VCRT State/Effects |
| 7 | 064e90 | 064ecc | ACRT ptd::initialize/uninitialize; aynı ACRT State/Effects/FLS callback domain |
| 8 | 067890 | 0678cc | **Zorunlu lowio provider**; gerçek handle-array state/dependency'leri henüz typed core değil |
| 9 | 06f9bc | 006500 | CommandLine A/W acquisition + A5A00/A5A08 publication; mevcut argv State ile ortak live binding gerekir |
| 10 | 06f5f8 | null | multibyte_control::initialize_multibyte(ids,shared Effects) |
| 11 | null | 006500 | AL1 no-op; normal reverse çağırır, rollback atlar |
| 12 | null | 064538 | argv_bridge::cleanup_064538(shared CleanupState,argv Effects) |
| 13 | null | 0644f8 | lifecycle_callbacks::uninitialize_multibyte(shared current slot/counter/heap domain) |
| 14 | 006500 | 0644a0 | environment_cleanup::uninitialize_callback(shared narrow/wide Environment refs) |
| 15 | 06447c | 006500 | lifecycle_callbacks::initialize_onexit_tables(shared termination::State) |

Pair tablo VA `[0x18008aa60,0x18008ab60)`,16*16 byte. Initialize0 veya uninitialize0 gerçekten null binding olarak kalmalıdır; “placeholder success” ile doldurulmamalıdır. Lowio/commandline mevcut üretim gövdesi bulunmadığı için bir typed binding modülü bunları sessiz no-op yapmamalıdır. 006500 retained asm yalnız MOV AL,1;RET olduğundan bilinçli no-op farklıdır.

## Eksik iki leaf ve argüman tuzağı

0644f0 retained fonksiyon listesinde yok; dosya0x638f0, **7 byte** `33c9e9b190feff`: XOR ECX,ECX; JMP0x18004d5a8. LLVM static disassembly de doğruladı. Bu callback walker argümanını **zorunlu0 ile değiştirir**; VCRT termination bayrağını, outer terminating'i veya callback argümanını aynen forward etmek yanlıştır. Aynı shared VCRT PTD/FLS ve VCRT lock state'ini kullanan mevcut `vcrt_ptd::uninitialize_04d5a8(state,effects,0)` bağlanabilir. Bu leaf'e yeni REA Evidence ID atanmadı.

0704e8 retained listede yok; dosya0x6f8e8, **11 byte** `4883252055030000b001c3`: AND QWORD[A5A10],0; MOV AL,1;RET. LLVM doğruladı. Aynı heap State'in handle alanını sıfırlar; HeapDestroy/HeapFree veya handler/cookie sıfırlama yoktur. Şu an core API'de bu leaf için isimli üretim fonksiyonu bulunmadı; wrapper/new recovered leaf ya da explicit typed hook gerekir.

## İmza alanları

`initializers::Effects` dört ayrı invoke alanı içerir: boolean initializer→uint64, uninitializer(Address,uint32), void initializer, int initializer→uint64. Pair modu yalnız ilk ikisini kullanır. C ve C++ table invoke alanlarıyla aynı function-pointer cast'i paylaşılmamalıdır. Rebuilt opaque callback IDs kullanılıp switch/typed closures ile gerçek source çağrısı yapılabilir; original VA'lar metadata selector olabilir, native çağrılabilir adres değildir.

Boolean walker **AL** sınar. Bool core dönüşü güvenle0/1'e genişletilebilir;067538 gerçek uint64 cookie yüksek bitlerini döndürür ve düşük byte1'dir, tam dönüş==1 testi yapılmamalıdır. Uninitializer core'larının bool dönüşleri walker tarafından ignored'dur. `invoke_uninitializer(...,argument)` için06755c'ye **uint8_t(argument)** ile düşük byte aktarılmalıdır: 0x100 düşük byte0 olup cleanup yapar;0x101 düşük byte1 olup21 module cleanup'ını atlar. C++ `bool(argument)` dönüşümü bu ikisini yanlış sınıflar. Diğer no-arg callback'ler argümanı kullanmaz;0644f0 ise daima0 verir.

Mevcut070588 normal reverse ve0704f4 rollback invoke_uninitializer'a argument0 geçirir. Outer0645cc terminating=true walker'ı tamamen bypass eder; bu outerflag'i06755c callback'ine capture ile taşımak mevcut kaynak sözleşmesi değildir. Normal reverse yalnız uninit!=0 ister; rollback önceki pair'in **init!=0 && uninit!=0** olmasını ister. Callback'ler sonraki pair girişlerini değiştirebilir: tablo fixed-size/live kalmalı, binding modülü başlangıçta bütün callback listesini snapshot'layıp walker sırasını sabitlememelidir. Bilinmeyen ID yanlış signature alanında açıkça reddedilmeli veya o alana özel mandatory delegate'e bırakılmalıdır.

## Gerçek ortak state ve lifetime bağlantıları

- Cookie önce Entry'nin security_init_cookie04c134 yoluyla seçilmeli. runtime::State.cookie const olduğu için seçilmiş cookie ile kurulmalı; heap.cookie, onexit.cookie, cache EncodedGlobals ve invalid/termination handler decode provider'ları aynı değeri görmeli. runtime proc/module tabloları initialize_thunks ve resolver/uninitialize_thunks arasında **aynı State** olmalıdır.
- EncodedGlobals.a4f10, termination::State.encoded_callback_a4f10 referansının **aynı hücresi** olmalı. EncodedGlobals.a5178 ile heap.encoded_handler_0a5178 mevcut API'de iki by-value alan: yalnız başlangıçta eşit kopya koymak ileride shared mutation alias'ı sağlamaz. Yeni composition bunu açık coherent projection/provider routing ile çözmeli veya kapsamı değişmez initialization ile sınırlamalıdır. a4f00 invalid global handler provider'ından aynı encoded state'e yönlenmelidir; a4f20/a5a18 diğer handler consumer'larına ait state'i gölgelememelidir.
- termination.onexit.application_0a47a8, Entry::State.onexit_table, registration ve execution için **aynı Table nesnesi**. Pair15 **termination.onexit.crt_0a52a8** ile **termination.quick_table_a52c0**'ı başlatır. Entry.quickexit_table A47C0 farklıdır; CRT quick A52C0 yerine geçirilmemelidir. Onexit State/cookie ve bütün callback storage'ı late cleanup bitene kadar hayatta kalmalı.
- locks::State ve ACRT/VCRT native bank'leri vcrt_bridge::LockEffects, PTD/resource lock4/5, lowio lock7, onexit lock2, heap new-handler lock0 çağrılarında ortak olmalı. Native lock Banks kendi canlı SDK nesnelerini taşır; originalVA bank adresi kullanılmaz. Pair4 init'ten önce lock tüketimi yapılmamalı; pair4 uninit sonrasına ihtiyaç duyan provider saklanmamalı.
- heap::State aynı allocator/reallocator, PTD, argv, environment, multibyte, onexit ve lowio provider'larında ortak heap domain olmalı. Pair5 uninit bu State'in handle'ını0 yapar; snapshot copy bunu tüketicilere yansıtmaz. Typed allocated nesne ömürleri ve free/unbind sırası provider sorumluluğu. Pair6 VCRT cleanup, heap reset'ten önce; pair2 module cleanup, locks uninit'ten sonra statik ters sıradadır.
- ACRT ptd::State ve Effects, Pair7, multibyte required_ptd, locale ThreadEffects, error hücresi ve destructor callback0649cc arasında aynı olmalı. **VCRT PTD farklı State**'tir: Pair6 ve outer LifecycleBridge aynı VCRT State'i kullanır. İkisini tek FLS index'e birleştirmek yanlış olur. FLS callback providers ve borrowed data native FLS teardown'a kadar yaşamalı.
- Locale current slot, initial identities, maskFE, pair A1938 ve MB current slot aynı `locale_bridge::Globals` domaininde. ImageState const image data'ya sahiplik sağlar; mutable Globals ve bounded Memory adapter ayrıca gerekir. Pair0 direct locale store ref artırmaz. Pair10 MB init, Policy üzerinden argv/env tekrar-init çağrıları ve Pair13 shutdown aynı initialized byte, current slot, counter registry ve heap'i kullanmalı. `RecordNative.bind_system_cp` initialize'dan önce tamamlanmalı.
- Pair9 CommandLineA sonucu argv::State.command_line tarafından tüketilen aynı bounded narrow view olmalı; CommandLineW için A5A08 ayrı borrowed wide view gerekir. A/W provider belleği parse süresince canlı ve tam NUL-terminated olmalı; GetCommandLine pointer'ları argv allocation free listesine eklenmez. Wide commandline A5A08, environment wide A5290/A5298 ve wide argv A59F8 farklı alanlardır.
- Pair12 CleanupState.argv referansı, configure_0638e8/ArgvBridge tarafından değiştirilen **aynı argv::State** olmalı. Aux A58A8/A58B0 ve wide argv A59F8 hücreleri de cleanup'a kadar canlı. Cleanup yeni global okumalarını free etkilerinden sonra yapar; erken pointer snapshots veya farklı copy state mutation sırasını bozar.
- Pair14 cleanup State.narrow, StartupBridge narrow environment initialize064054'ün kullandığı **aynı environment::State** olmalı. Current/initial wide cleanup view referansları stable kalmalı; live char* address-vector cleanup arayüzü native char16_t** ABI değildir. Current=initial alias'ı pointer identity üzerinden skip eder; dört environment globalini cleanup sonrası sıfırlamaz.

## Lowio ve diğer açık sağlayıcılar

067890 retained asm lock7→ensure_fh_exists0732a8(0)→başarıysa inherited067698 ve067788→unlock7→AL sonucudur. 0678cc A54A0'daki **128 QWORD/0x400 byte** slotu gezer; nonnull block'u destroy073258 sonra slotu0 yapar. Mevcut typed production lowio core bulunmadı. Zorunlu bool initialize/normal uninitialize provider'ı bu aynı live128slot state'ini ve lock/heap domainini korumalı; “dosya kullanmıyoruz” diye success callback uydurulmamalıdır. OS handle inheritance, native array/cell türleri ve destruction sözleşmesi ayrı bilinmeyendir.

06f9bc retained asm GetCommandLineA→storeA5A00, GetCommandLineW→storeA5A08, AL1. Narrow/wide bounded source dönüşleri, provider contract veya root'un ayrı gövde recovery'si gerektirir. Generic typed composition bu yeni özgün gövdeleri kendi başına recovered saymamalıdır. Dış kalan callbacks/heap/FLS/lowio, SEH, original ABI ve tüm DLL runtime eşitliği hâlâ kanıtlanmış değildir. Bir sonraki doğrulama own fixture ile gerçek16pair dispatch, low-byte matrix, rollback/null-init farkı ve state identity mutation testleridir; özgün DLL execution gerekmez.
