# preloader_l — çalışma talimatı

Bu rehber davranış yetkisi üretmez. En yeni kullanıcı talimatı ve güncel Library master üstündür; docs/GOVERNANCE_APPLICABLE.md / metadata/GOVERNANCE_BINDING_FIX31.json uygulanabilir alt kümeyi bağlar.

- BLUEFIELD NON_RELEASE, kod atası tam kullanıcı RE paketidir. BROWNFIELD QA/tool disiplini korunur; uygulama C++ otomatik alınmaz. Diğer FIELD projeleri/resmî release/canonical pointer değiştirilmez.
- preloader_l/SURGE adı, exports DllMain@1 / SURGE@2 / DllShutdown@3, optional surge.cfg, profil/avatar, production disk log kapatma, failure-atomic ve owned shutdown korunur. Reddedilmiş quota/cache/sidecar/INI-delay kararları eklenmez.
- FIX31 scope REVIEW31 önerisindeki unload stack region budget guardıdır: mevcut64MiB kalan bütçeyi aşan bölge taranmadan false döner. Aynı sayısal bütçe; module pin/lease/freeze/hook/CRT/CFG/profil davranışları korunur.6 ana inceleme açık; DEP05 kapsam incelemesi REVIEW31'de kapandı. Vendor1060 bytesı korunur.
- Source farkı otomatik revert yetkisi değildir. Kullanıcı talebi/upstream/own integration ayrı yazılır. Kayıtsız tarihsel gerekçeler uydurulmaz. Dependencies yalnız açık kullanıcı tetiklemesiyle güncellenir.
- Önceki64 C++/header fixture bytesı korunur. Yeni unload test TU'su gerçek production cpp gövdesini dahil eder; Linux test-only SDK/SEH syntax shimleri native ABI/SEH/OS kabulü değildir. MSVC kolu gerçek VirtualQuery/VirtualAlloc scanner fixtureı sağlar; burada NOT_RUN. Production graphına test shimleri giremez. quiesce timeout yalnız lease pollingdir; toplam teardown deadlineı diye raporlanmaz.
- Yeni source/test graph ve import/current hashleri, test roles ve includes bindingine kaydedilir. Standard include-TU adapterının namespace lookupi her explicit dependency updateinde incelenir. Vendor fork/generated copy/private member hack eklenmez.
- Yerel build/test için yayın onayı gerekmez. Golden zinciri, launcher sözleşmesi, /we4702, lisanslar ve exact ZIP/hash replay korunur. Beklentiler PASS almak için gevşetilmez.
