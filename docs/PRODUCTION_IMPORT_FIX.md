> Tarihsel snapshot: hook11/lifecycle ve metadata için son düzeltme
> [Bluefield uyumu](HOOK_ALIGNMENT.md) ve [Brownfield metadata denetimi](REFERENCE_METADATA_AUDIT.md) içindedir.

# Production, CRYPT32 ve son çıktı düzeltmesi

Görev: YF-PRODUCTION-20261029. Parent kaynak ZIP'i ve SHA256 yan dosyası korunur.

Kullanıcının gerçek MSVC production logunda DLL derlemesi ve Golden'ın ilk
beş aşaması geçti. OLE32 yoktu; yalnız VERSIONINFO resource'u vardı, iki PNG
read-only `.rdata` içindeydi. Altıncı FALCION aşaması
`scripts/__pycache__/golden_policy.cpython-313.pyc` nedeniyle durdu. Üst
Python `-B` ile başlatılsa da CORVEXON'un üç alt yorumlayıcısı `-B` almıyordu.
Bu gerçek neden, eski/yeni alt süreç davranışını karşılaştıran bir regresyonla
yeniden üretildi ve düzeltildi. Artık üç alt çağrıda `-B` var, pipeline ve
scoped PowerShell sürecinde `PYTHONDONTWRITEBYTECODE=1` miras alınır.

Önceki çalışmanın ürettiği cache için `clear_tool_python_cache.py`, yalnız
manifestte kayıtlı scripts/tools/tests Python kaynaklarının standart
cache-tag/opt bytecode dosyalarını temizler. Vendor'a veya gerçek kaynak
byte'larına dokunmaz, manifest yenilemez; bilinmeyen dosyaları silmez veya
FALCION'da yok saymaz. Bu yüzden kullanıcı mevcut kaynakta tekrar derleyebilir.

Kullanıcı 11. hook için Brownfield/Bluefield gibi gerçek CRYPT32 importu
istedi. Referansların ikisinde de doğrudan `&CryptQueryObject` ve `crypt32`
link girdisi doğrulandı. Yellowfield'deki önceki dinamik-only lookup, Anadius
baseline import listesini yeni hook'a yanlış uyguluyordu. Şimdi core
`crypt32.lib` üzerinden bağlanır; owned native target çözümünde SDK
`&::CryptQueryObject` adresi kullanılır. Gereksiz explicit crypt32 LoadLibrary/
FreeLibrary lease'i kaldırılır; normal PE dependency grafiği yaşam süresini
taşır. MinHook trampoline/callback/cleanup ve RE selector davranışı korunur.

`CryptQueryObject` bizim DLL'nin export'u olmaz: import tablosunda
`CRYPT32.dll -> CryptQueryObject` bulunur. Golden ve gerçek DLL incelemesi bunu
zorunlu kılar; public export sayısı beş kalır, OLE32 dönmez. Referans hashleri
ve native kullanıcı logu: [PRODUCTION_IMPORT_FIX.json](evidence/PRODUCTION_IMPORT_FIX.json).

Son dosyanın dist'e yazılması da parent pakette eksikti. Altı Golden aşaması
ve finalized **preloader_l.dll** kopyasıyla üç gerçek runtime çevrimi geçince
DLL/SHA256/receipt üretilir. Runtime host artık unload kontrolünde verilen
dosyanın gerçek basename'ini kullanır; unsigned adına sabit kontrol yapmaz.
Filtre `preloader_l.dll` olarak korunur.
[Golden ve dist akışı](GOLDEN_PROTOCOLS.md),
[yerel çıktı sözleşmesi](../metadata/LOCAL_DIST_CONTRACT.json).

Sentetik orchestration testleri her Golden hatasında ilk hata ile durmayı,
runtime hatası/değişen DLL nedeniyle dist verilmemesini, önceki output'un
korunmasını ve dosya-commit hatasında rollback'i doğrular. Bu testler MSVC
Golden PASS değildir. Gerçek Windows/MSVC'de güncel kaynağın bütün zinciri ve
dist üretimi henüz çalıştırılmadı; oyun çağrı izi de verilmedi.

Yerel doğrulama: Release/Wine173/173, Debug5/5 ve28 infrastructure regresyonu
geçti. Final adı preloader_l.dll olan gerçek LLVM DLL ile üç runtime çevrimi
geçti. Release ve Debug import tablolarında CRYPT32!CryptQueryObject var;
OLE32 yok, VERSIONINFO ve PNG veri alanları korundu. Native MSVC güncel
kaynak/dist ve oyun kabulü bu sonuçlardan ayrı, henüz çalıştırılmadı.
