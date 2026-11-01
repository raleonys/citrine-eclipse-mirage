# REDFIELD RF28 — B01 kaynak incelemesi ve build sınırları

2026-10-21 · LOCAL_BUILD / NOT_RELEASE

Debug için ayrı bir uygulama codebase'i yoktur. Production DLL, native test DLL'i
ve portable testler ortak çekirdeği kullanır. CMake MSVC Debug için /MTd, diğer
yapılandırmalar için /MT seçer. Windows Python betiği sabit /O2 /MT kullanır;
ayrı bir Debug seçeneği sunmaz. CMake tanımı çalıştırılmış Debug/CRT kabulü değildir.

Production DLL üç C export tutar. Native test DLL'inde aynı çekirdeğe yalnız
DLL entry TU'sunun PRELOADER_L_NATIVE_TEST=1 varyantı ve test export'ları eklenir.
abi_smoke ve native_acceptance test host'larıdır. Generic allocator/PTD iki TU'su
yalnız regresyon desteğidir; production link'e girmez. Bu build ayrımları
docs/ARCHITECTURE.md içinde ownership haritasıyla birlikte yazılmıştır.

B01 tamamlandı: M01 yapılandırma/parser/projection no-change kararıyla; M16 hex
mevcut ayrık-buffer önkoşulu header'da açıklaştırılarak; M20 build/paket/güncel
belgeler kaynak incelemesi ve yapısal doğrulamalarla kapatıldı. RFCL-001 ve
RFCL-002 kapalıdır. RF20 TCP notu docs/history/RF20_TCP_DELIVERY.md altında
önceki byte'larıyla korunur. Aktif262 uygulama dosyasından yalnız hex header
açıklaması değişti;261 dosya ve bütün implementation/test gövdeleri RF27 ile aynı.
ConfigStore, raw escape/typed/empty/alias/Online farkları, atomik publication,
borrow ömürleri ve surge.cfg modül ankrajı incelendi; yeni davranış eklenmedi.

| Kontrol | Sonuç |
|---|---|
| Temiz Linux GCC C++20 build |58 core+2 test-support;157 obje,97 linked çıktı PASS|
| Pozitif regresyon |96/96 PASS;RF27 ile stdout/exit96/96 aynı|
| Projection allocation transaction |115 hata noktasında eski snapshot korunuyor|
| Mimari negatifler |13 beklenen ret;constinit source probe PASS|
| Encoding / environment / gerçek POSIX süreç |16 /13 /7 kontrol PASS veya beklenen ret|
| Paket/ancestry/manifest/ZIP/fresh-extract |PASS;2 paket bozma beklenen ret|
| GetConfig legacy tanısı |FAIL_OBSERVED,exit14;pozitif96'dan ayrı|
| RF28 ASan/UBSan |NOT_RUN;RF27 kanıtı tarihsel saklandı|
| Windows/MSVC,Debug/CRT,native DLL,oyun,CMake |NOT_RUN|

GPL-3.0-only,LICENSE,vendor/MinHook pinleri,üç production export,35 route binding,
yedi hook ve CMake/build graph korunur. Kod kimliği preloader_l,CFG surge.cfg'dir.
1327 adoption ve1585 ordered recipe doğrulanır. Yeni recipe yalnız değişen header'ın
exact RF27 kaynağına bağlıdır;gereksiz tüm-codebase ancestry kopyası eklenmedi.
Canonical HF19/CURRENT ve immutable parent değiştirilmedi.

20 modülün5'i kapalı,15'i açık. Bu bir efor yüzdesi değildir. Sıradaki B02,
shutdown/module-pin ve bellek sahipliği incelemesidir. Refactor sonunda beklenen
tek maintained codebase,açık production/test sınırı,korunan ABI/CFG/davranış ve
kaynağa bağlı doğrulama kayıtlarıdır. Bütün codebase tamamlandı veya Windows/oyun
doğrulandı denmez. Gelecek source/dependency değişimi etkilenen kapanışı yeniden açar.

Windows'ta scripts\build_orchestration\build_verify.cmd çalıştırılır.
Çıktı build\msvc_x64\preloader_l.dll;surge.cfg DLL'in yanındadır.
Başlık RF28 REFACTOR CLOSURE B01 olmalıdır. Mevcut betik Debug profili üretmez.
Parent RF27 ZIP SHA-256:63429a0a9071ea95ed4728c7eb4a05ac9f4109b306079ff5bf77b3fdcb130155.
