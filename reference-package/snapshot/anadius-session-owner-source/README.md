# B30 — 023000 session-owner normal kurucusu

`construct_session_owner_023000` daha önce module-record-only kalan **bir özgün yordamın typed normal gövdesini** ekler. std::shared_ptr facade kullanılmaz: mevcut IoReference strong/weak alanları ve gerçek B29 socket-move kaynakları kullanılır. Session cancellation Control ile session IoReference farklı sahiplerdir. Bütün842 önceki kaynak aynıdır.

## Kanıtlı kuruluş sırası

Native04b0bc isteği0x1c8 →control strong1/weak1 ve084750 tableidentity →pending'den temporary'ye023d20 →weakobject/control temizliği23078/7b →temporary'den owner.socket'e ikinci023d20 →owner+a8 input16zero/size0/cap15 →owner+c8 dequeheader5zero →16-byte proxy allocation →proxy+8zero/headerproxy/backlink →gerçek02bc70 cipherreset230e2 →gerçektemporary024db0230eb →**outputobject/control230f9/fc** →weakself işlemleri. Tam pseudo/ASM satırları, Evidence ID ve hashes evidence.json/retainedmetinlerdedir.

FrameReadOwner/input, SocketObject, WriteFinishOwner queue, DequeProxy ve SessionCipher aynı SessionControl.object içinde yaşar. Actual C++ object adresi kimlik token'ıdır. owner+10/+a8/+c8/+f0 koordinatları typed registry etiketidir; native MSVC layout/offsetof/pointer cast olarak kullanılmaz. Proxy backlink'i typed deque header'ına gider. Normalfreshçıkış **strong1/weak2**: implicitweak ve owner'ın selfweak referansı.

Output `SessionPair` bir plain construction slot'tur; eski değeri RAII gibi bırakmaz. Output, weakself replacement ve eski weakdelete callback'lerinden **önce** yazılır. Callback output.control'ü değiştirse bile son strong release, callback öncesi kopyalanmış control üzerinde çalışır. Eski selfweak canlı strong'a sahipse değiştirilmez; expired ise yenisi bağlanıp eski weak düşürülür. Callback throw'unda outputpublication ve o ana kadarki sayaç prefix'i korunur.

## Allocation, lifetime ve başarısızlık kapsamı

Allocator, control için1c8 ve proxy için16 isteğini karşılayan **fresh, stable, live C++ projeksiyonlar** sağlar. Bunlar sizeof(hostclass) iddiası değildir. Her iki native çağrı nonnull-or-throw sözleşmesindedir. Retained04b0bc malloc/newhandler/throw akışı, pseudo'daki görünürdeki null dalını kullanılabilir normal null-owner başarısı yapmaz; bozuk lower nullptr döndürürse std::bad_alloc tanısı verilir.

Caller-owned kalıcı ConstructionScratch, temporary'nin state ve SocketObject depolamasını tutar. Proxy veya lower çağrısı throw ederse servis listesindeki temporary, bitmiş bir fonksiyon-stack nesnesine dönüşmez. Allocator partial control/proxy depolamasını tutmalıdır; caller scratch/servis/provider ömürlerini ayrı temizlik kanıtına kadar korur. Native EH/SEH rollback kurtarıldı iddiası yoktur. Tek kullanımlı scratch kör retry'ı engeller. Faz bayrakları yalnız tamamlanan normal adımları gösterir; bilinmeyen throw sonucunda güncel health/cleanup kanıtı değildir.

Son cached strong release dispose/delete çalıştırabilir. Gövde bundan sonra owner/control'e dokunmaz; yalnız bağımsız scratch.complete yazılır. scratch.allocation bu durumda dangling olabilir; complete, owner hâlâ canlı veya native quiescence sağlandı anlamına gelmez. C++sidecar'ın typed cipher storage initialization'ı native çağrı sayılmaz; gerçek02bc70 reset yalnız kanıtlı proxy-sonrası noktada çalışır.

## Doğrulama

