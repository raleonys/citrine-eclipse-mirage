# B13 — Online platform bağları

`OnlinePlatform`, online Effects arayüzündeki **15 platform politikasını** sahip olunan OS/encoding/status/file ve açık stream/Dialog kaynaklarına bağlar. Bunların **10'u mevcut RecoveredEffects gövdeleridir**; yeniden yazılmadı. Beş sınır export lookup, filesystem error, literal log, log finish ve dialog'dur. **7 foreign invocation +2 phase metodu** owned lower'da kalır; B3 OnlineExecution iki phase'i kendi shared Execution/ready durumuna bağlar.

**61 çekirdek +54 entegrasyon =115 yeni kontrol PASS.** Bağımsız reviewer `/root/b13_foreign_audit`: **PASS**. Testleri root çalıştırdı; reviewer5source/2EXE hashini ve765eski kaynağın aynı olduğunu doğruladı, testleri yeniden çalıştırmadı. Toplam770 kaynak; önceki765 dosya değişmedi.

## Kanıt ve korunan sıra

035bd0 pseudo Evidence `ev_01da19447a43bf9f2a4a248ddc7f93acc033773f2f55e70da7f7fc84fd393c99`, asm `ev_42d4b70085bbc786efef3d59635d848e062d1ecab79b8bd4e827ea88833cf586`.035b60 pseudo `ev_f2254ea855134db4962d20dde46b3686ce230f94b3578001b0c25fc3400e4387`, asm `ev_d120db083716acb73a3aab91edb94f7ae2910ca33f5cd2a7baa1d1e72b62a030`.0366f0 pseudo `ev_a10737dd11c06584a1d8bfb25cfe4eb94945bdc1d2b23aca89b5fddc0066d5f4`, asm `ev_1fcea4b34ce7b7c2ba51c2e17ba2693f4d7fdb691fc078637e4d3b78e8cf139f`. [Tam kaynak/byte kanıtı](source-evidence.json), [foreign çağrı tablosu](foreign-call-audit.md).

035c09 error-mode suppression→035c18/035c53/035c95 filesystem-codepage widening→035d8d0450d0 **wide-string memory copy**, LoadLibraryW→035e26normal-path mode restore. Diskte temp kopyası/normalizasyon eklenmez. Conversion exception yoluna yeni rollback eklenmez. Filesystem flags8 ile dialog flags0 ayrımı korunur.

035e62 version_check ve035f60 create_online SDK lookup'ları yalnız iki gözlenen adı statik NUL-terminated storage'dan geçirir. Procedure/object tokenları yeni adapter içinde invoke/dereference edilmez;7foreign çağrı gerçek bir lower uygulaması gerektirir. Yedi gözlenen log literal'ı retained rdata ile eşleşir; borrowed view keyfi C-string olarak taranmaz. Desteklenmeyen literal/export açık domain hatasıdır.

03600d online bayrağı log/getter'lardan önce yazılır. Kısmi username/persona/userId atamaları ve status/file hata prefix'leri korunur. Offline3field profil guard'ı ve avatarın daha önce seçilmesi testedir. Cleanup slot0 döndükten sonra object'i, FreeLibrary'den sonra module'ü sıfırlar; BOOL'u yok sayar. Slot0 için destructor/allocator anlamı tahmin edilmez.

## Sahiplik ve test sınırı

Owner'ın internal RecoveredEffects adapter'i kendi kaynaklarına ve this'e reference tutar; **copy/move kapalıdır**. Kullanılan10core metot remaining'e geri forward etmez; recursive delegation yoktur. Yıkım sırası borrowed adapter→lower→Dialog/stream→resources. Streambuf ayrıca yaşamalı ve tüm stream/SDK/foreign kullanıcıları caller tarafından serileştirilmelidir. Destructor implicit module unload yapmaz; cleanup source yoludur. Constructor hiçbir module/file/log/UI işlemi başlatmaz.

Testler actual online/worker/cleanup ve B3/B4 Runtime kaynaklarını kontrollü modül handle/export/foreign/OS/join/callback/profile-memory/UI sağlayıcılarıyla çalıştırır. Başarı, eksik modül/export, sürüm uyuşmazlığı, offline profil, stream/encoding/getter/status/destroy hataları, kısmi state, timeout/retry ve cleanup gözlenir. Gerçek LoadLibrary/FreeLibrary/GetProcAddress/MessageBox veya orijinal/rebuilt DLL çalıştırılmadı. Gerçek kullanıcı CFG/token/profile/achievement içeriği okunmadı. Yalnız küçük kendi stringleri üzerinde SDK encoding çalışır.

Compiler filesystem/error/stream politikası, native MSVC ABI veya gerçek provider gövdeleriyle eşdeğerlik değildir. Configuration19/21, Services17/17 korunur. Yeni canonical native gövde/kapanış0;487mapped/491eligible/124normalclaim/52tarihsel kapanış aynı. Toplam DLL yüzdesi türetilmez; tam DLL **BLOCKED**. [Güncel Resources denetimi](resources-audit.json). B10 bağımsız inceleme borcu ve AV engelli version girdilerinin no-retry sınırı korunur.

B14: host/hook OS ve engine sınırları; callback/original kimlikleri ve sahipliği korunarak ilerle. Öncelik1→2→3→5→4, genel CRT kuyruğu beklemede.

## B13 ortak doğrulama

**61 çekirdek + 54 entegrasyon = 115 kontrol PASS.** Ortak build 231 nesne (227 C++ / 4 C), 257 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 765 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 770 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
