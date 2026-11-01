# N6 — native request string değişim gövdeleri —24 Ekim2026

`request_mutation.hpp/cpp`, `03ce30` hex decode ve `034730` replace normal gövdelerini typed `ReceiveBufferString` alanları üzerinde ekler. Önceki gerçek010040/008350 string kaynakları yeniden kullanılır; yeni sayılmaz. `034730` reallocation dalı zorunlu02b940 sağlayıcısına gider; bu alt gövde kurtarılmış sayılmaz. N5 compatibility adapter değiştirilmez ve native033d70 akışının bütünü kapatılmış sayılmaz.

Hedef SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA `binary_session`,6.5.0 sunucu/katalog eşleşmesini ve Ghidra12.1.4 kullanılabilirliğini doğrular. Aynı girdiye bağlı retained assembly/pseudocode kullanıldı; orijinal binary açılmadı/yüklenmedi/çalıştırılmadı/değiştirilmedi. Gerçek CFG/token/achievement içeriği okunmadı. CRT üretimi beklemede ve önceki kaynakları korunur.

| Gövde | Adresli native kanıt ve kaynak davranışı |
|---|---|
|03ce30|03ce32..03ce42: size/2 bir kez saklanır,0 çift loop'u atlar.03ce50..03ce60: canlı capacity ve data seçimi; her çiftte high/low byte okunup03cee1'de doğrudan aynı string'e yazılır.0–9/A–F/a–f nibbles, diğer byte0; odd sonbyte atılır. High digit yolunda ASCII DL'den0x30 çıkarılmaz;03ced0 low8 shift sonucu digit<<4 ile aynıdır.03cef1..03cefb:010040(value,half,0) tail JMP; size/NUL, bütün decode yazmalarından sonra değişir.|
|034730|03473d..03476c: old size saklanır; offset>size ise00cec0; erase=min(requested,size-offset).034770..03478c: eşit erase/insert için tek078d30; size yazılmaz.0347a6..0347e0: shrink için önce insert bytes, sonra saved destination+erase'ten tail+1 (NUL dahil); size ancak iki copy bitince saved old-size ile yazılır.|
|034730 büyüme|0347e9..034801: growth=insert-erase ve unsigned capacity-oldsize kıyası. Yer varsa03480b'de new size **copy öncesinde** yayımlanır;034818..03487d cached pointer ve source-overlap split hesabıyla suffix+NUL,first ve remaining olmak üzere3 COPY; zero-byte çağrılar korunur. Yer yoksa034887..03489f:02b940(value,growth,0,offset,clampedErase,source,insert); alt gövdenin return identity'si aynen döner.|

033d70 native caller033df2'de03ce30'u request prefix üzerinde, ardından033e00'da042480 cipher callback'ini çağırır.033fbc'de034730(offset markerposition,erase10,source180080097,insert0) kullanılır. Yeni normal gövdeler bu prefix değişimini çağrı bitmeden görünür hale getirir; ancak N5 std::string semantic adapter'ı hâlâ bunları kendi033d70 orchestration'ına otomatik bağlamaz.

Kanıt kimlikleri:

- 03ce30 pseudo `ev_133ad329397d833d572bf37cfc559a97e5cc1d655e776ff3b426ed2f69869b99`, asm `ev_80f220817eb9f3aaedd370919dd41b31eff96692957601bd8e30baf3951ce863`.
- 034730 pseudo `ev_79c5bb775604e465f8a4a96c8523886a2042e896365181bcb9796c1b687361de`, asm `ev_a8b95492c2d53d260f7dfda3766b34547d4ae2a025fc29e358a10599135608d9`.
- 02b940 lower boundary pseudo `ev_f221fb3cc36fe09dd9caa0f3215baa048e64812b380d8145aa83469378390c26`, asm `ev_30e5199683fa1f5ae0a663c2aaf7a0d7e9b025a364160464f517665e10ac75f2`; context kanıtıdır, yeni gövde değildir.

