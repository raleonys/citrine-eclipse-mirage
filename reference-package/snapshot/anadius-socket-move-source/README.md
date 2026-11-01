# B29 — Socket taşımasının iki gerçek normal gövdesi

`move_socket_023d20` ve `transfer_socket_028950`, daha önce tanımı bulunmayan **iki özgün yordamın typed normal algoritmasını** C++'a taşır. Mevcut SocketObject/AcceptorState/Control/SocketService kaynak türleri kullanılır; native MSVC class-layout overlay veya yeni forwarding wrapper eklenmez. Normal-source eşlemesi native ABI/provider kapanışı değildir.

## İş sırası ve adres kanıtı

023d20 service ve başlangıç alanlarını kurar; executor table78 ve table88'i kopyalar, seçili slot+8 etkisini gerçek destination/source payload'ları üzerinde çağırır, **source table90'ı çağrıdan sonra yeniden okur**, ardından028950'yi çağırır. Initialized table180084928 için slot8'in1800228c0 olduğu retained rdata ile bilinir; bu hedefin gövdesi eldeki canonical/ASM kayıtlarında yoktur. `ExecutorMove` bu kesin alt etkiyi caller'dan ister; bir image VA çalıştırılmaz veya boş başarılı implementation uydurulmaz.

028950 tam64-bit socket/state'i aktarır ve source'u invalid/zero yapar; cancellation strong count artırılır, eski destination owner bırakılır, callback sonrasında source pair yeniden yüklenip temizlenir. Virtual dispose sonrası weak count canlı okunur. Servis kapısı yalnız yeni head düğümünün eklenmesini korur. **Source düğüm listeden çıkarılmaz**; eski head ise previous link'i doğal olarak yeni destination olur. Protocol/options post-gate aktarılır ve source'un2/zero alanları kurulur. Entry service, control callback object.service'i değiştirse de korunur.

Native023000 iki kez bu taşıma yolunu kullanır: pending→temporary→owned. Üç düğüm, invalid pending ve temporary dâhil, temporary024db0 çalışana kadar listede bulunur. Mevcut gerçek destructor yalnız temporary'yi çıkarır; taşınan aktif handle ve cancellation owner son owned socket'te kalır. Fonksiyonların Evidence ID/ASM/pseudo kayıtları `evidence.json`, tam retained metinler ve bağımsız `source-audit.json/.md` içindedir.

Destination/source nesneleri **ve bunların referansla tuttuğu AcceptorState alanları ayrı** olmalıdır. Destination fresh/unlinked olmalı; servis, kontrol, executor ve callbacks caller tarafından canlı ve serialize tutulmalıdır. Unsupported metadata girdileri etki öncesi reddedilir. Bu koruma valid-memory normal native alanının dışındaki girdilere tanıdır; native error ABI'si diye sunulmaz.

## Greeting geçicisinin düzeltilen ömrü

Mevcut semantic Session::start_025720 içinde greeting string'i ilk read sırasında hâlâ yaşıyordu. Native ASM **25911 heapfree →25919 startread** sırasını gösterir. `server.cpp` içindeki dar block, ikinci reset ve blocking-send loop tamamlandıktan sonra string'i read başlamadan bırakır. Reset sayısı, partial/error/zero-progress davranışı değiştirilmez. Throwing write yolu read/reaccept başlatmaz.

Own allocator observer, gerçek8192-byte canlıversion ile üretilen uzun greeting'in synchronous-send'de görülen heap adresini izler; release ilk receive öncesinde tek kez görülür. B28 ZIP'indeki eski server.cpp ayrı ownEXE'ye derlendiğinde, gerçekAES response/partialsend/EOFdrain ve finalsocketcleanup ardından yalnız hedef lifetime predicate'inde **beklenen temiz exit1** verdi. Bu negatif sonuç PASS sayılmaz. Old/current source hashes, compile/run, cleanup proof ve diagnostic greeting-negative-control.json içindedir.

## Test ve kalan sahiplik sınırı

29 core kontrolü iki move/üç düğüm, tam alan değerleri, source invalidation, shared-control sayacı, callback-sensitive reloads, service caching, weak count mutation, table90 canlı okuma, gate/protocol sırası, object/state alias ve executor exception prefix'lerini sınar. 21 entegrasyon gerçek iki move+temporary024db0 ile mevcut Session/MessageHandler/Router/XML reader/SessionCipher akışını çalıştırır. Greeting, şifreli QueryContent, borrowed async-send span'larıyla partial send ve EOF drain; error-accept/nullSession/reaccept ve greeting-write-error prefix'leri sınanır. Yalnız fake socket/SDK ve memory effects kullanılır.

Bu aşama hâlâ caller'ın yaşattığı semantic Session kullanır. **023000 factory tamamlandı veya raw detached job native lifetime/quiescence kanıtlandı denmez.** Native023000 owner0x1c8/dequeproxy16 allocation, explicit outputpair publication ve weak-self bağlama ayrıB30 işidir. Native sıralama: iki move→handler reset→temporary024db0→outputpair publish→weakself callbacks. Allocator nonnull-or-throw olmalıdır; pseudo null dalından kullanılabilir normal null-owner başarı icat edilmez. Cancellation Control ve session IoReference ayrı sahiplerdir.

Generic CRT bekler. Yeni normal-source gövde **2**, source-mapped487→489, eligible491→493, normal-body claim124→126; canonical2076/2381 paydaları ve52tarihsel native kapanış korunur. Yeni native ABI/provider kapanışı0; tam DLL hazırlığı BLOCKED. Kaynak sayısından genel DLL yüzdesi türetilmez. Öncelik1→2→3→5→4.

## B29 ortak doğrulama

**29 çekirdek + 21 entegrasyon = 50 kontrol PASS.** Ortak build 247 nesne (243 C++ / 4 C), 289 normal test grubu ve 19 framework terminal senaryosu içerir. Önceki 837 kaynak aynı; 1 önceki dosya açık hash kaydıyla güncellendi; toplam 842 kaynak. Yeni typed normal gövde 2, yeni native ABI/provider kapanışı0; 489 adres, 493 karşılaştırılabilir derlenmiş etiket ve52tarihsel kapanış. Bileşim ile raw-all defterleri ayrıdır. Gerçek alt sağlayıcılar, foreign ABI, image unload ve PE girişi açık kalır.
