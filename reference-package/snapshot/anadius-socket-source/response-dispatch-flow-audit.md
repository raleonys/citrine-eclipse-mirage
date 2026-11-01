# N2 — response executor dispatch ve gönderim kuyruğu

24 Ekim2026. anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA6.5.0 aligned, Ghidra12.1.4 available. Aynı SHA'ya bağlı saklanan tam pseudocode/assembly kullanıldı; yeni hedef analizi veya orijinal DLL/oyun çalıştırması yapılmadı. Gerçek CFG/token/achievement içerikleri okunmadı. Önceki üretim kaynakları ve CRT değiştirilmedi.

Yeni normal gövdeler **021420 response dispatch,028270 enqueue ve022240 borrowed invoke**. Eklenen yardımcı/köprü metotları yeni native işlev sayılmaz. Eski Session::do_write_028270 semantic kaynağı korunur. Bu üç normal gövdenin kaynakta bulunması native allocator/executor/STL/EH/SEH/ABI veya tam DLL eşdeğerliği demek değildir.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| 0x180021420 | ev_03285e4590c59bdfef637bd131232ec356cb691047dceda259799e10be2eb802 | ev_485e1499aa6085002207d4e979d89f21170510588cb32527b4e03d79fecdcc79 |
| 0x180028270 | ev_f01c15c5ec6846574a424ff84a94d532bd1a1c4a9f781ad08433a5f12e0f868f | ev_8a9098273f1cf3fed4bf7cd29a29a081bfa18c0706bf118f25c7907784a35bd5 |
| 0x180022240 | ev_e35a89febdbea23a270dccb10d8cfe5a5c5368948c104fb1cf8cf977b23263e8 | ev_54bb972843defb1bce958dbe220882fc0bc6dcb6ffa7ef18b3a3e1ed1ec7b462 |

Tam hedef kesitleri response-dispatch-evidence.*; bağlamlar response-dispatch-context.*; hash/kapsam response-dispatch-evidence.json. Adresli kaynak karşılaştırması ve sentetik testler aynı ajan tarafından yapıldı; ayrı bağımsız reviewer onayı iddia edilmez.

## 021420 transfer, environment ve scheduling

021458..493 incoming task.function saklanır; response'un dört-qword prefix'i local'e taşınır; incoming response size0/cap15/ilkbyte0 yapılır; sonra object/control alınır ve incoming owned pair sıfırlanır. Function alanı ve response prefix'inin diğer15 baytı korunur. Bu transfer ilk executor sorgusundan ÖNCEDİR. Sonraki hata için otomatik rollback veya native unwind uydurulmaz.

021497..49b source.executor slot20 FUNCTION saklanır;02149f..4a7 source.operations slot18 query çağrılır;0214b8 saklanan factory20(prepared1,query,literal180084425) çağrısıdır. Query executor/factory binding'ini değiştirse bile cached function kullanılır. Factory20 ve factory88, mevcut023dd0 constructor slot58 veya completion prepare slot40 ile birleştirilmez; ayrı zorunlu projeksiyonlardır.

0214bb..512 prepared1→first taşıması mevcut **023cb0 normal kaynak** sırasıyla aynıdır: destination.operations/schedule saklanır, source.operations/schedule empty sentinel olur, saved table slot10 move çağrılır, source.state0 yazılır, destination.executor CANLI source.executor'dan alınır, source.executor empty sentinel olur. 021517..530 prepared1 CANLI destroy kimliği017a30 guard değilse destroy edilir. 021532/539 first.executor bu callback SONRASINDA yeniden okunur. 021541 onun slot88 FUNCTION değerini first.query öncesinde saklar;021566 factory88(prepared2,query,opaque scratch) çağrılır. Uninitialized scratch baytları kurtarılmış sıfırlar sayılmaz.

