# GF05 — Özgün handler dispatch kaynakları ve eksik üretim bağımlılıkları

**GREENFIELD / SOURCE_LINKED / NOT_RELEASE.** GF04 parent ve resmî HF19 kaynağı SHA sabittir.

## Gerçekte yapılanlar

- `request_dispatch.cpp`, `handler_registry.cpp` ve `release_server.cpp`: HF19 kaynakları birebir; GCC ve Clang tarafından ayrı `gf_hf19_dispatch_units` statik arşivinde derlendi. `nm` sonucu makine-kodlu exportları ve çözümlenmemiş bağımlılıkları `metadata/GF05_DEPENDENCY_GRAPH.json` içinde kayıtlıdır.
- Windows-native `asio_release_bridge.cpp` ve `.hpp` aynı özgün kaynakla taşındı. Windows IOCP zorunluluğu kaldırılmadı; bridge portable koda çevrilmedi. `gf_hf19_bridge_units` yalnız `WIN32 AND MSVC` içinde ayrıca derlenmek üzere hazırlanmıştır. Bu ortamda Windows derlemesi yapılmadı.
- 42 yeni dosya + 58 önceki GF01–GF04 dosyası toplam 100 SHA-identical HF19 kaynağı oluşturur. 635 upstream Asio dosyası değişmedi.
- `gf_preloader` target'ı aynı no-op `SURGE` ve zero-owner `DllShutdown` sözleşmesini korur. Dispatch/Asio/bridge hedefleri DLL'ye bağlanmadı. Sahte runtime/handler stub'u eklenmedi.

## Eksik kapanış

Derlenen dispatcher, orijinal HF19 `RequestView` parser'ını, Config2 read lease'ini, event queue'yu ve çok sayıda route handler gövdesini çağırır. `gf_hf19_dispatch_units` statik arşivi bu sembolleri henüz çözmez. Arşivin başarılı derlenmesi final link veya LSX çalışması değildir.

Gerçek çalışma için: config + parser + handler sources + event queue + transaction/lease; ardından native `ReleaseAsioBridge` + lifecycle/port/retry + shutdown/fail-close; sonunda `SURGE/DllShutdown` tam owner yayın/geri alma, localhost gerçek LSX, Windows MSVC /MT ve /MTd yükleme kabulü gerekir.

## Windows adımı

`script\RUN_WINDOWS_GF05.cmd Release` ve `Debug` x64 Native Tools Developer Prompt'ta çalıştırılabilir. Bu komutlar derleme + export testini hedefler; çalışmadıkları müddetçe sonuç `NOT_RUN` kalır.

**Yeni resmî release / CURRENT_ARTIFACT değişikliği yok.**
