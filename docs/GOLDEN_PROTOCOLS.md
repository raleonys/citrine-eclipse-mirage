# Golden araçları ve yerel zincir

Brownfield'den seçilip uyarlanan gerçek araçlar:

`MSVC Build → URSYNEX Golden Finalizer → ARYRA Golden Layout → MUSTAVEX Golden Metadata → CORVEXON Golden Trail → FALCION Golden Cleaning`

| Aşama | Uygulama ve sınır |
|---|---|
| MSVC Build | `check_msvc_shipping_link_command.py`; hedefin gerçek link reçetesi ve yalnız bağlı response dosyaları |
| URSYNEX | `ursynex_golden_finalizer.py`; tek sınırlı PE mutatörü, atomic replace, checksum finalizasyonu |
| ARYRA | `verify_aryra_golden_layout.py`; bounds/alignment/overlap/section-order kontrolü, read-only |
| MUSTAVEX | `verify_mustavex_golden_metadata.py`; sınırlandırılmış resource/metadata/export-name kontrolü, read-only |
| CORVEXON | `verify_corvexon_golden_trail.py`; export/ordinal/import/CRT-cookie/checksum kontrolü, read-only |
| FALCION | `check_falcion_golden_cleaning.py`; düz kaynak paketi, aynı source manifesti, CRC/byte/hash ve dosya kümesi |

`run_golden_pipeline.py` aşamaları bu sırayla çalıştırır; ilk hatada durur.
Sadece ayrı geçici aday kopyasını değiştirir, özgün build DLL'ini korur ve
read-only aşamalarda hash değişmediğini denetler. Altı aşama ve son isimli DLL
runtime testi geçerse hash isimli build adayı ve aşağıdaki dist bundle’ı yazılır.
Library veya resmî pointer yoktur.

## Güncel ürün hedefleri

Kullanıcının eksik hedefleri tamamlama talimatıyla dört profil
`READY_LOCAL_TARGET` durumuna getirildi. Bunlar Yellowfield ürün kurallarıdır;
`tests/support/golden/` altındaki sabit fixture kimlikleri etkinleştirilmez.

| Profil | Güncel kural |
|---|---|
| URSYNEX | Brownfield’in açıkça seçilen timestamp0xCD268EB1, linker14.0 ve Rich-yok profilini kopyaya uygular, checksum’u yeniden hesaplar. Son kullanıcı talimatıyla import descriptor’ları da sabit DLL sırasına permüte eder; isim/thunk/IAT verileri korunur. |
| ARYRA | MSVC altı section veya statik UCRT `.fptable` ile yedi section; sıralama, alignment, dosya sınırı ve çakışmama. `.fptable` tam virtual0x100/raw0x200, index4, characteristics0xC0000040 ve sıfır verili olmalıdır. Diğer section RVA/boyutları zorlanmaz. |
| MUSTAVEX | AMD64/PE32+/Windows GUI, OS/subsystem6.0, linker14.0, timestamp0xCD268EB1, Rich yok; birebir Brownfield VERSIONINFO. PNG/RCDATA/manifest yok. COFF symbol/debug/certificate yok. Internal export adı `preloader.unsigned.dll`. |
| CORVEXON | Üç tam C export/ordinal; CRT entrypoint exported DllMain'den ayrı ve executable `.text` içinde. CRYPT32/WS2_32/KERNEL32/USER32/SHELL32/MSWSOCK dışında statik import yok; MSWSOCK yalnız AcceptEx/GetAcceptExSockaddrs, CRYPT32 yalnız CryptQueryObject. `/MT`, writable image-backed `/GS` cookie, geçerli checksum. |

OLE32, önceki COM known-folder adaptöründen geliyordu; RE ortam değişkeni
okumasına dönüşle kaldırıldı. Son kullanıcı düzeltmesiyle 11. hook için
`crypt32.lib` ve doğrudan SDK `&CryptQueryObject` hedefi kullanılır; CRYPT32
normal PE importudur. Önceki beş-modül sınırlaması bu yetkili genişletmeyi
dışarıda bırakıyordu ve düzeltildi. PNG, ordinary read-only C++ veridir.
Brownfield VERSIONINFO dosyasının birebir byte'ları korunur.