34 core kontrolü exactrequest/prefix, weakclear'ıniki move arasındaki yeri, input/dequezero-beforeallocation, proxybacklink/cipher-beforetempcleanup, output-before-oldweakcallback, strong1/weak2, expired/live selfweak dalları, callback outputmutation ve throwprefix'i, nullallocation diagnostics ve stablepartialtemporary'yi sınar. Seçili kontrollü disposer gerçek024db0 sockettemizliği/proxy/selfweak bırakma yapar; bunun native026570 gövdesi olduğu iddia edilmez. Kurucu içindeki son cachedstrong release'in silmeye yol açabilen dalında post-access bulunmadığı kaynak incelemesiyle doğrulandı; bu dahili reentry senaryosunun runtime testi yapıldığı iddia edilmez. Gerçek heap deletion aşağıdaki downstream sonweak testindedir.

32 entegrasyon kontrolü factory'nin ürettiği owner/input/control/cipher ile gerçek0280d0/025c40/021d10/027710 EOF ve025480/021770/028d30 frame/message kaynaklarını çalıştırır. GerçekXML/GetConfig/MemoryPrinter, plaintext ve aynı factorycipher ile şifrelenen/ayrı bir client cipher örneğiyle çözülen cevaplar kullanılır. **Response scheduler kontrollü job adapter'ıdır**; gerçek021420 schedulingbackend/IOCP tümü çalıştı denmez. 033d70 alt çağrısı mevcut semantic message core'a bağlı adaptördür; bütün native033d70/dispatcher sağlayıcıları bu testle kapanmış sayılmaz. Readstrong, hamdetachedjob mesaj işlemine kadar yaşar; responsejob strong edindikten sonraEOF/read bırakılır, jobdrain gerçeksocket/input/proxy/selfweak temizliğini tetikler; observerweak bırakılınca gerçekC++heapblock silinir. Seçilihostdisposer native026570/017420/026290 gövdeleri değildir. Ayrıntılı frozenortakbuild sonucu aşağıdadır. Native concurrency/quiescence kapanışı ileri sürülmez. Hedef DLL/game, gerçeksocket/network veya kullanıcıCFG/token/profile içeriği çalıştırılmaz/okunmaz; yalnız ownEXE ve injectedmemory/SDKeffects.

## Açık kalan gerçek alt gövdeler

Retained084750 vtable: slot00=026570 dispose; slot08=017420; slot10=026290. 017420, slot10'a flags1 ile gider;026290 flags&1 olduğunda table restore ve1c8 sizedfree yapar. Dispose CONTROLbase alır: nativecontrol+d8=owner+c8 deque, control+b8=owner+a8 input, control+20=owner+10 socket, control+18=owner+8 weakself. Bu üç gövde **bu pakette kurtarılmış sayılmaz**; IoReference implementasyonu gerçekresourcecleanup veownedselfweakrelease'i outerimplicitweak bırakılmadan önce sağlamalıdır. Sadece dispose sayacı artıran no-op, üretim destruction sözleşmesi değildir.

Sonraki B31 bu dispose/delete zinciridir; ardından native025720 acceptedstart ve tam session-owner lifecycle. 0228c0 executor alt gövdesi,021b00/0271a0 accept, nativeEH/unload/PE,lisans ve detachedquiescence açık kalır. GenelCRT bekler; öncelik1→2→3→5→4.

Yeni normal gövde **1**; eşlenen489→490, eligible493→494, normalclaim126→127. Canonical2076/2381 ve52tarihsel native kapanış korunur. Yeni native ABI/provider kapanışı0; tamDLL BLOCKED. Bu sayılardan güvenilir genelDLL yüzdesi türetilmez.

## B30 ortak doğrulama

**34 çekirdek + 32 entegrasyon = 66 kontrol PASS.** Ortak build 248 nesne (244 C++ / 4 C), 291 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 842 kaynak aynı; 0 önceki dosya açık hash kaydıyla güncellendi; toplam 846 kaynak. Yeni typed normal gövde 1, yeni native ABI/provider kapanışı0; 490 adres, 494 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
