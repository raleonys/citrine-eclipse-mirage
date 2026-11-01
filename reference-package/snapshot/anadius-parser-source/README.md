# Ham Config2/VDF parser mantığının C++ kurtarımı

`parser.cpp`, mevcut Windows pseudocode/assembly kanıtlarından ham byte girdisinin mantıksal ağaca dönüştürülmesini yeniden kurar. C++20 olarak derlenir; **gerçek dosya okuyucusu, özgün MSVC hash konteyneri veya tamamlanmış DLL değildir**. Kullanıcı CFG'si bu çalışma veya testler için okunmadı/kopyalanmadı. Test girdilerinin tamamı sentetiktir.

Kaynak DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Windows adresleri `0x180000000` tercih edilen görüntü tabanındaki VA değerleridir. [evidence.json](evidence.json), yordamların pseudocode/assembly kimliklerini ve satırlarını, grammar bulgularını, sınırları ve kaynak hash'lerini içerir. Mac sembolleri yalnızca yapısal destek sağlar; platformlar arası sürüm eşitliği ve adres eşlemesi kanıtlanmış değildir.

## Kanıtlanan byte dilbilgisi

```text
Grammar       = EOF olana kadar EOL / Sep / Object
Object        = Quoted, sıfır veya daha fazla Sep, '{', ObjectMembers, '}'
ObjectMembers = '}' gelene kadar list(KeyValue / Object, Seps) / Seps
KeyValue      = Quoted, bir veya daha fazla Sep, Quoted
Quoted        = '"', sıfır veya daha fazla Character, '"'
Character     = Escaped / Regular
Escaped       = '\' ardından '"' veya '\'
Regular       = CR veya LF olmayan bir byte (üst kural '"' ile biter)
Sep           = ASCII whitespace / // yorum / /* yorum */
```

Bu notasyon hata fırlatma ve action durumunu tek başına anlatmaz; kod bunları da modellemeye çalışır. Root seviyesinde scalar key/value çifti kabul edilmez; quoted object adı zorunludur. Quoted isim ve değer boş olabilir. Anahtar ile scalar değer arasında en az bir Sep gerekir. ObjectMembers dış `until` döngüsü sebebiyle **üyeler birbirine bitişik olabilir**: `"R"{"a" "1""b" "2"}` kabul edilir, `"R"{"a""1"}` kabul edilmez. Genel bir VDF parser davranışı varsayılmadı; bu ayrım doğrudan `0138e0` ve `013cc0` akışından çıkarıldı.

Whitespace kümesi HT/LF/VT/FF/CR/SP'dir. `//` yorumu LF/CRLF dahil veya EOF'ye kadar tüketilir. `/* */` yorumu ilk `*/` ile sonlanır; iç içe yorum desteği yoktur. Açık block yorum başarısız olur. Marker ile çağrılan denemeler cursor'u geri alır; object body'deki doğrudan block denemesi ise hata öncesi EOF'ye veya başarısız açılıştaki `/` sonrasına ilerleyebilir.

Yalnızca `\"` ve `\\` decode edilir. `\n`, `\t`, `\q`, `\u1234` gibi tanınmayan escape dizileri **literal backslash ve devamındaki byte'lar olarak kalır**; escape tablosuna yeni kurallar eklenmedi. Quoted string içinde ham CR/LF hata verir. Diğer byte'lar, NUL ve geçersiz UTF-8 dahil kabul edilir; grammar UTF-8 doğrulayıcısı değildir.

`019ad0`, baştaki üç byte'ı `EF BB BF` ile karşılaştırıp eşleşirse yalnızca bir kez siler. Bu kaynak aynı işlemi uygular. Sabit, salt okunur PE kontrolünde VA `0x180083900`, dosya offset `0x82100` adresinde `EF BB BF 00` olarak doğrulandı. İkinci BOM veya UTF-16 BOM dönüştürülmez. Parser hata konumu BOM silindikten sonraki byte girdisine göredir.

## Yordam ve action eşlemesi

| Windows VA | Rol |
|---|---|
| `0x180014310` | Root grammar: EOF/EOL/Sep/Object; geçersiz girişte `0144d0` |
| `0x180013810` | Quoted token, kapanış quote veya `014550` |
| `0x180013640`, `0x1800141a0` | Escape çözümü ve Regular byte fallback |
| `0x1800135b0`, `0x180013460` | ASCII space, tek satır yorum |
| `0x180013520`, `0x180013ab0` | Sep / Seps ve rewind ayrımları |
| `0x180013ed0`, `0x180014030`, `0x1800140b0` | Block yorum doğrudan/marker bileşenleri |
| `0x1800138e0` | Önce KeyValue, sonra Object alternatifleri |
| `0x180013b70`, `0x180013cc0` | ObjectName + `{` + ObjectMembers; body hatası `014450` |
| `0x180012ff0` | ObjectName action: quoted isim sonrasında, `{` kontrolünden önce yeni frame |
| `0x180012cf0` | Tamamlanmış scalar kaydın parent'a atanması |
| `0x180012bf0` | Tamamlanmış object frame'inin parent/root'a taşınması |
| `0x1800164f0` | Mevcut node'un adı/değeri/çocuk konteynerinin tamamıyla değiştirilmesi |
| `0x18001e200`, `0x180019260` | BOM karşılaştırmasındaki substring ve erase |
| `0x18003b3f0` | Ayrı MSVC ifstream dosya okuma sınırı; bu modülde uygulanmadı |

