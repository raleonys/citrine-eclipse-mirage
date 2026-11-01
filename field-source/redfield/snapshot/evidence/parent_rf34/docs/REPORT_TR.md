# REDFIELD RF34 — Worker ve geçici heap sahipliği

21 Ekim 2026 · LOCAL_BUILD / NOT_RELEASE.

M02'nin dokuz ve M11'in beş güncel dosyası, gerçek çağıranlar ve platform/test
sınırlarıyla birlikte incelendi. WorkerOwner, SessionRetirement ve tek shutdown
bütçesinin mevcut yapısı bu kaynak kapsamına uygun bulundu. Thread işaretlenmeden
temiz bayrağına güvenilmiyor;join sonrası hook/service temizliği,physical release,
handle close ve DLL unpin görevleri seri halde tamamlanıyor. Başarısız görevler
retry için korunuyor. Gereken callback eksikse schedule işlemi başlamıyor.

Transient allocator'ın ham platform belleğindeki intrusive sahip nesnesi artık
açık placement construction ile kuruluyor. Ek heap çağrısı,new-handler veya PTD
bağı eklenmedi. Header layout,payload offset,heap callback sayısı ve başarısız free
sonrası tek sahiplik devri korunuyor. Bu değişiklik, önceki sürümde gözlenmiş bir
oyun çökmesi iddiası değildir;C++ nesne ömrünü platform ham belleğinden ayıran
mimari düzenlemedir.

Callback descriptor kopyalanır;context ve callback kodu ise başarısız free ve
detached retry batch bitene kadar canlı kalmalıdır. Count/byte metrikleri yalnız
kuyruktaki düğümleri ölçer;detached batch varken sıfır olabilir. Pending fence bu
batch'i ayrıca sayar;üreticiler quiescent olduktan sonra geçmeden DLL pin bırakılmaz.

Üç mevcut regresyon genişletildi. Worker testi finalize/close/unpin başarısızlık
sırasını,başarılı cleanup'ın tekrar edilmemesini ve callback shutdown reentry'sinin
busy dönmesini denetler. Allocator testi byte-array tabanlı raw storage veorijinal
descriptor değiştikten sonra yakalanmış context ile deferred free'yi sınar.
On altı latch kontrollü interleaving,detached batch sırasında yeni başarısız free,
başarısız eski batch ile yeni kuyruğun birleşmesi,recursive retry reddi ve her raw
blok için tek başarılı free sonucunu kapsar.

58 portable core nesnesi sıfırdan derlenip yedi modül programı çalıştırıldı;
retirement testleri gerçek yerel TCP/kalan completion akışlarını kullanır.
Dört odaklı program ASan+UBSan altında geçti. Windows-layout SemanticValue
kaynağı sentetik process-heap API ile ayrıca çalıştırıldı;bu gerçek SDK/native
Windows testi değildir. Finalize'ı atlama,cleanup'ı tekrar etme,eşzamanlı kuyruğu
düşürme vedetached batch'i yok sayma varyantlarının dördünü de testler reddetti.

Normal developer aggregate temiz,izole kaynak kopyasında98 regresyonun tamamını
geçti;RF33'ün98 stdout/exit sonucu aynı. Legacy GetConfig exit14 ayrı
FAIL_OBSERVED tanısı olarak kaldı. Bu karşılaştırma yeni hata interleavinglerini
eski sürümün koştuğu iddiasına dönüştürmez;yalnız eski program sonuçları korundu.

Kaynak incelemesi kapanan modüller:M01,M02,M11,M15,M16,M17. Kalan14 modülün
incelemesi açık. M02/M11'in14 kayıtlı dosyası,source/hash/license/role,ömür,hata,
somut mevcut boundary refactor gerekçesi ve güncel test kanıtıyla qualified oldu;
toplam16 code qualified,284 code ve4 vendor integration pending. Önceki origin
veya test PASS tek başına yeterli sayılmadı. Her dosya için yeniden yazım gereği
uydurulmadı;uygun state-machine algoritması kaynak ve entegrasyon gerekçesiyle korundu.

M12 generic CRT/PTD test-support review'u bu turda kapanmadı. Debug composition,
tam native/CRT/stress/blackbox taşıması,typed SDK ABI ve RFCL-005 oyun profili nedeni
açık. Yeni hook,route,SDK handshake/crypto veyaoyun davranışı eklenmedi.

Gerçek Windows/MSVC/native/CRT/Debug/oyun NOT_RUN. CMake ortamda yok;LSan
çalıştırılmadı;odaklı ASan detect_leaks=0 kullanır.3publicexport,7hook,35binding,
surge.cfg,preloader_l,lisans/vendor pinleri ve BROWNFIELD HF19/CURRENT korundu.
Kaynak review/qualification kapanışı gerçek Windows/native veya release kabulü değildir.
