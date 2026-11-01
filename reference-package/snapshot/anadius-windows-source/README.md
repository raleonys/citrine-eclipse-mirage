# Lisans akışına Windows status ve yol bağlantısı

`WindowsEffects`, lisans Effects arayüzündeki status, lexical yol ve encoding çağrılarını somut kurtarılmış modüllere bağlayan yeni birleştirme kodudur. Ayrı özgün fonksiyon veya kurtarılmış özgün sınıf tanımı iddiası değildir. Kaynaklar `../anadius-status-source`, `../anadius-path-source`, `../anadius-encoding-source` klasörlerindedir; bu klasör kendi başına DLL üretmez.

Bağlantının sırası önemlidir:

```cpp
// app: çağıranın sağladığı kalan uygulama/exception/provider etkileri.
status_recovered::NativeStats stats;
encoding_recovered::NativeCalls encoding;
encoding_recovered::PortableErrors errors;
windows_recovered::WindowsEffects windows(app, stats, encoding, errors);
file_recovered::NativeFactory io;
file_recovered::NativeTextEnvironment text;
file_recovered::LicenseFiles complete(windows, io, text);
license_recovered::Handlers handlers(state, complete);
```

`LicenseFiles` dışta olmalıdır: iç token okuyucusunun status çağrısı böylece WindowsEffects→NativeStats'a ulaşır. Nesneler ve caller state referansları tüm senkron çağrı süresince yaşamalıdır. Kalan saat, online sağlayıcı, log, dialog, exit ve özgün filesystem/no-response exception temsili `app` üzerinden zorunlu devredilir; başarılı varsayılan gövde eklenmez.

Status'tan gelen Win32 error bitleri signed lisans alanına aynı bitlerle taşınır; permissions özgün lisans çağrı yeri gibi kullanılmaz. Lexical filename/parent/join UTF16 kod birimleriyle çalışır. Dosya sistemi genişletmesi CRT/ANSI/OEM kuralını, path→UTF8 dönüşümü ayrı sabit65001 yolunu kullanır.

`build.ps1`, gerçek status/path/conversion/I/O/SHA1/CBC/XML kodunu birleştiren test EXE'sini derler. **23 ortak kontrol** yalnız üretilen `build/windows-fixtures` üzerinde çalışır: Unicode dosya adı, regular/directory/missing status, UTF8 baytları, bilinen dosya SHA1'i, normal token dosyası, missing cached dosyası, invalidate/blocked state ve XML kaçışları. Status, yol, dönüşüm, file/hash/token bu testte mock değildir. Kalan uygulama etkileri kontrollüdür; özgün DLL/EXE/CFG veya gerçek token okunmaz/yüklenmez/çalıştırılmaz/değiştirilmez.

Sınırlar korunur: NativeStats yalnız flags3/cachedattrs=-1 yoludur; reparse tag başlatılmamış alana dayanacaksa OutOfEvidenceStatus verir. Path self-append'in allocation'a bağlı köşesi açıkça sınırlıdır. Linked CRT locale, filesystem sharing, exception/MSVC ABI ve tüm çalışma zamanı eşitliği kanıtlanmadı. Online/ASIO/hook/global başlangıç bağlantıları bu klasörle tamamlanmış değildir.
