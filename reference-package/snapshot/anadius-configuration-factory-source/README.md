# B52 — Somut configuration Resources factory

`configuration_factory::Configuration`, configuration arayüzünün 21 zorunlu metodunu ve streaming visitation yolunu sahipli tek nesnede toplar. Tam bir soyut `configuration::Dependencies` lower'ı istemez. 19 rota mevcut gerçek uygulamaları paylaşır: Core13, filesystem3, persistence2 ve logging1. Lisans uygulaması ile legacy document okuma, zorunlu ve sahipli iki dar etki olarak kalır. Özgün lisans işlemcisi veya tam legacy collection-order uyumluluğu uydurulmaz. Güncel production SourceFilePass gerçek parser/hash/application yolunu zaten kullanır ve legacy `load_document` metodunu bypass eder.

`Inputs`; stream+backing lifetime, filesystem, encoding/error sağlayıcıları, SDK tabloları ve persistence kaynaklarını taşır. Achievement Paths çağıran tarafından verilir. Constructor callback yayınlamaz veya I/O yapmaz. Configuration çağrıları serialize edilir; factory'nin oluşturulduğu thread ile startup thread'inin aynı olması zorunlu değildir. Sahipler uygulama/worker/callback kullanıcıları bittikten sonra emekli edilmelidir.

Ortak yardımcılar eski decorator'lar tarafından da kullanılır. Eski public virtual arayüzler ve HOST alanları korunur; persistence decorator'ının eski nested Impl bildirimi ve tek state allocation'ı, paylaşılan state üzerindeki ince köprüyle sürer. Signed-tolower sınırı, filesystem/UTF-8 codepage ayrımı, 0x104 SDK buffer büyümesi/LastError/ilk geçiş uzunluğu, entry işleme-silmenin FindNext öncesinde olması, conversion öncesi Parsing prefix'i ve conversion sırasında shared stream lease'inin bırakılması korunur. PNG void yolu write_binary false sonucunu önceki gibi yok sayar; exception'lar yayılır.

Destructor, bütün opaque sahipler bırakılırken Impl'i bağlı ve Retiring tutar. Bu aşamadaki bütün factory metotları kaynak/SDK çağırmadan reddedilir. Normal streaming visitor'ın aynı nesnede filename/remove çağırması desteklenir; genel bir normal-call reentry veya creator-thread kilidi eklenmez. Narrow effect capture'ları kendi backing state'ini veya Inputs lifetime'ını sahiplenmelidir. Keyfî concurrent kullanım, destructor sırasında owner'ı silme ve tüm image/callback admission'ı bu politikanın kanıtı değildir.

Yeni production dosyaları `factory.hpp/.cpp`; diğer iki C++ dosyası testtir. Önceki sekiz header/CPP yalnız lower-free yardımcıları paylaşmak için güncellenir. Testler gerçek source algoritmalarını sentetik memory file/SDK/stream/effect sağlayıcılarıyla kullanır. Birebir özgün MSVC ABI/EH, gerçek kullanıcı dosyaları, ağ veya original DLL çalıştırılması iddia edilmez.

DLL link tanısı, ortak build'in hash'i doğrulanan `configuration_factory.o` nesnesini açıkça alır; kaynak/common compile command/link argument/verbose consumption kanıtı saklanır. Opt-in CMake target aynı production kaynağını doğrudan içerir; CMake target'ın çalıştırıldığı iddia edilmez. Bu nesne seçimi loader factory çağrısı, runtime reachability veya başarılı PE üretimi değildir.

Tam DLL henüz üretilmedi. Kalan Bootstrap Services/Resources bileşimi, iki dar configuration etkisi, dış rawjob/foreign/hook finality, erken startup hata yolları ve loader attach/detach sözleşmeleri açık kalır. Yeni native gövde veya ABI kapanışı yoktur; toplam DLL yüzdesi ölçülmüş değildir.

[Kanıt](evidence.json) · [Bağımsız production incelemesi](architecture-review.md) · [Link yardımcıları incelemesi](gate-auxiliary-review.json) · [Sınırlar](resources-audit.json)

## B52 ortak doğrulama

**170 çekirdek + 96 entegrasyon = 266 kontrol PASS.** Ortak build 272 nesne (268 C++ / 4 C), 333 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 926 kaynak aynı; 8 önceki dosya açık hash kaydıyla güncellendi; toplam 938 kaynak. Yeni typed normal gövde 0, yeni native ABI/provider kapanışı0; 501 adres, 505 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
