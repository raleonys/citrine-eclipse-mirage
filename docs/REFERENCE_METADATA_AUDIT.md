# Brownfield metadata denetimi

Kullanıcının Compilation Timestamp dahil tüm metadata’yı Brownfield’e göre kontrol
etme talimatı uygulanır. Önceki compiler timestamp/linker/Rich değerlerini koruma
kararı kaldırılmıştır. Kaynak, sağlanan Brownfield arşivindeki gerçek
MUSTAVEX/ARYRA/URSYNEX politikalarıdır; parser fixture’ları değildir.

`docs/evidence/REFERENCE_METADATA_POLICY_AUDIT.json` 19 beyan edilmiş politika
grubunun Brownfield ile birebir karşılaştırmasını kaydeder. PE’nin her byte’ını
Brownfield DLL’iyle aynı yapma hedefi yoktur: kod, adresler, image boyutu ve
üç açık kullanıcı istisnası farklıdır.

## Final DLL için alanlar

| Alan | Beklenen değer |
|---|---|
| `machine` | `"0x8664"` |
| `pe_format_magic` | `"0x020B"` |
| `subsystem` | `2` |
| `os_version` | `[6, 0]` |
| `subsystem_version` | `[6, 0]` |
| `linker_version` | `[14, 0]` |
| `timestamp` | `"0xCD268EB1"` |
| `export_internal_name` | `"preloader.unsigned.dll"` |
| `rich_header` | `"ABSENT"` |
| `additional_resource_assets` | `[]` |
| `VERSIONINFO.fixed.dwSignature` | `0xFEEF04BD` |
| `VERSIONINFO.fixed.dwStrucVersion` | `0x00010000` |
| `VERSIONINFO.fixed.dwFileVersionMS` | `0x3FE24D7B` |
| `VERSIONINFO.fixed.dwFileVersionLS` | `0x9AF25405` |
| `VERSIONINFO.fixed.dwProductVersionMS` | `0xB54B7381` |
| `VERSIONINFO.fixed.dwProductVersionLS` | `0x01A10EC2` |
| `VERSIONINFO.fixed.dwFileFlagsMask` | `0x0000003F` |
| `VERSIONINFO.fixed.dwFileFlags` | `0x00000000` |
| `VERSIONINFO.fixed.dwFileOS` | `0x00040000` |
| `VERSIONINFO.fixed.dwFileType` | `0x00000002` |
| `VERSIONINFO.fixed.dwFileSubtype` | `0x6A1AAE62` |
| `VERSIONINFO.fixed.dwFileDateMS` | `0x00000000` |
| `VERSIONINFO.fixed.dwFileDateLS` | `0x00000000` |
| `VERSIONINFO.strings.FileDescription` | `EA Javelin Anticheat` |
| `VERSIONINFO.strings.LegalCopyright` | `Copyright 2023 Electronic Arts. All rights reserved.` |
| `VERSIONINFO.strings.ProductName` | `EA Javelin Anticheat` |
| `VERSIONINFO.strings.ProductVersion` | `3.9.5` |
| `VERSIONINFO.string_table` | `040904b0` |
| `VERSIONINFO.translation_language` | `0x0409` |
| `VERSIONINFO.translation_codepage` | `1200` |

Timestamp **0xCD268EB1** Brownfield’in sabit hedef değeridir; bugünkü gerçek
derleme saati değildir. Bir PE görüntüleyicisi bunu kendi saat diliminde tarih
olarak gösterebilir. Doğrulama ham 32-bit değeri karşılaştırır.

## Diğer kontroller

| Grup | Kural |
|---|---|
| Debug | Debug Directory, RSDS, REPRO, COFF symbol table yok; PDB paketlenmez |
| İmza | Authenticode/certificate/security table yok |
| Resource | Tek VERSIONINFO yaprağı; PNG/RCDATA/manifest yok; Brownfield RC byte’ları aynı |
| Avatar | Kullanıcı PNG’si salt okunur C++ veri, ayrı RE fallback korunur |
| Sections | `.text,.rdata,.data,.pdata,.rsrc,.reloc`; /MT UCRT profili `.pdata` sonrası `.fptable` ekler |
| `.fptable` | Index4, virtual0x100, raw0x200, chars0xC0000040; RW/non-executable ve sıfır verili |
| Yapısal geçerlilik | Alignment, dosya sınırları, raw/virtual non-overlap ve RVA sırası kontrolü |
| Hariç hedefler | Diğer section adres/boyutları, entrypoint RVA veya tüm image boyutu Brownfield’e eşitlenmez |
| Kimlik | Brownfield’in beyan edilmiş yasak eski proje sürüm/isim listeleri aynı |
| Export | Kullanıcının beş C export’u korunur; internal module adı `preloader.unsigned.dll` |
| Import | Kullanıcının altı modülü, CRYPT32 yalnız `CryptQueryObject`; OLE32 yok |
| CRT | Gerçek MSVC /MT, /GS cookie ve executable CRT entrypoint denetlenir |
| Bütünlük | PE checksum hesaplanır; read-only Golden aşamalarında byte değişimi reddedilir |

## Hangi dosya denetlenir

Ham `build/.../preloader.unsigned.dll` değiştirilmez. URSYNEX ayrı aday kopyasında
yalnız timestamp4, linker2, Rich bloğu ve checksum4 alanlarını değiştirebilir.
OS/subsystem6.0 ve Debug Directory yokluğu önkoşuldur; hatalı derlemeyi gizlemek
için bu alanları silmez. Diğer byte’larda değişiklik varsa finalizasyon reddedilir.

Tüm Golden kontrolleri ve final isimli runtime testi sonrası
`dist/preloader_l.dll`, SHA256 ve receipt yazılır. Dosya adının değişmesi internal
export module adını değiştirmez. Eski raw unsigned dosyaya bakmak final metadata
sonucunu göstermez.

```powershell
python -B tools/inspect_dll.py dist/preloader_l.dll --final --report build/final-metadata.json
```

Bu read-only kontrol MUSTAVEX/header/checksum/resource/avatar ve statik CRYPT32
sonucunu raporlar; ARYRA/CORVEXON/MSVC kapıları için tam production zinciri gerekir.
Ham DLL denetimi `--final` olmadan RAW_LINK_OUTPUT / NOT_FINALIZED farklarını gösterir.

## Kanıt sınırı

Politika karşılaştırması ve negatif PE regresyonları doğrulandı.
`docs/evidence/REFERENCE_METADATA_PE_PROBE.json`, gerçek LLVM development DLL’inden
üretilmiş açıkça işaretli **diagnostic fixture** üzerinde header/VERSIONINFO
finalizasyonunu doğrular. Fixture hazırlığında yalnız test kopyasının development
Debug Directory/COFF symbol pointer alanları sıfırlanmıştır; production finalizer
bunu yapmaz. LLVM section/import/CRT farkları nedeniyle bu fixture Golden değildir
ve dist’e taşınmaz. Yeni kaynak için gerçek Windows/MSVC production turu gereklidir.

Son kullanıcı talimatıyla URSYNEX izinli kapsamına tam import descriptor
permütasyonu eklendi. Brownfield header/VERSIONINFO/layout değerleri korunur.
Güncel karşılaştırma docs/evidence/WINDOWS_IO_REFERENCE_POLICY_AUDIT.json içindedir;
önceki19-group raporu önceki görevin snapshot’ıdır. [Güncel değişiklik](WINDOWS_IO_FIX.md).
