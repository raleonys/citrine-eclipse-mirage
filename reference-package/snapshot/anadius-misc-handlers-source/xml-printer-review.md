# MemoryPrinter bağımsız kaynak incelemesi

İncelenen dosya: `outputs/anadius-xml-source/memory_printer.cpp`. Dosya değiştirilmedi. Kapsam: özgün varsayılan, memory-only XMLPrinter yolu; geçerli/yaşayan C-string girişleri, başarılı allocation, boyutları normal protokol sınırlarındaki dizgiler. Özgün DLL çalıştırılmadı. Aşağıdaki bulgular tutulan REA assembly ve pseudocode incelemesidir; davranışın canlı diferansiyel testi değildir.

**Bu kapsamda değişiklik gerektiren bir byte-format hatası bulunmadı.**

| Özgün yordam | Assembly Evidence | Kontrol |
|---|---|---|
| `0x1800019b0` | `ev_4aed1f624e8b723ecdd4f51c92e236b86525f519467b11fcc605affa43fecf6a` | elementJustOpened=false, firstElement=true, textDepth=-1; entity tabloları |
| `0x180004080` | `ev_de54bb50c3ff742a13b271507166aefe66e522db30d50a023eb6f64722e068d3` | Önce önceki tag'ı `>` ile kapama; compact dalı firstElement yazımını da atlar; first/non-text newline+indent koşulu; ardından isim/derinlik/tag durumu |
| `0x180002f20` | `ev_3fd2f6515bbcffbae0fadea6115bc70bb7737317e4fea2c13a6b6fe745ce2e71` | Önce depth--, boş tag için `/>`; textDepth<0 ve !compact için newline+indent; `</name>`; textDepth reset ve root newline |
| `0x1800052c0` | `ev_bf15950286b3b8ad056a5e7885b537d947fab8b8fa75150ccad4bc07a02e3627` | Attribute'da beş entity; text'te yalnız &,<,>; NUL bitişi; diğer byte'ların korunması |
| `0x180005560` | `ev_ce8a68d6974b136583eeaf997f5aee1b2531f47e7ecca3012c760365aafd5dd6` | Boşluk, ham attribute adı, `="`, attr entity yazımı, kapanış çift tırnak sırası |
| `0x1800056b0` | `ev_5a7cd602d448321e7f7167b16801110c91aa45bddf47c57cdeae0ffadb6cfef9` | Önce textDepth=depth-1; pending tag `>`; CDATA tam delimiter'ları ve ham text, aksi halde text entity yolu |

Constructor'daki entity tabloları da değerlendirildi: `0x8a` tabanlı ilk tabloda `0x22,0x26,0x27,0x3c,0x3e`; `0xca` tabanlı ikinci tabloda `0x26,0x3c,0x3e` aktiftir. Bu, kaynak switch'indeki quote/apostrophe ayrımıyla uyuşur. Buffer'ın dışarı verilen tek NUL'u, varsayılan memory buffer'ın ilk NUL'u ve her yazımda korunan terminator kavramıyla uyuşur; BufferSize dış arayüzü bağımsız allocation kapasitesi anlamına gelmez.

Genel API için iki kapsam sınırı var:

- Özgün `OpenElement`, isim pointer'ını stack'e kaydeder; `CloseElement` o pointer'ın o andaki C-string'ini kullanır (`0x180004161`, `0x180002f4d`, `0x180002fb3`). Yeni kaynak isim kopyasını saklar. İsim backing buffer'ı open/close arasında değiştirilirse sonuç farklı olabilir. Mevcut handler/dispatcher'ın sabit isimleri bu sınırın dışına çıkmaz.
- Özgün `PrintString` bazı ham çıktı çağrılarını `INT32_MAX` ile sınırlar; son trailing bölüm tek çağrıyla sınırlandırılır (`0x1800053ea..0x180005400`). Kaynak std::string loop'u 2 GiB üzeri tek bölümün tamamını yazabilir. Normal LSX mesajları için etkisi yoktur; keyfi dev girdilerde tam eşdeğerlik iddia edilmemelidir.

Allocator/exception sırasındaki kısmi buffer veya stack değişiklikleri, FILE* writer, constructor compact/depth seçenekleri, virtual override'lar ve geçersiz stack/null girişler bu incelemenin eşdeğerlik iddiasına dahil değildir.
