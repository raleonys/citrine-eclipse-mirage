# Kaynak ve build sınırı

## Önceki çıktının düzeltilmesi

Önceki `src/recovered` düzeni, RE çalışma alanının büyük ölçüde taşınmasıydı.
Anadius uygulaması, özel CRT kurtarımı, ayrı preloader ve test kaynakları aynı
ağaçtaydı; ana hedef yalnız statik arşivler üretiyordu. Bu, Brownfield tarzı bir
uygulama projesi değildi. Önceki paket ve çalışma ağacı referans olarak korunur;
güncel ürün ağacının build girdileri değillerdir.

## Güncel seçim

- Anadius64 uygulama kaynakları: config, bootstrap sözleşmeleri, temel thread
  launch akışı, protocol/mesajlar, handler'lar, transport/IOCP, XML, registry,
  dosya, zaman, hash ve native platform bağlayıcıları.
- Ayrı preloader Core/Once/process-enumeration/mutex ve version wrapper projeleri
  alınmaz. Sadece açıkça istenen CryptQueryObject davranışı bağımsız platform
  arayüzüne ayrılır. Host artık dört allocation/trap işlemi ister; bütün eski
  preloader Host/HookEngine bağımlılığını taşımaz.
- Özgün özel CRT/VCRT kurtarım paketleri ve onlara bağlı tarihsel B2-B6
  composition deneyleri ürün hedefinde yoktur. Bunlar standart CRT'nin yerine
  otomatik bağlanmaz. RE girdileri ve önceki paketlerden erişilebilir.
- Dört aktif vendor, kullanıcının ayrı sunduğu `3RD PARTY` arşivlerinden alınır.
  Gulrak son kullanıcı talimatıyla kaldırıldı; orijinal arşiv korunur.
  Eski RE MinHook/TinyXML2 kopyaları aktif ağaçtan çıkarılır; Brownfield vendor'ı
  alınmaz. Vendor byte'ları ve TinyXML2'nin 500 varsayılanı değiştirilmez.
- Testler başlıklardan ve uygulama .cpp dosyalarından ayrıdır. Ürün kapsamından
  çıkarılan CRT/preloader testleri başarısızlık gizlemek için değiştirilmez;
  önceki pakette korunur ve kapsam listesinde tek tek belirtilir.
- Brownfield aktarımı yerel build yardımcıları, gerçek Golden araçları/zinciri,
  uyumlu negatif testler, metadata/script sahipliği, kaynak QA, beş operatör
  profili ve düz kaynak paketlemeyi kapsar. Uygulama kodu taşınmaz.

Güncel kapsam için
[BROWNFIELD_ADAPTATION.md](INFRASTRUCTURE_ADAPTATION.md) ve aktarım matrisi geçerlidir.
Final ürün Golden kabulü, eksik metadata/ABI sözleşmesi nedeniyle henüz yoktur.

`metadata/evidence/SOURCE_LAYOUT.json`, her taşınan dosyanın önceki yolunu ve hash'ini,
uygulanan yol/namespace dönüşümlerini, seçilen ve dışarıda tutulan modülleri içerir.
Bu, native orijinal davranış eşdeğerliği veya bağımsız kullanıcı imzası değildir.

## Gerçek DLL ve eksik ürün bağlantısı

`preloader_l` bir SHARED hedefidir; bütün seçili uygulama objeleri gerçek DLL
linkine girer. `preloader_l_source_host` statik arşive değil bu DLL'e bağlanır.
Bu, önceki statik-collection çıktısından farklı ve doğrulanabilir bir build'dir.

Derlenen DLL'in geliştirme C++ exportları nihai oyunun/Consumer'ın public ABI'si
değildir. Otomatik başlangıç, somut uygulama owner/sağlayıcı bağlantıları,
SURGE ABI'si ve gerçek native hook/teardown kabulü tamamlanmadan ürün çalışıyor
iddiası yapılmaz. Eksikler boş başarı döndüren implementasyonlarla kapatılmaz.
