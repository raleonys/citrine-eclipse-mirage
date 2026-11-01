# Production Golden zinciri

`build_production.cmd` gerçek bağlı yolu çalıştırır:

MSVC → URSYNEX → ARYRA → MUSTAVEX → CORVEXON → FALCION → atomic DLL publication.

## Derleme ve dosyalar

Production, seçilen Visual Studio'nun vcvars64 ortamını alır ve CMake NMake
Makefiles kullanır. Böylece HF19'un target-local link.txt/build.make ve referenced
response-file closure denetimi gerçek MSVC link kuralına bağlanır. Ayrı CMake kökü
`build/production/msvc` olur; eski Visual Studio çözüm cache'i ile karışmaz.
Debug/Native/CRT girişleri kendi mevcut Visual Studio köklerinde kalır.

- MSVC ham DLL: `build/production/msvc/preloader_l.dll`.
- Başarılı Golden DLL: `dist/production/preloader_l.dll`.
- Beş aşamalı receipt: `build/reports/golden_production.json`.
- Genel build receipt: `build/reports/build_production.json`.

Production'a özel `resources/golden_version.rc` HF19 allowlisted resource'unun
byte-identical kopyasıdır. `exports/production.def` aynı DllMain@1/SURGE@2/
DllShutdown@3 public ABI'sini ve Golden internal export-directory adını taşır.
Debug/test resource ve DEF dosyaları değişmez. C++ namespace preloader_l kalır.
Production linkinde /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO
/MANIFEST:NO ve Windows subsystem 6.00 açıkça verilir. MSVC normal CRT startup,
/MT, /GS, ASLR/NX/CFG korunur; custom entrypoint veya default-library bypass yoktur.

## Aşamalar ve hata davranışı

1. URSYNEX yalnız staged kopyada approved linker version/timestamp/Rich/checksum
   sonlandırmasını yapar. MSVC'nin ham çıktısına yazılmaz.
2. ARYRA section/alignment/bounds ve /MT fptable profilini doğrular.
3. MUSTAVEX allowlisted metadata/resource/internal-name/debug yüzeyini doğrular.
4. CORVEXON gerçek link kuralını, tam export/ordinal kümesini, import/CRT/entrypoint/
   SecurityCookie/checksum durumunu doğrular; kendi upstream kontrollerini korur.
5. FALCION mevcut sealed source manifestini, root allowlist'ini, kaynak hash'lerini,
   generated/archive/sensitive-material dışlamalarını ve flat DLL payload'ını doğrular.

Her aşamanın exit code'u, stdout/stderr'i ve DLL öncesi/sonrası SHA'sı receipt'e
girer. URSYNEX dışındaki aşamalarda byte değişimi ayrıca başarısızlıktır. Bir aşama
başarısızsa sonraki aşama ve yeni DLL publication çalışmaz. Önceki kabul edilmiş
dist dosyası varsa başarısız denemeyle değiştirilmez. Başarılı DLL aynı filesystem
üzerinde geçici dosyadan os.replace ile yayınlanır. Kaynak veya MSVC ham DLL'i
işlem sırasında değişirse publication reddedilir. --skip veya forced-PASS yolu yoktur.

FALCION yeni projenin **LOCAL_BUILD / NON_RELEASE_WORK** source layout'una
uyarlanmıştır. Build/dist/cache yerel geçicidir; audit evidence DLL payload'ına
alınmaz. Kaynak ZIP'in tam manifest/CRC/SHA/path kontrolü check_package.py ile
ayrı yapılır. FALCION source manifesti yenilemez; değiştirilmiş kaynağı kendi
kendine kabul etmez. HF19'a özgü uygulama dosya adları ve route-test metinleri bu
yeni projenin source layout'u olarak taklit edilmez. Resmî release/G0–G4 veya oyun
kabulü, LOCAL_BUILD Golden receipt'inden çıkarılmaz.

## Doğrulama sınırı

Bağlantı testi beş gerçek Python aracı sentetik PE üzerinde alt süreçlerde çalıştırır.
Her gerçek kapının başarısızlığı, link-policy reddi, read-only byte mutasyonu,
source drift, payload PDB ve sensitive-material sızıntısı, unsafe output ve
başarısızlıkta önceki output'un korunması sınanır. Sentetik PE çalıştırılmaz.
Bu sonuç Windows/MSVC ile yeni DLL'in üretildiği veya gerçek oyunun açıldığı
anlamına gelmez. O sonuçlar Windows build receipt'iyle ayrıca kaydedilir.
