# N1 — message thread ve uygulama dispatch bağlantısı

Hedef anadius64.dll SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA sunucusu6.5.0 aligned; Ghidra12.1.4 available. Aynı SHA'ya bağlı saklanan tam pseudocode/assembly kullanıldı. Orijinal DLL/oyun çalıştırılmadı veya değiştirilmedi; gerçek CFG/token/achievement içeriği okunmadı. CRT üretim kaynakları değişmedi; mevcut05e8e0/05e7a8 kaynakları yeniden kullanıldı.

Yeni normal gövdeler yalnız **0x180021770** ve **0x180028d30**. 028d30 için eski Session semantic gövdesi korunur; yeni ayrıntılı gövde aynı native işlevin başka kaynak görünümüdür. Köprü/helper/metot sayıları yeni native fonksiyon diye sayılmaz.

| Yordam | Pseudocode Evidence | Assembly Evidence |
|---|---|---|
| 0x180021770 | ev_f4884fc6f81199da63c2b34f733cd982fc0fada865bc01fed690b3c71bce8d8a | ev_267ad2fba0da2ea44cbb89f4ef891512c4b721af9c7a6c38b4be9571c4a1bc19 |
| 0x180028d30 | ev_0d8a8e35ac896b7bc4add247df507ee04b3056848c23b97a02d0e5360511e67a | ev_65681baf7e383e89ff3fa9f7f1032ce0865024b0d48e0ad7b76997e04088bc66 |

Tam kesitler message-dispatch-evidence.*; bağlamlar message-dispatch-context.*; SHA ve kapsam message-dispatch-evidence.json içindedir. Ayrı bağımsız reviewer onayı iddia edilmez; kaynak/assembly karşılaştırması ve sentetik kontroller aynı ajan tarafından yapıldı.

## 021770 thread giriş yordamı

021779 ve02177d closure+20 owner ve+28 function değerlerini saklar. 021781..78d string prefix'inin dört qword'unu local request'e taşır. 021792..7a2 closure string.size0/cap15/ilkbyte0 yazar; kalan prefix baytlarını korur. 0217aa saklanan function(owner,&local request) çağrısıdır. Varsayılan kaynak yönlendiricisi yalnız R4'ün kurduğu180028d30 kimliğini gerçek dispatch_message_028d30'a bağlar; image VA çağrılmaz. İzole testlerin explicit invoker parametresi native function çeşitliliği veya orijinal yürütme kanıtı değildir.

Local request için thread girişinin ayrı normal destructor'ı yoktur; tüketim callee'ye aittir. Callee sonrası0217ac exit-wait cleanup04a47c çağrılır. 0217b1 closure'ın CANLI string kapasitesini yeniden okur. >15 ise cap+1 unsigned64 bytes ile free; bytes>=4096 için header[data-8], bytes+=39 ve unsigned(data-base-8)<=31 kuralı vardır. 0217e7..7f7 closure string'ini tekrar size0/cap15/ilkbyte0 yapar. 0217fa..802 closure için sized delete(pointer,0x30), ardından EAX0 dönüşü vardır. Free closure ömrünü bitirebilir; sonrasında erişim yapılmaz. Callback/cleanup/free hata verirse yalnız tamamlanan normal prefix korunur; native EH/unwind uydurulmaz.

RecoveredMessageThreadApi::begin_frame_thread_05e8e0, önceki FrameReadApi hook'unu mevcut threads_recovered::begin_thread_05e8e0 kaynağına bağlar. Security/uint32 stack/opaque entry/closure/flags/output-id aynen geçirilir. Sağlayıcının Effects::invoke_entry'si route_message_thread_entry üzerinden kayıtlı canlı closure'ı seçer. Entry veya closure kimliği eksik/belirsizse UnsupportedInput bounded model sınırıdır; native NULL crash/fault davranışı diye sunulmaz. OS CreateThread/PTD/FLS/lifecycle backend'i hâlâ açık bağımlılıktır.

## 028d30 uygulama dispatch

028d5d request.size==0 ise028d67 yalnız00aa50 string cleanup bağımlılığını çağırır; owner/control/executor/message core okunmaz. Nonempty yolda028d8b handler adresi literal owner+f0 ve request ile033d70(response,request) çağrılır. Bu alt kaynak request'i değiştirebilir; yalnız string_view kopyalayan adapter bu sözleşmeyi tam karşılamaz. Response boş bile olsa dispatch akışı devam eder.

028da8 owner.control08 edinim için saklanır. NULL veya strong32=0, bad_weak_ptr026860 dalına gider. 028dc0..dd2 gerçek low32 locked CAS döngüsü, nonzero expected+1 unsigned32 ve failure sonrası güncel expected'i korur. UINT32_MAX→0 başarılı edinimdir. 028dd4/7 başarıdan sonra owner.object00/control08 yeniden okunur. Task.function=180028270, response prefix'i taşınır, orijinal response size0/cap15/ilkbyte0 olur ve task owned pair kurulur. Bu edinim033d70 SONRASINDADIR; core callback owner alanlarını değiştirebilir.

