# B18 bağımsız PTD incelemesi

**PASS.** Reviewer `/root/b18_ptd_audit`; üretim yazarı değildir. CallbackThreads, foreign admission, scope permission ve own test kaynakları okundu. Son quick kayıtların **42 çekirdek +35 entegrasyon =77 PASS** sonucu için14 kaynak ve2 EXE hash'i doğrulandı; testler reviewer tarafından tekrar çalıştırılmadı. Genel architecture review ayrı rapordadır.

Erken incelemede bulunan known-clean enter/catch yarışı düzeltildi: local flag/error kullanılır, active/operating yalnız outer catch gate'i altında sonlandırılır. Sonraki giriş eski catch tarafından poison edilemez. Operating sırasında değişen metadata gözlemleri gate ve short-circuit koşullarıyla korunur.

Borrowed PTD sahiplenilmez veya temizlenmez. Fresh initial-locale PTD'de yalnız errno/doserrno8byte değişebilir. Thread ve fiber kimliği denetlenir. Checked sentinel detach, destroy-once, observed raw free ve checked null clear aşamaları known-failure retry'de destructor/refcount/free işlemlerini tekrarlamaz. Raw free sonrası clear failure'da owned=false kalır. Unknown outcome veya diğer PTD mutasyonu active/poisoned state ve provider lease'i korur; native leave başarı sonucu döndüremez.

Final terminal test atfı da düzeltildi: core151..155 yalnız active&&poisoned predicate sonrası own-process TerminateProcess ile üretilir; genel SIGABRT239 bunları taklit edemez. Final core stderr'inde beklenmeyen secondary termination yoktur. Native cleanup child'da163 için başarısız raw release marker'ı ve abort observer gerekir; callback230 ile dönerse test başarısızdır. Önceki debugger kaydı yalnız kendi test EXE'sindeki Domain.leave→abort yoluna aittir.

Provider lease FLS index/heap/locale/lock'ları tutar. Foreign path controller phase/busy alanlarını okumaz. Incoming/outgoing LastError korunur. Gate PTD/body/original/provider/wait çağrıları boyunca tutulmaz. Pause yeni girişleri kapatırken aktif callback permission ve cleanup'ı tamamlayabilir.

**Sınırlar:** raw PTD allocation recovered calloc/new-handler/errno yoluna eşdeğer değildir. Genel worker/callback fallback-OOM, map/config/Execution/stream paylaşımı explicit ready proof gerektirir; entegrasyon kendi fake worker'larını foreign publication öncesinde tamamlar. Örtüşen callback başarısı, reentry, dynamic locale, gerçek oyun thread/fiber/hook/CRT/EH/ABI/unload eşdeğerliği açık kalır. Testler own std::threads kullanır; gerçek fiber switch testi yapılmadı. Stop/lease, disabled hook ve external quiescence kanıtı değildir.

Original/rebuilt DLL veya game çalıştırılmadı; orijinal/AV engelli version girdilerine dokunulmadı. Yeni native gövde/kapanış0.
