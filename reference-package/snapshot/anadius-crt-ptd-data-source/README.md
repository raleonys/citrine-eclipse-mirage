# PTD alanları, referanslar ve FLS temizliği

`ptd_data.cpp`, anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` için saklanan REA/Ghidra assembly kanıtından yeniden yazıldı. Bir C++ üretim nesnesidir; özgün kaynak veya tam CRT ABI'si değildir.

| Özgün VA | Kaynak | Kapsam |
|---|---|---|
| `0x1800648fc` | `construct_0648fc` | Gözlenen alanlar, multibyte ref artışı, lock4 altında güncel locale |
| `0x1800649ec` | `destroy_0649ec` | Exception tablosu, dokuz alan free çağrısı, multibyte/locale temizliği |
| `0x1800649cc` | `fls_destroy_0649cc` | Null ise işlem yok; diğer durumda alt alanlar, sonra PTD serbest bırakılır |
| `0x180064ae4` | `replace_locale_064ae4` | Önceki locale referansını bırakma, yeniden okuma, kimlik/refcount şartları, yeni referans |
| `0x1800647d4` | `retain_multibyte_0647d4` | Lock, +0x88'deki sayaca atomik artış, unlock |
| `0x18006489c` | `release_multibyte_06489c` | Atomik önceki değer1 ise ve statik kimlik değilse free |
| `0x180064854` | `current_locale_064854` | Lock aldıktan sonra global locale okunur |
| `0x180064814` | `clear_locale_064814` | Lock altında locale NULL yapılır |

PTD'nin `+0x3c0` alanındaki thread context'i burada serbest bırakılır. Bu bulgu, önceki thread çıkış gövdesine ayrıca free eklememizin neden yanlış olacağını açıklar. Dokuz free alanının sırası `70,58,60,68,48,50,78,80,3c0` şeklindedir; NULL çağrıları korunur. Her pointer serbest bırakmadan hemen önce okunur. Locale release çağrısından sonra hem PTD locale pointer'ı hem güncel locale kimliği yeniden okunur. Destructor context pointer alanını temizlemez.

`View` en az0x3c8 bayt geçerli PTD belleğinde memcpy kullanır. Exception tablosu, statik multibyte nesnesi ve başlangıç locale kimliği, yeniden kurulan image tarafından `Identities` ile sağlanır. Özgün image VA'ları çalıştırılabilir pointer yapılmaz. Locale refcount/ref release/add/free algoritmaları, gerçek atomik hücre, heap ve kilit politikaları zorunlu Effects sınırlarıdır.

**Sınır:** Dört helper özgün captured-pointer zincirlerini PTD pointer'ı ve sabit lock kimlikleriyle ifade eden yeni kaynak imzalarıdır. Yalnız normal instruction akışı yeniden yazıldı. MSVC `__crt_seh_guarded_call`, unwind funclet'leri ve SEH/finally garantisi kurtarıldı sayılmaz. Bu bağımlılıklardan exception kaçışı için ek RAII/unlock icat edilmez; özgün exception ABI eşdeğerliği iddia edilmez. Caller PTD/kimlik/lock sahipliğini ve geçerliliğini sağlamalıdır.

```powershell
& '.\outputs\anadius-crt-ptd-data-source\build.ps1'
```

206 kontrol: yalnız gözlenen byte alanlarının değişmesi, sayaç wrap/önceki değer kararları, statik/private locale yolları, dependency sırasında pointer değişimi, free sırası ve gerçek kurtarılmış FLS/PTD edinme/temizleme çekirdeğiyle Context sahipliği. Tüm heap/FLS/lock/locale/atomik etkiler kontrollüdür. OS thread/FLS veya özgün DLL/oyun çalıştırılmaz. Kanıtlar `evidence.json`, bounded assembly ve bağımsız inceleme `review.json` içindedir.