Tam kesitler `request-mutation-evidence.asm.txt` ve `.pseudo.c.txt` içindedir. API'de native image adresleri çalıştırılmaz; source adresleri explicit bounded memory token'larıdır. Byte COPY etkileri önceki compatibility sözleşmesini kullanır;078d30 native CRT/SIMD gövdesi kurtarılmadı.

**72.025 çekirdek +1.219 entegrasyon =73.244 yeni kontrol PASS.** Nihai ortak build kaydı, local/common validation dosyalarına hash üzerinden bağlanır. Standalone modül script'i bu aşamada ayrıca koşturulmaz. Aynı ajan assembly/source karşılaştırması ve sentetik testler yapmıştır; bağımsız reviewer PASS iddiası yoktur.

Çekirdek:65.536 high/low byte çiftinin bağımsız digit-table oracle'ı, inline ve heap binary input0..69byte, odd byte/NUL/capacity/heap token/tail byte davranışı; bounded read hatasında ilk decode yazmasının kalması ve size'ın henüz küçülmemesi. Replace için external source, clamped erase, equal/shrink/spare-capacity growth, safe self-overlap snapshot oracle, NUL taşınması, out-of-range before-effects,02b940 exact args/return identity/exception prefix, cached destination ve old-size ile canlı provider değişimleri, her copy failure prefix'i.

Raw alias aritmetiği düzeltilmez veya clamp edilmez. Örnek:oldsize16,offset10,erase5,insert6,source=base+6. Native split=suffix-source=9 olur; üçüncü count unsigned(6-9) olarak taşar. Yeni bounded byte provider bu erişimi reddeder; new size önce yayımlanmış ve önceki copy'ler gerçekleşmiş olabilir. Test bu prefix'i korur. Bu bulgu native uygulamadan erişilebilirlik, crash veya vulnerability kanıtı değildir; bozuk/out-of-contract parametre davranışı ve native078d30 fault/overlap davranışı için eşdeğerlik iddia edilmez.

Entegrasyon, controlled caller preparation içinde gerçek03ce30 ve/veya034730'u uygulayıp ardından mevcut N5 semantic adapter üzerinden N1/N2/N3/N4/send/finish kaynak zincirini çalıştırır. Borrowed/owned × decode/erase/both altı kombinasyonda13 mesaj; iki queue map büyümesi, tam-byte XML/NUL FIFO,150000-byte field ve65536-byte chunk oracle doğrulanır. Native prefix handler callback sırasında zaten hazırdır; N5 deferred writeback'e düşülmez. Decode sonrası controlled callback exception'ında ve erase sonrası actual028d30→semantic handler exception'ında prefix değişimi kalır. Deferred owned response da sınanır.

Bu yeni test **native033d70 state/decrypt/parse/response orchestration'ı değildir**. Cipher callback ve retry hazırlığı controlled caller tarafındadır; gerçek042480/native033d70 bağı kurulmuş sayılmaz. Registered handler/cipher/response constructor,023dd0 send constructor, completion ve AsyncSendApi kontrollüdür. SDK WSASend/gerçek network/socket/thread veya yeni027a20/IOCP/027c10 worker çalışmaz. Önceki N5 exception/callback visibility sınırlamaları olduğu gibi kalır.

N6 iki normal gövde ekler;13 önceki native-source closure ve1 ayrı N5 selected compatibility provider korunur. Full native033d70,02b940 allocation/copy/free, native globals/allocator/ABI/EH/SEH/concurrency/lifetime/fault eşdeğerliği ve tam DLL açık kalır. Test sayısı veya kaynak/adres oranından toplam yüzde hesaplanmaz.

Sıradaki N7:02b940 string replace growth/reallocation gövdesi; exact allocation/header/copy/free ve request-mutation lower-provider bağlantısı. Genel CRT üretimi beklemede kalır.
