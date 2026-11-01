# Bellek XML yazıcısı

`memory_printer.cpp`, anadius64.dll içindeki tinyxml2 XMLPrinter'ın LSX yolunda kullanılan varsayılan bellek yazıcısını C++20 olarak yeniden kurar. Girdi DLL'si SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Adresler bu DLL'nin `0x180000000` imaj tabanına aittir. Fonksiyon/kanıt kimlikleri ve ham entity tablosu `evidence.json` içindedir.

| İşlem | Özgün VA |
|---|---|
| Kurucu, varsayılan durum | `0x1800019b0` |
| Element açma / kapama | `0x180004080` / `0x180002f20` |
| String kaçışları | `0x1800052c0` |
| Öznitelik / metin / CDATA | `0x180005560` / `0x1800056b0` |
| Girinti / ham yazma / karakter | `0x180005270` / `0x180006770` / `0x180005770` |

Çıktı dört boşlukla girintilenir; boş element `/>` ile kapanır, kök kapanınca satır sonu eklenir. Öznitelikte beş, metinde üç karakter entity'ye çevrilir. Önceden kaçırılmış entity tekrar kaçırılır. İsim ve değerler ilk NUL'da biter; CDATA içeriği, `]]>` dahil, ham yazılır. `compact` çağrı parametresi ile `first_element` ve `text_depth` durumlarının özgün dalları korunur. Tampon boyutuna sonlandırıcı NUL dahildir.

Bu kaynak XML **okuyucusu değildir**. FILE çıktısı, keyfi başlangıç derinliği, constructor'ın diğer kipleri, tüm XMLVisitor metotları ve özgün MSVC bellek/istisna ABI'si kapsam dışındadır. Açılmamış elementi kapatma geçerli çağrı alanının dışındadır. Kaynak isimleri kendi string'lerine kopyalar; özgün yığın isim pointer'ları tutar. Geçerli ömürlü isimler için bayt çıktısı hedeflenir, allocation davranışı eşitliği ileri sürülmez.

Gerçek XML okuma bağlayıcısı ayrı `../anadius-xml-reader-source` klasöründedir. Upstream tinyxml2 kaynağı kullanır; özgün sürümün eşitliği iddia edilmez. Özgün okuyucuya ait kanıt ve kapsam bu klasördeki `reader-evidence.md` / JSON dosyasındadır.

Kapsam ayrıca çağrı süresince değişmeyen ve open/close arasında değiştirilmemiş element isimlerini ve `INT32_MAX` değerini aşmayan string uzunluklarını gerektirir. Özgün `0x1800053ea`–`0x180005400` son kaçışsız diziyi tek `INT32_MAX` sınırlı yazmayla bitirir; bu kaynakta 2 GiB üzeri girdi eşdeğerliği ileri sürülmez. Null C-string writer'ın geçerli girdi alanında değildir. Bağımsız denetim `../anadius-misc-handlers-source/xml-printer-review.md` içindedir.

`HandlerWriter`, yanıt işleyicilerini somut yazıcıya bağlar; öznitelik okuma ayrı zorunlu `AttributeReader` bağımlılığıdır. Ortak yönlendirici `../anadius-dispatch-source` içindedir.

`build.ps1` yazıcıyı derler ve yalnız sentetik test programını çalıştırır. 277 kontrol; bayt kaçışları, NUL, compact/karma metin, gerçek GetConfig/GetSetting/bağlantı durumu ve başlangıç LSX serileştirmesini kapsar. Okuma ve rastgelelik bu testte sentetiktir. Özgün DLL/EXE çalıştırılmadı; çalışma zamanı eşdeğerliği kanıtlanmadı.