028e18..3a ilk executor görünümünü kurar: owner+78 kaynak env, operations=owner+88, schedule=owner+98; seçilen table slot08 copy(destination,source) çağrılır. 028e3d owner+a0 executor alanını callback SONRASINDA alır. 028e48..64 ikinci görünüm için ilk env'in CANLI operations/schedule değerlerini saklar ve o tablonun slot08 copy'sini çağırır. 028e67 ilk env.executor alanını ikinci callback SONRASINDA yeniden alır. Native slot08 copy, mevcut IoHandlerVtable.move_slot10 ile aynı şey değildir; ayrı table→copy binding'i kullanılır. Bütün projection'lar aynı owner+78 nesnesi/alanlarını temsil etmelidir; kayıtlar native ABI overlay'i değildir.

028e6f..7d dispatch021420(second env,task,R8=0,R9=0) çağrısıdır. 028e83..8e second env'in CANLI table slot00 destroy'u,028e91..9c first env'in o anda CANLI table slot00 destroy'u çağrılır. Dispatch veya ilk destroy tabloyu değiştirirse sıradaki virtual destroy yeni canlı tablo üzerinden yapılır. Bu aşama021420'nin karmaşık executor/closure alt gövdesini yeniden kurmaz; zorunlu sağlayıcıdır. Yeni entegrasyon bu noktada sahipli bir controlled response consumer kullanır; gerçek028270/write queue/IOCP scheduler yürütüldüğü iddia edilmez.

028e9f..ed6 residual task.control strong old1 release ayrıntısı R4'ten farklıdır. Strong decrement eski1 ise cached control dispose slot00 çağrılır; weak decrement aynı cached control üzerinde yapılır. Weak eski1 ise **028ecf task.control yeniden okunur ve delete slot08 hedefi bu canlı pointer olur**. Dispose task.control'ü değiştirirse weak counter ve delete target ayrılır. Kaynak bu farkı korur; null canlı target model sınırı olarak reddedilir. Testler eski counter ve yeni delete nesnesini ayrı izler. Owner nesnesi dispatch sırasında yok olabilir; sonraki cleanup owner'a dönmez.

028ed9..f14 task.response canlı capacity/pointer ile heap cleanup;028f15..f47 residual original response cleanup;028f4c..59 original response reset;028f5d..f8e input canlı heap cleanup;028f93..fa3 input reset sırası korunur. Prefix reset yalnız size/cap/ilkbyte değiştirir. Free cap+1 overflow veya4096/header sınırlarını keyfi normalize etmez. Bounded header registry arbitrary host pointer okumaz. Native security-cookie epilogu028d6c..73 bu yeni normal kaynakta yeniden kurulmadı.

## Testler ve açık sınırlar

**621 çekirdek +199 entegrasyon =820 yeni kontrol PASS.** Çekirdek testleri iki executor copy'si/callback reload/destroy sırası, response0 ve embedded NUL, empty request, gerçek unsigned32 CAS wrap, residual strong/weak/live-delete hedefi, string header4096 ve unsigned8..39 sınırları, owner ömrünün dispatch sırasında bitmesi, thread prefix move/reset/sized free48, geç closure mutasyonu ve normal hata prefix'lerini kapsar.

Entegrasyon actual025480→unchanged05e8e0→05e7a8→new021770→new028d30→existing033d70 kaynak akışını çalıştırır. CreateThread test backend'i hem uygulamayı begin dönmeden bitirir hem deferred çalıştırır; gerçek OS thread veya DLL kullanılmaz. TLS/PTD context ve exit callback kontrollüdür; context uygulama sonrası gerçekten yok edilir. R4 erasure/rearm burada explicit capture sınırıdır; yeni test025c40/R1/R2/027710/scheduling/027c10 worker döngüsünü çalıştırmaz. Önceki testler ayrı kapsam olarak korunur, yeni tam uçtan uca worker kanıtı diye birleştirilmez.

Mevcut033d70 gövdesi ve pinned tinyxml2 compatibility reader/MemoryPrinter kullanılır. Kayıtlı iş komutu ve cipher sağlayıcıları sentetiktir. Normal-return string adapter'i request değişikliğini bounded owned string'e geri yazar; response'un NUL/full byte uzunluğu korunur. Düz XML, hex decode sonrası değişen request, decryption-failure ve malformed-XML boş response, empty-frame bypass sınanır. Adapter native033d70 allocation/string ABI'si veya exception sırasında ara mutation eşdeğerliği iddiası değildir.

Native033d70 string/core projection,021420 executor/owned closure,028270 queue,00aa50 native string cleanup,04a47c lifecycle callback, gerçek category/allocator/globals/vtable/control/SEH/EH/concurrency/MSVC ABI ve tam DLL bağlantıları açık kalır. CRT üretimi aşama5'e kadar beklemede. Sonraki N2:021420 response dispatch ve onun028270 bağlantısının kaynak/ownership denetimi; gereken CRT alt işleri aşama5 engel listesine aktarılır. Toplam DLL yüzdesi test veya kaynak tanım sayısından türetilmez.
