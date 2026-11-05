# Senkron attach koşulları — 0x180035850 doğrulaması

Kullanıcının seçimi: **özgün senkron başlangıç korunacak**. DllMain sonrasına taşınan Initialize API'si seçilmedi. Bu aşamada C++ kaynağı değiştirilmedi; derleme, test, link veya hedef çalıştırması yapılmadı.

REA 6.8.0 sunucusu `C:/tools/rea-6.8.0/node_modules/rea-agents/scripts/rea.mjs` ve kullanılabilir Ghidra 12.1.4 doğrulandı. `anadius64.dll` salt statik açıldı; SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` önceki hedefle eşleşti. İki taze kayıt export edildi, analiz oturumu kapatıldı. `version.dll` okunmadı.

## Ortak loaded listesi için şüphe elendi

Taze pseudocode `ev_b1b6bd3bd55c75b89e66d0e673d6192f5de38762352c0cc95047ecda15e049b4` ve assembly `ev_31ddc9c81d9f0318f52788c16be5f6debe0fa616e8c3d008d72bab7a5acead34` aynı `0x180035850` girişini gösteriyor:

| Native aralık/adres | Gözlem |
|---|---|
| `0x180035879–0x18003588d` | Main=true doğrudan `0x1800a3ea8` listesiyle `0x180035260` çağırır. |
| `0x180035918–0x18003592e` | Worker=false önce `0x180036880(2)`, sonra `0x1800a3ec0` listesiyle yükleme yapar. |
| `0x1800359c2–0x180035a2e` | Ortak `0x1800a3ed8/+8/+16` vector boyut/kapasite erişimi, gerekirse `0x180035120`, ardından `0x180034e60` ile ekleme bulunur. Bu aralıkta açık bir uygulama kilidi çağrısı görülmez; yardımcıların/allocator'ın iç kilitleri hakkında iddia yoktur. |
| `0x180035a33–0x180035a3f` | Yalnız worker=false, `0x180036820(&0x1800a3c3e)` çağırır. |

Mevcut `Bootstrap::initialize` kaynak zincirinde senkron `boolean_worker_035850()(true)` döndükten sonra üçüncü payload allocate edilir ve library worker oluşturulur (`bootstrap.cpp:43–48`). Dolayısıyla normal tek attach zincirinde ana ve worker listesi eklemeleri eşzamanlı değildir. Bu sonuç yalnız loader thread başlatma davranışına dayanmaz; üçüncü CreateThread'in henüz çağrılmamış olmasına dayanır. Kaynağa ek kilit veya faz beklemesi gerekmedi. Cached route'ların farklı çağıranlarca keyfi tekrarı ve reentrant ikinci application attach ayrıca desteklenmiş sayılmaz.

## Gerçek senkron yol ve koşulları

| İşlem | Mevcut zincir ve koşul | Loader açısından açık iş |
|---|---|---|
| Domain factory/constructor | `make_initial_domain` SDK tablolarını doğrular; NativeRuntime constructor yalnız API'yi saklar, InitialLocale ilk snapshot kimliklerini bağlar. | API tablosunda LoadLibraryExW bulunması, constructor'ın onu çağırdığı anlamına gelmez. Factory sözleşmesi hâlâ loader dışındadır; bu audit onu genişletmez. |
| Domain initialize | Heap initialize → ACRT lock 0/4/5 initialize → FLS/PTD initialize. | İlk attach/prefix rollback ve gerçek compiler CRT/FLS bağlamı denetlenmeli. Dinamik locale/NLS operasyonları bu constructor zincirinden çıkarılamaz. |
| Argüman/console | `019910` argümanları/flag'leri kurar, `038760` noConsole sıfırsa AttachConsole; bazı dallarda AllocConsole/GetConsoleWindow/MoveWindow/freopen çağırır. | User32 yolu koşulludur. noConsole flag'i yolu atlayabilir; özellik sessizce kapatılmadı. |
| Configuration | Gerçek FilePass/configuration/persistence yolu ilk worker'dan önce çalışır. | Dosya okuma tek başına yasak değildir; somut callback/encoding/allocator/CRT grafiği ayrıca değerlendirilmelidir. ProgramData/LocalAppData mevcut adapter'da environment okumasıdır; SHGetKnownFolderPath uydurulmadı. |
| İlk worker | `029ea0` için begin/detach; join beklemesi yok. | Oluşturulmuş thread'den sonraki attach hatası loader altında join ile geri alınmış sayılamaz. |
| Host kurulumu | `between_workers → initialize_host_00e310`. Başarılı engine initialize sonrasında connect ve beş registry hook'u denenir; BlockReruns ve Documents ek hook'ları koşulludur. | Başarılı engine yolunu boş configuration tek başına etkisiz kılamaz. Seçilen MinHook enable yolu Freeze/SuspendThread kullanır; loader bağlamı hâlâ açık. Engine hatası zorlanarak özellik tamamlanmış sayılmadı. |
| Online worker | İkinci begin/detach; main bu worker'ın hazır olmasını beklemez. | Worker'ın kendi runtime policy/RO initialization çağrıları, controller attach çağrılarıyla karıştırılmamalı. `thread_entry_05e7a8` policy==2 ise initialize_runtime çağırır. |
| Ana extra-DLL listesi | `035850(true) → 035260 → LoadLibraryW`; liste configuration'dan ilk worker'lar oluşturulduktan sonra alınır. | Boş liste yüklemeyi atlar; dolu liste DllMain içinde LoadLibraryW yapar. Başarısız load ayrıca UI yoluna ulaşabilir. Sıralama değiştirilmedi. |
| Library worker | Ana çağrı bittikten sonra üçüncü begin/detach; wait2 ve tamamlanma flag'i bu worker'dadır. | Ana attach'e wait2 eklenemez. Ortak loaded listesi için normal attach zincirinde yeni kilit gerekmiyor. |
| Shutdown/termination | Mevcut Controller Runtime close/destruction → Domain close yapar; açıkça loader dışı yol. | Pre-detach host cooperation, image/foreign/CRT ömrü, failed attach ve non-null reserved süreç sonlanması ayrı kalır. |

[Microsoft DLL uygulama kuralları](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices), DllMain içinden LoadLibrary, User32 ve thread eşzamanlaması için kısıtlar belirtir. [DllMain sözleşmesi](https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain) başarısız attach, FreeLibrary ve süreç sonlanmasını ayırır. Bu belgeler her execution'da deadlock gözlendiği anlamına gelmez; mevcut üretim tasarımının hangi koşulları gerekçelendirmesi gerektiğini gösterir.

## Envanter ve sonraki dar hedef

Eski canonical envanterdeki `no_definition_mapping`, eksik algoritma sayısı değildir. Mevcut kaynak adres etiketleriyle yapılan kaba taramada kalan adayların üst sıraları string/map/stream/callable yardımcıları içeriyor. Örneğin saklanan pseudocode'da `0348d0` decimal string oluşturma, `01e6e0` string range constructor, `02c370` C-string uzunluğu+append, `020c10/020c80` callable wrapper kurulumudur. Bunlar yeni DLL işlevi gereksinimi olarak sayılmadı; eski tablo yeniden üretilmedi, compiled-symbol kanıtı yenilenmedi.

Sonraki inceleme, **ilk worker oluşturulmadan önceki attach hata prefix'leri ile oluşturulduktan sonraki prefix'leri** ayıracak. Bu, hangi kaynakların gerçekten geri alınabileceğini ve hangi durumda image/provider sahipliğinin tutulması gerektiğini belirler. İki `require_loader_*` hook hâlâ tanımsız; success/null stub eklenmedi. Bu aşama kod tamamlanma yüzdesini artırmaz. Önceki 69 C/C++ kaynak/header farkı derlenmeyi bekliyor; DLL/ZIP ve tarihsel build kanıtları korunuyor.

Makine tarafından izlenebilir satır/hash bağlantıları `synchronous-loader-audit.json`, taze REA kayıtları `library-phase-native.evidence.json` içindedir. Pseudocode özgün C++ değildir; Ghidra terminal-call/no-return sınırlamaları ve diğer bütün araç sınırlamaları export'ta korunur.
