# Aynı worker üzerinde açık kapanış devamı

Sonraki kaynak aşaması: [controller stop/join bağlantısı](join-binding.md).
Aşağıdaki controller bağlantısı sınırı bu belgenin önceki snapshot'ına aittir.

Durum: **kaynak uygulandı; derlenmedi ve çalıştırılmadı**. Değişiklikler
`worker.hpp`, `worker.cpp` ve `worker_tests.cpp` içindedir. Yazarın statik
incelemesi yapıldı; bağımsız inceleme veya compiler/runtime PASS yoktur.

Önceki Graph/sunucu düzeltmesi, bilinen false quiescence dönüşünü ayrı
`QuiescenceDeferred` sinyaliyle koruyordu. Üst Worker bu sinyali de genel hataya
çevirerek çalışma prosedüründen çıkıyordu. Yeni açık HOST seçimi, prosedür
çıkmadan aynı orijinal worker üzerinde kapanışı sürdürmeyi sağlar.

`Settings.close_retry_timeout_ms` varsayılan0 ile kapalıdır; INFINITE reddedilir.
Sonlu süre seçildiğinde yalnız Proxy'nin pending-socket destruction çağrısı bu
sinyali karşılar. Graph/backend Quiescing, henüz attested değil ve socket
sahipliği canlı olmalıdır. Worker/fiber/Domain PTD yeniden denetlenir. Graph,
Resources, Inputs, TLS ve hazırlanan bellek bu gerçek worker üzerinde tutulur.
Parent veya Worker metadata kilidi boyunca beklenmez.

Worker `WaitingClose` ve artan `Snapshot.close_epoch` yayımlar. Orijinal
controller/fiber, `retry_close(close_epoch)` ile yalnız güncel bekleme için bir
recheck sinyali verebilir. True, kapanışın başarılı olduğu anlamına gelmez.
Zero/eski/gelecek kimlikler ve bekleme dışı çağrılar false döner. Startup veya
stop event'leri beklemeyi uyandırabilir; retry yetkisi vermez. Aynı kimliğe
tekrarlanan istekler tek pending sinyalde birleşir. Alt katman yine false
dönerse yeni bir kimlik gerekir.

İlk bilinen ertelemeden itibaren bütün beklemeler ortak sonlu süreyi kullanır.
Süre, retry izni tüketilmeden önce de denetlenir. Bu bütçe yabancı callback'in
kendi içinde sınırsız bloklanmasını kesmez. Süre aşımı, kimlik/PTD bozulması,
event hatası ve gerçek istisnalar mevcut terminal retention yolunda kalır.
Server_start, configure, run_context veya çocuk iş gövdesi yeniden başlatılmaz.
Başarıdan sonra kaynak sahipleri yine worker üzerinde bırakılır; controller
ancak gerçek native/FLS join ardından finish yapabilir.

`Settings` ve `Snapshot` HOST yerleşimleri değişti; bütün tüketicileri yetkili
derlemede yeniden derlemek gerekir. `WaitingClose` enum sonuna eklendi, eski
değerler korunur. Base interface virtual-slot veya export eklenmedi.

Hazırlanan fakat çalıştırılmayan senaryolar:

| Mod | Kaynak beklentisi |
|---|---|
| `--close-normal` | İki false dönüş, iki ayrı controller kimliği, sonra true. Her proof aynı worker/fiber/PTD'de. Eski/yanlış controller ve spurious wake reddi; tek source start ve gerçek join/finish. Exit0. |
| `--close-timeout` | Sinyal verilmez; süre dolunca sahipli prefix korunur. Exit821. |
| `--close-disabled` | Seçilmemiş politika önceki terminal davranışı korur. Exit822. |
| `--close-unknown` | Opaque proof aynı exception türünü atsa da close wait'e alınmaz. Exit823. |
| `--close-poison` | Beklerken açık hata bildirimi gelir; cleanup/retry yapılmaz. Exit824. |

Bu aşama üretim controller/SourceBackend stop-join döngüsüne otomatik bağlanmadı.
Seçim ve sinyal sahipliği yalnız sentetik fixture kaynağında gösterildi. Bu
bağlantı sıradaki üretim hedefidir. Erken stop sonrası tüketilmemiş receipt'lerin
source drain politikası, loader hook'ları ve CRT/image/foreign finality ayrıca
açıktır. Mevcut DLL/ZIP, orijinal dosyalar ve eski build kanıtları değiştirilmedi;
bu değişikliklerden güvenilir toplam DLL yüzdesi çıkarılamaz.
