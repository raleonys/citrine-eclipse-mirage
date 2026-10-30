> Bu tablo ilk inceleme snapshot’ıdır. Kullanıcının sonraki talimatıyla standart
> Windows IO geri getirildi ve Gulrak kaldırıldı: [güncel durum](WINDOWS_IO_FIX.md).

# Windows filesystem farkının kaynağı

`gulrak/filesystem`, `ghc::filesystem` namespace’ini kullanan taşınabilir
kütüphanedir; Windows’u da destekler. Mac’e özel değildir. Anadius’un Mac
sürümünün backend seçimi, sağlanan Windows DLL kanıtından çıkarılamaz.

## Eldeki kanıt

| Katman | Kaynak / gözlem |
|---|---|
| Anadius Windows binary RE | `anadius-all-functions.pseudo.c` içinde `__std_fs_open_handle`, `__std_fs_code_page` ve MSVC filesystem yardımcıları |
| RE file source adapter | Sağlanan `anadius-file-source/native_io.cpp`: `std::filesystem::path` ve standart fstream |
| Yellowfield aktif IO | `src/platform/file_io_windows.cpp`: ghc path/stream; runtime internal namespace fs=ghc::filesystem |
| Yellowfield silme çağrısı | `Environment::remove_04948c` -> `fs::remove`; avatar cache temizliğinde kullanılır |
| ghc Windows silme | GetFileAttributesW, gerekirse SetFileAttributesW; sonra DeleteFileW veya RemoveDirectoryW |
| Anadius Windows silme | Handle açma; SetFileInformationByHandle disposition/basic-information dalları ve hata fallback’leri |

Fark, Yellowfield runtime bağlama görevinde ghc’yi aktif dosya IO ve storage
yolu olarak seçmemizden oluştu. Vendor arşivinin byte’ları değiştirilmedi;
uygulamanın hangi backend’i çağırdığı değişti. Aynı C++ filesystem API’sini
hedeflemeleri, Windows hata/attribute/paylaşım davranışlarının byte-byte veya
her durumda eşdeğer olduğunu kanıtlamaz.

Kütüphaneyi projeye vendorlamak tek başına bütün Windows dosya işlemlerini
ona taşımayı gerektirmez. Standart MSVC filesystem de derleyicinin C++
kütüphanesidir; gulrak ayrı bir uygulamadır. Bu tercih Mac/Windows ayrımı
gibi sunulmamalıdır.

Bu karşılaştırma yeni bir backend değişikliği uygulamaz. Mevcut görevde yalnız
Brownfield Windows API compile-target tanımları düzeltilir. Dosya silme/IO
yolunun değiştirilmesi ayrıca kendi davranış ve hata testleriyle ele alınmalıdır.

Son kullanıcı talimatı uygulanmıştır: Windows IO standard filesystem/fstream’e
geri bağlandı ve aktif başka kullanımı kalmayan Gulrak üründen kaldırıldı.
Yukarıdaki ghc tablosu değişiklik öncesini açıklar. [Güncel uygulama](WINDOWS_IO_FIX.md).
