# REDFIELD RF20 BUILD FIX03 — Kurulum sonrası WinError32

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

Kullanıcının FIX02 konsol çıktısı vswhere exit0 ve VsDevCmd exit0, ardından rf_vs_xxfa02xi klasöründe WinError32 gösteriyor. Hata compiler seçimi/derleme başlamadan temporary-directory context çıkışındadır. Visual Studio setup başarısızlığı veya C++ compilation hatası değildir. Hangi sürecin directory handle tuttuğu bu logdan tespit edilmemiştir.

## Düzeltme

Eski kod capture.cmd'yi disposable directory içine yazıp subprocess cwd'sini aynı klasör yapıyordu. Windows current working directory handle'ı directory silinmesini engelleyebilir; Microsoft açıklaması: https://devblogs.microsoft.com/oldnewthing/20101109-00/?p=12323 . Bu mekanizma kullanıcı loguyla uyumludur; kilit sahibi sürecin kesin teşhisi iddia edilmez.

FIX03, generated _rf_capture_environment.cmd'yi kalıcı build\msvc_x64 içine yazar ve cwd=build ile çalıştırır. Böylece VsDevCmd'den kalan descendant çalışma dizini disposable rf_vs_... klasörü değildir. Temp directory yalnız private environment.txt dump'ını tutar. ExitStack helper CMD'yi success/failure/timeout yollarında siler; ardından private dump directory normal strict TemporaryDirectory cleanup ile kaldırılır. Helper ve temporary environment dosyası bırakılmadığı positive/negative mock senaryolarında kontrol edilir. Cleanup hataları ignore edilmez, broad exception yutma veya false-PASS yolu eklenmez.

Fixed CMD argument token'ları, quoted setup environment path, UTF-16 private SET capture, CL/LINK sanitization ve mevcut canlı progress/logging korunur. Yeni human entrypoint yoktur; helper build altında internal generated dosyadır. Üç production export, SURGE, core/runtime/route/vendor kodu değişmez.

## Taze kanıt ve sınır

13 güncel environment mock scenario başarılı. Exact eski FIX02 ve yeni FIX03 builder, çıkış anında cwd'yi tutan modeled Windows directory-pin ile karşılaştırıldı: eski beklenen WinError32 modeliyle reddedildi; yeni stable-CWD ile başarılı olup helper/private capture temizledi. Bu native Windows lock reproduction değil, kontrollü failure modelidir.

7 gerçek POSIX case taze çalıştı: completion öncesi output, expected14, exit7 rejection, timeout process-group termination, split UTF-8/stderr,5s heartbeat ve aynı runner ile gerçek GCC C++20 compile/link/execute. Windows CMD/VsDevCmd/MSVC/taskkill/native DLL bu düzeltme için NOT_RUN. Full C++ regression ve sanitizer rerun yok; parent FIX02 70/70 evidence historical dizine taşındı, fresh FIX03 PASS değildir.

1233 C++/header/vendor/reference/ABI/build girdisi exact parent ZIP'e byte-identical karşılaştırıldı. Source/adoption/recipe/launcher manifest verifier, iki independent tamper fixture, deterministic repackage ve fresh extraction doğrulanır. Parent SHA-256: `af5d37d05f73cf730c67a582b85c5f7fc3302c54b58ca9f540289cb2e4fcb55b`. Fresh governance master read:2026-10-19.3/v5. GPL-3.0-only, semantic_version=null, NOT_RELEASE; canonical HF19/CURRENT korunur.

## Kullanım

Yeni paketi ayrı klasöre çıkarın; REDFIELD_RF20 içinden scripts\build_orchestration\build_verify.cmd çalıştırın. Environment ready sonrası compiler yolu ve CXX1/129 satırı beklenir; bu akış Windows'ta henüz doğrulanmadı. Tekrar hata varsa son terminal satırı ve msvc_environment.log inceleme girdisidir. Üretim DLL yalnız tam başarılı Windows akışında build\msvc_x64\preloader_l.dll konumunda beklenir.
