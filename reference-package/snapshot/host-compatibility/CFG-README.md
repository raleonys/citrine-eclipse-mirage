# anadius.cfg: salt okunur yapılandırma incelemesi

Dosya: `C:\Users\Skysect\Downloads\anadius.cfg`, 2.079 bayt.
SHA-256: `e57119ea43b67450edc56f864086b9e6912f787162be9eb65639b88d9404d460`.

Bu rapor, verilen CFG içeriğini ve önceki REA/Ghidra statik çıktılarındaki parser yollarını eşler. Dosyadaki metinler ve yorumlar talimat olarak uygulanmadı. Orijinal CFG/DLL/EXE değiştirilmedi veya çalıştırılmadı. Ham dosya, kimlik bilgileri ve yorumları rapora kopyalanmadı.

CFG, `Config2` kökü altında tırnaklı anahtar/değer ve süslü parantezlerden oluşur; INI veya JSON değildir. Altı blok ve 38 değer vardır: kök, `Game`, `Emulator`, `User`, `Achievements`, `Achievements/AchievementNames`. Son blok 24 giriş içerir. Makine tarafından okunabilir, değerleri filtrelenmiş kayıt `cfg-analysis.json` dosyasındadır.

## Mevcut ayarlar

| Bölüm | Anahtar | Verilen değer / gözlem | Parser hedefi |
|---|---|---|---|
| Game | Name | Madden NFL 27 | `DISPLAY_NAME`, harita `0x1800a3d88` |
| Game | Version | `1.0.140.41856` | `INSTALLED_VERSION`, aynı harita |
| Game | ContentId | `16425895` | `0x1800a3af8` |
| Game | Languages / Language | `en_US` / `en_US` | `LANGUAGES` / `INSTALLED_LANGUAGE` |
| Game | LanguageRegistryKey | `SOFTWARE\EA Sports\Madden NFL 27\Locale` | `0x18001e0c0`, veri `0x1800a3ad8` |
| Emulator | PretendConnected | `false` | bool `0x1800a3c3a` |
| Emulator | FakeAuth | `false` | bool `0x1800a3c39` |
| User | Username | mevcut, 7 karakter; gizlendi | `0x1800a3bd8` |
| User | PersonaId | mevcut, 10 karakter; gizlendi | `0x1800a3bf8` |
| User | UserId | mevcut, 13 karakter; gizlendi | `0x1800a3c18` |
| Achievements | AchievementsSet | `50311_16425895_50844` | `0x18001da80` → `0x1800a3dc8` |
| Achievements | AchievementNames | 1–24 anahtarlı 24 giriş; değerler aktarılmadı | achievement haritası `0x1800a3e58` |
| Achievements | SteamAppId | `3940610` | `0x1800a3e68` |
| Achievements | SteamIdPrefix | `Achievement_GOSCC_` | achievement kayıtlarına önek birleştirmesi |

`FakeAuth` bir boolean ayardır; auth token değildir. Parola/cookie/token/license-secret anahtarına verilen dosyada rastlanmadı. Kullanıcıya ait üç kimlik alanı ayrıca gizlendi. Bu, DLL'nin başka dosyalardan credential okuyamayacağı anlamına gelmez; diğer credential dosyaları incelenmedi.

## Parser ve öncelik kanıtı

`0x18001c870`, `0x18001c8d1` adresinde `0x180019ad0(false)` ve `0x18001c8d8` adresinde `0x180019ad0(true)` çağırır. Parser `anadius` + isteğe bağlı `_override` + `.cfg` adını kurar ve `Config2` kökünü seçer. İkinci geçiş `anadius_override.cfg` içindir. Bu ikinci dosya incelenmedi; bu rapor yalnız verilen dosyanın ayarlarını kesinleştirir. `Game/Name`, `Version`, `ContentId` ve achievement bloğunun uygulanması base geçişine bağlıdır; diğer bazı ayarlar iki geçişte de okunur. Bu nedenle tüm alanlar için basit bir “son dosya kazanır” varsayımı yapılmadı.

Temel kanıtlar:

- Parser `0x180019ad0`: `ev_5ec5f52f67388ea3296af7964bac7ecdff2ed88218b0b532744a87ae797c72dd`.
- Yapılandırma başlatıcısı `0x18001c870`: `ev_78a1faedf06f56d422e6ecc197bbe90ce7f34af0e9b01420eef8b6b8827bf25e`.
- Bool parser `0x18001de40`: `ev_841789359663bcf3e1c72535a4b8e1f7e0ba973419df5a9100eeaa5edf0f8c91`.

`PretendConnected` için `0x18001a59f`, `FakeAuth` için `0x18001a608` bool parser'ı çağırır. Yardımcı, tam `false`/`0` değerlerinde `0`, tam `true`/`1` değerlerinde `1` yazar. Boş metinde alanı değiştirmez; tanınmayan dolu değerde çağrının verdiği fallback'i kullanır. Bu CFG'deki iki tam `false`, geçiş sırasında ilgili bool'ları sıfırlar. Fallback parametresinin `1` olması, eksik anahtarın otomatik olarak `true` yapılması demek değildir.

