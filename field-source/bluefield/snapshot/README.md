# preloader_l — BLUEFIELD FIX31

Tam C++20 x64 Windows DLL kaynak paketi; NON_RELEASE. Teknik çıktı preloader_l.dll, görünür kimlik SURGE; DllMain@1 / SURGE@2 / DllShutdown@3 korunur. Kaynak tabanı kullanıcının tam RE paketidir. BROWNFIELD uygulama C++ kodu alınmaz; QA/operatör disiplini ve Golden araçları mevcut kapsamda kullanılır.

FIX31: Unload stack scan son bellek bölgesiyle64MiB bütçeyi aşamıyor; aşan bölge okunmadan unload reddediliyor. Diğer runtime/vendor politikaları aynı. REVIEW31 filesystem kapsam kapanışı taşındı;6 ana inceleme açık. Ayrıntı docs/RE_SOURCE_DEVIATIONS_FIX31.md.

Windows Production için scripts/build_orchestration/build_production.cmd; Debug, Native ve CRT için aynı klasördeki ilgili CMD kullanılır. CMake proje adı preloader_l; Windows MSVC x64 gerekir. Production Golden zinciri derlemeye bağlıdır. Derleme LOCAL_BUILD üretir; resmî yayın değildir.

surge.cfg ve surge_override.cfg isteğe bağlı module sibling overlaylerdir. Varsayılan SURGE profil/kimlik, veri kökü ve avatar seçimi önceki kullanıcı talimatlarıyla korunur. Upstream pinler, iki handler workerı ve session FIFO politikası değiştirilmedi.

Windows DLL/native/game kabulü yalnız gerçekten çalıştırılan sonuçla raporlanır. Portable testler ve synthetic Golden fixtureları onların yerine geçmez. Current kaynak kontrolleri metadata/GOVERNANCE_BINDING_FIX31.json ile bağlanır; tarihsel evidence current acceptance değildir.
