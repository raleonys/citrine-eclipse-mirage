# Köken ve bağımlılıklar — FIX16

Uygulama kod atası kullanıcının dll-reverse-engineering(6).zip paketidir; SHA2560734efcca48a69871e7af080fb9e08f3b16525fb7cc577686e6134a6d3dfd0f9. Her import için original/current SHA ve uyarlama IMPORTS.json içinde tutulur. Encoding/status modülleri FIX15te aynı RE paketinden bağlandı.

FIX16da aşağıdaki dependency dosyaları ve pinler FIX15 ile aynıdır. Vendor patchi eklenmedi. Pinlerin otoritesi metadata/THIRD_PARTY.json; son kullanıcı Asio arşivi kanıtı metadata/ASIO_UPSTREAM_FIX14.json.

| Bağımlılık | Sürüm | Commit |
|---|---|---|
| TinyXML2 | 11.0.0 | `8224e427b655b83dae5e2298f1e6919523a78737` |
| MinHook | 1.3.4 | `8af6b4acae5a9388fd742b56fa79ece89d96f823` |
| Asio | 1.38.2 | `026dce530a6c39a8aa6d98e5177482310c4a01e5` |
| PEGTL | 4.0.0 (prerelease main snapshot) | `3b075432e94f6f906425350b43d03cd9cebfa4f1` |
| gulrak/filesystem | 1.5.17 | `3812f8a6f31ed67d32272ac33fe2b85c6b2c947e` |

Bu bağımlılık snapshotları kullanıcı talebiyle önceki aşamalarda upstreamden bağlandı; BROWNFIELD uygulama C++ kodu kopyalanmadı. Asio1.38.2 commit kimliği sürüm numarasından ayrıdır. gulrak yalnız module-sibling path construction için aktiftir; bütün filesystem backendinin gulrak olduğu iddia edilmez.

Yeni proje-authored kod GPL-3.0-only; complete LICENSE köktedir. Bu, RE girdi rekonstrüksiyonlarının lisansını geriye dönük yeniden atamaz. Upstream lisanslar/noticelar ve origin hashler korunur. Resmî yayımlama veya canonical artifact bu aşamada değiştirilmez.

FIX29 dependency update değildir; tüm vendor bytes/pinleri aynı. TinyXML2 implementation yalnız src/xml/upstream_parser.cpp içinden bir kez derlenir. Public global depth500 sabiti korunur; implementation namespace lookup source100 limitini parse sırasında seçer. Bu adapter kendi entegrasyon kodumuzdur ve gelecekteki explicit upstream updateinde review zorunludur. Packet XML h/cpp/license kopyası yalnız test referansıdır, zlib notice korunur.
