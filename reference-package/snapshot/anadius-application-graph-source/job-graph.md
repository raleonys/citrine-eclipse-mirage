# Graph çocuk mesaj kapsamı ve Jobs seçimi

Durum: **yalnız kaynak kodu; derlenmedi, proje testi/link/DLL çalıştırılmadı**.
Dört dosya statik olarak incelendi. Tam prepared Profile/çocuk Graph pozitif
testi henüz yoktur; eski PASS ve ZIP kayıtları bu değişikliği kapsamaz.

Graph'ın özel Logic nesnesi artık bağımsız `native_profile::JobLogic` arayüzünü
uygular. `process_native_job` mevcut `Logic::process_native` ve dolayısıyla
mevcut033d70 gövdesini seçilen Parent, Strings, Logic adresi, oturum nesli ve
owner/control kimliği altında çağırır. Eski Logic/SessionLogic virtual slotları
değiştirilmedi; özel Graph Logic nesnesine yeni bir HOST interface eklendi.

Çağıran canlı çocuk Parent Operation, izin ve oturum strong/Logic/prefix ömrünü
korumalıdır. Açık checkpoint, çağıranın tam iş kaydını doğrulayan güvenilir
admission işlemidir; yalnız dolu bir std::function olması job claim kanıtı
değildir. Graph doğrudan JobToken/closure claim'i üretmez veya devralmaz.
Profile'ın bu arayüzü seçecek tam mesaj bağlantısı henüz eklenmedi.

Kapsam hazırlığında `job_binding` önce ayrılır. Paylaşılan JobCall Parent
lease/izinini ve Graph Impl'i tutar; Graph yalnız weak geri bağ taşır. Callback
aktarımı kilit dışında ve rezervasyon altında yapılır. Her izin/kaynak/callback
kontrolü sırasında aynı Logic satırı ve canlı control yeniden doğrulanır.
Checkpoint çalışırken ayrı checking mührü, recursive provider girişini reddeder.
Parent/proof/opaque callback boyunca Graph metadata kilidi tutulmaz.

Oturuma bağlı Effects, başka oturumun Effects nesnesini kapsam içinde kabul
etmez. Ortak event/clock/Router yardımcıları aynı canlı actor kapsamını kontrol
eder. Public update/enqueue, snapshot/getter/retained ve yaşam döngüsü işlemleri
asıl worker'a bağlı kalır; snapshot/getter'lar çocuk kapsamı yayımlanmışken
mutable alanları okumadan reddedilir.

Normal dönüşte invocation, Graph active/router/opaque durumu, event/achievement
kilitleri ve writer envanteri kapanmış olmalıdır. Kapsam önce geçersizleşir,
callback hedefi girişler kapalıyken emekliye ayrılır; son kaynak/izin kontrolleri
geçtikten sonra weak kayıt kaldırılır. Hata halinde bilinmeyen prefix poisoned
kalır; tahmini unlock, replay veya native EH temizliği yapılmaz. Kalıcı prefix
gerekirse çağıranın string ve oturum backing'ini tutması zorunludur.

**Alt sağlayıcı sınırı:** Sonraki kaynak adımında somut Information'ın iki dar
çocuk girişi Graph'a bağlandı; ayrıntı:
`../anadius-information-effects-source/job-information.md`. Content de sonraki
kaynak adımında dar senkron kapsamla bağlandı:
`../anadius-content-effects-source/job-content.md`. Licensing henüz asıl worker
kontrollerini korur. Bu sağlayıcıya ihtiyaç duyan
çocuk istekleri tam uygun kabul edilmez; bilinmeyen kısmi akış sonrası otomatik
yeniden deneme yapılmamalıdır. Dış online/global mutasyonların ayrıca serialize
edilmesi gerekir. Bu adım native concurrency eşdeğerliği iddiası taşımaz.

`ProfileSettings.jobs` artık açık bir `shared_ptr<session_profile::Jobs>` seçer
ve Graph kurulumu bunu Profile Dependencies'e geçirir. Boş değer mevcut owned
InlineJobs davranışını korur. Seçim kendi başına çocuk mesaj yetkisi vermez.
ProfileSettings/Options HOST yerleşimi değiştiği için bu tipleri kullanan bütün
tüketiciler, ancak açık derleme talebi geldiğinde yeniden derlenmelidir.

`graph_integration_tests.cpp`, mevcut18 mesajlı senaryoyu hem varsayılan hem
açık seçilmiş aynı-worker sentetik Jobs ile tarif eder. Start/detach seçimi,
pasif kurulum ve backend ömrü kontrol edilir. **Çalıştırılmadı; bu senaryo native
çocuk thread veya JobLogic kapsamını doğrulamaz.**

Sırada somut alt sağlayıcılara dar çocuk yetkisi, tam Profile request/response/
closure akışı ve prepared pozitif/negatif entegrasyon fixture'ı vardır. Mevcut
Jobs seçimi, native scheduler veya gerçek thread/FLS sonlanması sağlamaz. İki
üretim loader hook'u ayrıca açıktır. Toplam DLL yüzdesi hesaplanmadı.