021570..5a3 prepared2→intermediate taşıması aynı gerçek023cb0 kaynağıdır;0215a7..5b8 prepared2 CANLI guard dışıysa destroy edilir. 0215c3..5f5 intermediate→final üçüncü gerçek023cb0 taşımasıdır. 021601..60c intermediate destroy **koşulsuzdur**, empty guard dahil. Böylece toplam üç move ve iki koşullu/üçüncü koşulsuz ara cleanup korunur. Ara callback'ler destination.executor veya tablolarını değiştirebilir; source ve destination aynı canlı alanları temsil etmelidir.

02160f..61a final.schedule tablosu CANLI okunur. Slot18 borrowed nonnull ise descriptor{invoke180022240,&local task} ile çağrılır; native local task burada response ve shared reference sahibi kalır. Slot18NULL ise slot10 publish FUNCTION önce saklanır; local task ikinci local'e taşınır; ilk local response/pair boşalır;021691 required020cb0 ikinci local'i owned functor slotuna dönüştürür. 02169e cached publish(final,mutable slot) çağrılır. Provider schedule binding'ini değiştirirse cached publish korunur. 0216a1..6b0 slot CANLI tekrar okunur; nonnull ise current functor.call_slot00(0) discard çağrısıdır. Slotu alan publisher pointer'ı null yaparak sahipliği devralır.

020cb0 ve native022280 owned functor gövdeleri bu aşamada yeniden kurulmadı. Mandatory contract ve testte sahipli compatibility functor vardır; borrowed çağrıyla aynı native closure/lifetime uygulaması olduğu iddia edilmez. Sonraki N3 bu boşluğu hedefler. Retained022280 invoke yolunda0219d0 çağrılır; testte022240 üzerinden çağıran compatibility functor, native022280 gövdesi diye sunulmaz.

0216b3..6be final'in CANLI operations.destroy00,0216c1..6cf first'in o anda CANLI operations.destroy00 çağrılır.0216d2..704 local task residual control için cached strong old1 dispose, AYNI cached weak old1 delete uygular. N1 028d30'daki live delete-target reload burada yoktur.021707..739 residual local response heap cleanup yapar. Source incoming task'a dönülmez. Security-cookie epilogu02173e..748 yeni normal kaynakta yeniden kurulmadı.

## 022240 borrowed invoke

02224a task.function,022251 task.object owner'ını009eb0 öncesinde saklar.02225a required009eb0(destination,&task.response) KOPYA KURUCUSU çağrılır; kaynak response korunur.02225f bağımsız kopyanın dönen string pointer'ı saklanan function(owner,result) çağrısına iletilir. Kaynak return-reference provider sözleşmesini korur; native string-copy implementation/ABI tamamlanmış sayılmaz.009eb0 callback task.function/object değiştirirse cached değerler kullanılır. Borrowed helper kaynak response veya shared control'ü taşımaz/bırakmaz; kaynak string ve control cleanup dış021420'ye aittir.009eb0 gövdesinde009edb..009ef6 source size/data okunur;009f0e kaynak16byte kopyalanır veya009f7e allocation ve009f98 size+1 byte copy yapılır; source reset yazımı yoktur. Test sağlayıcısı bağımsız owned copy üretir. Heap kaynak dispatch dönüşünde temizlenirken queued copy'nin daha sonra gönderilebilmesi ayrıca sınanır. Local string için ayrı normal cleanup uydurulmaz; native callee tüketir.

Varsayılan token router yalnız180028270'ı gerçek enqueue_response_028270'a bağlar. Image VA çağrılmaz. Explicit callee injection yalnız izole caller testleri içindir; unsupported/belirsiz projection hata davranışı UnsupportedInput model sınırıdır.

## 028270 gönderim kuyruğu

Owner+c8 deque görünümü mevcut WriteFinishOwner ile aynıdır: map owner+d0,map_size+d8,offset+e0,count+e8. Aynı sidecar queued-write02a500 ve finish025a70 tarafından kullanılmalıdır. 0282a3..2b8 unsigned(map_size<=count+1) ise required026670(deque,1) çağrılır; count+1 unsigned64 sarılır. Generic std::deque büyümesi native026670 yerine geçirilmez.

