# Greenfield GF06 — Gerçek modüllerin kaynak kapanışı

**SOURCE_LINKED / NON_RELEASE_WORK / NOT_RELEASE** — parent GF05 ve resmî HF19 kaynakları SHA ile sabittir.

## Doğrudan HF19 kaynak aktarımı

GF05'e ek 76 HF19 kaynak/header/test dosyası ve değiştirilmemiş 364 PEGTL vendor dosyası alındı. Asio'nun önceki 635 dosyası değişmedi. Aktarım listesi ve dosya SHA'ları `metadata/GF06_REUSE_LEDGER.json` içindedir.

Ayrı dormant static target'lar: `gf06_xml_units`, `gf06_config_units`, `gf06_event_units`, `gf06_state_units`, `gf06_handlers_units`, `gf06_full_handler_units`, `gf06_protocol_units`. Bunlar derlendi fakat `gf_preloader` hedefinin üretim DLL'sine **bağlanmadı**. Tüm original handler gövdelerini derlemek, gerçek handler işleme/kullanıcı akışı kabulü değildir.

Gerçek HF19 `tests/unit/test_request.cpp`, gerçek `request_parser`/TinyXML2/attribute/value codec kaynaklarıyla bağımsız yürütülebilir test olarak bağlandı; yanıtsız statik arşiv testi değildir. PEGTL grammar için valid, invalid ve bounded trailing-block-comment kontrolü var. `event_queue.cpp` Linux'ta bilerek gerçek process-heap allocation yapmaz; Linux testinde yalnız doğru fail-closed ve boş FIFO davranışı kanıtlandı. Windows process-heap pozitif FIFO davranışı ve FreeLibrary retry henüz koşulmadı.

## Kaynak sembol matrisi

GF05 `nm` snapshot'ındaki 58 `U` proje sembolünden 49'unun özgün tanımı GF06 arşivlerinde mevcut; 9'u bu yeni arşivlerde yok (bir kısmı GF04/GF05'in başka statik kütüphanelerinde zaten mevcut). Ancak GF06 arşivleri ek sembol bağımlılıkları getirdi. **49/58, final-link tamamlanma yüzdesi değildir.** Durumun tam kanıtı: `metadata/GF06_SYMBOL_CLOSURE.json`.

## Gerekli sonraki gerçek sınır

Config2 and event queue'nun final source-link kapanışı; release-server/Asio bridge kaynağının gerçek Windows x64 MSVC /MT /MTd derlemesi ve yüklenmesi; yalnız bundan sonra dikkatli transactional `SURGE`/`DllShutdown` gerçek owner bağlanması ve LSX/Consumer acceptance. Bekleyen Win32 dependent source `runtime_configuration_file.cpp` sadece Windows target'ına ayrıldı; üretim koduna portable sahte Win32 shim eklenmedi. Orijinal modüllerde Linux kısmi TU build'inde kullanılmayan yardımcı fonksiyon uyarıları tutuldu ve ilgili *research* target için yalnız -Wno-error=unused-function uygulandı.

**Canonical HF19, CURRENT_ARTIFACT ve resmî yayın kimliği değiştirilmedi.**