Duplicate key, son tamamlanmış node atamasıyla **bütün subtree'yi değiştirir**, merge etmez. Scalar da önceki object'in çocuklarını silebilir. Kaynak `Node.children` içinde ilk oluşturulan key slotunu, son atanan node içeriğini; `completed_assignments` içinde tekrarlar dahil tamamlanmış atama sırasını tutar. Bunlar mantıksal/action kayıtlarıdır, özgün bellek dolaşım sırası değildir.

Rewind cursor'u geri alsa da action durumunu geri almaz. Örneğin bir KeyValue denemesinden sonra ObjectName action'ı çalışıp `{` eşleşmeyebilir; açık object frame'i kalabilir. Sonraki liste denemesi aynı metni tekrar action'a sokabilir. `State` bu nedenle accumulator/key/value, tamamlanmış root ve açık frame'leri ayrı tutar. Test, hata öncesi tamamlanmış üyelerin ve mükerrer orphan frame action'larının korunmasını kapsar. Exception sonrasında MSVC destructor/unwind yolunun fiziksel belleğini taklit ettiği iddia edilmez; State analiz için gözlemlenebilir bir modeldir. Her yeni belge için yeni `State` kullanın.

`014310` incelenen normal çıkışlarında EOF'de low-byte 1 döndürür. Uyuşmazlık yolları `014450`, `0144d0`, `014550` üzerinden parse exception'a gider. Normal plain-false çıkış bulunmadı; `019ad0` sonucunu yine de guard eder. Bu modül başarılı girdide true, sentaks hatasında kendi `ParseError` tipini üretir. Özgün PEGTL/MSVC exception metni, demangled rule metni, type ABI ve unwind cursor yan etkileri henüz eşdeğer değildir.

## Configuration sınırı ve özgün hash sırası

`to_configuration_node` ham mantıksal ağacı configuration modülünün `Node` tipine geçirir. Bunun için **zorunlu `BinaryIterationOrder` sağlayıcısı** ister. Kaynakta üretim sağlayıcısı yoktur. Sağlayıcı her node için Windows MSVC hash/list konteynerinin config kopyalama aşamasındaki gerçek sırasını vermelidir. Eksik/mükerrer/bilinmeyen anahtarlı sonuç reddedilir. Kaynak sırayı sıralayarak, text sırasından veya Mac libc++ konteynerinden türeterek uydurmaz.

Dolayısıyla bu modül grammar boşluğunu daraltır; `Dependencies.load_document` gerçek dosya okuyucusunu ve hash/list adaptörünü tamamlamaz. `03b3f0`, önce boş string kurar; başlangıç stream state'i sıfırsa seek/tell/read yolunu izler, sıfır değilse okuma bloğunu atlar. Stream hatası/exception maskeleri, kısmi read ve allocation hata davranışlarının birebir kurtarımı ayrıca gerekir. Dosya açmayı deneyip hatayı otomatik boş belgeye çeviren bir taklit eklenmedi.

## Derleme ve doğrulama

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\outputs\anadius-parser-source\build.ps1
```

LLVM-MinGW workspace derleyicisiyle C++20 `-O2 -Wall -Wextra -Werror`, object dosyası ve sentetik test EXE'si üretir. Configuration kaynağı yalnızca `Node` sözleşmesi için link edilir. **115 sentetik assertion PASS**: grammar kabul/redleri, yorum ve whitespace, adjacent üyeler, escape/NUL/bozuk UTF-8, BOM, duplicate subtree replacement, hata öncesi action durumu, byte/line/column ve mandatory sıra sağlayıcısı. Sıra sağlayıcısının testteki ters çevirme örneği yalnızca mock'tur; Windows sıra algoritması değildir.

Derleyici ve çıktı hash'leri [validation.json](validation.json), test çıktısı `build/test-output.txt` içindedir. Yalnızca kendi sentetik test EXE'miz çalıştırıldı. Hedef DLL/EXE yüklenmedi veya çalıştırılmadı; gerçek CFG okunmadı; dosya okuma/yazma/silme veya ortam işlemi uygulanmadı. Özgün runtime ile karşılaştırmalı test yapılmadığından davranış eşdeğerliği hâlâ kanıtlanmış değildir.
