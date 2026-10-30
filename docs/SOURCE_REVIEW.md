Bu belge önceki delta incelemesinin kaydıdır. Sonraki geniş tarama ve yeni düzeltmeler: [tüm kaynak denetimi](BROAD_SOURCE_AUDIT.md).

# Kaynak deltası, düzeltmeler ve test katkıları

Yeni kullanıcı RE paketi incelemesindeki dört bulgu düzeltildi. Eklenen karşılaştırmalı test ayrıca ikinci document/configuration girişinin Config2 kökünü kaybettiğini ortaya çıkardı; bu beşinci adaptör hatası da düzeltildi. Bu belge tüm özgün DLL ile kaynak/ABI eşdeğerliği iddiası değildir.

## İncelenen kapsam

Önceki arşive göre yeni12modülün33CPP/HPP dosyası ve değişen11öncekiCPP/HPP dosyası tek tek sınıflandırıldı. Paket içindeki24yeni testCPP için uygunluk/karşılık kaydı çıkarıldı. Ana pseudocode/assembly dökümleri öncekiyle byte-identical; kaynak bileşimi ilerlemesi yeni native fonksiyon gövdesi sayılmaz.

- [44 kaynak/başlık için karşılık ve karar](evidence/SOURCE_DELTA_REVIEW.csv)
- [24 yeni test dosyası için uyarlama kararı](evidence/SOURCE_TEST_ADOPTIONS.csv)
- [Güncel politika](../metadata/CONFIG_PLATFORM_POLICY.json)
- [Çalıştırılmış doğrulama](evidence/SOURCE_REVIEW_VALIDATION.json)

Tablodaki işlev adları kaynakta gezinmek içindir; çağrı adlarını da içeren bir metin envanteridir, compiler AST veya yürütülmüş satır kapsamı değildir. Uygulama sahibinden gerçek Windows adaptörüne giden ilgili çağrı aileleri ayrıca incelenmiştir. İlgisiz private CRT/domain yolları ürün kapsamına alınmamıştır.

## Uygulanan değişiklikler

| Konu | Önce | Sonra / yeni regresyon |
|---|---|---|
| UTF8 CFG yolları | resolve_path filesystem ANSI/OEM codepage seçiyordu. | Özgün0192d0→0145d0 gibi UTF8/65001; ASCII, Latin/Türkçe/CJK/nonBMP, absolute/UNC/relative, counted NUL, malformed input ve iki gerçek CFG yükleme yolu sınanır. Ayrı filesystem-codepage işlemleri korunur. |
| Avatar ilk açma hatası | binary writer false sonucuna ek runtime_error üretiliyordu. | İlk open/state hatası göz ardı edilir; alt write/close exception'ları yayılır. Gerçek açılamayan yol ve configure devamı sınanır. Invalid image enum dosyaya erişmeden reddedilir; sentinel dosyanın değişmediği kontrol edilir. |
| Dizin yan-etki sırası | Bütün liste alındıktan sonra filtre/silme yapılıyordu. | Ortak cleanup_cached_avatars, senkron visitor üzerinde exact `_AVATAR_` filtresini işler. Somut Environment std::directory_iterator kullanır; callback bitmeden increment olmaz. Vector API eski sağlayıcılar için optional fallback'tir. Geç enumeration ve filename/narrow/remove hataları, önceki etkilerin korunması, sonraki etkilerin durması sınanır. |
| Thread hataları | Genel runtime_error yalnız indeks metni taşıyordu. | Yedi binary tablo kodu16/22/3/12/1/36/11 + generic category + context; gerçek Windows Execution ve Events aynı standart compiler helper'ı kullanır. Invalid indeksler güvenli reddedilir. |
| Document kökü | Environment.load_document seçilmiş Config2 alt ağacını tam document sanıyordu; üst çağıran tekrar child(Config2) arıyordu. | document arayüzü Config2'yi kendi kökü altında döndürür. Aktif hash girişinin subtree sözleşmesi değişmez. Ana/override dosyaları iki adaptörden de beklenen Unicode yolu verir. |

Ek savunma: UTF8 giriş boyutu SDK int sınırından önce kontrol edilir; bilinmeyen avatar kimliği fallback olarak kabul edilmez. Bunlar yeni bir private CRT veya farklı filesystem backend'i gerektirmez.

## Test katkısı ve sınırlama

Yeni configuration_platform_contracts yalnız production core'a bağlanır. Gerçek dosya/parser/hash/adaptör yollarını ve NativeFactory'yi çalıştırır. Konfigürasyon unit fixture'ları gerçek initialize_configuration gövdesine kontrollü stream/callback hataları verir. Late FindNext failure için gerçek Windows API enjeksiyonu yapıldığı iddia edilmez: mevcut Win32/std iterator uygulamasında normal silme, callback exception, reentry/nested cursor ve compatibility collection ayrıca gerçek dosyalarla sınanır.

Native callback fixture'ı artık Effects/Context/Binding kurulduktan SONRA original slot, map ve seçilmiş portu değiştirir. Callback sonuçları aynı owner depolamasından geldiği için equal-valued snapshot kopyaları bu testi geçemez. Busy close/drain/retry ve LastError testleri zaten vardır ve korunur. Yeni callback/domain paketleri aynen taşınmaz.

Binary output testine write exception sırasında tam bir destructor ve uydurulmuş explicit close olmaması kontrolü eklendi. Mevcut close-exception/open-failure testleri korunur. Başarısızlıklar başarı gibi normalize edilmez.

Karşılaştırılan test ailelerinden literal stream flush/private PTD/FLS/single-controller admission modelleri mevcut standard-CRT ürünün sözleşmesi değildir. Doğrudan taşınmaları yanlış bir davranışı zorunlu hale getirirdi. ByteWide staging'in arbitrary/unaligned domain'i bu tur üretime alınmadı; mevcut typed SDK pointer sözleşmesinde yeni bir crash gösterilmedi. Debug log formatı ve ürün UI başlığı mevcut onaylı politikalardır. Std::filesystem create/remove motoru korunur; farklı compiler sürümlerinin bütün error-prefix'leri orijinalle eşdeğer ilan edilmez.

C++ Dependencies arayüzüne optional virtual visitor eklendi; geliştirme C++ tüketicileri bütün kaynaklarla yeniden derlenmelidir. Üç production C export/ordinal, hook11/static Crypt32, metadata/PNG politikası, dört vendor pini,64production/74referenceTU ayrımı ve önceki onaylı transport/persistence/lifetime kararları değişmez.

## Kabul sınırı

Kaynak denetimi, kontrollü regresyon, Wine cross-build ve native Windows/game kabulü ayrı tutulur. Önceki build için kullanıcının Windows/Golden/oyun başarısı tarihsel kanıt olarak korunur; güncel kod için native MSVC/game tekrar koşulmalıdır. Beş düzeltme ve genişleyen testler, başka hiçbir hata bulunamayacağının ispatı değildir. Çalıştırılmamış platform/domain koşulları doğrulama kaydında açık bırakılır.
