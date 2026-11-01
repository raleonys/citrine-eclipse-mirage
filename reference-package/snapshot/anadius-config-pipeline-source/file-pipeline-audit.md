# Config2 dosya pipeline denetimi

Kapsam yalnız019ad0'ın path→byte→BOM→parser→owned document sınırlarıdır. Retained REA assembly/pseudocode ve mevcut file/path/encoding/parser/config kaynakları okundu. Kullanıcının CFG/token dosyası okunmadı; native I/O, environment, registry, dosya yazma/silme veya özgün DLL execution yapılmadı. Üretim kaynakları değiştirilmedi; root dışında REA işlemi açılmadı.

## Kesin normal akış ve çağrı noktaları

019ad0 asm `ev_b75b58538a829894dee04456d214f29001ee3a09dd2d8c04d5b393fa8534df88`, pseudo `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd`:

| Çağrı noktası | Yordam / rol |
|---|---|
| 019b0f..019b4d | dar filename `anadius` + isteğe bağlı `_override` + `.cfg` string oluşturma |
| 019b7b | 010540 filename filesystem codepage widening |
| 019b8d | 019820 caller State executable_path A39E0 parent_path |
| 019ba3 | 016b00 parent/filename join |
| 019bce | 03b3f0 UTF16 path ile binary tüm-dosya okuma |
| 019bec,019bfb | 01e200 substring(0,3),0108b0 EF BB BF ile karşılaştırma |
| 019c23 | eşleşirse019260 erase(0,3), yalnız bir BOM |
| 019c85 | 010430 parser input-owned kopya + cursor/source label `config file` |
| 019d1a | 014310 grammar;019d1f low AL guard |
| 019d42 | 016ad0 ile root Config2 lookup/default insertion (012820) |
| 019d52 | 0165c0 ile Config2 node'unu local destination'a deep copy |
| 019d6b,019d79 | parser result/input nesnelerini yok etme |
| 019d8d..019dc7 | path filename + UTF8 conversion/log; parse/copy sonrasında |
| 019e04,019e14 | local Config2'dan Game lookup ve başka node copy; sonrasında uygulama ayar atamaları |

014310 incelenen ordinary başarı çıkışında AL1 döndürür; syntax mismatch throw helper'larına gider. Guard korunmalı ama adapter sahte normal parse-false üretmemeli. Read/parse/order conversion exception'ları log ve ayar uygulamasından önce yayılmalı. Eksik Config2 lookup'u özgün mutable container'a default node ekler; logical `configuration::Node::child` empty-node sonucu bu değer sınırını sunar fakat native container mutation/ABI'sini taklit etmez.

## 03b3f0 stream sınırı

Asm `ev_cb304cf806c8f77471c7aad5e5d42745fb5c6a321687823f1a11e5a06a733148`, pseudo `ev_ad6cc2139952e6a5b4801ee44818938562b836f6b702e7924bc854bcd3096820`:

- Başta boş dar string kurar.039f70 ctor'a mode0x20/share0x40/construct1 gider (03b45a).
- İlk stream state **tam0** ise03b4aa seek(0,end),03b4b8 tell/fpos iki bileşen toplamı,03b4ca resize(length,0),03b4d9 seek(0,begin),03b4f4 istenen bütün length read;03b4fe close.
- gcount çıktı string'ini kısaltmaz; kısa read sonrası sıfırla dolmuş tail kalır. Negatif/bozuk tell otomatik boş başarıya çevrilmez; unsigned büyük allocation/length hatası domain'i korunmalı.
- İlk state nonzero ise okuma/explicit close bloğu atlanır; boş string döner, stream destructor yine yürür. Bu boş string parser'da boş başarı olabilir; “dosya açılamadıysa parse error/false” eklemek kanıtlı akış değildir.
- Close failure'da stream state/exception maskesine göre throw olabilir; bunun tam davranışı injected Input provider'a ait. Mevcut `read_binary_03b3f0` null factory sonucu için ayrıca integration-contract invalid_argument verir; bu bir original null-stream yolu değildir.

Mevcut `file_recovered::Factory/Input` bu sınırı uygular. NativeFactory gerçek dosya okuyabilen seçilmiş std::fstream uyumluluk backend'idir; bu audit veya sentetik entegrasyon testinde kullanılmamalı. Injected memory Factory yalnız test byte'larını ve kayıtlı çağrı sırasını sağlamalı; `open_output` kullanılmamalı/açıkça reject edilmelidir.

## BOM, byte grammar ve yaşam süresi

Özgün BOM karşılaştırması sadece0..min(3,size) slice'ını EF BB BF ile karşılaştırır; bir kez3byte siler. UTF16 BOM veya ikinci UTF8 BOM dönüştürülmez. `parser::parse(raw,state)` **zaten aynı bir-BOM işlemini kendi içinde yapar**. Yeni adapter önce strip edip sonra parse çağırırsa iki BOM'u tüketme bug'ı oluşur. Raw byte'ları doğrudan geçir; UTF8 doğrulama/ANSI decode/newline normalize/embedded NUL truncate ekleme.

