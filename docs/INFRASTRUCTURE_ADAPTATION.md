# Brownfield aktarımı

Kaynak: kullanıcı tarafından yüklenen HF20 full-source candidate ZIP'i;
frozen Brownfield değiştirilmedi. Dosya düzeyinde input/output SHA kayıtları
[TOOLING_PROVENANCE.json](../metadata/evidence/TOOLING_PROVENANCE.json) içindedir.

Seçilen parçalar: beş build yardımcısının operatör UX akışı ve paylaşılan MSVC
yardımcıları; dört gerçek PE Golden aracı; MSVC link policy denetleyicisi;
PE ve restricted-import negatif testleri. Metadata ownership ve FALCION
mekanikleri, Brownfield uygulama/sürüm bağımlılıkları temizlenerek uyarlanır.

Brownfield'in debug/runtime/session/socket/native/release host'ları kendi export
ABI'sini ister. Bunları kopyalayıp Yellowfield'de olmayan exportları varmış gibi
göstermek yerine mevcut dinamik source host'u, yeni standart CRT probe ve
Yellowfield CTest hedefleri kullanılır. Bu host'lar geldiğinde tam oyun/native
kabulün tamamlandığı ileri sürülmez.

Eski uygulama route/state/Asio testleri ve geçmiş release/Library/G0 kayıtları
taşınmaz. Gerekçe ve incelenen dosyalar güncel aktarım/uyarlama kayıtlarındadır.
SURGE görseli özgün kullanıcı girdisinden korunur; Brownfield'in üçüncü taraf
ürün metadata'sı resource dosyasına alınmaz.
