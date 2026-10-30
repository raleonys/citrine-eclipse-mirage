# Araçlar

- `preloader_l_source_host`: geliştirme DLL'ine dinamik bağlanır; calendar,
  SHA-1 ve parser çağrılarını gerçekten çalıştırır.
- `preloader_l_crt_host`: compiler standart CRT heap/unwind/container/thread/TLS kontrolü.
- `inspect_dll.py <dll>`: read-only PE/import/export/resource raporu ve orijinal SURGE
  görselinin RCDATA 101 içinde SHA eşitliği. `python -B` ile çalıştır.

Golden araçları `scripts/` altında, build komutları `scripts/build_orchestration/`
altındadır. Brownfield'in kendine özgü native/debug/session ABI host'ları kopyalanmaz.
Bu araçlar mevcut SURGE arayüzlerini sınar; tam runtime/game kabulü sağlamaz.