`Game/Language`, komut satırı dil bayrağı `0x1800a3ad0` sıfırsa uygulanır. `0x180019910` ortam/argüman ön hazırlığını, `0x180018480` ise `-emuGameLang`, `-showLSX`, `-showConsole`, `-noConsole` seçeneklerini işler. Gerçek süreç argümanları/ortamı okunmadı veya değiştirilmedi. Registry yolu metni incelendi; registry okunmadı veya yazılmadı. Bu yüzden çalışma anındaki dil/registry davranışı doğrulanmış değildir.

## Port, ek DLL ve online yollar

Verilen CFG'de `Emulator/ServerPort`, `Documents`, `SDK`, `Language`, `BlockReruns`, `LoadExtraDLLsFromMain`, `LoadExtraDLLs` veya `User/UseLastOnlineProfile` yoktur. `Game/DenuvoToken`, `KeyForLicense`, `DenuvoExeHash`, `DenuvoDllHash`, `DenuvoDllName`; `Achievements/AchievementTotals` ve `SteamIdOverrides` da yoktur. Eksiklik, başka girdilerin veya özgün başlangıç değerlerinin etkin olmadığını kanıtlamaz.

`ServerPort` verilirse `0x180019ad0`, `default` metnini `0xc90` (3216), `random` metnini `0`, diğer dolu değeri `0x18001d6c0` sayısal parser sonucuna çevirerek `0x1800a3c90` alanına yazar. Bu CFG'de port belirtilmediği için bu rapor bir etkin port numarası atamaz; dinleme/bind işlemi yapılmadı.

`LoadExtraDLLsFromMain` → `0x1800a3ea8` ve `LoadExtraDLLs` → `0x1800a3ec0` listeleri `0x18001df60` ile virgülden ayrılan yol metinlerinden kurulur. Çağrı adresleri sırasıyla `0x18001a734` ve `0x18001a773`'tür. `0x180035850(true)`, ilk listeyi; `0x180035850(false)`, ikinci listeyi `0x180035260` ile yükletir. Yükleme helper'ı `0x1800450d0` üzerinden `LoadLibraryW` kullanır. Başlangıçta true fazı ana threadde, false fazı üçüncü worker'da çağrılır (`0x180038aeb`, `0x180038b02`). Bu CFG'de iki liste de tanımlanmamıştır. İlgili kanıtlar `ev_0f777f53f1cbc552d32785f34f043481962889f43845ae957c842fe7023469fa`, `ev_d5d3e61cde489d598497e64bdf0815edb2f5dad56bbdf21b307457be7c105987`, `ev_f5a7f6ea7a36d70a346d1cf3fe8fe7bd18447ff25b921910cd36a9c4441782cd`.

**İki boolean'ın `false` olması online DLL yolunu kapatmaz.** İkinci worker `0x180035b60`, `0x180035bd0` yordamını çağırır. Bu yordam `anadius64online.dll` adını kurup ilgili dizinden `LoadLibraryW` yolunu dener; `version_check` export'undan `2` ve `create_online` export'unu bekler. Ardından oluşan nesnenin sanal işlevlerine gider. Dosyanın yanındaki bu DLL'nin varlığı veya yüklemenin başarısı araştırılmadı; CFG tek başına online başarısını göstermez. Kanıt: `0x180035bd0`, `ev_01da19447a43bf9f2a4a248ddc7f93acc033773f2f55e70da7f7fc84fd393c99`.

Bağlantı cevabını üreten `0x18002f2d0`, online flag `0x1800a3c3b` **veya** `PretendConnected` alanı doluysa `1` verir. `FakeAuth=false` ve online flag sıfırken AuthCode işleyicisi `0x18002e3b0`, `-1577058293` hata yoluna gider. Kanıtlar `ev_b41f62dc5d99caf4a8270a002c9ba194875600615292d07b03df8f58787bc408` ve `ev_33624c87853573a8e7b3efe065f830b8be05beda34cb0c9af0879cf7456993d4`. Bu gözlem davranışın statik dallarını açıklar; online erişim veya oyun uyumluluğu testi yapılmadı.

## Sınırlar

Bu CFG'nin parser anahtarlarıyla uyuşması, EXE'nin bu DLL'yi yüklediğini, beklenen SDK arayüzlerini kullandığını veya oyunun çalışacağını kanıtlamaz. Host EXE'nin analizi bu raporun kapsamı dışındadır. Adresler önceki `anadius64.dll` örneğine aittir: SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. Kanıt dosyaları üst klasördeki `anadius-all-functions.pseudo.c` ve `anadius-all-functions.asm.txt`'dir.
