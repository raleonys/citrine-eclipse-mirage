# R4 — frame tamamlaması ve yeniden okuma

İncelenen görüntü: anadius64.dll, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA 6.5.0 sunucu kimliği aligned; Ghidra 12.1.4 available. Yeni hedef çalıştırılmadı; aynı SHA'ya bağlı saklanan tam pseudocode ve assembly dökümleri kullanıldı. Yeni normal gövdeler yalnız **0x180025480 ve 0x1800280d0**. CRT üretimi değişmedi.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| 0x180025480 | ev_d3aaf411d3595e43ae2c3c1c8309ed42f85a4434602216330b5e2c58aa788a3b | ev_17a121443001ec46ed018ca1fad07bc2cfd74c2e067c1f3ccf0bb7bd84b95c31 |
| 0x1800280d0 | ev_dcc519a12be00908ae90c988173ae9147418f1eb4771d95b34d3148914b0d9a2 | ev_36d45e7ea060ccb9497a038a4fa3e7d55641d30c79a68da3da5ae7ff42e69c99 |

Tam hedef kesitleri frame-read-evidence.pseudo.c.txt / frame-read-evidence.asm.txt; alt bağımlılık kesitleri frame-read-context.* içindedir. Bağlam yordamları yeni kurtarılan fonksiyon sayısına eklenmez. Eski Session::do_read_0280d0 ve frame_completed_025480 semantic gövdeleri korunur; yeni frame_read.cpp ayrıntılı tanımlı alan görünümüdür. Aynı native adrese iki kaynak gövdesi bulunması ek native işlev anlamına gelmez.

## 025480 başarı yolu

0254a8 hata kodunu saklar. Kod0 yolunda native tail[0], logical ReceiveHandler.words_88_98[2], owner kimliğidir. İlk input string owner+a8, size owner+b8, capacity owner+c0. 0254cd..4e7 unsigned matched-1 ve min(input.size,matched-1) ile 006b30 kopya kurucusuna veri verir. matched0 tüm mevcut girişin kopyalanmasına yol açar. Bu çağrıya kadar kaynak adresi ve uzunluk saklıdır; kopya kurucusu zorunlu sağlayıcıdır ve bu aşamada onun native gövdesi eklenmez.

0254f2 operator new(0x30) döndükten sonra nonnull closure'a local string'in dört qword'u taşınır. Local size0/cap15/ilk byte0 olur; kalan prefix baytları sıfırlanmaz. 02551f closure.owner için handler owner'ını yeniden okur; closure.function=180028d30. Null allocation yolunda closureNULL kalır ve mesaj local'de sahipli kalır. Bu dal native'de bulunduğu için korunur; throwing-new uygulamasının gerçekten NULL dönmesi varsayılmaz.

025556 çağrısı 05e8e0(0,0,entry180021770,closure,0,&id)'dir. Entry ve function adresleri opaque veri kimlikleridir, görüntü VA'ları çağrılmaz. HandleNULL ise terminal04a648(6), low32id0 veya detach04a458 tam EAX sonucu nonzero ise terminal04a648(1). 025584 TEST EAX nedeniyle 0x100 gibi low8'i0 olan sonuç da hatadır; decompiler byte-return görünümü burada kullanılmadı. Bu hatalarda native normal akış input erase/rearm'a ulaşmaz. Yeni RAII rollback/closure cleanup/unwind uydurulmaz. Thread closure'ı begin dönmeden yok edebilir; çağırıcı closure'ı yeniden okumaz.

025594..5ca residual local string'i detach başarıdan sonra temizler. Capacity>15 için bytes=cap+1 unsigned64; bytes>=4096 ise header[data-8] base ve extra39; unsigned(data-base-8)>31 terminal05c974. Native04b0f8 callsite pointer ve size argümanları korunur. Null closure dalı local heap mesajını gerçekten serbest bırakabilir; taşınmış normal closure dalında local inline boş olduğundan free yoktur.

0255cf input için owner'ı tekrar okur. Current size ve hedef string saklanır; consumed=min(matched,current size), remaining=size-consumed. 0255fc 078d30(data,data+consumed,remaining+1), NUL dahil kopyalar. Burada mevcut bounded ReceiveBufferApi.copy_bytes sözleşmesi kullanılır; özgün CRT/SIMD078d30 gövdesi eklenmiş değildir. 025601 kopyadan sonra saklanan string hedefinin size'ını remaining yapar. 025604 yeniden CANLI handler owner'ını okuyarak 0280d0 çağırır. Kopya callback'i owner veya size değiştirirse saved hedef store ve canlı rearm kimliği ayrı kalır. Rearm owner/handler ömrünü bitirebilir; sonrası alan erişimi yoktur.

## 025480 hata yolu

