# Config2: bölüm kopyası ve son koleksiyon sırası statik denetimi

Kapsam `019ad0` içindeki lookup, copy, liste tüketimi ve kapasite geçmişidir. Yalnız hash'e bağlı retained REA pseudocode/assembly ve mevcut kaynak kanıtları okundu. Kullanıcı CFG/token içeriği okunmadı, özgün DLL/EXE çalıştırılmadı; üretim kaynakları değiştirilmedi. Bu normal-akış kaynak algoritması için statik denetimdir; native allocator/ABI/SEH/fault zamanlaması kanıtı değildir.

Hedef SHA256: `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`.
`019ad0` pseudo Evidence `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd`; assembly `ev_b75b58538a829894dee04456d214f29001ee3a09dd2d8c04d5b393fa8534df88`.

## Sahiplik ve copy modeli

- `019c4c`: ilk Config2 destination çocuk tablosu `local_188`, fresh `014a90` ile bucket8/mask7/load1 olur. `019c5a`: bölüm destination `local_428`, `014c60` ile fresh node olur. `019c68`: record destination `local_288`, ayrı fresh node olur.
- `019d42` Config2 default-inserting lookup; `019d52` seçilen node'u `local_1c8` içine `0165c0` ile kopyalar.
- Aynı `local_428` bölüm destination'ı Game → Emulator → User → Content → Entitlements → Achievements boyunca yeniden kullanılır. Arada destroy/reinitialize yoktur.
- `0162f0`, destination list'inin mevcut kayıtlarını **anahtara göre değil, mevcut liste konumuna göre** source list kayıtlarıyla eşleştirir: `016340..016385`, key overwrite `01636a`, payload recursive `0165c0` `016377`, iki liste cursor'ını ilerletme `01637c/01637f`. Source overflow `0163a7` → `010bc0` ile yeni kayıt clone; destination tail `016413..016458` silinir. Sonunda `0163e1` growth ve **her zaman** `0163ec` rehash vardır.
- Bu nedenle yalnız parent bucket sayısını saklamak yeterli değildir. Eski destination kaydının payload alt tablosu recursive assignment'ta kapasitesini saklar. Örneğin Achievements/AchievementNames destination alt tablosu önceki Content/Entitlements kayıt payload'ından büyük bir bucket kapasitesi devralabilir. Source slotlarını yeniden adlandırmadan önce destination traversal sırası snapshot alınmalıdır.
- `0165c0` name/value string kopyasından sonra çocuk tablosuna `016610` → `0162f0` uygular. `016ad0` her lookup için node+0x40 tablosuna `016ae0` → `012820` çağırır; eksik anahtar default node ekler, yalnız const lookup değildir.

## Bölüm seçimi ve kopyalama

| Bölüm | Config2 lookup | Aynı local_428'e copy | Koşul |
|---|---|---|---|
| Game | 019e04 | 019e14 | her iki geçiş |
| Emulator | 01a3da | 01a3ea | her iki geçiş |
| User | 01a7a2 | 01a7b2 | her iki geçiş |
| Content | 01aaa5 | 01aab5 | yalnız base (`param_1==0`) |
| Entitlements | 01b295 | 01b2a9 | yalnız base |
| Achievements | 01bcdf | 01bcf3 | yalnız base |

Config2 root'ta eksik bölüm eklenebilir. Bu parent rehash, zaten var olan bölümün çocuk listesini doğrudan yeniden sıralamaz. Yeni eksik bölümün child table'ı fresh8 olur.

## Aynı bölüm destination'ındaki anahtar lookup sırası

Game:

| Sıra | Anahtar | Lookup | Koşul |
|---|---|---|---|
| 1 | Name | 019e4c | yalnız base |
| 2 | Version | 019eb8 | yalnız base |
| 3 | ContentId | 019f24 | yalnız base |
| 4 | DenuvoToken | 01a0a5 | daima |
| 5 | DenuvoExeHash | 01a100 | daima |
| 6 | DenuvoDllHash | 01a14b | daima |
| 7 | DenuvoDllName | 01a196 | daima |
| 8 | KeyForLicense | 01a219 | daima |
| 9 | Languages | 01a27b | daima |
| 10 | LanguageRegistryKey | 01a2e7 | daima |
| 11 | Language | 01a32c | DAT_A3AD0 ==0 |
| 12 | LanguageRegistrySpoof | 01a398 | DAT_A3AD0 ==0 |

Base guard `019e24/019e27`; language flag guard `01a300/01a307`. `language_from_arguments` değeri bu yordam içinde değişmez. Denuvo path/default, token uzunluğu ve nonempty KeyForLicense dalları yardımcı etkiler ve throw zamanını etkiler; yukarıdaki normal devam eden ana lookup sırasını değiştirmez. Özellikle KeyForLicense boş olsa da lookup yapılır. Path/locale/license sağlayıcısı throw ederse sonraki lookup'lara ulaşılmaz; sıraları önceden topluca yürütmek bu kısmi etki zamanlamasını korumaz.

