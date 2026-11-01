# FIX15 — Kaynağa bağlı düzeltmeler ve yetki hesabı

Bu aşama, kullanıcının dört somut düzeltme grubuna verdiği “devam edelim” talimatını uygular. Kod atası tam RE paketi; parent FIX14 ZIP SHA256 `7c7a58e9b3740a4f75f05ae014e9c33ee3cbfdefa6f7fb02a0fc377cd3d600ca`.

## Sonuç

- 12 runtime dosyası değişti veya doğrudan kaynak modülü olarak eklendi. Bunların tamamı B15-R1..R4 yetki kayıtlarına bağlı. Toplam src dosyası120.
- Bütün upstream vendor dosyaları ve THIRD_PARTY.json FIX14 ile byte-identical. Başlangıç/thread/provider-null politikası, avatar, parser ve persistence algoritmaları değiştirilmedi.
- D09 (okumaya yazma denetimi), D08 (iki source encoding bindingi) ve D20 (config konumu metni) kaynak kapsamı içinde düzeltildi.
- D07de bilinen source status dalları geri bağlandı. Paketin açıkça unknown bıraktığı flags3 reparse-tag dalı için mevcut adapter korundu; yeni tag veya ret kuralı uydurulmadı.
- Önceki22 inceleme kaydının3ü kaynak kapsamı içinde kapandı. Kalan19 =18 değiştirilmemiş inceleme kaydı +1 kısmi status kaydı. Bunlar19 kesin bug demek değildir.

## Bu sapmalar neden oluşmuştu?

| Kayıt | Kaynakta görülen oluşma şekli | Yetki/dependency değerlendirmesi | FIX15 |
|---|---|---|
| D09 | RuntimePaths ownership check was reused on input/status methods; file-policy scope expanded from write into read. This is visible in source; no independent need was recorded. | Kullanıcının ihtiyacından veya upstream zorunluluğundan otomatik türetilemez. | Remove require_write from only input/status/read bindings. Provider output/create and all writes keep ownership. |
| D08 | The existing adapter implemented a shorter ACP/OEM flags0 path even though the package helper existed. Original rationale was not recorded; not an upstream requirement. | Kullanıcının ihtiyacından veya upstream zorunluluğundan otomatik türetilemez. | Use exact packet helpers for locale65001 priority, strict flags, default-character error and invalid-flags retry; Linux QA conversion remains separate. |
| D07 | Application bindings used a reduced std::filesystem translation instead of the supplied source status module. Original reason not recorded; library migration does not require dropping these branches. | Kullanıcının ihtiyacından veya upstream zorunluluğundan otomatik türetilemez. | Storage/provider/license status route to source NativeStats/status. Packet unknown flags3 reparse-tag domain retains only prior adapter behavior; no invented tag, policy or research exception rejection. |
| D20 | Earlier rename changed the config basename while retaining the old EXE-directory sentence. The path migration and the explanatory text were not reconciled. | Kullanıcının ihtiyacından veya upstream zorunluluğundan otomatik türetilemez. | Change only game exe wording to preloader_l.dll. Config optionality, loader/search location and format unchanged. |

Özellikle D07/D08 için ilk kararın tarihsel gerekçesi kaydedilmemiştir. Sonradan “güvenlik gerekiyordu” gibi bir neden üretilmedi. Yukarıdaki açıklama kanıtlanan kod yolunu ve kapsam hatasını anlatır; geriye dönük izin değildir.

## Kaynak modüller ve testler

Encoding encoding.cpp/.hpp/native_encoding.cpp ve status status.cpp/.hpp/native_status.cpp doğrudan RE paketinden alındı. Uyarlamalar include/namespace ve macro guardlarıdır. Orijinal encoding synthetic() ve status mapping()/route() beklentileri değiştirilmedi. Native test gövdeleri yalnız Windows derlemesinde koşar; Linux bunları PASS saymaz.

Source testleri48 encoding ve9295 status sentetik kontrolünü kapsar. Bunlar locale65001 önceliği, strict flag8, default-character hata sonucu, invalid-flags retry, sharing32 fallback, permissions, known/missing errors ve reparse query çağrı sırasını kontrol eder.

