# Madden27.fixed.exe ile DLL/CFG bağlantısı

Verilen EXE ve CFG salt okunur incelendi. EXE, DLL'ler ve gömülü PE yüklenmedi veya çalıştırılmadı. Bu inceleme EXE'nin tamamının decompilation'ı değildir; PE başlıklarını, bölümleri, normal/gecikmeli importları, kaynakları ve mevcut DLL exportlarıyla isim eşleşmesini kapsar. CFG içindeki yorumlar talimat olarak uygulanmadı.

## Dosya kimlikleri

| Dosya | Bayt | SHA-256 |
|---|---:|---|
| Madden27.fixed.exe | 254177168 | `85c7d199dd9b63b558eafe8652b2beca87a38fe6183f601c8cb30dd1f36be297` |
| anadius.cfg | 2079 | `e57119ea43b67450edc56f864086b9e6912f787162be9eb65639b88d9404d460` |

EXE AMD64 PE32+, tercih edilen ImageBase `0x140000000`, giriş RVA `0x7c09fc0` / VA `0x147c09fc0`; 17 bölüm, 45 normal import tanımlayıcısı, 932 normal import ve 3 gecikmeli import girdisi içerir. Adresler dosyanın bağlı görüntü adresleridir; çalışma anındaki ASLR adresleri değildir.

## Ana programın DLL'lere bağı

| Import modülü | İşlev | EXE IAT VA | Lookup dosya ofseti | Özgün DLL exportu |
|---|---|---|---|---|
| `anadius64` | `anadius` | `0x151356014` | `0xf263e14` | ordinal3, `0x180017a30` |
| `VERSION.dll` | `GetFileVersionInfoA` | `0x15083e374` | `0xe746e44` | ordinal7, `0x180004dda` |
| `VERSION.dll` | `GetFileVersionInfoSizeA` | `0x15083e37c` | `0xe746e4c` | ordinal11, `0x180004f9c` |
| `VERSION.dll` | `VerQueryValueA` | `0x15083e384` | `0xe746e54` | ordinal27, `0x18000537b` |

Modül metni gerçekten **`anadius64`** biçimindedir; import tablosunda `.dll` son eki yazmaz. Girdi `.anadius` bölümündeki import descriptor ile ilişkilidir: descriptor RVA `0x11356024`, bölüm RVA `0x11356000`, ham ofset `0xf263e00`, sanal boyut `0x712`, ham boyut `0x800`, bölüm bayrakları `0xc0000040`.

`anadius` exportu, `CDX` ve `RUNE` ile aynı `RET 0` gövdesine gider. İşlevsel başlatma, DLL'nin giriş zincirindedir: `0x18004bcfc` → CRT dispatcher `0x18004bbc8` → uygulama DllMain `0x180038870` → `0x180038970`. Son adresteki başlatıcı yapılandırmayı ve worker yollarını hazırlar. EXE import kaydı, export gövdesinin emulator mantığını yürüttüğü anlamına gelmez. Export gövdesi REA kanıtı: `ev_269a52c149a8d060bc06f58d2667406ae1fc6d07e89999585766b0ae17e254f7`; giriş zinciri kanıtları üst dizindeki ana rapordadır.

EXE ayrıca `WINMM.dll` üzerinden 25 isim ister. Bunların hepsi incelenen version.dll'nin WinMM sarmalayıcı exportları arasında da vardır. `VERSION.dll` için istenen üç export da özgün ve yeniden derlenen dış katmanda bulunur. Bu 29 hedefle ilgili importun isim karşılaştırmasında eksik eşleşme yoktur. WinMM isimlerinin kapsanması, EXE'nin çalışma anında version.dll'yi WinMM adıyla yüklediğini göstermez; DLL arama/yükleme sonucu gözlenmedi. Tüm isimler ve adresler `host-pe.json` içindeki `linkage` dizisindedir.

Gecikmeli importlar: `DSTORAGE.dll!DStorageGetFactory`, `d3d12.dll` ordinal102 ve ordinal101. Bunlar bu iki DLL'nin export rekonstrüksiyonundan ayrı bağımlılıklardır.

## CFG'nin etkisi

`CFG-README.md` ve `cfg-analysis.json`, Config2 ağacındaki ayarları Windows DLL parser'ına adresleriyle bağlar. CFG'nin oyun adı Madden NFL 27'dir. `FakeAuth=false` ve `PretendConnected=false`, sırasıyla `0x1800a3c39` ve `0x1800a3c3a` alanlarını sıfırlar. Bunlar `anadius64online.dll` yükleme girişimini engellemez: `0x180035b60` worker'ı → `0x180035bd0`, `version_check()==2` ve `create_online` export sözleşmesini kullanır. İlgili online DLL'nin gerçekten bulunması veya yüklemenin başarısı bu incelemede doğrulanmadı.

Bu CFG'de ServerPort, LoadExtraDLLs ve LoadExtraDLLsFromMain anahtarları yoktur. Bu, varsayılanları veya diğer girdileri devre dışı saymak için yeterli değildir. `anadius_override.cfg`, gerçek süreç argümanları, ortam, registry ve diğer kullanıcı dosyaları incelenmedi. Kullanıcı kimliği alanları raporda gizlendi; ham CFG teslim paketine kopyalanmadı.

## Kanıt ve sınırlar

- `host-pe.json`: bağımsız salt okunur PE parser gözlemleri, dosya/RVA/VA koordinatları ve export isim eşlemesi. REA tarafından üretilmiş Evidence değildir.
- `host-rea-resource-evidence.json`: REA `inspect_pe_resources`, `ev_bf04424048809677286e6ea6e29730843b5dee3523f051a9d1bd215326e1c309`; kaynak ağacı kapsamı complete, 75 ağaç girdisi / 35 kaynak. Kaynak tablosu oyun adı ve sürüm metinlerini bildirir; bu metinler dosyanın özgünlüğünü doğrulamaz.
- Debug kaydı `Madden.Main_Win64_retail.pdb`, age1, ham GUID baytları `8b63bc27461f9548aec35b2152a1892d` bildirir. PDB dosyası bulunmuş veya indirilmiş değildir.
- `fixed` dosya adı, düzenleme geçmişi ya da uyumluluk kanıtı değildir. EXE'nin PE sürüm kaynağıyla CFG oyun sürümü farklı alanlardır; sırf metinleri farklı diye uyumsuzluk çıkarılmaz.
- İsim/ordinal eşleşmesi ABI, DLL yükleme sırası, hook başarısı, oyun protokolü veya tam çalışma zamanı eşdeğerliği kanıtı değildir. EXE üzerinde tam fonksiyon analizi yapılmadı.

Sonuç: Verilen EXE ile anadius64 exportu ve version.dll'nin üç version exportu arasında somut statik bağlantı kuruldu. Yeni dosyalar tam C++ kurtarımını tamamlamaz; başlatıcı, yapılandırma ve protokol rekonstrüksiyonunun hangi ana programla eşleşeceğini sınırlar.
