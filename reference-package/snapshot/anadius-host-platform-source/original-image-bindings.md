Kaynak güncellemesi: 2026-11-03. Derleme/test/link yapılmadı.

OriginalCrt artık Resources, host/native Context ve Online factory yollarında
açıkça seçilebilir. Cooperative varsayılanı ve gerçek dış proof gereklilikleri
korunur. Resources'ın OriginalCrt yolunda stop_sources opsiyoneldir; boş slot
için sabit true/false callback üretilmez. prepare/take_resources/SourceBackend
aynı lifetime seçimini taşır. Durdurma yolu seçilmemişse manuel stop_and_join
false ile sahipleri tutar; özgün CRT exit callback dispatch'i buna dönüştürülmez.

Host için select_original_owned_engine gerçek aynı OwnedEngine'i API/code
sahibi olarak seçer. install_host, Resources ve Context profillerinin eşleşmesini
denetler; OriginalCrt için providers_ready/external_quiescence slotları boştur.
Gerçek Publication aynı Runtime original array'ine bağlanır ve başarılı create
sonucu enable'dan önce slot makbuzuna dönüşür. Callback kabulü bu makbuzun doğru
callback bitini Setup/Ready aşamasında denetler. Salt nonzero pointer veya hazır
flag seed'i kullanılmaz. Host map'i source initialize_host içinde ilk hook
create'inden önce hazırlanır; gerçek shared Publication metadata mutex'iyle
slot yayımı korunur.

Original per-hook Domain, metadata kontrolünü PTD/body girişinden önce yapar.
Yayımlanmamış slot normal reddedilir ve rezervasyon bırakılır; unknown proof
outcome tutulur. Kabul edilen callback gerçek source body ve kendi native wait
dallarını çalıştırır. Host setup, yayımlanmış bir Full body beklerken tamamlanabilir;
all-worker blanket proof veya sonraki controller activate API'si eklenmez.
Single-active, reentry ve PTD sınırları korunur. Bu seçili typed profile genel
native callback eşzamanlılığı/port paylaşımı/SDK reentry için sertifika değildir.
Bu sınırlamalar ek güvenli kaldırma bitirme koşulları olarak sunulmaz.

Özgün CRT cleanup'ı attach thread'inden farklı bir serialized actor'da gelebilir.
Original engine seçimi Fresh aşamada bunu açıkça açar; gerçek lower API status,
overlap/reentry ve unknown-outcome denetimleri korunur. Cooperative engine'in
mevcut controller şartı değişmez. Farklı actor initialize/uninitialize senaryo
kaynağı eklendi; bu senaryo da çalıştırılmadı.

Native Context OriginalCrt close, gerçek admission pause ve Binding active-call/
close kontrollerini yapar; ek dış unload callback'i çağırmaz. Kaynak application
detach'ine close/join eklenmedi. Compiler CRT'nin gerçek kayıtlı source exit
callback sırası esas alınır; cooperative cleanup ayrı source API yoludur.

Online için select_native_original_online gerçek GetModuleHandleExW/FreeLibrary
CodeLease sağlayıcısını seçer; selection sırasında module SDK çalışmaz. Runtime
Online başlangıcı gerçek load/resolve/version/create/getter kaynaklarını kullanır.
OriginalCrt local close ancak gerçek module/object/binding/active-call durumu
boşaldıktan sonra ilerler; foreign_quiescent callback'i kurulmaz veya çağrılmaz.
Bu yerel sonuç foreign background kullanıcıları/image unload için kanıt değildir.

Kaynak senaryoları: gerçek Publication makbuzu olmadan ret, yayımlanmış foreign
Full callback aktifken host setup completion ve gerçek PTD scope; Original Context
Binding close; gerçek owning engine seçimi; gerçek Online worker/destroy/free/lease
emekliliği ve sıfır foreign proof çağrısı. Native configuration/host/Online/Worker
factory'lerini birleştiren kaynak senaryosu gerçek fresh Runtime constructor'ına
ve hiç yayınlanmamış kapanış yoluna gider. Kontrollü SDK fixture'larıdır;
senaryolar çalıştırılmadı, gerçek uygulama attach veya hook patch yürütülmedi.

Resources/AssemblyInputs/Online Options ve Context/Domain constructor-factory
kaynak ABI'leri değişti. Sonraki açık derlemede bütün tüketiciler birlikte
yeniden derlenmelidir. SDK callback imzaları değişmedi; orijinal MSVC sınıf/
exception/CRT ABI eşdeğerliği iddia edilmez.

require_loader_attach_runtime hâlâ tanımsızdır. Kalan doğrudan iş bu gerçek
kurulumu kalıcı image sahibine taşımak, kökleri publish öncesi sahiplenmek ve
gerçek ApplicationEntry'yi bir kez senkron çağırmaktır. Detach kaynakta tanımlı
ama derlenmedi. Son gerçek link/derleme ve teslim ZIP'i değiştirilmedi.
