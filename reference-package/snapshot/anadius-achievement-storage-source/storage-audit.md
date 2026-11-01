# Achievement storage 01eb00: statik normal akış denetimi

Kapsam yalnız retained REA assembly/pseudocode ve aşağıda belirtilen dört sabitin özgün PE'den salt okunur doğrulamasıdır. Kullanıcı CFG/token/achievement dosyaları okunmadı; DLL/oyun çalıştırılmadı; üretim kaynakları değiştirilmedi. Özgün `anadius64.dll` SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Tam pseudo SHA256 `26fe3e2e2c6fe6459d39d8f3a95738b2f83bb7d6b333b61249bdc00abd1d5acb`, asm SHA256 `9b8c894f85b1ca0fd090a95993ecd5048e59a41ae2459e2bbdc80215e8027a42`.

Ana yordam `0x18001eb00`: pseudo `ev_f7cbf2f5b9d3d17681ffb7f9a17c6ed5cf2952ce07087685262cb3dccfe53b3e`, asm `ev_bd10a5a82a8307f47db504ea903686ef7b68be9597de0c3a48f59707493363e8`. Çağrı adresleri aynı tercih edilen0x180000000 tabanına aittir.

## Path ve XML yükleme

`A3A00` configuration::State.user_directory; `A3DC8` achievements_set (lengthA3DD8); `A3DE8` ayrı mutable XML storage path (lengthA3DF8); `A3E58` achievement map; `A3E68` steam_app_id (lengthA3E78); `A3E88` ayrı mutable watcher INI path. Son iki storage path mevcut configuration::State içinde alan değildir; sahiplikleri ayrıca sağlanmalıdır.

`01eb46..01eb50` set length0 ise path recompute atlar. Dolu set, açık uzunluklu `achievement-` + set + `.xml` oluşturur; `01ed44` filesystem codepage widen006d70, `01ed7f` path append016b50 ve `01ed8b` civarı XML path ataması yapılır. Prefix12byte ve suffix4byte; UTF8 widening olduğu varsayılmaz. Set boşalırsa önceki XML path **silinmez**.

`01ee4b..01ee53`, XML path length nonzero ise: document constructor (process entities1/whitespace0), `01ee77` read_binary03b3f0, `01ee96` Parse004280 c_string ve size_t(-1), sonra `01eedd` document ErrorID0 kontrolü. Parse dönüş register'ı karar için kullanılmaz. Embedded NUL sonrasına devam edilmez. Parser error olduğunda map import atlanır; Steam branch yine çalışabilir. Normal parse-false veya throw→empty catch icat edilmemeli.

`01eeee` document FirstChildElement(name=null); `01ef01` root FirstChildElement(name=null). Root ve çocuk isimlerine filtre yoktur. `01f69b` NextSiblingElement(name=null) ile yalnız doğrudan çocuklar işlenir; recursive iniş yoktur. Root yoksa veya çocuk yoksa map değişmez.

## XML child ve achievement alanları

Her çocukta `01ef2d` Attribute("Id",null). Attribute yoksa çocuk atlanır. Attribute present ama empty ise boş map key geçerlidir. Key C string uzunluğuyla alınır. Mevcut key varsa object reset edilmez; yoksa default değer eklenir: field00=1,field04=0,field08=0,timestamp=`0000-00-00T00:00:00`,display_name/steam_id boş,field70=0. `01f0de` default timestamp sabit kopyasını gösterir.

| Çağrı / store | İşlem |
|---|---|
| `01f603` → `003b70`, store`01f608` `[node+44]` | IntAttribute("Count",0) low32 → field04 |
| `01f619` → `003b70`, store`01f61e` `[node+48]` | IntAttribute("Progress",0) low32 → field08 |
| `01f62f`, copy`01f651` `[node+50]` | Grant varsa timestamp replaces; empty present de replaces; absent retains |
| `01f656` current `[node+80]` length check; `01f66d`,copy`01f691` `[node+70]` | display_name yalnız şu an boşsa Name attribute aranır; present value empty dahil atanır |
| `01f69b` | next sibling; duplicate Id sırayla aynı object'i değiştirir |

XML MaxProgress okumaz; field00 mevcut kayıtta korunur. Config tarafından dolu Name, storage Name ile üzerine yazılmaz. Daha önceki duplicate Name boş kaldıysa sonraki duplicate doldurabilir. Count/Progress attribute'ı eksik veya parse başarısızsa default0 yazar; önceki count/progress korunmaz. Grant present-empty, timestamp'i boşaltır.

## IntAttribute biçiminin kritik ayrımı

`003b70` pseudo `ev_56cb76adb60b08dd04b08ed6fce95b42976a383eca0e2b7761c7afcb1ca37415`, asm `ev_153cf06313c36f9169a114330dd971a4060d7bdad38bd9feb6ce7a51c21c35fd`. Default uint32 hücresi0 başlatılır; attribute bulunduğunda `006190` converter sonucu bool çağıran tarafından kullanılmaz, sonuç hücresi low32 döner.

