# Composed receive/read-until — 0x180025c40

R3 aşaması **bir yeni normal gövde** ekler: `composed_receive_025c40`. `RecoveredComposedReadEffects`, önceki zorunlu `IoCompletionEffects::receive_body_025c40` sağlayıcısını bu gerçek kaynakla kapatır; mevcut write/constructor/completion/scheduling sınıf zincirini korur. `025480` frame tamamlaması ve `0280d0` uygulama başlangıç köprüsü R4 işi olarak açıktır.

Hedef SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Pseudocode kanıtı `ev_987f9b5735e7f43bac4690a5614f3bbd07f7463da6bc2081c7fff428c7a9fce8`; assembly kanıtı `ev_92dfa0f4b810ad3afcfb9df2b941fd14b2290a6abc953cfaa54c553b58d910e9`. REA 6.5.0 kimliği ve Ghidra 12.1.4 kullanılabilirliği doğrulandı; aynı hedefin saklanan kanıtları yeniden kullanıldı. Yeni native analiz veya özgün hedef yürütmesi yapılmadı. Tam kesitler `composed-read-evidence.*`, yardımcı bağlamlar `composed-read-context.*`, oturum kaydı `composed-read-rea-session.json` içindedir.

## Tanımlı alanlar

| ReceiveHandler alanı | Native handler-relative karşılığı | Anlam |
|---|---|---|
| words_60_78[0..3] | +00/+08/+10/+18 | stream, string kimliği, etkin boyut veya UINT64_MAX sentinel, maksimum boyut |
| byte_80 / dword_84 | +20 / +24 | delimiter byte ve çağrı flag'i |
| words_88_98[0..2] | +28/+30/+38 | tarama/sonuç konumu, son receive isteği, frame owner kimliği |
| object_a0 / control_a8 | +40/+48 | taşınan sahiplik çifti |

Bu alanlar mantıksal projection'dır; native STL/vtable/handler ABI overlay'i değildir. Source binding tabloları aynı canlı string/stream nesnelerini göstermelidir. Eksik veya çift kimlik model sınırı `UnsupportedInput` üretir.

## Adresli akış

Adreslerde ortak `0x180` öneki kısaltılmıştır.

| Native adres | Gözlenen sözleşme ve kaynak karşılığı |
|---|---|
| `025c70..025ccd` | Flag +24'e yazılır. Flag1 dışındaki çağrıda kullanılmayan uzunluk `last_request-bytes` unsigned64 hesaplanır; etkin boyuttan çıkarılır, alt sınır0. Gerçek `010040` ile commit/küçültme önce yapılır. |
| `025cd2..025ce0` | Commit sonrasında incoming error canlı okunur. Error veya bytes0 ise delimiter aramasına girmeden frame tamamlanır. |
| `025ce9..025d51` | UINT64_MAX etkin boyut sentinel'i `min(string.size,maximum)` olarak çözülür. Tarama aralığı ayrıca gerçek string boyutu ve maksimumla sınırlanır. Empty string data token'ı0 olur. |
| `025d80..025e97` | Genel buffer iterator burada tek buffer ile örneklenmiş. Pozitif advance tek buffer sonunda kalır; nonempty begin'den negatif advance begin'e döner. Boş aralıkta nonzero cursor için ilerleyen normal yol kurulmaz; projection açıkça reddeder. Bu bozuk durum native başarı/fault/sonlanma davranışı olarak sunulmaz. |
| `025ed8..025f47` | Delimiter byte bir kez alınır; byte taraması NUL dahil bütün byte değerlerini destekler. Eşleşme varsa +28'e delimiter dahil index+1 yazılır; istek0 olur. |
| `025f61..026016` | Eşleşme yok ve etkin boyut maksimuma eşitse +28=UINT64_MAX, istek0. Diğer yolda +28 taranan aralık uzunluğudur. İstek `min(remaining,65536,max(spare,512))`; spare unsigned `min(capacity,maximum)-effective` olarak hesaplanır. |
| `02601b..02602b` | İstek +30'a yazılır. Flag0 ve istek0 ise hemen finish; herhangi nonzero flag için gerçek async receive yolu sürer. Böylece flag1, hazır delimiter veya limit durumunda dahi sıfır uzunluklu receive ile completion'ı erteler. |
| `026033..02608d` | Sonuç sentinel ise yerel error3 ve misc category seçilir; değilse incoming error kopyalanır. Category callback'inden sonra incoming code ve +28 yeniden okunur; hata varsa matched0. Native frame tail +38 ile `025480` çağrılır. Sonrasında handler erişimi yoktur. |
| `0260b3..02612b` | Prepare öncesi etkin offset saklanır. Offset>maximum veya request>maximum-offset ise önce `024390` message `0x180084840` ile, sonra aynı opaque exception üzerinden `023930` throw sınırı. Geçerli yolda gerçek `010040` prepare çağrısı yapılır. |
| `026130..0261a0` | Resize sonrasında stream, request, string ve maximum canlı yeniden okunur; eski offset korunur. Slice bu yeni görünümle sınırlanır. Allocator kimliği literal stream+0x68, flags0 ve gerçek `021d10` çağrısıdır. Sonrasında handler/stream erişimi eklenmez. |

