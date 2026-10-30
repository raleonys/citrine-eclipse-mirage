# Windows IO geri dönüşü ve kullanılmayan vendorun kaldırılması

Kullanıcı Windows filesystem farkının düzeltilmesini ve aktif başka kullanımı
kalmayan Gulrak’ın tamamen kaldırılmasını istedi. Önceki beş vendor hedefi bu
kütüphane için son kullanıcı talimatıyla değişmiştir.

| Alan | Güncel uygulama |
|---|---|
| NativeFactory input/output | Sağlanan RE adaptörü gibi std::filesystem::path + std::ifstream/ofstream |
| Dizin oluşturma/tarama/silme | Runtime namespace fs=std::filesystem; Windows/MSVC compiler-owned filesystem |
| Debug log akışı | std::ofstream; #ifndef NDEBUG guard korunur |
| Gulrak kullanımı | Uygulama, include, target veya test backend’inde kalmadı |
| Gulrak vendor | third_party/filesystem, CMake interface target, include bağı ve current lock kaydı çıkarıldı |
| IO testleri | Unicode/binary/rename/remove/error kontrolleri standard filesystem ile korunur |
| Kalan vendorlar | Asio, MinHook, PEGTL, TinyXML2; özgün member hashleri ve TinyXML2 500 aynı |
| Asio mesajı | Brownfield _WIN32_WINNT=0x0601/WINVER=0x0601 tanımları tüketici hedeflerine bağlı |

Gulrak’ın orijinal kullanıcı arşivi ve önceki kaynak paketleri dışarıda korunur.
Güncel product snapshot’ta vendor payload’ı yoktur; geçmiş metadata karşılaştırma
kayıtlarında adı köken bilgisi olarak kalabilir.

Bu dönüş Windows/MSVC standard filesystem implementasyonunu seçer. Güncel
MSVC CRT’nin tarihsel Anadius CRT’siyle bütün import/byte/edge-case davranışlarının
birebir aynı olduğu iddia edilmez. LLVM/Wine testleri Microsoft STL implementasyonu
için gerçek Windows testlerinin yerine geçmez.

## Production export ve log açıklaması

Production .def artık Brownfield ile aynı üç C export içerir: DllMain@1,
SURGE@2, DllShutdown@3. DllWaitReady/DllServerPort uygulaması ve ordinal4/5
kaldırıldı. Ayrı development DLL’e taşınanlar C++ kaynak API’leridir. Runtime
host hazır olmayı/portu ek public API yerine mevcut EALsxPort/LSX greeting ve
profile cevabıyla doğrular.

DllMain’in export edilmesi Windows loader başlangıcı için zorunlu değildir.
Windows CRT entrypoint’i DllMain’i çağırır. Export ordinal’leri sayısal ABI
kimlikleridir; alfabetik gösterim veya .def satır sırası çağrı/başlangıç/hook
sırasını belirlemez.

`surge.log` yazıcısı yalnız NDEBUG olmayan Debug build’de derlenir. Release
yeni log üretmez ve eski bir Debug loguna ekleme yapmaz; kullanıcı dosyası
otomatik silinmez. Mevcut logun bulunması tek başına Production’ın yazdığını
göstermez. Entegrasyon regresyonu her iki build’de bu durumu denetler.

Önceki startup-build-fix paketiyle oyun açılışı kullanıcı tarafından doğrulandı.
Bu yeni IO/build ayarı için gerçek Windows/MSVC ve oyun tekrarı ayrı olarak
gereklidir. Güncel sonuçlar `metadata/evidence/WINDOWS_IO_FIX.json` içindedir.

## İstenen import DLL sırası

Kullanıcı final listede orijinal Anadius sırasını istedi: CRYPT32, WS2_32,
KERNEL32, USER32, SHELL32, MSWSOCK. Bu, export ordinal sırası değildir. Finalizer
yalnız tam IMAGE_IMPORT_DESCRIPTOR kayıtlarını yer değiştirir; descriptorların
RVA/isim/thunk/IAT değerleri değiştirilmez ve diğer byte’lar guard ile korunur.
Modül kümesi farklıysa işlem fail-closed durur. Null descriptor korunur, checksum
yeniden hesaplanır; CORVEXON final sırayı ayrıca denetler.

Bu sıra yalnız Golden sonrası dist/preloader_l.dll için hedeflenir. Ham
build/preloader.unsigned.dll linker’ın sırasını korur. LLVM development DLL’i
dinamik CRT modülleri içerdiğinden shipping kabulü sayılmaz; aynı permutation
yardımcısı üzerinde canlı load/LSX/shutdown/unload kontrolü ayrı kanıtlanır.
