# B12 — Library Effects log/UI/hata politikaları

`LibraryEffects`, mevcut arayüzün beş log/UI/hata metodunu somut kaynak politikalarına bağlar. `Dialog`, typed SDK sınırında038bc0/0452d0 akışını sağlar. Diğer iki phase metodu owned lower'a yönlenir; B3 `LibraryExecution` bunları mevcut shared Execution ve ready durumuna bağlar. Yeni veya varsayılan phase state üretilmez; tam üretim Resources factory'si sağlanmış sayılmaz.

**53 çekirdek +39 entegrasyon =92 yeni kontrol PASS.** Bağımsız inceleme `/root/b12_ui_error_audit`: **PASS**. Reviewer kaynaklara yazmadı; testleri root çalıştırdı, reviewer7kaynak ve2EXE hashini doğruladı. Önceki758 kaynak aynı; yedi yeni dosyayla toplam765. B10 inceleme borcu ayrı açık kalır.

## Adres ve kanıt

Retained `0x1800863e0` tam olarak `Loading ` önekidir; read Evidence `ev_1585c277e2b5dffb5a54c26a1db122db985c13d72fea8b0b6272dbea69b9b082`.035260 kaynak sırası: prefix→UTF8→006860 counted formatted string insertion→0088e0 newline/flush→load. Dönüşüm/stream hatasında daha önceki çıktı ve hata modu etkileri geri alınmaz.

038bc0 assembly: **038bfc title conversion →038c09 body conversion →038c2f MessageBoxW(nullptr,body,caption,flags)**. Dialog pseudo Evidence `ev_5fe2a22e48a2f1a6c5771fc4c6ef4fe75b8ef06f743100261e1d6ece6d1baa76`, asm `ev_0ee948b57b5a0b042dbfa8ae1605ebd4e7170d08b24e71e12c3017ec7c1b3ff8`.0452d0 pseudo `ev_9359087b086634ebd4505730f1a1efc2350f3262fb8d4ced53b676b1dc8240a3`, asm `ev_76e87ce9f231422d8bffdcdacd0af24863934f8b8d67dfa3ad4f2a1251ba381d`. [Tam kanıt kaydı](source-evidence.json).

Dialog CP65001 **flags0** kullanır; strict flags8 helper kullanılmaz. Empty input API çağırmaz. Nonempty input query0 olsa da ikinci çağrıyı yapar; query/fill hataları denetlenmez, ilk query boyu korunur. Embedded NUL conversion'a dahildir; endpoint c_str'da kesilir. Library arayüzü void olduğundan özgün MessageBox dönüş eşlemesi kullanılmaz ve tamamlandı sayılmaz.

Özgün0452d0 uzunluğu low32 int'e indirger; yeni helper **INT_MAX üstünü açık seçilmiş domain sınırıyla reddeder**. Bu native dal değildir. Negatif SDK count geçerli Win32 sonuç sözleşmesinin dışıdır. Devasa geçerli storage testi yapılmadı; geçersiz devasa string_view üretilmedi.

00cd70→00f200→category object0a10d8 ve00cdb0→00cb50→object0a10f8,00a8b0 system_error constructor/04d228 throw zincirine gider. Mevcut `PortableErrors` compiler generic/system politikası yeniden kullanılır. Bu singleton'ların pointer/name zinciri retained veride tam doğrulanmadı; native what/locale/RTTI/EH eşitliği iddia edilmez.

## Sahiplik ve doğrulama sınırı

Explicit shared stream **ve streambuf**, shared Dialog, owned Errors ve lower phase Effects yaşamalıdır. Lower önce yıkılır. Tüm stream kullanıcılarını (B10/B11 dahil) caller serileştirir; iki ayrı log çağrısı yeni bir atomic line transaction'a çevrilmez. Constructor log,UI,module load veya callback başlatmaz.

Testler actual035260/035850/035ab0 ve B3/B4 Runtime kaynaklarını çalıştırır. Sentetik module handle sonuçları, typed UI endpoint, kontrollü join/callback/foreign sağlayıcılar kullanılır. Load hatasında reconversion/UI/sonraki dosyaya devam, query/fill/stream/UI exception etkileri, worker ready/commit sırası ve timeout/retry cleanup doğrulanır. Yalnız kendi stringleri üzerinde gerçek UTF8 SDK dönüşümü çalışır; **LoadLibrary/FreeLibrary/MessageBox, orijinal veya rebuilt DLL çalıştırılmaz**. Native concurrent worker/genel ABI eşdeğerliği kanıtı değildir.

Configuration19/21 ve Services17/17 seçilmiş kaynak bileşimi korunur. Yeni canonical native gövde/kapanış0;487mapped/491eligible/124normalclaim/52tarihsel kapanış değişmez. Toplam DLL yüzdesi türetilmez; tam DLL **BLOCKED**. Gerçek kullanıcı CFG/token/achievement içeriği okunmadı, orijinaller değiştirilmedi; AV engelli version girdileri yeniden denenmedi. [Güncel Resources denetimi](resources-audit.json).

B13: online Effects OS/module-copy ve foreign-object sağlayıcıları; farklı UTF8/flags davranışlarını koruyarak log/dialog politikalarını uygun sınırda yeniden kullan. Öncelik1→2→3→5→4; genel CRT kuyruğu bekler.

## B12 ortak doğrulama

**53 çekirdek + 39 entegrasyon = 92 kontrol PASS.** Ortak build 230 nesne (226 C++ / 4 C), 255 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 758 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 765 kaynak. Yeni native gövde ve kapanış 0; 487 adres, 491 karşılaştırılabilir derlenmiş etiket ve 52 native kapanış korunur. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
