# FIX04 davranış düzeltmesi ve açık sınırlar

Sınıflandırma: BEHAVIOR_FIX / NON_RELEASE_WORK. Kullanıcı config zorunluluğunun
ve eksik profilin düzeltilmesini istedi. Bu çalışma genel oyun kabulü değildir.

| Alan | Önceki somut sapma | FIX04 durumu | Kanıt / sınır |
|---|---|---|---|
| Constructor | State sıfır/boş; yalnız iki alan sonradan kuruluyor | Input 036d20 mantıksal başlangıcı uygulamaya bağlandı | Pseudo + ASM; beş pointer literal'i donor PE'den salt okunur doğrulandı |
| Base config yok | required read ile başlangıç kesiliyor | Boş belgeyle varsayılanlar korunuyor | Gerçek Application/Service testinde dosya oluşturulmadan başlatılır |
| Boş / farklı kök | Eksik Config2 için yeni exception | Kaynaktaki boş node anlamı korunuyor | Boş belge ve Other kökü ayrı senaryolar |
| Kısmi User | Eksik kimlikler boş kalıyor | Constructor değerleri korunuyor | Tam 14 profil XML attribute karşılaştırması |
| Override | Base zorunlu; override tek başına kullanılamıyor | Override tek başına uygulanabiliyor | Base-only Game alanı sınırı da kontrol edilir |
| Profil kabulü | Username/PersonaId/UserId boşsa kaynak dışı genel hata | Ek doğrulama kaldırıldı; kaynak atamaları korunuyor | Provider'ın boş değerleri için kaynak unit senaryoları korunur |
| SDK | Dosyaya bağlı boş değer | 10,5,96,47512 | Constructor immediate byte'ları |
| Settings / game | Boş attribute map'leri | 6 / 14 kaynak başlangıç girdisi | Settings yanıtının tamamı ve game yanıtı alanları |
| Cache konumu | Yalnız BLUEFIELD veri dizini | Native profil cache'i özgün anadius/LSX emu konumunda | Testler gerçek kullanıcı dizini yerine ayrı sentetik root kullanır |
| Cache koşulları | Kaynak gövdesi vardı; yanlış root bağlanmıştı | Provider auth başarısızlığı + UseLastOnlineProfile + cookie koşulları korunur | Provider yokken koşulsuz cache okuma eklenmedi |
| Avatar | Tek piksellik ortak placeholder; özel yola dosya yazımı | İki özgün 208x208 PNG ve source koşulları geri getirildi | Donor VA, file offset, SHA256, PNG CRC ve boyut kanıtı |
| Profil handler | Body input ile eşleşiyor | Değiştirilmedi; doğru başlangıç state'i sağlandı | Handler varlığı tek başına native çalıştığı kanıtı değildir |
| Config hatası | Var olan bozuk dosya hata veriyor | Syntax/I/O/non-file hataları açık kalır | Dosya yokluğu ile karıştırılmaz |

Config uygulama gövdesinin 46 section-qualified scalar/collection anahtarı
değiştirilmedi. Mevcut config unit testleri onların davranışını kapsar; bu,
46 anahtarın tamamının oyun içinde çalıştırıldığı anlamına gelmez. CLI dil
önceliği constructor'dan sonra ve config'den önce kalır.

## Doğrulama kapsamı

- `test_startup_compat.cpp`: gerçek Application, Service, parser, router ve XML
  yoluyla dosyasız/boş/kök/kısmi/override/boş alan senaryoları. Profilin 14 alanı
  ve settings yanıtı bütün olarak karşılaştırılır; dosya yaratılmadığı kontrol edilir.
- `test_runtime.cpp`: gerçek TCP, framing, concurrent profil istekleri ve persistence.
- `test_online.cpp`: kaynak online/cache dalları, yanlış format, auth başarısızlığı,
  provider yokluğu ve lifecycle. Bu test native ABI testi değildir.
- Windows fixture: gerçek DLL yükleme ile config yok/kısmi/yalnız override;
  sentetik version-2 provider ile başarılı profil ve geçerli/bozuk cached profile.
  Fixture kodunun bulunması testin çalıştırıldığı anlamına gelmez.
- Çalıştırılan sonuçlar `evidence/portable_debug.json`, `portable_sanitizer.json`
  ve dış FINAL_QA raporundadır. Windows/MSVC, original provider, Consumer ve
  tam Golden kabulü bu ortamda NOT_RUN.

## Açık kalan işler

- Özgün constructor'daki statik license cipher key, configuration::State'in
  parçası değil; bu projeksiyon ona yeni bir uygulama eklemez. Licensing
  constructor/commit eşdeğerliği EVIDENCE_BLOCKED/OPEN olarak kalır.
- Avatar byte'ları pinned donor örneğinden doğrulandı; oyun UI'sinde görselin
  gösterilmesi native/Consumer kabulünün parçası olarak açık kalır.
- Transport backend, yükleme/zamanlama, native ABI ve unload için önceki
  mimari sınırlar devam eder. Hook startup riskinin gerçek oyundaki etkisi
  doğrulanmadı. Bu çalışma o bulguyu kapatmaz.
- Native oyun isteğinin GetProfile'a ulaştığı ve UI'nin yanıtı kullandığı
  henüz gözlenmedi; mevcut kullanıcı semptomunun kapandığı iddia edilmez.
- BLUEFIELD'in eski özel cache dizini otomatik taşınmaz; kaynak konumu tekrar
  kullanılır. Başka proje veya kullanıcı dizinlerinde genel dosya taraması yoktur.

Bu dosya genel fonksiyon eşdeğerliği sertifikası değildir. FIX04 yalnız somut
config/profil sapmalarını düzeltir ve önceden eksik olan test senaryolarını ekler.
