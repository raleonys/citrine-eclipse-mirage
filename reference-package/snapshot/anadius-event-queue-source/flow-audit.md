# Event queue: bağımsız statik akış incelemesi

İnceleme yalnız retained REA pseudocode ve assembly üzerinden yapıldı. Özgün DLL/oyun çalıştırılmadı; kullanıcı CFG/XML/INI/token dosyaları okunmadı. Hedef SHA-256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.

Retained listeler: `outputs/anadius-all-functions.pseudo.c` SHA-256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`; `outputs/anadius-all-functions.asm.txt` SHA-256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| 02c400 queue getter | `ev_8a31862931ad4a3cf8f80e4584226c396cdf07e3d33f8c2981f44e90db73bcd0` | `ev_992d7a1ed7c4d433e484c79367b7dd1f74c462ca9b896b914dd622e7e16a5d9a` |
| 02c620 mutex getter | `ev_0e415284624e38d46c3199e2803819e3710cab686ae0325f3d0446636fa42c9d` | `ev_f3d62ed2c858e85fd60711534757a15f5b1c757ed963ea527e961154296f1cd8` |
| 032b10 IGO handler | `ev_91196662eb3fd450e8c602b238d0876b7596738c737f63ce4707318feab10bc8` | `ev_4c836ad1a44251a54bc1e32b8ebf17ba7b0256f3e8ba7e76205239331b70900f` |
| 030010 achievement grant | `ev_8e0c6e33949acd640bbdc56b56ceea86f8bb373eca849ee8c6a34d8a0ac1b175` | `ev_62f5fc49da4ec38f00325c7e0b857c87556da27030afd17618aec1e4238cb16c` |
| 033d70 message processing | `ev_7a38e9dc5496630753954e74801f942d9111225ca95c1c603a4ae22b88f504df` | `ev_971ebfa9e879b1b543e464bfc367047eed787a968cfaef065ce973d0b4f34bd6` |

## Getter guardları ve normal sıra

Her iki getter, `_tls_index` ile seçilen TLS kaydının +4 alanındaki signed32 epoch'u okur. Dış koşul **thread_epoch < global_guard**'dır: queue `02c439..02c43f`, mutex `02c641..02c647`. Eşitlik veya thread epoch'un büyük olması fast path'tir. Fast path hiçbir initializer/header/footer/atexit çağırmaz ve aynı global nesnenin adresini döndürür.

Yavaş yolda `04b838` header çağrılır; dönüşte **global guard yeniden okunur ve yalnız -1 ise constructor çalışır**. Queue `02c4c2..02c4ce`, mutex `02c65d..02c669`. Header başka bir thread'in tamamlamasını bekleyip guard'ı başka değere taşıdıysa constructor, atexit ve footer atlanır. Header'ın native lock/wait/TLS güncellemesi bu incelemede yeniden kaynaklaştırılmadı; zorunlu runtime sınırıdır.

Mutex getter `02c620`: global mutex `0x1800a3920`, guard `0x1800a3970`. Constructor çağrısı `02c67a →04a698`, ikinci argüman **2**. Sonra `02c686 →04b770` ile cleanup callback `0x18007e030` atexit'e verilir; EAX kontrol edilmez. Ardından `02c692 →04b7cc` footer çağrılır. Normal dönüş yine aynı mutex global adresidir. Constructor/registration/footer provider exception veya native SEH/unwind eşitliği ayrıca kanıtlanmış değildir.

Queue getter `02c400`: global deque `0x1800a3978`, guard `0x1800a39a0`. Guard -1 sahibinin normal constructor dalı şu sırayı izler:

1. `02c4ea →04b0bc(0x50)` geçici string tahsisi; length0x44=68, capacity0x4f. Sabit event baytları `02c505..02c536`, NUL `02c539`: `<LSX><Event sender="EbisuSDK"><IGOEvent State="DOWN"/></Event></LSX>`.
2. `02c540..02c558` global deque'nin beş qword hücresi sıfırlanır: proxy, map, map_size, offset, size.
3. `02c56c →04b0bc(0x10)` iterator proxy tahsis edilir; proxy owner global deque, next0 olur (`02c57d..02c584`).
4. Gerekirse `02c5ad →026670(queue,1)` map büyütür; offset maskelenir, ilk string block slot'u seçilir; gerekirse `02c5fb →04b0bc(0x20)` block ayrılır.
5. `02c477 →009eb0` geçici seed string'i deque block'a **kopyalar**, taşımaz; `02c47c` size artar. `02c498 →04b910` geçici seed string'i yok eder.
6. `02c4a4 →04b770` cleanup `0x18007e000` callback'ini kaydeder; dönüş kodu gözlenmez. `02c4b1 →04b7cc` footer, sonra aynı global deque adresi döner.

Native deque map/block/proxy/SSO/allocator footprint'i ve growth034660/026670/009eb0 iç ABI'si bu iki getter'ın normal mantığını modellemekten ayrı tutulmalıdır. Host `std::deque<std::string>` kullanımı yalnız seçilmiş FIFO compatibility storage olabilir; özgün bütün deque fonksiyonları veya allocation/exception eşitliği diye sunulmamalıdır. Atexit callback kimliklerini belgelemek, native callbacks/lifecycle'ı çalıştırmak değildir.

`atexit` başarısızlığı normal koda göre constructor'ı geri almaz, footer'ı atlamaz; dönüş kodu yoksayılır. Buna karşılık bir constructor/provider exception'ı normal footer satırına ulaşmaz. Retained gövde içinde explicit catch/abort yoktur; bu, EH metadata/funclet yoluyla `_Init_thread_abort` veya temizlik bulunmadığını kanıtlamaz. Genel static-initialization failure/retry/unwind/native concurrency eşitliği açık bırakılmalıdır. Bu akış için guard/TLS başlangıç değerleri caller tarafından sağlanmalı; tahmin edilmemelidir.

## Üretici ve tüketici ilişkileri

**IGO032b10**: XML success yanıtı önce kurulur. `032b75 →02c620` mutex getter, `032b85 →04a6ac` lock, count INT_MAX sınır denetimi, ardından `032ba0 →02c400` queue getter. `032c10 →034660` aynı DOWN event'ini ayrıca ekler; `032c53` unlock. Dolayısıyla ilk cold IGO isteği normal başarıda iki aynı DOWN olayı bırakır: getter'ın seed'i ve isteğin append'i. Sonraki IGO istekleri yalnız bir yeni DOWN ekler.

**Achievement030010**: completion dalında `030778 →02c620`, `030785 →02bc10` event lock guard, `03078b →02c400` queue getter, `0307cd →034660` achievement XML event append, `0307e3` event unlock. Bu sırada dış achievement mutex de tutulur. İlk cold completion seed DOWN + achievement event sırası üretir. Partial grant veya zaten complete olan kayıtta event getter/append'e ulaşılmaz.

**Message033d70**: olay drenajı yalnız encrypted modda ve response encryption başarıyla tamamlandıktan sonra yapılır. `034223 →042610` response encryption failure'ı getter'a ulaşmaz; unencrypted branch `03420f →03457d` de event getter'ı çağırmaz. `0342f4 →02c400` queue getter **event mutex alınmadan önce** çağrılır; böylece ilk consumer da seed yaratabilir. Size nonzero ise `034307 →02c620`, `034317 →04a6ac` lock, count sınırı ve locked size recheck `034332..034337` izler. Bu recheck aynı yakalanmış queue nesnesini kullanır; getter tekrar çağrılmaz.

Front string kopyalanır, isteğe bağlı loglanır, `0343c2 →042610` ile event encryption denenir. Başarıda hex+NUL response'a eklenir; failure `0343c9 →0344bf` doğrudan aynı pop yoluna gider. **Normal dönüşte event encryption failure bile bir front event tüketir.** Pop sayısı bir mesaj yanıtında en fazla birdir; loop ile bütün queue boşaltılmaz. `034573` unlock. Allocation/log/encryption exception'ları için tam unwind behavior bu normal kontrol incelemesinin dışındadır.

Üç typed adapter aynı storage ve mutex/global guard context'ini paylaşmalıdır. Content `initialized_locked()` görünümü, native çağrının aslında queue getter olduğunu korumalı; cold seed'i atlamamalı veya çift eklememelidir. Messages relaxed check'in native veri yarışları ve chosen backend'in atomik count tercihi ayrı sınırlardır; host backend'in daha tanımlı erişimi native memory-model eşitliği kanıtı değildir.