0282bd..2d9 map_size yeniden alınır; normalized=(map_size-1)&offset yazılır; absolute=count+normalized ve initial_index=(map_size-1)&absolute saklanır. 0282e1..2ea initial map slotuNULL ise0282f1 allocate04b0bc(32) çağrılır. 0282f6 map pointer'ı CANLI yeniden alınır ve sonuç **cached initial_index** slotuna yazılır. 0282fe..30c map_size/map tekrar CANLI alınır, destination index=(current map_size-1)&cached absolute hesaplanır. Allocation callback size/count/offset/map değiştirirse cached index ve fresh alanların ayrımı korunur; arbitrary bounds native fault diye modellenmez.

028310..31c destination dört qword önce0 yapılır;028320..32a input prefix'i oraya taşınır;02832e..33a input size0/cap15/ilkbyte0 olur.02833d CANLI count++ yapar;028341 yeni count==1 ise02834e gerçek02a500(owner) başlatır. Yeni queued-write bridge default gerçek021fd0'ı kullanır. Nonfirst item için queued-write context'i okunmaz. 028354..396 queued-write dönüşünden SONRA input CANLI residual capacity/pointer/header'ını temizler ve yeniden reset eder. Queue/owner ömrü02a500 sırasında bitebilir; sonrasında yalnız input kullanılır. Input'un kendi ömrü native son kullanıma kadar korunmalıdır.

Heap cleanup ortak kuralı: capacity<=15 free yok; unsigned64 bytes=cap+1;bytes>=4096 ise bytes+=39,header[data-8] allocation base ve unsigned(data-base-8)<=31. Eksik/belirsiz header/bellek/owner/factory/schedule UnsupportedInput model sınırıdır; yanlış native durum için crash/exception eşdeğerliği iddia edilmez. Native security-cookie epilogu028399..3a1 yeniden kurulmadı.

## Doğrulama ve kalan işler

**662 çekirdek +115 entegrasyon =777 yeni kontrol PASS.** Çekirdek: factory20/88/publish callback caching, gerçek üç023cb0 taşıması, guard/koşulsuz destroy sırası, borrowed/owned transfer/discard, cached residual control, callback exception prefix'leri, deque unsigned count wrap/non-power-of-two mask, grow predicate,32-byte allocation, allocation sonrası fresh map/cached absolute, first-start sonrası geç input cleanup, owner ömrünün bitmesi ve022240 cached function/owner davranışı.

Entegrasyon actual **028d30→021420→023cb0→022240→028270→02a500→021fd0→021a80→0261d0→025a70→next02a500** kaynaklarını aynı queue/shared control üzerinde çalıştırır. Yanıtlar embedded NUL'lu8byte,0byte ve150001byte; sonuncusu65536+65536+18929 gönderilir. FIFO ve full-byte payload bağımsız oracle ile doğrulanır. Owned dispatch anında, deferred veya unclaimed flag0 discard olabilir. Yeni final RecoveredResponseDispatchApi, N1 MessageDispatchApi::dispatch_response_021420 hook'unu actual021420'a bağlar.

Bu entegrasyonda033d70 iş çekirdeği response provider'ı,020cb0/022280 functor,009eb0 string copy,026670 büyüme,023dd0 send environment ve completion tüketimi kontrollü adapter'lardır. AsyncSendApi sentetiktir; SDK WSASend/gerçek ağ/socket/OS thread çağrılmaz. Controlled completion consumer actual026a00 storage release ve021a80 invoke'ı çalıştırır; **native027a20/IOCP routing/handler scheduling/027c10 worker burada yürütülmez**. Önceki N1/R3/queued-write testleri ayrı kapsam olarak korunur; toplam yeni uçtan uca worker veya native ABI kanıtı diye birleştirilmez.

Sonraki N3:020cb0 owned-response factory ve022280 call/discard gövdeleri;009eb0/026670/string/deque alt bağımlılıkları adresle izlenir. Native ownership/global/allocator/polymorphic/string/executor/SEH/EH/concurrency/MSVC ABI ve tam DLL bağlantıları açık. CRT üretimi aşama5'e kadar beklemede. Yeni kaynak veya test sayısından toplam DLL yüzdesi üretilmez.