Gerçek uygulama regresyonu: owned input factory ve provider read/status ile SURGE dışındaki mevcut dosya okunur; aynı hedefe write denemesi reddedilir ve dosya byte preimageı korunur. Yazma sahibi/provider output denetimleri kaldırılmadı.

## Governance ve AGENTS

Library master MD/JSON version9 bağımsız olarak yeniden okundu; revision2026-10-25.1 ve hashler mevcut PROJECT referansıyla eşleşti. Pakete normatif uygulanabilir alt küme kondu; Library-only governance debt/release history kopyalanmadı. Tam master mirror olduğu iddia edilmez.
AGENTS kısaltıldı ve operasyonel rehber olduğu belirtildi. Kendi teknik tercihi, test beklentisi veya paket PASSi kullanıcı yetkisi üretemez. Yeni kapsam kontrolü değişen runtime dosya kümesini ve dependency/governance hashlerini denetler; anlamsal onayın yerine geçmez.

## Açık kalan kaynak kararları

| ID | Durum | Konu |
|---|---|---|
| DEP05 | REVIEW_NO_AUTOMATIC_REVERT | gulrak/filesystem kapsamı kısmi |
| D01 | REVIEW_NO_AUTOMATIC_REVERT | Bootstrap ve ek DLL phase sırası |
| D02 | REVIEW_NO_AUTOMATIC_REVERT | IO/handler thread düzeni |
| D03 | REVIEW_NO_AUTOMATIC_REVERT | Session okuma/işleme/yazma ardışıklığı |
| D04 | REVIEW_NO_AUTOMATIC_REVERT | Accept hatasında servis sonlandırma |
| D05 | REVIEW_NO_AUTOMATIC_REVERT | Hookları tek batch ile etkinleştirme |
| D06 | REVIEW_NO_AUTOMATIC_REVERT | Ek global kilitler ve attempt counter senkronizasyonu |
| D07 | PARTIAL_SOURCE_RESTORED_PACKET_UNKNOWN_DOMAIN_RETAINED | Dosya status dalları basitleştirilmiş |
| D10 | REVIEW_NO_AUTOMATIC_REVERT | Path çözümleme adapterının farklı API kullanması |
| D11 | REVIEW_NO_AUTOMATIC_REVERT | Provider factory null sonucu hata yerine return |
| D12 | REVIEW_NO_AUTOMATIC_REVERT | Source wait_steps yerine readiness/port kontrolü |
| D13 | REVIEW_NO_AUTOMATIC_REVERT | Conservative unload ve fault-domain korumaları |
| D14 | REVIEW_NO_AUTOMATIC_REVERT | Kaynak exit/error etkisinin SessionFailure ile karşılanması |
| D15 | REVIEW_NO_AUTOMATIC_REVERT | XML derinlik denetimi parse sonrasına taşınmış |
| D16 | REVIEW_NO_AUTOMATIC_REVERT | Avatar seçim isteğinin provider yan etkilerine uzanması |
| D17 | REVIEW_NO_AUTOMATIC_REVERT | getenv yerine sahipli _dupenv_s kopyası |
| D18 | REVIEW_NO_AUTOMATIC_REVERT | Çift configuration orchestration yolu |
| Q02 | REVIEW_NO_AUTOMATIC_REVERT | Test oracle bağımsızlığı ve aşırı kaba diagnostic |
| D21 | REVIEW_NO_AUTOMATIC_REVERT | Staging ve unload iç algoritma sınırları |

Önceki tam paket envanteri,387 RE kaynak dosyası ve151 modül crosswalkı FIX14 kayıtlarında korunur. Current FIX15 crosswalk tüm120 src dosyasını içerir. Original module inventories are historical coverage, not current per-file checksum claims. Yeni kayıtlar RE_SOURCE_DEVIATIONS_FIX15.json, RE_FINDINGS_FIX15.csv ve SOURCE_CHANGES_FIX15.patch içindedir.

## Doğrulama sınırı

Debug/Production/ASan ve temiz ZIP replay sonucu son QA kaydında source digest ile bağlanır. Native Windows/MSVC/gerçek DLL Golden çalıştırılamadığında NOT_RUN yazılır. Bu, kaynak farklarının tespiti için engel veya zorunlu non-release oyun kabul kapısı değildir.
