# B43 bağımsız kaynak incelemesi

**PASS — sınırlı kaynak bileşimi. Üretim DLL teslimi BLOCKED.**

Altı dondurulmuş kaynak/test hash'i ve güncel immutable kanıtlar doğrulandı. Üretim ve iki test yazarından bağımsızım; EXE/DLL veya test çalıştırmadım. Ortak derleme bu incelemede **PENDING**; quick PASS ortak derleme PASS değildir.

Profile'ın bütün Environment alanları boşsa owned configuration State kurulumundan önce seçilir. Concrete provider type/factory/handler/response/copy/accept tablolarını sahiplenir. Explicit legacy config ve eski override'lar korunur; caller tablolarıyla defaults karıştırılmaz.

Context-free thunks, process-wide weak registry üzerinden nonreused owner/projection capability çözer; raw uintptr→object cast veya current/TLS singleton yoktur. Callback boyunca shared Control pinlenir. Sabit kapasiteli rows exact adres, payload/state/table/schedule/executor doğrular. Registry→Control lock sırası korunur; registry node hazırlama/weak-node retirement gate dışında yapılır.

Copy yeni ref/cap oluşturur, native023cb0 move ref'i transfer eder, destroy bir kez düşürür. Partial move kontrolü source ops/schedule'ın zaten empty olduğu prefix'i ve hâlâ own executor'u doğrular; unspecified destination alanlarını okumaz. Empty sentinel payload okumaz. State1 accept schedule export edilir; mevcut default state0 accept doğrudan invoke kolunu kullanır.

Owned handler slot'u call(1) öncesi tüketilir; self-destroying callback sonrasında eski task/env okunmaz. Response gerçek022e40 ile force-post1 yapar. Bu aynı-worker HOST politikasıdır. Checkpoint'e özel checking seal recursive query'yi durdurur; legitimate task içi nested source copy/destroy korunur. Yakalanıp yutulan lifecycle/probe/handler hataları sticky poison bırakır.

Profile completed pending Application'ı environment bind öncesi tutar; başarısız bind graph'ı geri almaz. Reap, kaynak bindinglerini kaldırır ve slot env'i observer weak drop'tan önce release eder. Close için refs0 yeterli değildir: dış drain attestation ve gerçek Parent/IO/Accept/queue/work sıfırları gerekir. Deques ve Environment lease'leri Runtime drained2 kontrolünden önce bırakılır. Registry unregister, checkpoint target retirement ve lease drop'tan önce yapılır.

Güncel kanıtlar:

- `/root/acceptor_tests`: **200** ana kontrol; parent tarafından doğrulanan dokuz ayrı retained-prefix child **301–309**. Fresh environment.cpp/profile.cpp/core test; altı seçili actual-use hash, reviewed freeze altılısından farklı kaynak kümesi. Integration testi core'a dahil değildir.
- `/root/b22_integration`: **244**, child yok. V2 immutable proof/EXE; beş actual-use ve altı watched hash. ALL Environment config EMPTY; fixture Types/Table/factory/query/schedule/copy veya Policy environment override yoktur. Gerçek default provider ile 59 native packet, 17 plaintext FIFO response, wrapped8→16 büyüme, overlap AES/event ve deferred job/disposal akışı doğrulanır.
- Toplam **444** ana kontrol. Root yalnız author kanıtlarını normalize etti; EXE replacement veya tekrar çalıştırma yoktur. Önceki candidate/V1 kayıtları güncel kanıt değildir.
- B42 ZIP `457fe8a852e79b7e499e61bb815e39c623e8cc3a0874c18175242b1df6b83fdf`: 902 eski kaynağın **900**'ü aynı, iki Profile değişikliği ve dört yeni kaynakla **906**. Kullanılan immutable archive hash'i ZIP üyesiyle eşleştirildi; tam hash JSON'dadır.
- Planlanan ortak kapsam: **265 object / 261 CPP + 4 C / 315 ordinary group + 19 terminal**; composition **99 dosya / 49 object**, compiler observations **86**. Bunlar burada yürütülmüş common PASS sayılmaz.

Native/body/ABI-provider closure delta **0**. **501 mapped / 505 eligible / 139 normal body / 52 historical closure** ve son yeni native-body aşaması **B38** korunur.

Native0228c0/RTTI/vtable/object ABI, native EH veya concurrency kanıtı yoktur. Borrowed config/table pointers provider backing'ini outlive etmemeli; token rejection freed C++ vtable pointer'ına çağrıyı güvenli yapmaz. Raw struct copy retain değildir. Unknown prefix temizlenmez veya yeniden denenmez; dangling stack projection descriptor yeniden okunmaz. SDK/message/job/formatter içindeki bütün keyfi callbackler instrument edilmiş sayılmaz. Dış drain trusted precondition'dır; sayaçlardan türetilmez. Full Resources, socket/CRT worker/job providers ve loader attach/detach açık kalır.

`work/b43-register.py` salt okunur staged preview'sunu ben hazırladım ve strict/AST kontrolünü yaptım; apply/generator/build/probe/test çalıştırmadım. Production/test incelemesi bağımsızdır; pipeline yazarlığı için bağımsızlık iddiası yoktur. Kesin altı hash ve current q/actual record/log/EXE/archive hashleri eşlik eden JSON'dadır.
