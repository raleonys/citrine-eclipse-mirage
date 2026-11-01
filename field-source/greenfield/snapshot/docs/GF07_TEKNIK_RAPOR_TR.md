# GF07 — Gerçek Config2 / Entitlement / Presence entegrasyonu

**2026-10-19 — NON_RELEASE_WORK / NOT_RELEASE**

## Sınır ve sonuç
GF06 (SHA-256 `5b098264b0df613cdc2f2b5c2950ef277ed29eb55aeeec9bb4060df815e291a1`) üzerine eklenen 21 kaynak/test/header üyesi birebir HF19 ZIP'ten alınmıştır. Böylece toplam 197 adet HF19 dosyası değiştirilmeden taşınmış olur. Eski Asio 635 dosya ve PEGTL 364 dosya korunmuştur.

Gerçek HF19 `config::runtime_configuration`, Config2 parser, service state, LSX request parser ve gerçek `QueryContent`, `QueryEntitlements`, `QueryPresence` handler'ları *ayrı test executable'ında* birlikte link edildi. 7 özgün HF19 test kaynağı (Content empty/descendant, entitlement grammar, mixed Presence/Content/Entitlements, Config parser hardening/section policy/runtime parser) Linux'ta gerçek kaynakla çalıştırıldı ve PASS. `test_presence_content_entitlements.cpp` gerçek LSX XML request'lerini parse edip yanıtları, nested-field authority, yanlış filter, invalid reload sonrası önceki state'in kalmasını ve clear sonrası boş sonucu kontrol ediyor. Bu, GF06'nın yalnız ayrı .a compile sınırından anlamlı bir ilerlemedir.

## Kalan engeller
Tam original `request_dispatch` testini portable full link etmeyi denedik. `avatar_path.cpp` gerçek kaynak dosyası `#error avatar_path.cpp is Windows-only` nedeniyle Linux'ta derlenemez; boş bir Win32 shim ile kandırılmadı. `test_dispatch`, `test_runtime_configuration_routes` ve `test_config_overlay_pair` orijinal testleri pakete taşındı, fakat yalnız **opt-in** Windows deep link probe altında etkinleştirilebilir. Testlerin Windows'ta link/geçişi henüz kanıtlanmadı. `gf_preloader` içindeki `SURGE` no-op, `DllShutdown` sıfır-owner; release IOCP bridge ve shipping LSX sunucusu gerçek bağlantısı yok.

## Kanıt doğruluk kuralı
`source_linked_integration` = gerçek kaynakla test executable bağlantısı; **`shipping_DLL_linked` değildir**. Yükleme, Windows x64 `/MT`/`/MTd`, export/ordinal, native IOCP, gerçek oyun ve Consumer testlerinin hiçbirisi çalıştırılmadı. Kalan gerçek Win32/CRT/Asio kaynak sahipliği tamamlanmadan shipping link açılmayacak.

## Sonraki iş
Windows x64 Native Tools'ta DLL export/load testi ve opt-in deep-link dependency raporu; sonra eksik runtime ownership graph ve real transaction queue bağlanması. İzin olmadan canonical HF19/CURRENT_ARTIFACT değiştirilmeyecek. Resmî release değil.