Emulator, koşulsuz sıralı dokuz lookup:

| Anahtar | Lookup |
|---|---|
| Documents | 01a438 |
| ServerPort | 01a4ac |
| PretendConnected | 01a589 |
| FakeAuth | 01a5f5 |
| SDK | 01a637 |
| Language | 01a676 |
| BlockReruns | 01a6e2 |
| LoadExtraDLLsFromMain | 01a724 |
| LoadExtraDLLs | 01a763 |

User, koşulsuz sıralı on bir lookup:

| Anahtar | Lookup |
|---|---|
| Username | 01a7e1 |
| PersonaId | 01a820 |
| UserId | 01a85f |
| Avatar | 01a89e |
| UseLastOnlineProfile | 01a8e0 |
| Country | 01a922 |
| Currency | 01a961 |
| IsSubscriber | 01a9a0 |
| IsSteamSubscriber | 01a9df |
| SubscriberLevel | 01aa1e |
| IsUnderAge | 01aa5d |

Normal ilerleyen geçişte dokuz farklı Emulator ve on bir User lookup'u local_428 tablosunun Content'ten önce en az bucket64'e büyümesini zorlar. Giren bölüm zaten daha büyükse kapasite512 veya daha büyük olabilir. `014a90`'ın8fresh varsayımı her bölüm copy'sine uygulanamaz. Game'in language guard'ı sadece boyutu değil liste konumunu ve sonraki positional descendant kapasite eşlemesini de etkileyebilir.

## Koleksiyon ve record yolları

Base-only guard `01aa7d/01aa80`: override doğrudan sona atlar, Content/Entitlements/Achievements seçimi veya koleksiyon tüketimi yapmaz.

- Content: `01aab5` bölüm copy sonrası local_3e0 sentinel listesini doğrudan gezer. Her record node'u ayrı, yeniden kullanılan `local_288` içine name/value ve çocuk tablosu `01ac26` → `0162f0` ile kopyalanır. Lookup sırası **Name `01ad06`, Version `01adbe`, State `01ae51`**. Bu lookup'lar ayrı local_248 tablosunu değiştirir; local_428'deki Content record'u veya outer listeyi değiştirmez. Vector APPEND sırası outer local_428 traversal sırasıdır.
- Entitlements: `01b2a9` bölüm copy sonrası aynı outer destination'ın sentinel listesi gezilir. Aynı ayrı record destination local_288 / local_248, `01b406` → `0162f0` ile tekrar kullanılır. Lookup sırası **Group `01b553`, Version `01b627`, Type `01b6e1`, EntitlementTag `01b7b7`**. Local_288, Content'ten Entitlements'a kapasite taşıyabilir fakat local_428'e geri aktarılmaz. Vector APPEND sırası outer traversal'dır.
- Achievements: `01bcf3` bölüm copy sonrası **AchievementIDs `01bd67` önce**, sonra **AchievementsSet `01bdce`** lookup edilir; `01da80(Set,IDs)` çağrılır. İki mutable text çağrısını C++ function arguments içinde belirsiz değerlendirme sırasına bırakmamak gerekir.
- AchievementNames `01be69`, AchievementTotals `01c042`, SteamIdOverrides `01c5b3` lookup'ları bulunan node'un child sentinel'ini **doğrudan** gezer; arada ek bir node copy yoktur. Dolayısıyla bu üç child table'ın tüketim sırası, recursive positional section copy sırasında oluşan sıradır.
- Arada SteamAppId `01c158` lookup/assign-if-nonempty; güncel/önceden korunmuş global steam_app_id boş değilse SteamIdPrefix `01c23c` lookup'u ve global achievement ağacı üzerinde prefix+ID güncellemesi vardır. Empty CFG SteamAppId, eski nonempty state'i silmediği için guard yalnız CFG değerinden türetilemez. Bu parent lookup büyümesi contained child list'leri yeniden sıralamaz fakat parent liste/allocator/fault akışını etkiler.

## Uygulama sınırı önerisi ve açıkler

Doğru yapı, log sonrasında mutable Config2/section/record node'larıyla lookup ve helper etkilerini sırayla yürütür. `Dependencies::load_document` aşamasında state/override bilinmeden son sıra önhesaplamak, provider throw'larında kısmi uygulama/log zamanlamasını taşımaz. Root'un yeni mutable uygulama çekirdeği yönü uygundur; bir pure prepass tam runtime eşdeğerliği olarak sunulmamalıdır.

Bu audit normal devam eden yolların çağrı sırasını ve container sahipliğini yanıtladı. Hash growth/rehash modelinin üretim uygulaması, bütün native tahsis/exception ABI'si, helper provider'ların kendi etkileri, tam DLL eşdeğerliği bu audit'in doğruladığı işler değildir. Gerçek kullanıcı CFG içeriği bu sonuç için gerekli olmadı ve okunmadı.
