# Güncel kaynak yolu ve kalanlar — 3 Kasım 2026

Durum: SOURCE_REVIEW_ONLY. Bu adımda üretim veya test kaynağı değiştirilmedi;
derleme, link, test ve DLL çalıştırılmadı. Son tam build'den farklı55 kaynak/
başlık dosyası hâlâ derlenmemiştir. Yeni toplam tamamlanma yüzdesi yoktur.

İzlenen sınırlı zincir: Image → aynı Domain NativeThreads → Worker configure →
NativeJobs → prepared Profile/Parent → Graph JobLogic → NativeLogic033d70 →
Information/Content/Licensing çocuk çağrıları → mevcut sonuç/receipt yolu.
Bu incelemede bu seçili yol için yeni eksik bağlantı saptanmadı. Bu, her girdinin,
native fonksiyonun, eşzamanlılığın veya DLL davranışının tamamlandığı anlamına gelmez.

| Alan | Güncel kaynak durumu | Gerçekte kalan |
| --- | --- | --- |
| Loader attach/detach | İki tanım ve gerçek Image kurulumu mevcut | Güncel derleme/link ve loader davranışı doğrulanmadı |
| Mesaj thread seçimi | Image artık NativeJobs seçiyor; exact Domain/Parent bağlanıyor | Gerçek mesaj/SDK/thread davranışı doğrulanmadı |
| Mesaj ayarları | SDK sürümü ve ShowLSX başlangıç bayrağı bağlı | Canlı global değişimi ve gerçek log çıktısı doğrulanmadı |
| Graph/Policy | Original profile ait kurulum çelişkisi ve ek startup bariyeri kaynakta düzeltildi | Tam original session + büyük kalan event kuyruğu retirement kapsamı açık |
| Configuration | Aktif FilePass parser/hash/copy yolu mevcut; scalar legacy reader seçilmiş | Genel legacy collection alanı kısıtlı; aktif parser eksikliği değildir |
| Host/Online/Resources | Original profil ve gerçek sağlayıcı seçimi kaynakta mevcut | Native ABI, mapping, callback concurrency ve SDK sonuçları doğrulanmadı |
| Aggregate/CRT | Seçili state/initial cells/storage/normal cleanup projeksiyonları mevcut | Tam MSVC aggregate/CRT/EH ABI eşdeğerliği yok; genel CRT önceliği ertelenmiş durumda |
| version.dll/preloader | Önceki ayrı kaynak/teslim kapsamı tarihsel | Bu çalışma yeni version DLL derlemesi veya ikili erişimi yapmadı |

Eski blockers.json, kaynakta sonradan eklenen bazı parçaları hâlâ eksik kod olarak
tanımlıyordu. Bunlar artık seçili kaynak bağlantısı ile doğrulanmamış native davranış
olarak ayrıldı. Önceki durum/metinler history yedeğinde ve historical_baseline
alanında korunuyor. Source-implemented durumları native eşdeğerlik veya PASS değildir.
Original profil için ek cooperative unload kanıtı bitiş şartı olarak eklenmedi.

Son gerçek full-link sonucu hâlâ önceki iki eksik loader sembolüyle başarısız olan
tarihsel sonuçtur. Yeni kaynakların o hatayı gerçekten kaldırdığı henüz linkle
gözlenmedi. Yeni derleme açıkça istenmeden çalıştırılmayacak. Build kanıtları,
package receipt ve ZIP değiştirilmedi.

Sonraki kaynak çözümlemesi için kullanılacak iş: eski canonical fonksiyon eşlemesini
güncel kaynaklara karşı karşılaştırıp, uygulama/IO alanında gerçekten tanımsız kalan
adres ve çağrıları kanıtlarıyla seçmek. Tarihsel unmapped etiketi tek başına eksik
fonksiyon kanıtı değildir; sırf bu etiket için çalışan kaynağa ikinci gövde eklenmemeli.
