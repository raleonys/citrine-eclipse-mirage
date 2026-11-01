| Giriş: scripts/build_orchestration altında | Build ve çıktı | Kapsam |
|---|---|---|
| build_verify.cmd | build/verify/msvc_x64/preloader_l.dll | Mevcut developer aggregate;96 regresyon+ayrı legacy diagnostic,Windows'ta ABI/native host;dist'e yazmaz |
| build_production.cmd | build/production/preloader.unsigned.dll | /O2 /MT /GS aday+public ABI smoke;Golden bağlaması eksik olduğundan ardından exit1;nihai dist DLL yok |
| build_debug.cmd | build/debug;dist/debug/preloader_l_debug.dll | /Od /Zi /MTd /GS;ayrı objeler,MinHook objeleri,PDB/MAP;mevcut ABI host |
| build_native_acceptance.cmd | build/native;dist/native_acceptance/preloader_l_native_test.dll | /MT /GS;native define yalnız entry varyantında;mevcut native host |
| build_crt_acceptance.cmd | build/crt;dist/crt_acceptance | /MT /GS üretim-ABI test DLL'i+native test DLL'i;export/entry/import ve mevcut host kontrolleri |

**Windows'ta normal derlenmiş developer DLL için build_verify.cmd kullanılmaya
devam edilir. Yeni yol build\verify\msvc_x64\preloader_l.dll;surge.cfg DLL'in
yanında olmalıdır.** Bu DLL Golden ile finalize edilmiş shipping çıktı değildir.
build_production.cmd bu checkpoint'te Golden aşamasında açıkça başarısız olur;
aday DLL gerçekten derlenip ABI host'undan geçirilmeden bu noktaya geçmez.
Gerçek Windows çalışması bu ortamda yapılmadığından bu adımların mevcut makinede
başarılı olacağı doğrulanmış değildir.

Profiller shared core kaynaklarını bir kez maintain eder,her biri kendi build
dizininde yeniden derler. Bir profilin objesi diğerinin linkine alınamaz. Test
support/regresyon objeleri operator DLL'lerine giremez. Production include kökü
test-support header'larını görmez. Native macro'nun production/core varyantına
taşınması reddedilir. Tüm Windows bağımlılık C/C++ objeleri ilgili profile göre
aynı /MT veya /MTd ile derlenir. Default CRT startup korunur;custom /ENTRY,
/NODEFAULTLIB,/MD veya üretime /DEBUG:FULL sızması profile policy'de reddedilir.

Build kilidi OS tarafından tutulur;process çökünce lock handle'ı bırakılır,
stale PID dosyasına güvenilmez. Yol traversal/symlink reddi,profil bazında eski
çıktı temizliği ve stale receipt iptali vardır. Live process çıktısı/heartbeat,
nonzero exit ve timeout davranışı korunur. Visual Studio'nun quoted UTF-16LE
SET capture'ı ve disposable dizini process CWD yapmayan RF20 düzeltmesi aynı
ortak helper'da kalır. Compiler/linker hash'leri ve kaynak input envanteri final
receipt öncesi yeniden denetlenir;sonradan eklenen input da fark edilir.

Başarılı scoped nonproduction host kontrollerinden sonra dist kopyaları aynı
dizindeki temp dosyaya yazılır,hash karşılaştırılır,atomik rename ve readback
yapılır. Batch ortasında hata çıkarsa o profilin dist'i temizlenir. Başarı
receipt.json,hata failure.json olarak ayrıdır. Üretimin dist/preloader_l.dll
yolu bu primitive üzerinden de engellenir;--skip-golden gibi bir seçenek yoktur.
build/dist generated çıktı sayılır ve source verifier'ın immutable paket
envanterinden ayrı tutulur;kaynak ZIP bunları içermiyor. CMake developer aggregate
olarak tutuldu;production dist için ikinci bir finalization otoritesi değildir.

URSYNEX Golden Finalizer → ARYRA Golden Layout → MUSTAVEX Golden Metadata →
CORVEXON Golden Trail → FALCION Golden Cleaning sırası kayıtta korunur,ancak gerçek
beş validator ve destination resource/layout/metadata bağlaması henüz aktarılmadı.
Production bu yüzden CANDIDATE_BUILT_GOLDEN_PENDING/hata verir;eski PASS veya boş
validator ile kabul üretmez. Üretim dist finalization M20.3'te gerçek Golden
uyarlamasıyla bağlanacak. Bu checkpoint dört akışın tam HF19 parity kapanışı değildir.

Yeni dahili PE kontrolü x64 DLL,üç public export veya altı native export ve exact
ordinals,nonzero executable entry,ASLR/NX/high-entropy ve dynamic CRT DLL import
bulunmamasını denetler. Bu kontrol beş Golden protokolünün veya CRT'nin bütün
davranış testlerinin yerine geçmez. Native/CRT host kapsamı mevcut RF29 kapsamıdır;
HF19 senaryo/stress eşlemesi ve debug-only diagnostic composition hâlâ açık.
DLL entry içindeki guard'lı native fault/telemetry composition ayrıca ayrılmadı.

