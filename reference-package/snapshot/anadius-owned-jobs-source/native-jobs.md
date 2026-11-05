# Seçimli native Jobs bağlantısı

Sonraki kaynak kaydı: [prepared native fixture](../anadius-application-graph-source/prepared-native-fixture.md).
Tam uçtan uca senaryo kaynağı artık mevcut; henüz derlenmedi/çalıştırılmadı.
Bu belgenin aşağıdaki15dosyalık incelemesi önceki implementation aşamasına aittir.

Durum: **yalnız kaynak kodu; derleme, proje testleri, link ve DLL çalıştırılmadı**.
15 kaynak dosyası bağımsız statik incelemeden geçti. Bu, seçili HOST scheduler
politikasıdır; orijinal native sınıf/ABI veya çalışma zamanı kanıtı değildir.
Yeni translation unit, export veya var olan base interface virtual-slot
değişikliği yoktur. Yeni bağımsız interface/base'ler ve header değişiklikleri
gelecekteki yetkili derlemede ilgili tüketicilerin yeniden derlenmesini gerektirir.

`owned_jobs::NativeJobs`, mevcut `ProfileSettings.jobs` / `Dependencies.jobs`
üzerinden açıkça seçilir. Varsayılan InlineJobs seçimi korunur. Sağlayıcı aynı
DomainStorage tarafından sertifikalı shared NativeThreads ve prepared Parent
ister; Graph'ın mevcut `install_server_prepared` rotası kullanılabilir. Somut
Graph JobLogic ve Policy seçimi önceki Profile çocuk mesajı yolundan gelir.
Legacy callable runner native yola çevrilmez. Metadata satır kapasitesi en çok64
olabilir; kaynak kapasite sınırında tahmini inline fallback/yeniden deneme yoktur.

Profile, yeni bağımsız PreparedJobs interface'ini tanıyıp exact Application/Ports
ve admission ile bağlar. Start, stabil satırına owning JobHandoff'u alır ve
launch'tan önce scheduler work_count'a bir iş pini ekler. Mevcut recovered
`begin_thread_05e8e0` aynı Domain NativeThreads üzerinden kendi rebuilt entry
adresini çağırır; orijinal image VA yürütülmez. Dönen gerçek handle/id kaynak
Frame yoluna gider, ayrı duplicate handle fiziksel çıkış kontrolü için tutulur.

Çocuk, Parent'a girmeden önce başarılı detach yayımını event üzerinde bekler.
Asıl worker gerçek source handle'ını bir kez kapatır, başarılı sonucu kaydeder
ve event'i yayımlar. Frame'in start/detach kayıtları whole Parent Operation
bitmeden commit edilir. Sonra çocuk acquire → process_message → return_memory
yapar.033d70/çocuğun04a47c'si ve asıl-worker yanıt/021770 devamı önceki staged
politikayı kullanır. Bellek dönüşünden sonra facade'a kaynak erişimi yapılmaz.

Çocuk, original-affine Runtime API'si yerine typed native
PostQueuedCompletionStatus ile null-OVERLAPPED, seçilmiş HOST key `0x504a4f42`
ve monoton32bit receipt gönderir. Key0/1/2 rotaları korunur. Key tek başına
yetki değildir: Runtime → Application → NativeJobs exact satırı, başarılı
logical return, detach, tüketilmemiş receipt ve iş pinini doğrular. Hazır yanıt
bir kez complete_job ile tüketilir. Sonra dispatcher bir work birimini azaltır;
pending exception ve son-work stop sırası mevcut kaynak düzenini kullanır.
Yanlış/duplicate receipt ikinci decrement yapamaz, sayaç0 UINTMAX'e sarılmaz.
Bu pin otomatik work0 kapanışını korur; harici erken stop/drain politikası ayrıca
bekleyen işleri gözetmelidir.

PreparedJobs seçiliyken Profile'ın genel ready-result taraması native receipt'i
atlamaz. Bildirim ve normal completion sonrası Policy callback'leri yine asıl
worker'da çalışır;021770 cleanup nonreentrant Policy hook'una iç içe sokulmaz.
Native child response scheduling açılmış sayılmaz; yanıt asıl worker'dadır.

Mantıksal return/yanıt tüketimi fiziksel çıkış değildir. Satır ancak gerçek
wait0 signaled, GetExitCodeThread=0, duplicate wait handle ve barrier kapanışı
biliniyorsa geri kullanılabilir. AdmittedDispatcher'ın yeni senkron POD bakım
hook'u fiziksel get ve hemen SDK error capture'dan sonra, Parent yeniden
alınmadan önce çalışır. Runtime hook'un context'ini pinler ve RAII ile kaldırır.
İşlem hatası değeri hook'tan etkilenmez. Böylece uzun oturumlarda biten fiziksel
satırlar ve PreparedMemory'nin bağımsız duplicate handle kayıtları reaped olur.

Çocuk acquire/body/return ve kilit dışı bakım ayrı bir source gate ile
serialize edilir; gate alınmadan önce çağıranın Parent Operation tutmadığı
ve Parent'ın açık olduğu denetlenir. Böylece reaping'in mutating rezervasyonu
çocuk acquire/girişiyle yarışmaz; bekleyen başka işler eski fiziksel satırların
geri kullanımını sürekli ertelemez. NativeJobs bu
Parent'taki prepared mesaj işlerini münhasıran yönetir; dış manuel child
offer'ları bakım/lifecycle ile yarışmamalıdır. Root'un outside-Operation/PTD
preflight'i kendi SDK poll/close çağrılarından önce çalışır. Her SDK dönüşünde
seal kontrolü ve başarılı close için null commit vardır. Bekleme/close/post
belirsizliğinde sahiplik tutulur, tahmini cleanup veya çağrı tekrarı yapılmaz.

Profile external_quiescent fiziksel reaping'i Parent dışında, ayrı harici drain
kanıtından önce ister. Close Parent altında ancak gerçek kaynak/IO/work/queue
boşluğu ve harici attestation ile biter. Sonraki Environment/Policy kapanışı
ertelenirse NativeJobs Closed durumu aynı Application ve original actor için
idempotent true döner. Son NativeThreads sahibi, facade/Impl/gate hâlâ canlı ve
Retiring mühürlüyken kilit dışında bırakılır; unattached erişim reddedilir.

Çalıştırılmamış kaynak senaryoları üç dosyadadır: NativeJobs pasif constructor/
bound seçim ve son-provider deleter'ı; dispatcher exact/duplicate/foreign/key-
only/work1/work0/pending exception; maintenance error-capture/outside-Parent/
clear sınırları. Bunlar tam native start → child → receipt → response → FLS
reap fixture değildir. **Yeni kodun compiler veya runtime PASS sonucu yoktur.**

Kalan öncelik: gerçek prepared Graph/Profile + NativeJobs uçtan uca sentetik
fixture; erken detach/stop, post/duplicate/wait/close belirsizlikleri ve kapanış
retry senaryoları. Aynı Domain allocator/new-handler ve foreign/global state
eşzamanlılığı, SDK/code ömrü ve harici kernel drain çağıranın açık sözleşmeleridir.
Ready post başarısızsa otomatik post tekrarı veya tahmini work-pin bırakma yoktur;
uyuyan worker için harici wake/stop ve belirsiz-prefix politikası hâlâ gerekir.
İki üretim loader hook'u ve CRT/image finality de açıktır. DLL tamamlanma
yüzdesi yalnız eklenen satır/test sayısından hesaplanmamıştır. Eski ZIP ve
donmuş derleme kanıtları tarihsel snapshot'larına aittir.
