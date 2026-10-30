# Başlangıç kilidi ve build girişleri

Kullanıcı son `yellowfield-bluefield-aligned-source.zip` paketinden derlenen
`dist/preloader_l.dll` ile oyunun hiç açılmadığını, Bluefield’in aynı
ürün görevini yerine getirdiğini bildirdi. `surge.cfg` isteğe bağlı kalmalıdır.
Derleme/test başarısı gerçek oyun uyumu olarak sunulamaz.

## Doğrulanmış başlangıç hatası

Native registry sorgusu tracked `ClientValue` için başlangıcın server/provider/extra
aşamalarını beklerken eklediğimiz `registry_gate` kilidini tutuyordu. Başlangıcın
ilerlemesi için başka thread’in registry callback’i gerekirse iki taraf birbirini
bekliyordu. Bluefield bunu FIX19’da `callback_lock::Guard/Pause` ile çözmüş;
önceki Yellowfield aktarımında bu gerekli parça eksik kalmıştı.

Yeni regresyon gerçek native callback girişlerini iki thread’de çağırır. İlk sorgu
phase beklemesinde dururken ikinci thread’in callback’i ilerlemelidir. Eski kod
5 saniyelik kontrollü gözlemde ilerleyemedi ve exit1 verdi; thread’ler test
tarafından serbest bırakılıp join edildi. Başarısız kanıt silinmez.

Düzeltme supplied Bluefield header’ını byte-identical kullanır; yalnız registry
callback’inin takip edilen recursive kilitleri wait süresince bırakılır ve sonra
yeniden alınır. Native binding lease beklerken canlı kalır; original-call,
match/wait/write sırası, sonuç ve LastError korunur. `registry.cpp` değişmez.
Network callback kilidinin ayrı kapsamı bu düzeltmede değiştirilmez.

Bu, başlangıcı engelleyebilen bir kaynak hatasının kanıtıdır. Kullanıcının
oyunundaki tek nedenin bu olduğu veya oyunun artık açıldığı henüz doğrulanmadı.

## Build logundaki diğer hata

Logdaki checkout adı önceki `yellowfield-production-import-fix-source`; klasör adı
ZIP hash’iyle tam kimlik kanıtı değildir. Derleme test-only
`fake_win32.cpp:159/160` C2589 ile durmuş; CTest’e geçmemiş.
`hook_engine_fixture_tests` bağımsız olarak vendor MinHook’a bağlandığı için
core’daki PUBLIC NOMINMAX tanımını almıyordu. Hedefe PRIVATE NOMINMAX eklendi.
Vendor kaynaklarına veya test gövdelerine bu hata için patch uygulanmadı.

## Beş girişin başlıkları

| Komut | Başlık | İşlev |
|---|---|---|
| build_unstable.cmd | COMPILE ONLY | Unsigned DLL derleme, başlangıç/kapatma kontrolü |
| build_debug.cmd | DEBUG BUILD | Debug DLL derleme, başlangıç/kapatma kontrolü |
| build_native_acceptance.cmd | NATIVE ACCEPTANCE | Native Windows testlerini derleme/çalıştırma |
| build_crt_acceptance.cmd | STATIC CRT ACCEPTANCE | Statik CRT ve DLL arayüz testleri |
| build_production.cmd | PRODUCTION BUILD | Shipping DLL derleme, Golden doğrulama ve dist çıktısı |

Başlıklar supplied Brownfield’in profil isimlerinden alındı; eski HF12 etiketleri
ve mevcut işi anlatmayan açıklamalar taşınmadı. Çıkış kodu, renkli sonuç, pause,
parametre aktarımı ve Golden kapıları korunur. Otomatik yayın eklenmedi.

`CLEARED_KNOWN_GENERATED_CACHE_ONLY` önceki FALCION hatasını önlemek için bilinen
first-party Python bytecode cache’lerini temizleyen yardımcının sonucudur.
`removed: []` hiçbir dosyanın temizlenmediğini gösterir. Build akışı artık başarıda
bunu konsola basmaz; ayrıntıyı seçili build klasöründeki
`PYTHON_CACHE_CLEANUP.json` içine yazar. Temizleme hataları stderr/nonzero olarak
üst akışı durdurur; sessizlik hata gizleme değildir.

## Açık kalan oyun incelemesi

Config olmadan varsayılanlarla başlama korunur. Bluefield config/provider yolları
DLL’e, Yellowfield yolları EXE’ye göredir. Bu deployment farkı kaydedildi;
dosya konumu doğrulanmadan yeni arama/öncelik politikası eklenmedi. Exportlar
SURGE/DllMain/DllShutdown açısından uyumlu; Yellowfield’in iki ek yönetim export’u
kullanıcının önceki açık talimatıdır.

Yeni kaynak için Windows/MSVC üretim turu ve gerçek oyun denemesi gereklidir.
Durum/ham test kanıtları: `docs/evidence/STARTUP_BUILD_FIX_STATUS.json`.

Çapraz doğrulama: Release173/173, ilgili Debug8/8, altyapı29/29 ve config
olmadan üç gerçek DLL runtime çevrimi geçti. Eski kilit testinin exit1 kaydı
korunur. Güncel oyun tekrarı ve yeni kaynakla native MSVC çalıştırması yapılmadı.
