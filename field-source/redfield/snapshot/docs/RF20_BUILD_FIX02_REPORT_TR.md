# REDFIELD RF20 BUILD FIX02 — Canlı derleme çıktısı

2026-10-20 · NON_RELEASE_WORK / NOT_RELEASE · semantic_version=null

Kullanıcı, compiler hiçbir şey derlemeden bekliyormuş gibi kaldığını bildirdi. Son beklemenin kök nedeni henüz NOT_ESTABLISHED'dır; yeni Windows çıktısı sağlanmadı. Kaynakta iki somut görünürlük kusuru doğrulandı: ilk54 core TU tamamlanıp ilk regression PASS basılana kadar ilerleme yoktu; her subprocess çıktısı completion'a kadar PIPE'ta tutuluyordu. FIX01 ortam hatasını görünür kılsa da devam eden işlemi canlı göstermiyordu.

## Davranış

FIX02 startup/OUTPUT, ENV discovery/setup, compiler yolu, source-input hashing, CXX index/total/path, LINK ve test RUN aşamalarını flush ile basar. Her süreç Popen ile gerçek PID/command olarak başlatılır; stdout+stderr log dosyasına doğrudan gider, ayrı reader yaklaşık50ms aralıkla yeni byte'ları incremental decode edip ekrana taşır. File log çalışma sürerken günceldir. Tam komut logda; console label dosyayı/hedefi gösterir. Başarı/expected diagnostic ayrımı ve exit kontrolü korunur. CMD Python -u kullanır.

Sessiz process için5 saniyede bir WAIT/geçen süre gösterilir; bu completion, derlenen dosya yüzdesi veya başarı iddiası değildir. Vswhere30s, VsDevCmd120s, compiler/linker180s, regression45s ve native host120s timeout sınırları korunur. Timeout/interrupt sahip olunan process tree'yi kapatır; POSIX process-group cleanup gerçek test edildi, Windows taskkill dalı NOT_RUN. Failure/timeout final acceptance receipt üretmez.

VsDevCmd dış stdout'u normal CMD encoding'inde canlı görünür. SET ortam dump'ı ayrı nested CMD /u ile UTF-16LE dosyaya yazılır; dump terminal/loga dökülmez ve temporary directory temizlenir. CL/_CL_/LINK/_LINK_ sanitization, fixed capture.cmd invocation, header/graph/object/linked-output hashes ve PE x64 kontrolleri korunur. Env hazır olmadan eski build.log okunmasın diye log başlangıçta sıfırlanır; environment log ayrı kalır.

Derleme sırası değiştirilmedi:54 core source,70 test ve bir unchanged GetConfig diagnostic, Windows production DLL/lifecycle/native testleri. Windows toplam129 compile eylemi; Linux125. Üretim DLL yolu build\msvc_x64\preloader_l.dll. DLL testlerden önce üretilmiş gibi gösterilmez; native/ABI/link failure gizlenmez. Yeni launcher veya test-skip yolu yoktur.

## Taze kanıt

Tam normal profil: **70/70 PASS**,125 object/71 linked-output hash ve execute guard PASS. GetConfig legacy diagnostic exit14 ayrı FAIL_OBSERVED. Build-input SHA-256: `22de5e6452de4640339d3014cb1649c450ddb42def1d4b2e7f8f9d09991afb66`.13 environment mock ve7 real process/GCC case PASS. İki tamper fixture expected rejection; ZIP CRC/manifest, deterministic repack ve final fresh extraction PASS. Windows/MSVC ve sanitizer bu FIX02 için NOT_RUN.

13 environment behavior case mock'tur; gerçek CMD/VsDevCmd çalıştırma kanıtı değildir.7 gerçek POSIX case: stdout completion'dan önce görünür; expected exit14 korunur; exit7 rejection/log; timeout group termination; split UTF-8 ve stderr;5s heartbeat; gerçek GCC C++20 compile→link→execute aynı runner üzerinden çalışır. Probe çıktısı REAL_CPP_EXECUTION_PASS. Gerçek compiler/path/object/binary hash'leri evidence'tadır. Probe, REDFIELD Windows DLL'si değildir.

Nihai tam normal70 regression sonucu metadata/ACCEPTANCE.json ve evidence/fix02_normal_receipt.json ile bağlanır. Sanitizer, Windows/MSVC/CMD/IOCP/native DLL ve oyun bu düzeltmede NOT_RUN. Eski RF20/FIX01 testleri historical dizinde tutulur ve fresh etiketlenmez.

1233 C++/header/vendor/reference/ABI/build graph girdisi exact FIX01 ZIP'e byte-identical karşılaştırılır. Yeni C++ TU, vendor/hook/public export değişimi yoktur. Full ZIP manifest/adoption/recipe/launcher verifier, independent tamper fixtures, fresh extraction ve deterministic repack tamamlanır. Parent SHA-256: `c6a9c82ae5527d655cd84b8dc11ed7ac358b3bbfa3014be7cf9028126b7b3811`. Governance2026-10-19.3/v5 fresh read yapıldı. GPL-3.0-only ve canonical HF19/CURRENT korunur. NOT_RELEASE.
