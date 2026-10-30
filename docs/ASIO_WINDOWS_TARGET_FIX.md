# Asio Windows hedef tanımı

Production logundaki mesaj `Please define _WIN32_WINNT or _WIN32_WINDOWS` ve
`Assuming _WIN32_WINNT=0x0601` satırlarıdır. Bu bir Asio compile-target mesajıdır;
MSVC tarafında `#pragma message` ile basılır, build’i durduran hata değildir.

Sağlanan Brownfield CMakeLists.txt:313 iki değeri açıkça tanımlar:

```cmake
_WIN32_WINNT=0x0601 WINVER=0x0601
```

Yellowfield aktarımında bu tanımlar eksik kaldığı için Asio kendi varsayımını
her ilgili translation unit için yazıyordu. Değerler Windows için
`preloader_l_asio INTERFACE` target’ına eklendi; bu target’ı kullanan uygulama,
host ve testler header’lar okunmadan önce aynı tanımı alır.

Uyarı susturma flag’i veya vendor patch’i eklenmedi. Eski Asio fallback değeri
aynıdır. Tanım, Windows API header görünürlüğünü seçer; Golden PE OS/subsystem
6.0 metadata hedefini değiştirmez. Bluefield’de görülen0x0602 değeri bu görevin
Brownfield referansı değildir. Brownfield’in diğer compile seçenekleri bu küçük
düzeltme kapsamında otomatik kopyalanmadı.

Önceki paketten oyun başlangıcının çalıştığını kullanıcı doğruladı; bu kayıt
korunur. Güncel build ayarı için Wine/cross-build ve source QA sonuçları
`docs/evidence/ASIO_WINDOWS_TARGET_FIX.json` içindedir. Yeni kaynakla gerçek MSVC
production çıktısı bu ortamda çalıştırılmış sayılmaz.

Bu düzeltme Windows IO/Gulrak kaldırma değişikliğiyle birlikte teslim edilir:
[WINDOWS_IO_FIX.md](WINDOWS_IO_FIX.md).