`ReadLengthError` native exception boyutunu tahmin etmeyen opaque kimliktir. MSVC throw/SEH/EH ve category singleton sağlayıcıları zorunlu kalır. Doğrudan fonksiyondaki isteğe bağlı `ComposedReadStart` yalnız izole caller test sınırıdır; recovered effects köprüsü ve entegrasyon varsayılan gerçek `021d10` kaynağını kullanır.

## Doğrulama ve sınırlar

**3.672 çekirdek + 1.714 entegrasyon = 5.386 yeni kontrol PASS.** Çekirdek testlerde bağımsız `std::string::find` oracle'ı, açık 512/spare/remaining/65536 request anchor'ları, sentinel/flag farkları, commit sırası, kategori sonrası canlı okumalar, allocator callback'i sonrası yeniden yüklenen string/stream/maximum/request ve literal pointer taşması sınanır. Terminal construct→throw sırası ile gerçek handler ömrünün callback sırasında bitmesi kapsanır. Boş aralık/nonzero cursor model sınırı ayrıca doğrulanır.

Entegrasyon gerçek **025c40 → 010040/008350 → 021d10 → 023dd0 → enjekte SDK → IO routing → 027710 → 021a40 → 025c40** zincirini çalıştırır. Inline, gerçek `022c60/022640` borrowed ve `022c60/022670` owned scheduling yolları kullanılır. Parçalı delimiter, embedded NUL, frame sonrasındaki verinin korunması, EOF, maksimum boyut, receive hatası ve başlangıç zero-receive ertelemesi kapsanır. IO ve owned task byte depolarının ömrü gerçekten sonlandırılır; son güçlü/zayıf referans temizliği doğrulanır.

Early completion/key2 ve failed-post fallback girdileri kontrollü dispatcher protokol fixture'larıdır. Yeni entegrasyonda dış worker/`027c10` döngüsü çalıştırılmış gibi sunulmaz; bu eski gövdelerin regresyonları ortak build içinde ayrıca bulunur. `025480` hâlâ kontrollü gözlem sınırıdır; uygulama frame/dispatch/erase/rearm entegrasyonu R4'te yapılacak.

Güncel doğrulama ortak `source-workspace/build.ps1` kaydına bağlıdır;46 socket grubu bu build içinde kapsanır. Standalone socket script'i yeniden koşturulmaz. Önceki22 yerel validation kaydı tarihsel config hashleriyle korunur; yeni kayıt ortak build'e bağlanır.

Aynı ajan kaynak/assembly karşılaştırmasını ve testleri hazırladı; ayrı bağımsız reviewer onayı yoktur. Normal prefix dışındaki native unwind/SEH, allocator/global/RTTI/polymorphic/storage/lifetime/concurrency/ABI ve tam DLL eşdeğerliği henüz kanıtlanmadı. CRT üretim gövdeleri değişmedi ve beklemede. Özgün DLL/oyun çalıştırılmadı veya değiştirilmedi; gerçek CFG/token/achievement dosyaları okunmadı.

Toplam DLL için doğrulanmış yüzde yoktur. Somut artış bir normal gövde ve önceki receive provider'ının gerçek kaynakla bağlanmasıdır. Sıradaki iş **R4: 0x180025480 / 0x1800280d0 frame tamamlaması ve yeniden okuma köprüsü**.
