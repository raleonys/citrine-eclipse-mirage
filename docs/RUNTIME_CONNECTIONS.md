# Somut uygulama bağlantıları

Güncel arayüz görevi: `YF-PUBLIC-ABI-20261028`. Runtime/config/resource/layout değişiklikleri korunur. Tarihsel karşılaştırma ile bugünkü uygulama ayrıdır.

| Alan | Önceki durum | Güncel bağlantı | Doğrulama ve sınır |
|---|---|---|---|
| Config koleksiyonları | Tam yol yok diye genellendi; bu tespit yanlıştı. | Mevcut config_hash/config_application tam yolu Application tarafından kullanılır. | 25 content, entitlement, achievement totals, gerçek main→override |
| Asio | Vendor ve IO fixture mevcut; gerçek app owner yok. | Gerçek RE ServerBackend/Session adaptörü, socket/completion/job ömürleri ve drain. | Gerçek loopback greeting/profile/content/encrypted/event/stop |
| Windows filesystem | NativeFactory önce std, sonra ghc kullanmıştı. | Son kullanıcı talimatıyla standard filesystem/fstream geri geldi; kullanılmayan Gulrak kaldırıldı. | Unicode binary IO, config/achievement/avatar yolları ve Debug/Release log kontrolü |
| Başlangıç State | Yalnız caller-seeded State vardı. | 036d20 pseudo/asm application alan projeksiyonu; SURGE görünen adı. | Config olmadan gerçek Application başlar |
| DLL bootstrap | Yalnız kaynak API geliştirme DLLi. | DllMain hazırlığı + loader lock dışında Application worker ve yönetim API. | Gerçek DLL otomatik başlar; soketten LSX profile cevaplar; stop tamamlanır |
| Native hook owner | 11 slot/source body vardı; somut ortak owner yoktu. | Tek config/original/context/MinHook owner, kaynak koşulları ve failure rollback. | 11 hook etkinliği + gerçek folder/process/registry SDK çağrıları; Wine kanıtı |
| Provider | Kaynak Effects portları, yeni isim bağlanmamıştı. | RE version2/factory/virtual ABI adaptörü, yeni ad, owner cleanup. | Kontrollü DLL: profile/auth/avatar/başarısız auth/destructor; gerçek harici provider ABI kabulü değil |
| Avatar/log/yollar | Resource ve politika metadata; runtime bağı yok. | LocalAppData/SURGE, byte-exact resource→handler, Debug-only olay logu. | Gerçek resource byte karşılaştırması; Release log yokluğu / Debug log varlığı |
| Kaynak/araç karşılaştırma hatası | ScalarDependencies sınırı tüm uygulamaya yansıtıldı. | Tarihsel tabloya açık düzeltme; yeni current-status kaydı. | Eski ZIPler ve ham doğrulamalar korunur |
| Native ürün kabulü | Eksik. | Hâlâ ayrı kabul işi; kaynak bağlantısı bu kanıtı üretmez. | Windows/MSVC/game NOT_RUN. 548-byte ABI iddiası geri çekildi; özgün Consumer import isimleri bu branded DLL ile aynı değil. |
| Brownfield resource metadata | Yalnız PNG; VERSIONINFO atlanmıştı. | Exact preloader_version.rc; fields/string/translation korunur. Son düzeltme: PNG yalnız read-only C++ avatar verisi; RCDATA yok. | Release/Debug gerçek PE resource kontrolü ve metadata tamper reddi PASS; diğer Golden/ABI değerleri ayrıca açık. |
| Brownfield iç modül düzeni | Gereksiz iç içe modül alt klasörleri | Tek modül katmanı;177 dosya taşındı | 261 gövde include dışında aynı; yeni build/runtime/QA kanıtları |
| Uygulama C export arayüzü | Binlerce C++ kaynak exportu uygulama DLLiyle karışıktı. | Üç Brownfield C isim/ordinal; DllMain@1 SURGE@2 DllShutdown@3; yönetim exportları kaldırıldı; SURGE RE RET-only markerı; DllShutdown gerçek cleanup; kaynak APIleri ayrı DLL. | Gerçek PE export denetimi, 7 negatif mutasyon, C17 import tüketicisi, üç load/socket/shutdown/unload döngüsü; Wine kanıtı. |

Güncel arayüz: [PUBLIC_DLL_ABI.md](PUBLIC_DLL_ABI.md). Ham sonuçlar: [PUBLIC_ABI_STATUS.json](evidence/PUBLIC_ABI_STATUS.json). Önceki runtime kanıtı: [RUNTIME_BINDING_STATUS.json](evidence/RUNTIME_BINDING_STATUS.json).