010430,009eb0 ile input string'i **kopyalar**, cursor begin/end ve line/column başlangıcını kurar, source label'ını kopyalar. Mevcut parser::parse string_view kullanır: adapter'ın owned `std::string bytes` nesnesi parse bitene kadar yaşamalı. Her belge için fresh parser::State gerekir; önceki root/actions/open frames tekrar kullanılarak karışmamalı. ParseError BOM sonrası byte/line/column konumunu verir; özgün PEGTL/MSVC exception ABI/metni/SEH aynı sayılmaz.

0165c0 önce node adı(+0) ve değeri(+0x20) string'lerini018b30 ile kopyalar, çocuk container(+0x40) için0162f0 çağırır.0162f0 source linked-list sırasını kopyalar/reuse eder, kalan node'ları oluşturur/siler; sonrasında017440/017570 rehash uygular. Yalnız grammar/action insertion sırası veya std::map lexical sırası final Windows copy-stage traversal sırasını kanıtlamaz. `parser::to_configuration_node(root,BinaryIterationOrder&)` bu nedenle **mandatory order provider** ister ve eksik/mükerrer/yabancı key'leri reddeder. Her node name parent key'de, value ve owned children'da taşınır; native node layout/allocator/rehash ABI klonlandığı iddia edilmez.

Return edilen configuration::Node, parser State veya input byte storage'a referans/string_view taşımamalı; mevcut converter strings/map/vector ile sahip olunan değer döndürür. Böylece özgün parser yok edildikten sonra local Config2'nun kullanılmasını logical olarak karşılar. Duplicate last completed assignment bütün subtree'yi değiştirir; adapter merge etmemeli.

## En küçük dürüst kaynak adapter kapsamı

Yeni borrowed `DocumentPipeline(Factory&,BinaryIterationOrder&)` benzeri bir composition şu üç **gerçek mevcut core** çağrısını bağlayabilir: `bytes=read_binary_03b3f0(path,factory)`; fresh parser State üzerinde `parser::parse(bytes,state)`; `parser::to_configuration_node(state.root,order)` ile owned belge return. BOM silmeyi parse'a bırak. Şu anki parse false döndürmez; guard eklenirse false için ayrı unsupported/result policy tanımlanmalı, sessiz error→empty catch eklenmemeli. Mevcut API'lere bağlanacak bu composition yeni recovered original body sayılmaz.

`configuration::Dependencies::load_document` override bu pipeline'a delegate edebilir. Parent/join override'ları gerçek path_recovered::parent_path_019820/join_016b00'u; filesystem_widen override'ı injected encoding Calls/Errors ile gerçek filesystem_widen_006d70'u kullanabilir. Wrapper010540 ve alttaki006d70 çağrı şekli native ABI değil logical source bağlantısıdır. Filename widening filesystem codepage seçer;0145d0 UTF8 widening ile karıştırılmamalı. File payload byte'larının parser'a verilmesi encoding dönüşümünden ayrıdır.

Diğer Dependencies metotları mandatory kalmalı. Tam initialize_configuration/load_and_apply, logging/path/ayarlardan sonra dizin, avatar, achievement persistence, environment veya ayrı license subsistemi etkilerine ulaşabilir. Minimal test parser/document pipeline'da durmalı veya sentetik Dependencies ile etkileri kayıt altına alıp gerçek OS çağrılarını reject etmeli. Kullanıcının anadius.cfg'si veya token dosyası fixture girdisi yapılmamalı.

Mandatory order provider üretim implementasyonu henüz bu incelenen kaynak setinde yoktur. Yalnız mock order ile test edilen adapter “gerçek Windows hash order çözüldü” diye raporlanmamalı. Order reconstruction başka aşamadaysa explicit provider injection ile açık bırak. Minimal own fixture; memory stream call order0x20/0x40, empty/nonzero-state, short-read zero-tail, parse error propagation/log skip, tek/çift BOM, duplicate replacement, owned-return lifetime ve provider-order rejection kontrollerini kapsayabilir. Gerçek fixture CFG'leri dahi bu audit için oluşturulmadı.

## Açık kalanlar

Windows container copy/rehash algoritmasının somut production order sağlayıcısı; özgün stream exception/fault/MSVC ABI'si; parse error text/type ABI; wrapper source-label fidelity; tam uygulama side-effect provider'ları ve tüm DLL eşitliği çözülmüş sayılmaz. Dosya/path/encoding compatibility backend'leri seçimdir; yerel mevcut CFG/token içeriği veya native DLL execution bu sınırları doğrulamak için kullanılmadı.