`006190` pseudo `ev_df86694acc6f6bf1570d4ca7b7f4eb7502b5899619e5c3f7a9e0dbfb8411ba5c`, asm `ev_552cb0956e43cd64cc074aec655e5b62d6d6e16d40ead43e9b99b12f5369d9ec`: prefix saptamak için high-bit olmayan karakterlerde CRT isspace ile ilerler; `0x`/`0X` görülürse özgün string'i `%x`, diğer durumda `%d` ile sscanf kaynağına verir. `%d` sonucunu `%u` diye sunmak yanlıştır. Pinned upstream tinyxml211 `XMLUtil::ToInt` bu format seçimine karşılık gelir; `ToUnsigned` `%u` kullanır ve otomatik ikame edilmemelidir. CRT locale/overflow/fault eşitliği ayrıca sağlanmalıdır; dönen/store edilen bitler signed32 field04/08'de tutulur.

Özgün PE salt okunur sabit doğrulaması: RVA7FFD0/file7E7D0=`%d`; RVA7FFD4/file7E7D4=`%x`; RVA83888/file82088=`0000-00-00T00:00:00`; RVA841C4/file829C4=`.xml`. Bu local PE byte kanıtıdır, yeni REA operation kaydı değildir.

## Steam watcher ve yazma kapısı

`01f6be..01f6c6` SteamAppId length0 ise kalan bütün watcher branch atlanır; eski INI path ve field70 değerleri silinmez. Dolu app'de önce `01f6f6` widen("achievements.ini"), `01f73f` widen(app_id), `01f771` widen("achievement_watcher") yapılır; hepsi filesystem codepage'dir. `01f7a5/01f7d4/01f802` append zinciri user_directory/achievement_watcher/app_id/achievements.ini kurar, `01f815` INI path atar.

Map bütün kayıtları key order ile gezilir. `01f884` her timestamp için041e10 çağırır; sonuç `[node+B0]` yani field70'e store edilir. Count veya Progress kapısı yoktur. Önceki map mutation'ları tamamlandıktan sonra bu tur başlar.

`041e10` pseudo `ev_80df4678956b9b7658e3d0b5553defdaa1244bbcda5a6974597668481318d415`, asm `ev_c6b31cf960ca4a4bf60c21d21f3719c757a8a088e5e42c6c5b0a0784d2fc858d`: zero tm9ints; stringstream/get_time format `%Y-%m-%dT%H:%M:%S`; stream state bits6 yoksa041ff0 dönüşümü, aksi halde0. `041ff0` Gregorian/month-normalizing integer epoch aritmetiğidir; `_mktime`/localtimezone çağrısı yoktur. Locale/get_time kabul/ret sınırı ve native stream ABI ayrı kanıttır; yalnız UTC gibi isim vererek host parser ile eşdeğerlik iddia edilmemeli.

`01f8f7` status01ea20(INIpath,flags3). `01f90d..01f919`: error nonzero VE `((uint32(kind)-1)&0xfffffff7)!=0` ise status018130 throw sınırı. Böylece error ile kind1 veya9 bu koşulu geçer. Sonra `01f91b..01f91e` kind2 ise yazma atlanır; diğer kind'larda `01f920` writer01f9a0 çağrılır. Normal existing regular file type2 olduğunda initialize writer'ı çağırmaz; missing kind1 yazmayı tetikleyebilir. Native status helper ve reparse provenance mevcut status-source kapsamıyla birlikte değerlendirilmelidir.

Writer01f9a0 ayrı yordamdır (pseudo`ev_62b9a1da24b690b13ccc9ef87ff538b194425801a3a0857c3220da35a50c024f`,asm`ev_488d569fd12cfa9fe60d4d35d04b56c9d44b52396427f3060d14d2a7e0125a5a`). Bu audit o yordamın tüm serialize/write branch'lerinin kurtarıldığını iddia etmez. 01eb00 writer'ı tek açık boundary olarak çağırabilir; çağrı kapısı ve önceki state mutation'ları korunmalıdır.

## Kanıt durumu

- Yanıtlandı: normal path/stale path kararları, XML iki seviye any-name traversal, Id/default/duplicate alan davranışı, numeric signed format seçimi, Steam path/epoch turu/status/write kapısı.
- Kısmen yanıtlandı: mevcut XML compatibility reader ve signed conversion helper kullanımı; pinned upstream sürümün özgün malformed-input/locale/overflow eşitliği kanıtlanmadı.
- Açık: writer01f9a0'ın tüm iç kodu (ayrı kapsam), native filesystem/error/allocator/stream locale/global/SEH ve bütün DLL runtime eşdeğerliği.
