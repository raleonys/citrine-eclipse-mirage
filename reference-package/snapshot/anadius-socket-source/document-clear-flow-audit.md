# N15 — Document clear ve child-node cleanup kaynak bağlantıları

25 Ekim 2026. `document_clear.hpp/cpp`,002e60/003110 normal akışlarını ve `RecoveredDocumentClearEffects` iki final bağlantısını ekler. N14 document ctor/parse/destructor, aynı bellek ve aynı error+6c hücresi üzerinde bu kaynakları seçer. Gerçek native DOM/parser/pool/vtable/heap/ABI/EH tamamlanmış sayılmaz.

## Kimlik ve kanıt

REA6.5.0 server/catalog hizası ve kullanılabilir Ghidra sağlayıcısı `document-clear-rea-session.json` ile doğrulandı. Orijinal hedef açılmadı/yürütülmedi. Retained Ghidra12.1.4 tam listing hashleri doğrulandı. Anadius64 SHA-256:`9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. İki hedef ve on context yordamının tam bölümleri evidence/context sidecar'larında; context işlevleri yeni kurtarılmış sayılmaz. ID/hash/scope kaydı `document-clear-evidence.json` içindedir. Aşağıdaki adresler0x180 image prefix'ini kullanır.

| Yordam | Assembly Evidence | Pseudocode Evidence |
|---|---|---|
|002e60|`ev_85978582117bd9f341daf92a73488d66e6901ef09a94b8f6c42e4e4a594b75a6`|`ev_d444b1fc601d178931a7e8d2639ae1302147db0aa91a4ac0ba78524478cc9132`|
|003110|`ev_635328ecb5bc687965c6bb53ce282983fd334bc548208011ed74d62e645db5bf`|`ev_1309c5f52e5b3d9c95ec97c721c6a49f19fcded8db1c04d21e5faf6c6874beb4`|

002e72 doğrudan003110(document) çağırır.002e77/002eaf canlı DWORD+104 sıfırdan farklıyken document+a8 tablosunun ilk node'u her döngüde yeniden okunur. Node+10 parent nonzero ise002e96→003030(parent,node); zero ise node+60 pool, canlı pool vtable+20 çağrısı(yalnız pool receiver) ve002eaa→003240(cached node). Track callback'i entry/count değiştirirse bile bu iterasyonun cached node'u silinir; sonraki iterasyonda live table/head/count yeniden okunur. Yapay iteration cutoff veya negatif sayacı reddeden guard eklenmez; sonlu ve eşlenmiş domain mandatory lower effects tarafından sağlanır.

002eba/+002ebd document+6c ve+90 DWORD'lerini sıfırlar. Canlı flags+78 bit0x200 set ise002ed3 pointer-only04b0f8(doc+80); sonra+78/+80/+88 sıfırlanır.002ee9 buffer+98'i canlı okur;002ef0 null olsa da pointer-only free çağırır. Ardından yalnız+98 ve+a4 sıfırlanır. Free callback'lerinin error/line/count üzerindeki önceki alanlara yazdığı değerler sonradan topluca silinmez. Vector+a8 ve capacity+100 resetlenmez. Bu yordam doğrudan pool-clear002d70/002e10/002dc0 çağırmaz; onlar N14 parser error dalında kalır.

003110 başlangıçta owner+38 child'ı okur; null ise+40 THEN+38 sıfırlanır.003140–003177 owner first/last ve node prev/next bağlantılarını tanımlı sırada günceller.00317b node vtable pointer'ını cache'ler; node+50/+48/+10 sıfırlanır, sonra captured table+38 hedefi okunup çağrılır.003190 full64-bit RAX sonucu test edilir; düşük32 veya AL yeterli değildir.

Probe zero ise canlı node+8 document, signed32 document+104 count okunur. Pozitif count search bound olarak, document+a8 table da cache olarak tutulur. Eşleşmede0031c7 count yeniden sign-extend edilir; son slot bulunan slota kopyalanır ve0031d7 DWORD count, swap store sonrasında canlı yeniden okunarak decrement edilir. Eski son slot sıfırlanmaz. Eşleşme yoksa yine destruction'a devam edilir. Bu davranış, sonraki003030 zero-extend ve003240 cached-count yollarıyla tek bir aynı helper'a indirgenmez.

0031de canlı node vtable'ını tekrar okur.0031e3 pool pointer'ını destructor'dan önce cache'ler;0031ea node slot+78(node,flags0). Callback sonrasında0031ed cached pool'un canlı vtable'ını,0031f6 slot+18 hedefini okur/çağırır. RDX=node assembly'de açıkça tanımlı; pseudocode ikinci argümanı atlamıştır.0031f9 owner+38 yeniden okunur; cached next üzerinde devam edilmez. Son owner reset sırası+40,ardından+38'dir. Raw target değerleri mandatory effects üzerinden geçer; orijinal image adresleri callable function pointer'a çevrilmez. Self-bound source adapter'in copy/move işlemleri kapalıdır.

## Doğrulama ve sınırlar

169 çekirdek +17.117 entegrasyon =17.286 yeni kontrol PASS. Unit testler empty/single/multiple node unlink, prev repair, full64 probe, signed zero/negative count, not-found ve swap-remove deadslot davranışını bağımsız sentetik grafikte sınar. Aliased swap-store sonrası live count decrement, link reset öncesi cached vtable ve reset sonrası target load, destructor öncesi cached pool/sonrası live pool vtable, explicit node argümanı ve release sonrası owner reload testlidir. Attached/unattached tracking, live table/count değişimleri, null free, free callback alan değişiklikleri ve bounded eksik-mapping/failure prefix'lerinde rollback uydurulmaması denetlenir.

Entegrasyon compatibility parser'ın verdiği belge için kontrollü tek-root ham node kaydı kullanır. Bu kayıt tam native DOM materialization, native parser allocation veya pool yapısı kanıtı değildir. Yeni gerçek clear/node kaynakları N14 ctor/parser/destructor→N13 dispatch→owned cipher→response queue/send/finish zincirinde çalışır. Eski lower clear/delete stub çağrıları sıfırdır. ECB/CBC,version retry/autodetect,ChallengeResponse/reset/rekey,13 yanıt/150KB payload,64KiB parçalar,deque büyümesi,borrowed/owned/deferred ömür ve event sırası korunur. Direct033d70 hata testleri probe/destroy/release exception'ında output önceden publish edilmişken parser buffer'ın canlı ve root'un detach edilmiş kaldığını doğrular; native EH cleanup varsayılmaz.

Bağımsız ikinci ajan iki üretim gövdesini ve final bağları retained assembly ile karşılaştırdı: bounded static source review PASS. Tam cpp/hpp SHA'ları ve dar kapsam `document-clear-source-review.json` içindedir. Bu, test yürütmesi veya full native equivalence review değildir.003030/003240,node+38/+78 ve pool+20/+18 virtual etkileri,free,0041b0 parser,remaining pool helpers/full DOM/native vtable/allocator/ABI/EH zorunlu açık sağlayıcılardır. Fixture teardown native unwind kanıtı değildir. Orijinal DLL/oyun/network/IOCP çalıştırılmaz; gerçek CFG/token/achievement içerikleri okunmaz. Ayrı girdi bütünlük kontrolleri hash-only'dir.

İki yeni unique adres eşlemesi,iki normal-body claim veiki typed source closure eklenir. Önceki616 C++/header kaynağı ve CRT byte olarak korunur. Ortak build196 nesne(192 C++/4 C),205 normal +19 terminal grup/78 socket;621 kaynak snapshot girdisi ve29 typed source closure vardır.38 eski yerel validation kaydı tarihsel kalır. Bu sayılar DLL tamamlanma yüzdesi değildir. Full DLL henüz tamamlanmadı; toplam yüzde ölçülmedi.

Sıradaki N16:003030 parent/node unlink ve003240 standalone node deletion normal gövdeleri; node/pool virtual/parser/native backend açık kalır. Öncelik1→2→3→5→4; genel CRT aşama5'e kadar bekler.
