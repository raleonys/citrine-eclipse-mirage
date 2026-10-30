# Root düzeni ve sahiplik

| Yol | Sorumluluk |
|---|---|
| `src/`, `include/preloader_l/` | RE'den seçilen uygulama; ayrıntılı köken SOURCE_LAYOUT.json |
| `cmake/` | Açık kaynak listeleri, Windows DLL, taşınabilir ve altyapı test hedefleri |
| `scripts/build_orchestration/` | Beş kayıtlı insan komutu, otomasyona açık PowerShell profilleri |
| `scripts/` | Golden araçları, kaynak QA, manifest ve paket kontrolü |
| `tools/` | Uygulama C API yükleyicisi, ayrı source host, compiler CRT host'u, read-only PE/resource raporu |
| `exports/` | Uygulamanın sabit altı isim/ordinal C export tablosu |
| `tests/unit/`, `tests/native/` | Mevcut Yellowfield kaynak/adapter testleri |
| `tests/infrastructure/`, `tests/support/golden/` | Uyarlanmış altyapı testleri ve açıkça sentetik politikalar |
| `resources/branding/` | Kullanıcının byte-pinned SURGE PNG'si; RCDATA 101 |
| `third_party/` | Kullanıcının beş orijinal arşivinden birebir kaynak ağacı; Brownfield vendor'ları alınmaz |
| `docs/`, `governance/`, `metadata/` | Tekil güncel dokümanlar, kapsam ve makine kayıtları |
| `build/`, `dist/` | Geçici yerel çıktılar; kaynak ZIP'ine girmez |

Brownfield'in topyekûn uygulama ağacı veya araştırma arşivleri bu köke alınmaz.
Kaynak paketinin kökü düzdür: ZIP açıldığında `CMakeLists.txt` doğrudan köktedir.
Golden akışı [GOLDEN_PROTOCOLS.md](GOLDEN_PROTOCOLS.md), operatör komutları
[DEVELOPMENT.md](DEVELOPMENT.md), kabul sınırları [VALIDATION.md](VALIDATION.md).
# Güncel dosya düzeni

Ana kaynak/header ağacı Brownfield gibi yalnız tek modül katmanı içerir:

```text
src/net/asio_service.cpp
src/net/transport_acceptor.cpp
src/platform/hook_setup.cpp
src/platform/dll_entry.cpp
src/runtime/achievement_storage.cpp
src/runtime/calendar.cpp
src/xml/native_document.cpp

include/preloader_l/net/asio_service.hpp
include/preloader_l/platform/hook_setup.hpp
include/preloader_l/runtime/achievement_storage.hpp
include/preloader_l/runtime/calendar.hpp
include/preloader_l/xml/native_document.hpp
```

`src/<module>/<file>` ve `include/preloader_l/<module>/<file>` altında ek modül
alt klasörü yoktur. Eski `transport`, `session`, `hooks`, `hook_engine`,
`windows`, `achievements`, `events`, `time`, `files`, `native` katmanları dosya
adlarıyla ifade edilir. Taşıma öncesi kaynak paketi korunur. 177 dosya taşınmış;
261 kaynak/header gövdesi include satırları dışında birebir doğrulanmıştır.
[Tam yol eşlemesi](../metadata/evidence/LAYOUT_MIGRATION.json).

