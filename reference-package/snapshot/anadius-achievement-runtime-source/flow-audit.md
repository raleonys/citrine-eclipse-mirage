# Achievement runtime: bağımsız statik akış incelemesi

Kapsam: `0x180041b60` UTC yordamı ve mevcut `0x180030010` grant kaynağının normal akışı. Özgün DLL/oyun yürütülmedi; kullanıcı CFG/XML/INI dosyaları okunmadı. Yalnız retained REA assembly/pseudocode incelendi. Yeni üretim köprüsünün kaynak incelemesi bu kaydın dışında; aşağıdaki sonuçlar uygulamadan önce bağımsız çıkarımdır.

Girdi DLL SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| `0x180041b60` | `ev_68595116803098f1d28c51d6c2cf79c7ec9e646a902f5a82d68490a03b628239` | `ev_7a1e946a32600dde2f69f281612ad78d0f37819a97d59a29a134a8f95817603d` |
| `0x180030010` | `ev_8e0c6e33949acd640bbdc56b56ceea86f8bb373eca849ee8c6a34d8a0ac1b175` | `ev_62f5fc49da4ec38f00325c7e0b857c87556da27030afd17618aec1e4238cb16c` |

Retained listings: `outputs/anadius-all-functions.pseudo.c` SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; `outputs/anadius-all-functions.asm.txt` SHA-256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

## 041b60 kontrol ve yazım sırası

1. `041b9c XOR ECX,ECX`, `041b9e CALL 05eeb8`: `time(nullptr)` bir kez çağrılır. Ham sonuç yerel int64'e yazılır; ardından `041bad CALL 05fffc` (`_gmtime64`) koşulsuz çağrılır. Ham zaman `-1` olsa bile gmtime çağrısını atlayan ayrı kontrol yoktur.
2. Gmtime null ise `041bb5 →041dc6`: varsa optional epoch pointer'a 0 yazılır (`041dcb`); boş string oluşturulur (`041dce..041de0`). Formatter çağrılmaz. Burada başka hata kodu/exception çevirisi yoktur.
3. Başarıda `041bbb..041bcd` iki 16 bayt + bir 4 bayt kopyalar: dokuz tm int alanı tam yerel snapshot olur. Sonraki calendar/format işlemi bu kopyayı kullanır.
4. Optional epoch pointer null değilse `041bd9 CALL 041ff0`; `041bde` sonucu pointer'a yazar. **Ham time sonucu yazılmaz**; gmtime snapshot'ından tekrar hesaplanan epoch yazılır. Optional pointer null ise bu hesap atlanır, format yine çalışır.
5. Epoch yazımı ostream kurma ve formatter çağrısından **öncedir**. Örneğin sonradan formatter/allocation exception'ı gelirse daha önce yazılmış epoch'un geri alınması görülmez. Bu yordamda catch yoktur; native SEH/unwind eşitliği ayrıca açık kalır.
6. `041ca2 TEST SIL,SIL`, `041ca5 CMOVZ`: yalnız düşük mode baytının sıfır olup olmadığı test edilir. 0 → `%Y-%m-%dT%H:%M:%S` (`086978`), diğer bayt değerleri → `%Y-%m-%d_%H-%M-%S` (`086990`). Tek bir 1 değeriyle sınırlı karşılaştırma değildir.
7. `041ccf CALL 03d610` formatter/stream insertion; sonrasında fail/bad durumuna göre sonucu sıfırlayan bir başarı denetimi yoktur. Stringbuf'taki mevcut aralık `041cf7..041d56` seçilir; nonnull data varsa `041d5e CALL 018b30` açık uzunlukla string'e kopyalanır. Dolayısıyla formatter'ın kısmi/boş çıktısı korunur; teorik NUL baytı da CStr/strlen ile kesilmez. Bu dal genel `time_put` algoritmasını, locale'i veya stream exception ABI'sini çözmüş sayılmaz.

## 030010 grant karşılaştırması

Mevcut `content_handlers.cpp` normal mantıkla uyumludur; aşağıdaki gözden geçirilmiş bölümde maddi bir uyumsuzluk bulunmadı:

- Missing AchievementId `03005f..030062` false dönüş; Progress yoksa 1, varsa `03008a` atoi. Achievement mutex `0300a2` alınır; hata/recursive-count taşması 5/6 terminal sınırlarıdır.
- Unknown ID için ilk increment Total olur, Count/Progress/epoch 0 ve placeholder timestamp kurulur. Sonrasında signed `Progress < Total` guard (`03062a..03062f`) vardır; unknown nonpositive total başlangıçta artmaz.
- `030635 ADD` modulo32 progress; `03064a CALL 041b60` epoch hedefi node+`0xb0`, mode0; string node+`0x50`'ye taşınır. Başarı tamamlama guard'ı signed; Count=1, Progress=Total (`030670..030677`).
- Completion event, achievement lock tutulurken ayrı event lock altında `030778 →02c620`, `030785 →02bc10`, `03078b →02c400`, `0307cd →034660` yolu ile kuyruğa girer. Event lock `0307e3` bırakılır. Kuyruğun lazy initialization ayrıntısı bu incelemede ayrıca yeniden çözülmedi; mevcut EventQueue sağlayıcısı bunu temsil eder.
- Achievement XML path doluysa yalnız Progress != 0 kayıtları XML'e yazılır. Signed negatif progress de dahil edilir. XML `030ac1 →03c1d0` ile önce yazılır; BOOL dönüş denetlenmez. INI `030b4c →01f9a0` daha sonra çağrılır. XML path boş olsa da güncelleme dalı INI çağrısına ulaşır.
- Yan etkiler ardından response achievement element oluşturulur. Achievement mutex en sonda `030bd1` bırakılır; yani timestamp, event, XML, INI ve response boyunca tutulur. Tamamlanmış achievement için tekrar çağrıda yan etkiler atlanır, response yine üretilir. Daha önceki state değişiklikleri yan etki hatalarında geri alınmaz.

## Köprü için zorunlu sınırlar

- Native grant aynı global achievement map ve iki global dosya yolunu kullanır. Yeni typed bridge shared State/map kimliğini korumalı veya farklı map kullanımını açık domain hatasıyla reddetmelidir; sessiz kopya ile başka state kaydetmemelidir.
- XML printer `CStr` son NUL'ü hariç kopyalar; time helper'ın stringbuf dönüşü ise explicit length taşır. Bu iki veri yolu aynı sanılmamalıdır.
- Gerçek XML/file/path/INI kaynaklarının birleştirilmesi yeni composition'dır; yeni özgün yordam kurtarımı sayılmaz. Saat, gmtime, time_put seçilmiş host compatibility backend olursa native CRT/locale parity diye sunulmamalıdır.
- XML/INI native file sharing, exceptions, global yaşam süresi, allocation/SEH/ABI ve runtime differential kapsam dışıdır. Başlangıç mutex/event global initialization tamamlandı iddiası çıkmaz.
