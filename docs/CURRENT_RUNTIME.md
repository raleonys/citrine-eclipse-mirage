# Güncel uygulama bağlantıları

Güncel makine kaydı [RUNTIME_CURRENT_STATUS.json](../metadata/RUNTIME_CURRENT_STATUS.json),
son doğrulama [native-allocation/VALIDATION.json](../metadata/evidence/native-allocation/VALIDATION.json)
içindedir. Eski kanıtlar kendi kaynak snapshot'larıyla tarihsel olarak korunur.

Tek Application sahibi config, Asio hizmeti, handler/event/achievement depolama,
provider, ek DLL ve hook ömürlerini yönetir. Başlangıç worker'ı loader lock dışında
çalışır. Üretim exportları **DllMain@1, SURGE@2, DllShutdown@3**; C++ kaynak API'si
ayrı preloader.source.dll hedefindedir. Readiness/port test export'ları yoktur;
araçlar mevcut EALsxPort ve LSX yanıtını gözler.

Son düzeltme yalnız native net_failure testinin MSVC aligned-CRT tahsis
enjeksiyonudur; ürün src/include gövdeleri değişmedi. Kullanıcının dört Windows
profili başarılı, Native Acceptance204/205 raporu ve düzeltmenin yerel sonuçları
[incelemede](NATIVE_ALLOCATION_FIXTURE.md) ayrı kaydedilir. Yeni native/oyun PASS
eski kayıttan türetilmez.

Uygulama RE temellidir. Bluefield seçilmiş hook11/lifetime bağımlılıklarının,
Brownfield altyapı/metadata ve onaylı dayanıklılık yaklaşımlarının donorudur;
bütün uygulama runtime'ları ithal edilmez. Standard compiler CRT kullanılır.
Üretim60 CPP ve korunan referans78 CPP ayrımı geçerlidir; referanslar silinmedi.
Dört model bileşimi üretimden ayrıldı; karma testler bölündü. Crypt tanılama TLS
guard'ı yalnız Debug'da kalır. [Dar temizlik kapsamı](BUILD_CLEANUP.md).

Dört kullanıcı vendor'u aktiftir: Asio1.38.2, MinHook1.3.4, PEGTL4 ve TinyXML2 11.
Vendor baytları değişmez; TinyXML derinliği500. SAFE-FREEZE yalnız pin'li MinHook
build kopyasına uygulanır. Windows IO standard filesystem/fstream ve onaylı
atomik persistence adaptörlerini kullanır. Gulrak aktif bağımlılık değildir.

Config isteğe bağlıdır: EXE yanında surge.cfg, sonra surge_override.cfg.
SDK kimliği10.6.2.31 ve wire10,6,2,31 sabittir. ServerPort/SDK eski CFG anahtarları
uygulanmaz. Listener3226–3236 arasında artan sırayla seçer; tüketimde startup
başarısızdır. Connect yalnız127.0.0.1:3216 / [::1]:3216 hedefini, çağıranın
const adresinin kopyasında yönlendirir. Sayaç kilidi startup beklemesini veya
original connect'i kapsamaz.

İki handler worker, bounded queue ve oturum başına FIFO vardır. Event rezervasyonu
ancak tam gönderimde kesinleşir; hata halinde yeniden denenebilir kalır. Greeting
asenkron; session/frame/output limitleri ve receive/frame/send deadline'ları
korunur. Achievement XML/INI yazımı rollback ve retry sahipliğiyle atomik batch
kullanır; power-loss atomik çok dosya işlemi iddia edilmez.

Native socket kapanışında documented would-block sonucu exact handle ile
retry edilir. Diğer close hataları uncertain quarantine'dır: sayısal handle'a
kör retry yapılmaz, cleanup başarısı engellenir. Client/listener ve geçici rollback
kaynakları korunur; read/write/timer callback'leri ve IO context drain tamamlanmadan
retirement kabul edilmez. Ayrıntı ve sınırlar [SOCKET_RETIREMENT.md](SOCKET_RETIREMENT.md).

DllShutdown eksik cleanup'ta aynı Application ve worker image lease'ini korur.
Hook quiesce kendi içinde sonsuz retry yapmaz; sonraki istekte tamamlanabilir.
Uncertain socket sonucu otomatik temizlenmez; üretim force-clear export/CFG'si
yoktur. Caller timeout'u provider/handler kodunu preempt eden toplam shutdown
süresi değildir. Native callback Guard/Pause, LastError, trampoline drain ve
retryable unload-guard sahipliği korunur.

Hook11 static CRYPT32!CryptQueryObject importunu, korumalı UTF16 seçimini ve
32768-unit/NUL dahil heap-free tarama sınırını korur. Avatar/fallback read-only
C++ verisidir; PE resource yalnız pin'li VERSIONINFO'dur. Golden metadata ve
import sırası finalize edilen kopyaya uygulanır; raw compiler çıktısı final kabul
sayılmaz. Debug gerçek request/response/event/error/config değerlerini mevcut
showLSX kapısıyla kaydeder; Release surge.log oluşturmaz veya mevcut Debug loguna
eklemez.

Yerel LLVM-MinGW/Wine sonuçları native Windows/MSVC, Golden veya oyun kabulü
değildir. Son değişen kaynak için bunlar bekliyor. Donor'lar emekliye ayrılmadı;
registry provenance, aggregate memory budgets, shutdown liveness ve kalan
kaynak/test/araç incelemesi açıktır.