025611 misc0285b0 çağrısından sonra 025616 error.category nesnesini yeniden okur. Yalnız saklanan kod2 ve category.object+8 kimliğinin misc kimliğiyle eşitliği log'u bastırır. Diğer yolda 025646 begin006880(global1800a4130,prefix1800845c8), 02565f saved category'nin canlı slot10 message çağrısı, 025678 append007990, 02568c returned stream'in signed32 vtable+4 ayarıyla widen01e510(adjusted,10), low8 put00fe90, flush00e1e0 ve residual message free sırası korunur. Read prefix'i write prefix1800845d8'den farklıdır. Kod başta saklanır, category misc sonrasında, message virtual slot begin sonrasında okunur. Flush sonrasında cleanup canlı local capacity/pointer'ı kullanır. Hata yolunda input owner projeksiyonu gerekmez; erase/rearm yoktur.

## 0280d0 yeniden okuma

0280ec owner.control08'i edinim için saklar; NULL veya strong32=0, 028208 bad_weak_ptr026860 + native INT3 dalıdır. 028104..116 locked CAS nonzero expected+1 unsigned32 ile çalışır; başarısız CAS'in güncel expected değeri kullanılır. UINT32_MAX→0 başarılı edinimdir. Kaynak std::atomic seq_cst kullanır; özgün native concurrency kanıtı sayılmaz.

028118..174 başarılı edinimden sonra object00/control08'i yeniden alır; stream=owner+10, string=owner+a8, effective/max=UINT64_MAX, **delimiter0**, flag0, scan/request0, frame owner=owner ve owned pair kuruludur. 02818d error category native literal object1800a10f8'dir. Mevcut Category nesne adresi değil name-function token'ı taşıdığından context.success_category_a10f8 aynı nesnenin tutarlı logical projeksiyonu olmalıdır; object VA yanlışlıkla name identity yerine geçirilmez ve yeni category callback'i eklenmez. 0281b7 gerçek025c40(handler,error0,bytes0,flag1) çağrılır.

0281bd sonrası yalnız residual local handler.control okunur. Strong old1 dispose slot0, ardından AYNI cached control weak old1 delete slot8 çağrılır. Dispose handler/owner control'ünü değiştirirse weak release başka control'e yönelmez. Callee owner ömrünü bitirebilir; çağrıdan sonra owner tekrar kullanılmaz. Normal actual025c40→021d10 owned pair'i taşıyıp local'i temizler. Explicit function-pointer callee injection yalnız izole testlerde kullanılır. RecoveredFrameReadApi final köprüsü gerçek025480→0280d0→025c40→021d10 varsayılanlarını kullanır.

## Doğrulama ve sınırlar

**1.216 çekirdek +315 entegrasyon =1.531 yeni kontrol PASS.** Çekirdek testleri bağımsız payload oracle, matched0/1/UINTMAX, inline/heap/4096 eşiği, nullable closure, handle/id/detach full32 hataları, header unsigned sınırları, log sırası, provider exception prefix'leri, dört ayrı owner okuma noktası, gerçek atomik strong edinimi ve residual control/lifetime sonlarını kapsar.

Yeni entegrasyon actual025480/0280d0/025c40/010040/008350/021d10/021a40 kaynak döngüsünü çalıştırır: hazır iki frame, bütün split noktaları, boş frame, trailing partial data ve EOF. Thread, 023dd0 environment ve completion tüketimi kontrollü adapter'larla sağlanır. **Bu yeni test 027710, handler scheduling veya worker/027c10 döngüsünü yürütmez.** Önceki R3 testleri constructor/IO routing/027710/borrowed/owned scheduling'i ayrıca sınar; bunlar yeni R4 tam worker zinciri kanıtı diye birleştirilmez. Sonraki native entegrasyon bu sınırı kapatmalıdır.

Eksik/belirsiz sidecar kimliği veya yetersiz bounded bellek UnsupportedInput model sınırıdır; native hata davranışı sayılmaz. Provider'lar native son kullanıma kadar tutarlı string/owner/category/control/stream yaşamı sağlar. 006b30 kopya kurucusu, 021770 thread trampoline, 028d30 uygulama dispatch'i, native closure/allocator/vtable/global/category/ownership bağlantıları ve MSVC ABI/EH/SEH/concurrency eşdeğerliği açık. 0256e3 güvenlik cookie epilogu yeni normal kaynak gövdesinde yeniden kurulmadı. Bağımsız reviewer onayı verilmedi; adresli kaynak karşılaştırması ve testler aynı ajan tarafından yapıldı. Orijinal DLL/oyun çalıştırılmadı/değiştirilmedi; gerçek CFG/token/achievement içeriği okunmadı. Toplam DLL tamamlanma yüzdesi üretilmez.
