# REDFIELD RF20 BUILD FIX04 — Eksik allocator link bağımlılığı

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

FIX03 kullanıcı logunda Visual Studio ortamı hazırlandı, 54 çekirdek dosya ve ilk test derlendi. İlk testin linkinde `allocate_transient_bytes` ve `release_transient_bytes` için LNK2019, ardından LNK1120 oluştu. Bu nedenle DLL aşamasına ulaşılamadı. Kullanıcının orijinal logu ve SHA-256 kaydı evidence altında saklandı; bu sonuç burada yapılmış yerel Windows çalıştırması olarak sunulmaz.

Windows `value_codec_semantic.cpp` allocator header'ını ve iki fonksiyonu kullanıyordu; uygulama derleme grafiğinde yoktu. Linux dalı malloc/free kullandığından önceki Linux regresyonları bu eksikliği yakalamadı. Bu paketleme bağımlılığı eksikliği önceki teslimatlarda gözden kaçtı.

FIX04, SHA-256'sı doğrulanmış orijinal HF19 arşivinden `src/runtime/memory_allocator.cpp`, onun `current_errno_slot` bağımlılığını karşılayan `src/runtime/ptd_runtime.cpp` ve PTD header'ını byte-identical alır. İki TU hem testlerin hem DLL'in ortak çekirdek grafiğine eklenir; CMake aynı grafiği okur. Gövdeler bölünmez, allocator stub ile değiştirilmez. Transient failed-free sahipliği, pending release listesi, retry ve errno davranışı orijinaldir. RF20 yaşam döngüsüne PTD initialization eklenmez; transient API buna ihtiyaç duymaz.

15 orijinal allocator/PTD testi ile bir yeni semantic/allocator testi eklendi. Toplam 56 çekirdek TU ve 86 regresyon vardır. Toplam yeni byte-identical dosya19, kümülatif1133; mevcut24 derived recipe değişmez. Parent C++/header/export dosyalarının tamamı aynı byte'larla korunur. FIX03 ortam hazırlama ve canlı ilerleme kodunda yalnız FIX04 sürüm etiketi değişir. Production ABI ve GPL-3.0-only korunur; resmî HF19/CURRENT değiştirilmez.

## Güncel doğrulama

| Kontrol | Sonuç |
|---|---|
| Temiz GCC C++20 tam normal derleme |86/86 PASS;143 object,87 linked output|
| Orijinal GetConfig legacy diagnostic |exit14, ayrı FAIL_OBSERVED|
| 15 allocator/PTD testi + Windows semantic dalı |normal/ASan/UBSan:48 pozitif çalıştırma PASS|
| Allocator olmadan aynı Windows dalı |iki transient sembol eksikliğiyle beklenen link reddi|
| Allocator var, PTD yok |current_errno_slot eksikliğiyle beklenen link reddi|
| Environment mock / gerçek POSIX runner |13 /7 PASS|
| Gerçek HF19 arşivinden yeni19 dosya byte karşılaştırması |PASS|
| Manifest/adoption/recipe/graph/launcher +4 bağımsız bozulma testi |PASS / beklenen ret|
| Deterministik ZIP, CRC ve temiz çıkarım verifier |PASS|
| FIX04 yerel Windows/MSVC/DLL/oyun |NOT_RUN_HERE|

Windows dalı testi Linux ELF üzerinde gerçek `_WIN32` SemanticValue gövdesini ve Windows sınıf düzenini derler; `__declspec`/`__stdcall` tanımları ELF için kaldırılır, yalnız Heap API'leri kontrollü malloc/free shim'iyle sağlanır. Gerçek allocator/PTD uygulamaları linklenir. Entity decode, sahiplik/self-alias, null heap, allocation failure, failed free ve retry denenir. Bu test Windows ABI, gerçek Heap API, MSVC veya DLL kabulü değildir. ASan leak detection kapalıdır; leak-free iddiası yoktur. Tam86 testin sanitizer profilleri bu FIX04'te çalıştırılmadı.

## Çalıştırma

Yeni ZIP'i ayrı klasöre çıkarın. `REDFIELD_RF20` içinde `scripts\build_orchestration\build_verify.cmd` çalıştırın. Windows akışı 56 core +86 test +1 legacy diagnostic +DLL/host birimleriyle toplam147 compile eylemidir. Önce testler, sonra production DLL linki, lifecycle smoke ve native test DLL/host çalışır. Production DLL yolu `build\msvc_x64\preloader_l.dll`; ana log `build\msvc_x64\build.log`.

Bu teslimat kaynak paketidir; burada üretilmiş Windows DLL içermez. Windows'ta tüm aşamalar geçmeden tam DLL kabulü iddia edilmez. Kaynak parent FIX03 SHA-256: `b96c3ac63b5d7bfd0f0608966d1e5fd2471c5fefba00be940d303f08638b364f`. HF19 kaynak arşivi SHA-256: `c7e16beac0618c7f364810261f8377fdfcfb7499d184a603615690ec269e24ab`. Governance master2026-10-19.3/v5 geliştirme kuralları bu görevde okundu; full master hash pinleri parent'tan korunur.