```sh
python -B scripts/run_golden_pipeline.py --check-target
```

Bu komutun `READY_LOCAL_TARGET`/exit0 sonucu yalnız hedef tanımlarının
hazırlığını gösterir. Tam Golden için `build_production.cmd` gerçek MSVC
Release DLL'ini derleyip altı aşamayı çalıştırır. İlk aşama compiler kimliği,
link komutu, kaynak ve build receipt hashlerini doğrular. LLVM/Wine veya
sentetik PE, MSVC build kanıtı yerine geçmez.

Tam zincirde exit0 yalnız yerel Golden kabulüdür. Exit1 kontrol/derleme
hatasıdır; exit3 ürün hedefi eksik olduğunda BLOCKED'dır. Hedef tanımı hazır
olması, gerçek MSVC Golden veya oyun PASS iddiası değildir. Başarılı yerel
adayın yanında `.dll.sha256` üretilir; yayın yapılmaz. Kaynak ZIP paketleyici
ayrıca her ZIP yanında `.zip.sha256` üretir.

Bütün read-only aşamalarda aday hash’i sabit kalır. URSYNEX yalnız izinli timestamp,
linker, tam Rich bloğu ve checksum alanlarını değiştirebilir; diğer byte’lar
değişirse durur. Özgün build DLL'inin değişmediği de
zincir sonunda kontrol edilir. Fixture ve bozuk-girdi regresyonları ayrı
korunur; fixture'lar aktif ürün politikasını açamaz.

## Yerel dist çıktısı

Altı aşama geçtikten sonra finalized kopya **preloader_l.dll** adıyla mevcut
runtime host'a verilir. Üç gerçek yükleme/başlatma/socket/profile/kapatma/unload
çevrimi geçmeli ve adayın byte'ları değişmemelidir. Ardından şu dosyalar yazılır:

- `dist/preloader_l.dll`
- `dist/preloader_l.dll.sha256`
- `dist/LOCAL_DIST_RECEIPT.json`

Böylece runtime adı Bluefield hook11 filtresiyle eşleşir. Ham link çıktısı
`preloader.unsigned.dll` kalır; export-directory internal adı ve üç C export
kopyalama sırasında değiştirilmez. Önceki hash adlı build adayı da korunur.
Golden/son runtime kontrolü başarısızsa yeni dist verilmez; önceki başarılı
bundle varsa kendi hash/source receipt'iyle korunur. Güncel denemenin sonucu
build dizinindeki `golden-local/GOLDEN_REPORT.json` içindedir. Compile, Debug,
Native ve CRT profilleri dist'e yükseltilmez; resmî yayın yapılmaz.

Bütün alt Python yorumlayıcıları `-B` ve/veya miras alınan
`PYTHONDONTWRITEBYTECODE=1` kullanır. Operatör akışı önce, yalnız manifestteki
birinci taraf Python araçlarına ait bilinen `__pycache__/*.pyc` dosyalarını
temizler. FALCION rastgele cache/ek kaynak dosyalarını hâlâ reddeder.
Ayrıntı: [Production düzeltmesi](PRODUCTION_IMPORT_FIX.md).

Tam alan tablosu ve önceki preserve-compiler kararının düzeltmesi: [metadata denetimi](REFERENCE_METADATA_AUDIT.md).

Son kullanıcı import-sırası düzeltmesi: URSYNEX, yukarıdaki header/Rich/checksum
alanlarına ek olarak yalnız tam import descriptor kayıtlarını
CRYPT32/WS2_32/KERNEL32/USER32/SHELL32/MSWSOCK sırasına permüte eder.
Descriptor isim/RVA alanları kendi kaydıyla birlikte taşınır; import thunk/name/IAT
byte’ları ve directory yerleri korunur. Modül kümesi farklıysa işlem reddedilir.
Checksum bundan sonra hesaplanır. CORVEXON sırayı read-only denetler. Bu ek izin
önceki yalnız-header mutasyon kapsamını son kullanıcı talimatıyla genişletir.
