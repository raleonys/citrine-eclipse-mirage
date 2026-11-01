# GF08 — Win32 ReleaseServer sahipliği ve dosya yolu sınırı

- GF07 parent immutable; canonical 3.2.5 SQUALERIS HF19 ve `CURRENT_ARTIFACT` değiştirilmez.
- GF08, üç **gerçek özgün HF19 testi** (`test_filesystem_path`, `test_asio_session_timeout_policy`, `test_asio_delivery_completion`) ile platformlar arası sözleşmeleri ölçer.
- `third_party/filesystem` içindeki 8 dosya (tüm upstream `ghc` header'ları, LICENSE ve commit kaydı) kaynak kimliği korunarak doğrudan alınmıştır.
- `release_server_acceptor.cpp`, `release_server_lifecycle.cpp`, `release_server_worker.cpp` ve `winsock_native.cpp` sadece Windows-only ve opsiyonel ayrı OBJECT derleme hedefine alınmıştır. Bu hedef **shipping DLL ile bağlı değildir**.
- Kaynakları yeni namespace veya sahte Win32 yordamlarıyla yeniden yazmak YASAK. Windows-only `#error` korunur.
- `gf_preloader` halen GF00: `SURGE` no-op ve `DllShutdown` sıfır kaynak dönüşü. `DllMain@1`, `SURGE@2`, `DllShutdown@3` kaynak export sözleşmesi değişmedi.
- Linux GCC/Clang native Windows x64 MSVC/`/MT`/`/MTd`, PE export tablo ve `LoadLibraryW` testinin yerini tutamaz.
- Windows x64 Visual Studio Native Tools Prompt: `scripts\RUN_WINDOWS_GF08.cmd`. Release ve Debug derlemeleri, CTest export host testi, orijinal Win32 owner TU object compile ve SHA receipt denenecek. GF08 araştırma hattında hiçbir resmi yayın yapılmaz.
- Windows build/link başarısızsa gerçek hata ve loglar döndürülmeli; başka platformdaki PASS kanıtlarıyla kapatılamaz.
- Testlerde gerçek Windows IOCP oturumu, bütünleşik handler dispatch, Config2 disk erişimi ve CollegeFB27/Madden27 çalıştırılması iddia edilmez.

## Gelecek öncelikleri

1. Native GF08 owner object + DLL/LoadLibrary export kabulü.
2. Tam `request_dispatch` transitive link haritası ve eksik Windows runtime state sahipleri.
3. `SURGE` / `DllShutdown` altında tek oturum/tek lease/tek shutdown owner yaşam döngüsü; ancak native kabul ve gerçek uygulama bağlandıktan sonra.
4. Gerçek oturum üzerinden sözleşme testi; test-only socket classifier ile shipping behavior ayrımı korunacak.
