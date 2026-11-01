# FIX31 — unload stack taramasında kalan region bütçesi

NON_RELEASE tam kaynak devam paketi. Exact kaynak parent FIX30; REVIEW31 ayrı rapor/evidence devamıdır, yeni source parent değildir. Karşılaştırma tam RE paketidir; özgün DLL binary eşdeğerliği iddiası yoktur.

REVIEW31de görülen hata: while(scanned<64MiB) yalnız önceki bölgelerin toplamını sınırlıyordu; son region bütçeyi aşsa da tümü okunabiliyordu. Bu kendi unload entegrasyonumuzun kontrol akışı sorunudur.64MiB seçimi kaynak/upstream zorunluluğu veya ayrı kullanıcı sayı onayı değildir. Kayıtsız eski motivasyon uydurulmaz.

Exact REVIEW31 patchi uygulandı: limit<=cursor denetiminden sonra, herhangi bir read öncesinde limit-cursor>64MiB-scanned ise false. Başka runtime satırı değişmedi. Aynı budget, whole-region refusal; sığmayan bölge kısmen taranıp safe sayılmaz. Öncekine göre daha erken unload reddi mümkündür. Modül pin/lease ve trampoline sahibi korunur. Freeze/MinHook, driver/thread enumeration, callback/CRT, config/profil/avatar, persistence, vendor ve tüm retry değerleri aynı.

Yeni testte scanner yeniden yazılmadı: actual unload_guard.cpp tek test TUya include edilir. Linux declaration/API/SEH-syntax shimleri gerçek Windows değildir. Linker-wrapped memcpy actual body read sayısını gözler; glibc fortified inlineın sayacı gizlememesi için yalnız test TUda fortify inline ve memcpy builtin devre dışıdır. Shipping hardening flagleri değişmez. MSVC kolu gerçek VirtualAlloc/VirtualQuery scanner fixtureı sağlar, burada NOT_RUN.

150 kontrol: küçük allocation success;16 register/module word rejection; query/guard/noaccess/uncommitted rejection; ilk region ve kalan8 byte üstündeki regionun read öncesi reddi; tam bütçede conservative false; son izinli8 byte; thread exclusion; open/suspend/context/resume/enumeration failure ve handle cleanup sırası. Parent body/mutants aynı test driverını düşürmelidir. SDK layout/SEH/access violation ve gerçek native suspension bu modele dahil değildir.

Unload/timeout incelemesi: NativeHooks::quiesce(timeout) yalnız active_calls pollingini deadline ile kontrol eder; ardından external thread scan ve MH_Uninitialize deadline almaz. Destructor5s, bütün destructor5s garantisi değildir. Worker100ms quiesce denemeleri +10ms retry sınırsız tekrar edebilir; failureda1000ms sleep modu pinli tutar. DllShutdown timeout tüm export süresinin ölçülmüş boundu sayılmaz. Bu tur yeni total deadline/cancellation politikası eklenmedi. Thread snapshot/yeniden giriş/SEH ve shutdown callerının module reference sözleşmesi native test gerektirir. D13 ve D21 bu yüzden bütünüyle kapanmaz.

Güncel review: DEP05 karma filesystem sahiplik incelemesi REVIEW31de kapandı; evrensel gulrak göçü/IO eşdeğerliği değildir.6 açık: D02 worker/order; D06 broad lock/reentry; D07 unknown-tag fallback; D13 native quiescence; D14 exit/CRT; D21 numeric policy/whole deadline. D07 güncel metni tarihsel FIX14 özetinden ayrıldı; storage/license yanında OnlineSession cookie/profile callerı da vardır. Bu rapor düzeltmesi runtime status politikasını değiştirmez.

Build/QA final sonuçları exact ZIPye bağlı dış FINAL_QA ile verilir. Windows/MSVC/DLL/native SDK/SEH/IOCP/provider/CRT/game/actual DLL Golden ve full-core/LSan NOT_RUN. Bu alt hatanın portable PASSı tüm unload güvenliği veya altı ana inceleme kapanışı değildir.
