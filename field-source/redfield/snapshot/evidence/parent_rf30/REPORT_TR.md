# REDFIELD RF30 — ortak derleme motoru ve ayrı profiller

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

Bu tur M20.2'nin derleme altyapısı refactor edildi. RF29'daki compiler/process,
Visual Studio ortam hazırlığı ve output doğrulaması build_common.py içindeki
tek BuildSession'a ayrıldı. Developer runner ve dört operator girişi aynı
compile/link/input/object/tool denetimini kullanır. Eski HF19'un dokuz build
betiği exact referans/hash ile arşivlendi;kilit,confinement,eski çıktı temizliği
ve dist davranışları bu mevcut altyapıya uyarlandı. İkinci bir maintained eski
PowerShell backend'i eklenmedi. 262 uygulama dosyasının261'i RF29 ile byte olarak
aynı;bir mevcut profil regresyonu gerçek örnek CFG'yi okumak üzere genişletildi.
Production C++ gövdeleri aynı;runtime davranışına yeni özellik eklenmedi.

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

| Doğrulama | Sonuç |
|---|---|
| Temiz gerçek GCC C++20 | 58 production+2 support,157 object,97 linked çıktı |
| Regresyon | 96/96 PASS;RF29 stdout/exit96/96 aynı |
| Yeni profil/PE/output kontrolleri | 55 beklenen sonuç;Windows süreçleri taklit,PE örnekleri sentetik |
| Önceki boundary/import kontrolleri | 30 beklenen sonuç;gerçek GCC header görünürlüğü dahil |
| Mimari /encoding /VS ortam /POSIX process | 13 /16 /13 /7 beklenen sonuç |
| Gerçek nm objeleri | Production'da generic allocator/PTD modeli yok;supportta var |
| Paket/ZIP/repack/fresh extraction | PASS;2 paket bozma beklenen ret |
| Legacy GetConfig | FAIL_OBSERVED exit14;96 pozitiften ayrı |
| Windows/MSVC,Debug,native,CRT,Golden,CMake,oyun | NOT_RUN |

Sanitizerlar production C++ gövdeleri değişmediğinden bu tur yeniden çalıştırılmadı. 281
maintained-code dosyası ve4 vendor integration yeni mandatory refactor kuralına
göre PENDING;QUALIFIED0. Normal local build pending review ile çalışabilir;
verify_source.cmd --require-refactor-complete mevcut durumda beklenen exit1 verir.
4 önceki source review kapalı,16 modül açık. M20 kapanmadı;sonraki iş M20.3
Golden/resource/link policy,ardından M20.4 coverage/composition ve B02–B07.
GPL-3.0-only,preloader_l,surge.cfg,pinler,üç export,35 binding,yedi hook ve
HF19/CURRENT korunur. Governance normative mirror2026-10-21.1.

## GetConfig exit14 ve oyun içinde eksik profil bildirimi

Eski test_get_config.cpp normal GetConfig/34 servis sırası,id/sender veentity
escaping kontrollerini geçiyor.Son kontrolde id="bad&bogus;" için invalid_request
bekliyor.Mevcut tek TinyXML2 akışı literal metni koruyup yanıta bad&amp;bogus;
yazıyor.Bu beklenti farkı exit14 üretiyor;normal GetConfig yanıtının bütünüyle
başarısız olduğu anlamına gelmiyor.Diagnostic ayrı FAIL_OBSERVED kalıyor;beklenti
değiştirilerek PASS'a çevrilmedi.Nihai entity sözleşmesi M07/M08 incelemesinde
açıkça ele alınmalı.

Somut örnek dosya kusuru:RF29 examples/surge.cfg yalnız Emulator içeriyordu.
Yüklenen CFG overlay_loaded olur;mevcut/pinned HF19 politikası eksik User
alanlarını default yerine boş döndürür.Eski örnekle Persona veUserId boştu.Yeni
örneğe mevcut constructor default'larıyla dokuz explicit User alanı eklendi;
kullanıcı bunları kendi yerel değerleriyle düzenleyebilir.Parser/handler/default
politikası değiştirilmedi.Gerçek örnek existing test_profile_response'a registered
argüman olarak verilir;Python regression runner veCTest bunu graph'tan alır.Build
input digest örneği de bağlar.Yeni test eski örneği exit1 ile reddeder,yeni
örnekle exit0 geçer.

Gerçek GCC/core probe GetConfig34 servis veentity escaping davranışını,eski CFG'de
username/user-id0byte,yenide5/13byte olduğunu doğruladı.GetProfile ayrı bir yoldur;
exact lowercase index attribute'u olmadan yanıt verilmez.Current transport
plaintext LSX testlerinden geçer;SDK handshake/session crypto aktarımı hâlâ
deferred.Oyundaki eksik profilin gerçek istek/yanıt izi bu ortamda yok;örnek
kusurunun bulunması oyundaki tek kök nedenin kanıtı değildir.Bildirim tracker'da
açık takip edilir.Evidence:evidence/rf30_getconfig_profile_qa.json.
