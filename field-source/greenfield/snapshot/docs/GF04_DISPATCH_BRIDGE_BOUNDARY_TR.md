# GF04 — Route selection ve teslim kararının doğrudan aktarımı

**RESEARCH/DEVELOPMENT ONLY — NOT_RELEASE.** GF03 kaynak ağacı değişmez parent olarak kullanılmıştır.

## Gerçekten derlenen özgün HF19 kodu

- `route_lookup.cpp` ve `route_inventory.cpp`: 42 isimli route + alias/uyumluluk envanterini orijinal hash ve kesin isim karşılaştırmasıyla seçer. Bu seçim gerçek request/handler dispatch işlemi değildir.
- `socket_exchange_write_completion.cpp`: tam yazma başarısını görmeden response/event teslimini tamamlanmış saymayan production completion FSM. Grant/store/event mutation ve ACK üretmez.
- Özgün `test_router.cpp` byte-identical kullanıldı. Bağımsız `test_route_delivery_contract.cpp` bilinen/yanlış isimleri, yanlış büyük-küçük harfi, hash collision davranışını ve eksik yazma/retry reddini doğrular.
- Önceden taşınan production Asio owner dosyaları aynı derleme grafiğinde ayrı `gf_hf19_asio_owner` kütüphanesinde derlenmeye devam ediyor; TCP testleri gerçek loopback kullanıyor, stub classifier/dispatcher ile.

## Henüz bağlanmayan production zinciri

`SURGE` -> bootstrap/configuration -> `ReleaseServerContext` -> `asio_release_bridge` -> `AsioServiceOwner`/`AsioSessionOwner` -> handshake/session crypto -> frame classification -> `request_dispatch` -> handler registry/service store -> prepared transaction -> send completion -> events/rollback -> `DllShutdown` owner drain.

Eksikler: `src/core/request_dispatch.cpp` handler registry/config lease/event state ve tüm route handler setine bağlı; `src/runtime/asio_release_bridge.cpp` ve `release_server_*` Windows IOCP ve model-service/Win32 resource state'e bağlı; production Asio session code için callback/dispatcher içinde test stub'u yasaktır. Tam zincir bağımlılık ve kaynak sahipliği kanıtı olmadan SURGE'ye bağlanmaz.

## ABI ve risk sınırı

`preloader_l.def`: `DllMain@1 SURGE@2 DllShutdown@3` değişmedi. `SURGE` hâlâ no-op; DllShutdown yalnız mevcut zero-owner lifecycle'ı kapatıyor. Kullanıma hazır LSX servis veya oyunda çalışabilirlik iddiası yok. Windows native ve gerçek Consumer kabul NOT_RUN.

## Sonraki entegrasyon kararı

GF05 kapsamı sadece `request_dispatch`+handler graph **ve** gerçek Asio worker/bridge/shutdown sahipliğini birlikte çözmek olmalı; yarım bağlantıyı shipping'e taşımayın. Öncelik gerçek Windows MSVC /MT ve /MTd / export/load kabulüdür.
