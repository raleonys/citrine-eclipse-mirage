Kaynak güncellemesi: 2026-11-03. Derleme/test/link yapılmadı.

`Stop::source_quiescent(ServerView)` mevcut gerçek Stop kayıtlarını okur.
Sorgu özgün worker/fiber/PTD üzerinde, Parent Operation dışında yapılır.
ServerView kimliği ve bütün kök adresleri aynı binding'e ait olmalıdır.
Bağımsız work proof ile kabul edilmiş Requested/Draining durumu olmadan,
aktif controller post/route/call veya pending loan varken true dönmez.
Kaydedilmiş her completion için observed + acknowledged zorunludur.
SessionIoPrepared aktif binding kontrolü de korunur.

Sorgu cancel, IOCP dequeue, join, wait, callback veya close yapmaz. Kısa Parent
Operation, Stop metadata kilidi bırakıldıktan sonra alınır. Gerçek kernel
packet/receipt protokolü değişmez. Kuyrukta kalmış null-OVERLAPPED wake tek başına
canlı loan değildir; son IOCP retirement ve gerçek dış proof hâlâ gereklidir.
Yanlış graph veya bilinmeyen yerel sonuç Stop'u poison eder; yabancı aktör
kimlik reddi source graph'a girmez. Sonuç yalnız yerel bir önkoşuldur.

ApplicationWorker, configure ve controller-selected Stop uzlaşmasından sonra
SourceOwned Policy'ye aynı Runtime Stop shared_ptr'ını bağlar. Eksik Stop,
AcceptOnly kapsamı ve farklı bir proof sahibi reddedilir. Explicit mode'a bu
alan eklenemez. Alt bağımsız Policy/Profile kullanıcısı SourceOwned'u Stop'suz
seçebilir; üretim Worker yolu bu istisnayı kabul etmez.

Policy, yerel sayaç kontrolü ve Stop sorgusunu opaque external_drained
callback'inden önce ve sonra uygular. İki sorgu da Parent dışında başlatılır.
Yerel false, eldeki sahipleri tutarak gerçek close/retry protokolüne döner;
harici true sonucu bunu geçersiz kılamaz. Harici false da hâlâ false'dur.
Geçici post/route canlılığı için mevcut açık close-continuation ve controller
retry politikası gereklidir; bu değişiklik onu kendiliğinden seçmez.

Lifecycle'a shared_ptr eklenmesi HOST by-value yerleşimini değiştirir. Sonraki
açık derleme talebinde bütün Options/Graph/Worker tüketicileri birlikte yeniden
derlenmelidir. Donmuş build ve ZIP yeni kaynakları kapsamıyor.

Mevcut gerçek pipe Stop/Runtime/Worker test kaynağına SourceOwned + SessionIo
senaryosu eklendi. Configure proof sahibini kurmuyor; Worker gerçek Runtime
Stop'unu bağlamak zorunda. Mevcut dış proof callback'i de aynı Stop sorgusunun
true olduğunu denetliyor. Bu senaryolar çalıştırılmadı; native AcceptEx veya
üretim loader davranışı için PASS/sertifika iddiası yoktur.

İki production loader hook'u hâlâ tanımsızdır. Gerçek unmanaged/foreign callback
sonlanması, entry epilogue/image/code ömrü, CRT teardown, per-image assembly ve
desteklenen özgün senkron loader başlangıcı bu yerel makbuzla çözülmüş sayılmaz.
