<!-- CURRENT-FULL-BUILD -->
Güncel temiz tam derleme (2026-11-05T19:38:00.051488+00:00): **296 nesne, 356 test grubu ve 19 framework terminal senaryosu PASS**. Derleyici: clang version 23.1.4 (https://github.com/llvm/llvm-project.git 7a941dfb2877a0b8cf30323fefa85ad019e0b0f5). 1017 kanonik kaynak ve 3 ek bağımlılık başlığı, toplam 1020 dosya güncel hashlerle doğrulandı; bekleyen derlenmemiş fark yok. Üretim DLL link durumu: **LINKED_NOT_RUNTIME_VALIDATED**. Link completed; DLL was not loaded or executed. Ayrıntı: `source-workspace/full-build-verification.json`, `dll-link-gate-source/current-full-link.json`, `SON-DURUM.txt`. Önceki aşama/araç kayıtları tarihseldir.
<!-- END-CURRENT-FULL-BUILD -->

# Koda nereden bakmalı?

Çalışma klasörü: `C:\Users\Skysect\Documents\Codex\2026-10-16\mcp\outputs`.

| Dosya | İçerik | Derleme durumu |
|---|---|---|
| `version-rebuild/loader.cpp` | version.dll dış yükleyicisinin okunabilir C++ yeniden yazımı | Dış DLL derlendi |
| `version-rebuild/wrappers.asm` | 197 export sarmalayıcısı | 6.556 derlenmiş komut statik eşleşti |
| `version-rebuild/version.def` | 198 export adı/ordinal | Derlenen PE ile doğrulandı |
| `version-rebuild/version.rc` | Sürüm kaynağı | Özgün 560 baytla tam eşleşti |
| `preloader-source/preloader_core.cpp` | Gömülü preloader çekirdeğinin türlendirilmiş C++ yeniden yazımı | Nesne dosyası derlendi; 152 mock kontrolü geçti |
| `anadius-source/bootstrap.cpp` | anadius64.dll başlatma/thread/çıkış akışı | Nesne dosyası derlendi; 191 mock kontrolü geçti |
| `anadius-thread-source/threads.cpp` | CRT context, thread başlatma/çıkış ve bootstrap bağlantısı | 655 sentetik kontrol; native thread/PTD/ABI sınırı açık |
| `anadius-crt-runtime-source/runtime.cpp` | Altı runtime/çözümleme gövdesi, cookie ile kodlanmış cache, policy ve RoInitialize/RoUninitialize | 3.688 kontrol; loader/PEB/provider etkileri zorunlu |
| `anadius-crt-error-source/errors.cpp` | Dört CRT error gövdesi, 45 OS hata girdisi ve PTD/global hücre seçimi | 4.151 kontrol; başlangıç errno12/DOS8 binary'den doğrulandı |
| `anadius-crt-ptd-source/ptd.cpp` | Yedi PTD/FLS edinme, sentinel ve başlatma/çıkış gövdesi | 1.510 kontrol; 064dd0 yalnız state0, gerçek FLS yaşam süresi açık |
| `anadius-crt-ptd-data-source/ptd_data.cpp` | Sekiz normal akış gövdesi; alan/ref/locale ve FLS temizliği | 206 kontrol; +0x3c0 context free yeri kanıtlandı; SEH/unwind açık |
| `anadius-crt-heap-source/heap.cpp` | Altı calloc/free/new-handler/heap başlangıç gövdesi | 1.482 kontrol; gerçek heap/lock/handler etkileri zorunlu |
| `anadius-crt-invalid-source/invalid.cpp` | Yedi invalid-parameter/context yordamı ve scoped cleanup | 1.583 kontrol; gerçek handler/Watson/SEH sınırı açık |
| `anadius-crt-glue-source/crt_glue.cpp` | Heap/PTD/data/error/invalid/runtime/thread kaynak bileşimi; altı tipli FLS API noktası | 202 ortak kontrol; bütün thread Effects kaynakları bağlı, native provider/yaşam süresi zorunlu |
| `anadius-crt-cache-source/cache.cpp` | Cookie/complement, 31 yordam cache başlangıcı, 21 modül temizliği ve sekiz encoded global | 5.555 kontrol; native entropy ve ortak global depoların bağlanması açık |
| `anadius-crt-onexit-source/onexit.cpp` | Onexit/atexit kayıt, büyüme/fallback, LIFO ve reentrant kayıt | 15.570 kontrol; recursive lock2/heap/callback etkileri zorunlu, SEH açık |
| `anadius-crt-entry-source/entry.cpp` | Dış CRT/DllMain state ve process/thread attach/detach akışı | 914 kontrol; static-UCRT normal akışı, gerçek loader/VCRT/app bağlantıları açık |
| `anadius-crt-initializers-source/initializers.cpp` | Dokuz initializer/ACRT/PTD gövdesi ve yedi metotlu dış CRT köprüsü | 904 ortak kontrol; gerçek entry/cache/onexit/PTD kaynaklarıyla rollback ve LIFO; bütün16 alt sistem bağı açık |
| `anadius-vcrt-ptd-source/vcrt_ptd.cpp` | Ayrı128B VCRT PTD ve process/thread yaşam döngüsü | 565 kontrol; static kayıt, sentinel, LastError ve rollback; native FLS açık |
| `anadius-vcrt-api-source/api.cpp` | Plain3/5 cache, resolver ve FLS/TLS/critical-section fallback | 185 kontrol; TLS fallback callback'i düşürür; gerçek loader/API açık |
| `anadius-crt-lock-source/locks.cpp` | ACRT14/VCRT1 kilit bankası ve ayrı init resolver yolları | 1.024 kontrol; EAX, signed index, ters cleanup; native storage sağlayıcısı zorunlu |
| `anadius-crt-cpu-source/cpu.cpp` | CPUID/XGETBV feature/model kararları | 10.373 kontrol; sentetik register sağlayıcıları; image başlangıç değerleri ayrıca kanıtlı |
| `anadius-vcrt-bridge-source/bridge.cpp` | CPU/VCRT yaşam döngüsünü gerçek dış CRT/ACRT/heap/API/lock kaynaklarına bağlar | 443 ortak kontrol; FLS ve TLS fallback, ayrı PTD'ler; native runtime eşdeğerliği açık |
| `anadius-crt-support-source/support.cpp` | Yedi SList/stdio-qword/TLS-slot gövdesi | 4.438 kontrol; snapshot ve free öncesi Next; native interlocked sağlayıcı zorunlu |
| `anadius-crt-environment-source/environment.cpp` | Narrow environment init/conversion/array/free ve gerçek JMP thunk | 27.951 kontrol; beş giriş, failed-publish ve kısmi cleanup; native dönüşüm/multibyte açık |
| `anadius-crt-termination-source/termination.cpp` | Cexit, managed/terminal çıkış ve durum/encoded callback akışı | 1.148 kontrol; on gövde, actual onexit core; yalnız normal akış, native process çıkışı yok |
| `anadius-crt-startup-bridge-source/startup_bridge.cpp` | Yedi dış CRT metodu ve iki terminator range'ini gerçek kaynaklara bağlar | 1.732 ortak kontrol; ortam hatası/TLS reload/LIFO/mutable terminator; tüm alt callback/OS/ABI bağları açık |
| `anadius-crt-argv-parser-source/parse.cpp` | Native komut satırı ayrıştırması, iki geçiş ve DBCS byte kuralları | 57.718 kontrol; sayım/yazım/quote/backslash sırası ve canlı bounded depolama |
| `anadius-crt-argv-source/argv.cpp` | Strict64bit tahsis ve configure modes0/1/2 | 6.777 kontrol; gerçek parser; wildcard çekirdeği ayrı kaynakta; native arama sağlayıcısı zorunlu |
| `anadius-crt-byteclass-source/byteclass.cpp` | İki locale byte sınıflandırma gövdesi ve parser adapter'i | 7.173 kontrol; düşük8/full32mask, kısa devre ve PTD bit temizliği; locale kurucusu açık |
| `anadius-crt-argv-bridge-source/argv_bridge.cpp` | Gerçek argv/heap/errno/invalid/CRT bağlantısı ve064538 cleanup | 2.181 ortak kontrol; aynı argv/PTD depoları, OOM12/invalid22 ve free/clear sırası |
| `anadius-crt-wildcard-storage-source/storage.cpp` | String/prefix append, dört slotluk geçici vektör ve kapasite büyümesi | 14.148 kontrol; extra+1, NULL copy/Watson ve free sırası |
| `anadius-crt-wildcard-search-source/search.cpp` | Wildcard path prefix, find döngüsü, dot filtreleme ve arama temizliği | 2.022 kontrol; gerçek append; native find/conversion sınırları zorunlu |
| `anadius-crt-sort-source/sort.cpp` | Genel qsort ve slot-adres karşılaştırıcısı | 4.831.748 kontrol; karşılaştırıcı18byte offline kanıtlı, retained Ghidra envanterinde giriş değil |
| `anadius-crt-wildcard-source/wildcard.cpp` | Mode2 genişletme/tek output tahsisi, mbsdec, codepage ve kaynak adapter'ları | 4.010 ortak kontrol; gerçek search/append/qsort/configure; native sağlayıcılar ve SIMD footprint açık |
| `anadius-crt-realloc-source/realloc.cpp` | Malloc ve realloc gövdeleri; sıfır/NULL, retry, hata ve korunmuş eski blok | 800 kontrol; gerçek heap/new-handler/free çekirdekleri bağlı |
| `anadius-crt-recalloc-source/recalloc.cpp` | Msize ve recalloc; eski fiziksel kapasiteden başlayan sıfırlama | 65.549 kontrol; gerçek realloc/heap kaynakları, native Heap API açık |
| `anadius-crt-recalloc-bridge-source/bridge.cpp` | Wildcard ve onexit tahsislerini aynı heap state üzerinden gerçek recalloc'a bağlar | 5.483 ortak kontrol; argv mode2, yerinde/taşınan büyüme, OOM ve LIFO |
| `anadius-crt-conversion-source/conversion.cpp` | ANSI↔UTF16 tampon kapasitesi, sahiplik ve hata akışı | 5.370 kontrol; 06004c/06e430; özgün obje ABI'si değildir |
| `anadius-crt-conversion-api-source/api.cpp` | Kod sayfası bayrakları, varsayılan karakter pointer'ları ve AreFileApisANSI çözümleme | 3.935.369 kontrol; 06ccc8/06cd58/066c1c, gerçek resolver slot0/candidate16 |
| `anadius-crt-conversion-bridge-source/bridge.cpp` | Dönüşüm/API/heap/errno/resolver kaynaklarını wildcard mode2'ye bağlar | 7.440 ortak kontrol; fallback/PTD hata hücreleri, native conversion/find/locale açık |
| `anadius-crt-module-path-source/module_path.cpp` | Sabit UTF16→dar tampon ve modül dosya adı | 3.469 kontrol; 0601c8/06ecdc, 34 hata yolu ve geniş261/dar260 |
| `anadius-crt-find-native-source/find_native.cpp` | Üç tipli SDK find çağrısı, canlı WIN32_FIND_DATAW depoları | 106.229 kontrol; dört bayt BOOL, 64bit handle, filename offset44/520 bayt; API fixture |
| `anadius-crt-find-bridge-source/bridge.cpp` | Find/GetModuleFileNameW SDK bağını gerçek argv/path/conversion kaynaklarına bağlar | 8.664 ortak kontrol; tipli264 WCHAR scratch, gerçek native find/path API çağrılmadı |
| `anadius-crt-locale-update-source/locale_update.cpp` | LocaleUpdate kurucusu ve iki yenileme yardımcısı | 399.644 kontrol; 058b04/064ef0/064f5c, fast path ve nested bit2 sahipliği |
| `anadius-crt-locale-thread-source/locale_thread.cpp` | Locale/multibyte PTD güncellemesi ve refs/lock sırası | 15.204 kontrol; 06f540/06f658/070968/0709d8; derin ref/free sağlayıcıları zorunlu |
| `anadius-crt-locale-bridge-source/bridge.cpp` | Gerçek LocaleUpdate/thread/PTD/heap kaynaklarını argv/find/path akışına bağlar | 8.553 ortak kontrol; aynı canlı PTD alanları, nested cleanup ve gerçek heap-free; native FLS/ref/locale kurucuları açık |
| `anadius-crt-locale-ref-source/locale_ref.cpp` | Locale ve LC_TIME alt referans sayaçları | 6.212 kontrol; 070634/0708c0/070838/070898, altı kategori ve modulo32 |
| `anadius-crt-locale-free-source/locale_free.cpp` | Locale/lconv/ctype/time/kategori derin temizleme | 17.297 kontrol; altı gövde, gerçek nested yardımcılar ve 87 LC_TIME pointer'ı |
| `anadius-crt-locale-resource-bridge-source/bridge.cpp` | Gerçek ref/free gövdelerini PTD/locale/argv ve heap kaynaklarına bağlar | 9.280 ortak kontrol; 120'den fazla tahsisli locale ağacı tamamen bırakıldı; native lock/atomic/FLS açık |
| `anadius-crt-multibyte-support-source/multibyte_support.cpp` | SystemCP, 257/256 tablo yayını ve atomik locale-changed işareti | 3.544 kontrol; -4 locale_info+c assembly ile doğrulandı |
| `anadius-crt-multibyte-control-source/multibyte_control.cpp` | Codepage değiştirme ve ilk başlatma | 128.916 kontrol; 0x228 snapshot, tam EAX ve AL dönüşü; builder ayrı arayüz; record köprüsünde gerçek kaynak bağlı |
| `anadius-crt-multibyte-bridge-source/bridge.cpp` | Gerçek multibyte/locale/PTD/heap kaynaklarını argv initializer'a bağlar | 10.377 ortak kontrol; dinamik ref/free, 513 tablo baytı, başarısız init; native/builder sağlayıcıları zorunlu |
| `anadius-crt-multibyte-tables-source/multibyte_tables.cpp` | SBCS sıfırlama ve case/type tablo üretimi | 54.392 kontrol; iki gövde; canlı static copy, CPInfo, tanımlı çıktı hücreleri |
| `anadius-crt-multibyte-builder-source/multibyte_builder.cpp` | Gerçek 06f674 kayıt üreticisi ve beş binary DBCS kaydı | 885.150 sınır/akış kontrolü; 240 ham byte ve UTF8 kısmi koruma; native sağlayıcılar açık |
| `anadius-crt-multibyte-record-bridge-source/bridge.cpp` | Gerçek builder/SBCS/case kaynaklarını multibyte/locale/PTD/heap/argv zincirine bağlar | 12.032 ortak kontrol; önceki sentetik builder kaldırıldı; alt06e210/06cc30 yeni NLS köprüsünde gerçek kaynaklara bağlı |
| `anadius-crt-string-type-source/string_type.cpp` | 06e210 dar girişten sınıflandırma, geçici wide tampon ve locale temizliği | 70.165 kontrol; 504/505 stack/heap, marker, sorgu/doldurma ve tam BOOL |
| `anadius-crt-lcmap-source/lcmap.cpp` | 06c8fc/06cc30 dönüşüm ve sort-key; 063168 bounded-count | 4.892 kontrol; EAX dönüşü ve source[count] okuması assembly ile doğrulandı |
| `anadius-crt-nls-bridge-source/bridge.cpp` | Gerçek NLS/locale/CAPI/heap kaynaklarını record/case/PTD/argv zincirine bağlar | 27.665 ortak kontrol; tracked çıktı, endbyte sağlayıcısı; 0671d4 sonraki dispatch köprüsünde gerçek kaynakta |
| `anadius-crt-locale-names-source/locale_names.cpp` | 07310c/0736d4; 228 özgün ad/indeks ve LCID satırı | 1.181.823 kontrol; ham veriye karşı bağımsız3.965 kontrol; kok/kok-in özgün arama kusuru korunur |
| `anadius-crt-nls-dispatch-source/nls_dispatch.cpp` | 0671d4/0672c0 modern API ve eski W/statik fallback seçimi | 12.581 kontrol; {3,16}, Ex9/W6 argüman, tam32-bit dönüş |
| `anadius-crt-nls-dispatch-bridge-source/bridge.cpp` | Gerçek dispatch/locale tablosu/shared resolver'ı NLS/record/PTD/argv zincirine bağlar | 27.943 ortak kontrol; cache,3 API yolu ve nested callback; native uçlar açık |
| `anadius-crt-nls-native-source/native_api.cpp` | CP/conversion/GetStringTypeW için yeni SDK adapter'ı | 143 kontrol; gerçek API + WINAPI fixture; tracked byte/WORD ve LastError |
| `anadius-crt-nls-dispatch-native-source/native.cpp` | Ex/W/LCID için yeni SDK adapter'ı | 92 kontrol; sort-key byte kapasitesi, query/failure/NULL ve LastError |
| `anadius-crt-nls-native-bridge-source/bridge.cpp` | Kurtarılmış NLS/resolver/record zincirini gerçek SDK çağrılarına bağlar | 2.114 kontrol; sentetik UTF-8, eski W/statik LCID yolu, CP932 ve504/505 tahsis sınırı; heap/FLS/global sağlayıcıları kontrollü |
| `anadius-crt-heap-native-source/native_api.cpp` | Altı tipli Windows heap API bağlantısı | 169 kontrol; tam SIZE_T/BOOL/flags, gerçek sahipli küçük bloklar ve edge fixture'ları |
| `anadius-crt-heap-native-bridge-source/bridge.cpp` | Kurtarılmış heap/malloc/realloc/recalloc/errno/NLS zincirini gerçek heap'e bağlar | 2.193 kontrol; byte-array yaşamı, retry/veri koruma/PTD errno ve504/505 NLS; FLS/global/ABI/SEH açık |
| `anadius-crt-fls-native-source/fls.cpp` | Tek domain için gerçek FLS/PTD/heap callback bağlantısı | 127 kontrol +3 terminal; release başarısızlığında ayrı native sahiplik, retry ve joined worker çıkış temizliği; global/locale/parallel/ABI/SEH açık |
| `anadius-crt-lock-native-source/native.cpp` | Sahipli14 ACRT/1 VCRT SDK kritik bölümü | 145 kontrol +1 terminal; tam BOOL, guard, recursive/contended kullanım; explicit lifecycle |
| `anadius-crt-atomic-native-source/atomic.cpp` | Sahipli hizalı LONG/Interlocked add/exchange/load | 88 kontrol;8worker sayaç, tam32-bit/wrap, özgün record ABI'si değil |
| `anadius-crt-sync-bridge-source/bridge.cpp` | Gerçek lock/resolver/Interlocked/FLS/PTD/heap kaynak bileşimi | 120 kontrol;4eşzamanlı worker, modern/fallback, errno/ref/callback ve partial-bank rollback; global/ABI/SEH açık |
| `anadius-crt-counter-native-source/counters.cpp` | Inline/pointed gerçek LONG counter registry ve Interlocked | 94 kontrol; typed borrow, registry-geneli quiescent bind/unbind,4worker |
| `anadius-crt-locale-native-source/native.cpp` | Gerçek locale ref/deep-free/resource/global-thread replacement kaynaklarını native zincire bağlar | 750 kontrol;133blok/18counter,4worker, sonref deepfree; özgün başlangıç global/tip/ABI/SEH açık |
| `anadius-crt-initial-locale-source/initial_locale_data.cpp` | Özgün ilk C locale/lconv/time/ctype/case/isim grafiği | 96bölge,116DIR64bağlantısı,3546bayt;94raw+2PEzero-fill |
| `anadius-crt-initial-multibyte-source/initial_mb.cpp` | İlk552bayt MB kaydı,257/256bayt tablolar ve imaj global değerleri | 2908fixture kontrolü;1066bağımsız derlenmiş-veri/PE kontrolü;maskFFFFFFFE |
| `anadius-crt-initial-state-source/initial_state.cpp` | Kendi belleğinde taşınmış başlangıç grafiği ve3gerçek LONG | 2866kontrol,116pointer,4worker gerçek locale refs;CRT initializer çalıştırmaz |
| `anadius-crt-locale-startup-source/locale_startup.cpp` | İlk locale ataması064468 ve kapanış065104/065338 | 3gövde;117kontrol;direct MOV,noextraretain,4worker nativeFLS/PTD/heap;SEH/tümCRT açık |
| `anadius-crt-environment-cleanup-source/environment_cleanup.cpp` | Dar/geniş environment kapanışının dört gövdesi | 447 kontrol;25 alias durumu,güncel sonraki okumalar,gerçek bounded free core;native wide ABI açık |
| `anadius-crt-lifecycle-callbacks-source/callbacks.cpp` | Onexit iki tablo başlangıcı06447c ve MB kapanışı0644f8 | 105 kontrol;ortak normal/quick tablolar,native heap/recalloc/counter,refwrap ve free hata dalı |
| `anadius-crt-pair-leaves-source/pair_leaves.cpp` | Heap/VCRT kapanışı, başarı leaf ve komut satırı başlangıcı |4 yeni gövde;140 kontrol;iki eksik leaf için yeni REA18bayt;ortak heap/VCRT/argv durumu |
| `anadius-crt-lowio-lifecycle-source/lowio_lifecycle.cpp` | Lowio başlatma/kapanış ve handle-array destructor |3 yeni gövde;639 kontrol;128slot,64CS/array;192 gerçek CS silme/3heaparray;alt sağlayıcılar zorunlu |
| `anadius-crt-pair-bindings-source/bindings.cpp` | 16 ACRT pair için gerçek kaynak callback dispatcher bağlantısı |1088 kontrol;16pair/20ID PE+derlenmiş tablo eşleşmesi;rollback/reverse/mutable/AL/low8/alias;0yeni özgün gövde |
| `anadius-thread-native-source/win32_effects.cpp` | Yedi SDK API çağrı noktası ile gerçek kurtarılmış runtime/error bileşimi | 367 kontrol; native API'ler tipli fixture; CRT heap/PTD/FLS açık |
| `anadius-bootstrap-native-source/native_entries.cpp` | Yeniden derlenen thread/exit/terminate/application giriş adresleri | 70 normal +4 terminal senaryo; CRT kayıt/loader/export bağı açık |
| `anadius-host-init-source/host_init.cpp` | Registry değerleri ve 6–10 hook için başlangıç kararları | 6.013 sentetik kontrol; native callback/trampoline ABI açık; motor adapter'i ayrı |
| `anadius-registry-source/registry.cpp` | Beş registry callback'i, değer/kapasite/kopyalama ve handle takibi | 991 sentetik kontrol; native SDK köprüsü ayrı kaynakta, gerçek CRT politikaları açık |
| `anadius-registry-source/abi_cells.hpp` | SDK DWORD/HKEY belleğini alias ihlali olmadan anında okur/yazar | Registry toplam991 kontrol; native pointer kimliği korunur |
| `anadius-native-source/native_callbacks.cpp` ve `original_effects.cpp` | On SDK imzalı host girişi, tipli original trampoline çağrıları ve resolver | 91 kontrol +3 terminal policy; gerçek kaynak host-init bağlantısı dahil |
| `preloader-native-source/native_shims.cpp` | Üç preloader callback'i ve application DllMain için SDK köprüsü | 51 kontrol +8 terminal policy; export/CRT/NativeHost hâlâ açık |
| `anadius-host-callbacks-source/callbacks.cpp` | İki process ve iki klasör callback'i | 192 kontrol; host başlangıcı ve ortak map yardımcılarıyla bağlantı dahil |
| `anadius-network-hook-source/connect.cpp` | Connect port kararı, yönlendirme, sayaç ve bekleme bağlantısı | Cleanup ile toplam 324 sentetik kontrol; socket açılmadı |
| `anadius-network-hook-source/hook_cleanup.cpp` | Disable listesi ve hook motorunu kapatma sırası | Seçilen motor adapter'i ayrı kaynakta; liste sırası/snapshot korundu |
| `hook-engine-source/vendor/minhook/` | Sabit commit'li resmî hook/buffer/trampoline/HDE kaynağı ve lisansı | Uyumluluk bağımlılığı; özgün kaynak/sürüm diye sunulmaz; 379 fake-Win32 kontrolü |
| `hook-engine-source/adapters.cpp` ve `preloader_engine.cpp` | Host-init/cleanup ve preloader için C++ motor bağlantıları | 472 ortak kontrol; NativeApi gerçek motora linklenir, native hook metotları testte çağrılmaz |
| `anadius-online-source/online.cpp` | Online yükleyici, sürüm denetimi, profil cache ve temizleme | 12.869 kontrol; online provider DLL iç kodu açık |
| `anadius-online-source/recovered_effects.cpp` | Online çekirdeği mevcut dosya/status/path/encoding/library kaynaklarına bağlar | Sentetik backend'lerle ortak doğrulama; gerçek DLL yüklenmedi |
| `anadius-config-source/configuration.cpp` | Config2 ayar uygulaması ve iki geçişli başlangıç | 46 anahtar; 150 sentetik kontrol geçti |
| `anadius-execution-source/execution.cpp` | Başlangıç bayrakları, mutex ve QPC/Sleep beklemesi | 10.201 sentetik kontrol geçti |
| `anadius-libraries-source/libraries.cpp` | Ek DLL listeleri ve main/worker yükleme fazları | 157 mock kontrolü geçti |
| `anadius-startup-source/startup.cpp` | Argüman/ortam seçimi, dil ve konsol başlangıcı | 155 sentetik kontrol geçti |
| anadius-socket-source/queued_write.cpp |02a500 kuyruktan gönderim başlatma ve025a70 next-write kaynak bağlantısı |1004 çekirdek +1597 entegrasyon kontrolü; özgün yordam1 |
| anadius-socket-source/write_finish.cpp |025a70 gönderim sonlandırma ve kuyruk/hata kaydı akışı |2493 çekirdek +639 entegrasyon kontrolü; özgün yordam1 |
| anadius-socket-source/environment_constructor.cpp |023dd0 ortam kurucusu ve gönderim kaynağına bağlantı |544 çekirdek +626 entegrasyon kontrolü; özgün yordam1 |
| anadius-socket-source/composed_write.cpp | Beş normal composed-write/async-send/allocation/publication yordamı ve kaynak bridge |3528 çekirdek kontrol; özgün yordam5 |
| anadius-socket-source/composed_write_native.cpp | SDK WSASend ve WSAGetLastError typed adapteri |593 SDK/entegrasyon kontrolü; adapter özgün yordam sayısına eklenmez |
| anadius-socket-source/handler_dispatch.cpp | Dokuz normal borrowed/owned scheduling,closure,invoke ve cache yordamı |3268 çekirdek kontrol; özgün yordam9 |
| anadius-socket-source/handler_dispatch_bridge.cpp | Önceki IO completion scheduling sağlayıcılarını kaynakla bağlar |242 entegrasyon kontrolü; özgün yordam sayısına eklenmez |
| anadius-socket-source/io_completion.cpp | Sekiz normal receive/send completion,environment move,reference cleanup,invoke ve storage release yordamı |12382 çekirdek kontrol; özgün yordam8 |
| anadius-socket-source/io_completion_dispatch.cpp | Posted router üzerinden IO callback bağlantısı |128 entegrasyon kontrolü; özgün yordam sayısına eklenmez |
| anadius-socket-source/posted_operation.cpp | Altı normal executor/posted callback/allocator/cleanup yordamı |4336 çekirdek kontrol; özgün yordam6 |
| anadius-socket-source/posted_operation_dispatch.cpp | Posted callback typed dispatcher bağlantısı |110 entegrasyon kontrolü; özgün yordam sayısına eklenmez |
| `anadius-socket-source/completion_dispatcher.cpp` |027c10 completion dispatcher,aynı kuyruk ve worker köprüsü |1970 kontrol; recursive requeue/GQCS/metadata/CAS/stop/terminal exception; özgün yordam1 |
| `anadius-socket-source/completion_dispatcher_native.cpp` |Set/GetLastError,GQCS,PostQueuedCompletionStatus,SetWaitableTimer SDK adapteri |845 SDK/entegrasyon kontrolü; tüm importlar enjekte |
| `anadius-socket-source/worker_run.cpp` |026320/029890 slotlar,0298b0 worker döngüsü,02a190 stop ve kaynak köprüsü |795 kontrol; TLS/counter/live cleanup; yeni özgün yordam4 |
| `anadius-socket-source/worker_run_native.cpp` |TlsGetValue/TlsSetValue/PostQueuedCompletionStatus/GetLastError SDK adapteri |577 SDK/entegrasyon kontrolü; tüm importlar enjekte |
| `anadius-socket-source/worker_startup.cpp` |02a000 startup/02a4a0 entry ve mevcut05e8e0 kaynak köprüsü |616 kontrol; hata/cache/live cleanup; yeni özgün yordam2 |
| `anadius-socket-source/worker_startup_native.cpp` |CreateEventW/GetLastError/CloseHandle/WaitForSingleObject/SetEvent/SleepEx SDK adapteri |821 SDK/entegrasyon kontrolü; tüm importlar enjekte |
| `anadius-socket-source/iocp_constructor.cpp` |024990 version/gate/IOCP/worker constructor ve registry bridge |1872 kontrol; kısmi hata/live cleanup/counter; özgün yordam1 |
| `anadius-socket-source/iocp_constructor_native.cpp` |Typed SDK version mask/verify/port/last-error/close adapteri |527 SDK/entegrasyon kontrolü; bütün SDK importları enjekte |
| `anadius-socket-source/scheduler_registry.cpp` |022fd0 sabit scheduler key,023b30 resolver ve SchedulerConstructionEffects |244 çekirdek + 838 entegrasyon; özgün yordam2;024990 sağlayıcı |
| `anadius-socket-source/service_constructor.cpp` |0228e0 constructor,027be0 gate helper ve ConstructorRegistryEffects |339 kontrol; store/effect sırası ve gerçek kaynak bağı; özgün yordam2 |
| `anadius-socket-source/service_constructor_native.cpp` |Enjekte SDK InitializeCriticalSectionAndSpinCount/GetLastError adapteri |212 SDK/entegrasyon kontrolü; native CS init/ağ/IOCP çalıştırılmadı |
| `anadius-socket-source/service_registry.cpp` |022fa0 sabit anahtar,0239c0 çift aramalı registry ve RegistryLifecycleEffects |454 çekirdek + 203 entegrasyon; özgün normal yordam2; constructor/CRT compare sağlayıcı |
| `anadius-socket-source/recycle.cpp` |0283c0 özgün bucket/list/free-pool recycler ve iptal adapteri |5564 kontrol; sıralı splice, kısmi failure, sayaç sarılması; özgün yordam1 |
| `anadius-socket-source/recycle_storage.cpp` |OwnedRecycleMap seçilmiş host depolama/insert/reuse |10400 kontrol; gerçek iptal zinciri; özgün constructor/allocator değildir |
| `anadius-socket-source/cancellation.cpp` |026aa0 dört map iptali ve028850 wake; QueueCancellationEffects |1937 kontrol; FIFO/fallback/live port/queued/wake; normal yordam2,0283c0 açık |
| `anadius-socket-source/cancellation_native.cpp` |Typed SDK PostQueuedCompletionStatus adapter |107 kontrol; gerçek SDK OVERLAPPED üyesi, enjekte post ve kaynak zinciri; native ASIO layout/dispatcher açık |
| `anadius-socket-source/close.cpp` |026e50 durum bilgili kapatma; mevcut SDK adapter |221 kontrol; rawstatus/kategori/state/linger/retry; normal yordam1 |
| `anadius-socket-source/closing.cpp` |026fb0 cancel/close/reset ve ClosingLifecycleEffects bağı |143 kontrol; engineCAS0/0, canlı socket, reset-before-release; normal yordam1,026aa0 açık |
| `anadius-socket-source/lifecycle.cpp` |0211b0/0237f0/024db0 normal akışları; LifecycleRuntime→acceptor bağı |244 kontrol; liste/sahiplik/callback sırası; özgün yordam3, native ASIO/allocator açık |
| `anadius-socket-source/lifecycle_native.cpp` |Borrowed SDK CRITICAL_SECTION gate adapter |165 kontrol; enjekte importlar, constructor API çağırmaz; init/delete caller sorumluluğu |
| `anadius-socket-source/acceptor.cpp` |020e20 acceptor kurulumu ve025310 kapatma; kısmi AcceptorBackend |364 kontrol; category/LastError sırası, zero-error yolları, cached service/live port ve close retry; yeni özgün yordam2 |
| `anadius-socket-source/acceptor_native.cpp` |Typed SDK WSASocket/IOCP/options/bind/listen/close/ioctl adapter |308 kontrol; enjekte importlar, gerçek ağ/IOCP yok; ASIO/lifetime/SEH açık |
| `anadius-socket-source/operations.cpp` |028690 adres ve02a200 senkron gönderim normal akışları; kısmi Server/Transport bağı |376 kontrol; category→LastError sırası, broadcast10022, signed DWORD, select0 retry; yeni özgün yordam2 |
| `anadius-socket-source/native.cpp` |SDK türleriyle enjekte/default Windows API adapterı |308 kontrol; gerçek WSABUF pointerı, sockaddr/flags/nfds, mandatory kategori sağlayıcısı; testte gerçek ağ yok |
| `anadius-session-source/handler.cpp` |Session Handler→gerçek mesaj reset/greeting/process bağlantısı |19 kontrol; iki mantıksal reset/canlı version/NUL/owned input; yeni özgün yordam0 |
| `anadius-session-source/routing.cpp` |Mesaj Effects→gerçek Router ve Session zinciri |1154 kontrol; gerçek XML/AES/challenge/handler/event kaynakları; bellek içi transport/scheduler, native socket/ASIO açık |
| `anadius-event-queue-source/queue.cpp` |02c620 mutex ve02c400 lazy queue getter normal akışları |2054 kontrol; signed epoch/header/recheck,flags2/seedcopy,atexit/footer; alt CRT/deque zorunlu sınır |
| `anadius-event-queue-source/adapters.cpp` |Achievement/IGO/message için ortak Context/FIFO bağlantısı |133 kontrol; cold DOWN olayları, cleanup kayıt sırası, tek olay tüketimi; OwnedStorage seçilmiş uyumluluk backend |
| `anadius-achievement-runtime-source/utc.cpp` |041b60 güncel UTC zamanı normal akışı |76 kontrol; copied tm→recomputed epoch→format sırası; host clock/time_put compatibility sınırı |
| `anadius-achievement-runtime-source/runtime.cpp` |Grant030010→gerçek XML/file/INI kaynak bağlantısı |163 kontrol; ortak State/Paths, event/XML/INI sırası, yeniden okuma; native globals/ABI açık |
| `anadius-calendar-source/calendar.cpp` | 041ff0 saf takvim / epoch hesabı |31.760 kontrol; wrap32, signed truncation, bağımsız instruction oracle; OS time çağrısı yok |
| `anadius-timestamp-source/timestamp.cpp` | 041e10 wrapper, ClassicIso ve achievement StorageEffects bağlantısı |192 kontrol; klasik ASCII sabit biçim, EOF önekleri, Grant→epoch→INI; genel locale/native ABI açık |
| `anadius-achievement-writer-source/writer.cpp` | 01f9a0 Watcher INI yazımı |36kontrol;SteamIdsections,CRLF,signedcount/progress,unsignedepoch;gerçekpath/file kaynakları |
| `anadius-achievement-storage-source/storage.cpp` | 01eb00 achievement XML import ve INI başlangıç kararı |116kontrol;Count/Progress %d/%x,default/Name/Grant/statusgate;startup persistence bağlantısı;epoch için ClassicIso/takvim kaynağı bağlanabilir; OS/nativeABI açık |
| `anadius-config-startup-source/startup.cpp` | Bootstrap argüman/konsol/configuration kaynaklarını gerçek file/hash yükleyici ve ortak profil devamına bağlar |335 kontrol;ConfigurationPass overload;paylaşılanState,iki geçiş,profil/avatar/ortam;native/global/CRT ABI açık |
| `anadius-config-hash-source/hash_table.cpp` | Büyüme, kapasite ve recursive konumsal kayıt kopyası |4929 kontrol;8anahtar sınırı yok;65satır parse512/rootcopy128/descendant512 |
| `anadius-config-application-source/application.cpp` | Mutating lookup ve son koleksiyon sırasıyla gerçek Config2 yükleme/uygulama girişi |95 kontrol;6bölüm/recordreuse;partialerror/callback;genelstartupcaller henüz yönlendirilmedi |
| `anadius-config-order-source/order.cpp` | İlk seçilen Config2 kopyasının sınırlı hash/list sırası |2185 kontrol;container başına en çok8 farklı anahtar;growth ve son section-copy sırası açık |
| `anadius-config-pipeline-source/pipeline.cpp` | Gerçek dosya/parser/ilk kopya ve scalar uygulama bağlantısı |189 kontrol;kendi fixture dosyası;base/override;5 sıralı koleksiyon tüketimi açıkça reddedilir |
| `anadius-parser-source/parser.cpp` | Ham Config2 grammar ve action çekirdeği | 115 parser kontrolü;yeni hash/application kaynakları growth ve son tüketim sırasını normal akışta bağlar;native ABI açık |
| `anadius-server-source/server.cpp` | Sunucu/oturum, NUL framing ve yazma kuyruğu | Mock ASIO/transport ile doğrulandı |
| `anadius-messages-source/messages.cpp` | İlk mesaj, challenge, 39 girişli dispatcher ve cipher framing | Mock XML/crypto ile doğrulandı |
| `anadius-handlers-source/handlers.cpp` | Yedi yapılandırma/profil/ayar/oyun bilgisi yanıtı | 628 sentetik kontrol geçti |
| `anadius-crypto-source/crypto.cpp` | Somut AES-128 ECB/CBC, PKCS#7, challenge/anahtar üreteci | 135.410 deterministik kontrol geçti |
| `anadius-xml-source/memory_printer.cpp` | LSX bellek XML yazıcısı ve entity kaçışları | 277 kontrol; okuyucu ayrı bağımlılık |
| `anadius-misc-handlers-source/misc_handlers.cpp` | 18 ayrı yanıt gövdesi / 24 komut kaydı | 288 sentetik kontrol geçti |
| `anadius-content-handlers-source/content_handlers.cpp` | İçerik, entitlement, achievement, avatar yanıtları | 1.023 sentetik kontrol geçti |
| `anadius-license-source/license.cpp` | Üç lisans yanıt gövdesi, token deneme/bloklama ve ticket akışı | 992 sentetik kontrol; somut dosya/hash/token bağlantısı ayrıca file-source içinde |
| `anadius-xml-reader-source/reader.cpp` | Gerçek XML okuma uyumluluk bağlayıcısı | 241 kontrol; upstream tinyxml2 kaynaklı, özgün sürüm eşitliği kanıtlanmadı |
| `anadius-dispatch-source/router.cpp` | Mesaj katmanını 33 kayıtlı kaynak gövdesine bağlayan entegrasyon | Gerçek okuyucu/AES/yazıcı ve lisans gövdeleriyle 58 ortak kontrol |
| `anadius-hash-source/hash.cpp` | Streaming SHA-1 ve dosya karşılaştırma döngüsü | 6.899 deterministik kontrol |
| `anadius-token-source/token.cpp` | AES-CBC ile token dosyası çözme ve ham etiket araması | 579 sentetik kontrol |
| `anadius-file-source/binary_io.cpp` | Tüm-dosya okuma/yazma gövdeleri | 78 I/O kontrolü; native backend yalnız üretilen fixture'larda çalıştırıldı |
| `anadius-file-source/license_files.cpp` | Lisans gövdelerini gerçek I/O, SHA-1 ve CBC'ye bağlar | 33 ortak kontrol; status/path bağlantısı ayrıca windows-source içinde |
| `anadius-status-source/status.cpp` | Dosya tipi/permission/error ve flags3 Win32 yolu | 9.307 kontrol; native testler yalnız üretilen dosyalarda |
| `anadius-path-source/paths.cpp` | Windows root/filename/parent/append/join kuralları | 328 deterministik kontrol |
| `anadius-encoding-source/encoding.cpp` | Filesystem codepage, UTF dönüşümleri ve hata akışı | 57 kontrol; Win32 sentetik string dönüşümleri |
| `anadius-windows-source/windows_effects.cpp` | Status/path/encoding'i dosya/lisans katmanına bağlar | 23 ortak kontrol; kalan uygulama/provider/ABI sınırları açık |
| `source-workspace/build.ps1` | C/C++ modülleri için ortak derleme/test girişi | 177 nesne (173 C++ / 4 C), 2 kısmi + 1 uyumluluk arşivi, 167 test grubu +19 terminal senaryo |
| `anadius-all-functions.pseudo.c` | 2.076 yerel fonksiyonun adresli decompiler dökümü | Derlenebilir C++ değildir |
| `anadius-all-functions.asm.txt` | Aynı fonksiyonların assembly kanıtı | Kaynak rekonstrüksiyonu için başvuru |

Kaynak klasörlerindeki README dosyaları derleme komutlarını, hangi özgün adreslerin yeniden yazıldığını ve eksik bağımlılıkları açıklar. `*.hpp` içindeki soyut arayüzler tamamlanmamış sistem/alt sistem sınırlarıdır; boş başarı döndüren üretim uygulamaları değildir.

**İki DLL'nin tümü bağımsız C++ kaynağına dönüştürülmüş değildir.** Derlenen version dış katmanında gömülü PE hâlâ özgün makine kodudur. `preloader-source` ve `anadius-source` tek başına oyun klasörüne konulacak DLL üretmez. Derleme ve mock test başarısı, özgün binary ile tam davranış eşdeğerliğini kanıtlamaz. Özgün/yeniden derlenen DLL'ler ve verilen oyun EXE'si çalıştırılmadı.

`host-compatibility/README.md`, son verilen Madden27.fixed.exe'nin DLL import bağlantısını; `CFG-README.md` ayarların parser/global adreslerini gösterir. Kullanıcı kimlikleri gizlenmiştir; ham CFG pakete dahil değildir. `mac-symbol-comparison` Mac sembollerini, `anadius-type-recovery` Windows RTTI envanterini içerir.

Güncel teknik durum ve kalan işler `DLL-ANALIZ-RAPORU.md` ve `KAYNAK-DURUMU.md`; kaynak/binary statik doğrulamalar `version-build-verification` içindedir. `dll-reverse-engineering.zip`, bu teslimlerin paketidir. Derleyici araç zinciri, büyük oyun EXE'si ve özgün DLL dosyaları pakete dahil değildir; derleme için README'deki araç zinciri yolunu kendi kurulumuna göre belirtmek gerekir.

İki DLL'nin toplam tamamlanması için bu aşamadaki kaba tahmin yaklaşık %80'dir. Ölçüm yöntemi/sınırlar ve önceki %40 tahmini `TAMAMLANMA-TAHMINI.md` içinde açıklanır; test sayısı tamamlanma oranı değildir.





