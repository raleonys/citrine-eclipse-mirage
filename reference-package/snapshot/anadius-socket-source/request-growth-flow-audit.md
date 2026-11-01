# N7 — request replace büyüme/yeniden tahsis —24 Ekim2026

`request_growth.hpp/cpp`, **02b940 normal gövdesini** ekler. `RecoveredRequestGrowthApi::grow_replace_02b940`, N6 `RequestMutationApi` alt sağlayıcısını gerçek kaynak gövdesine bağlar.034730 artık bu derived adapter seçildiğinde büyüme dalını kaynakta çalıştırır. Önceki034730/03ce30/010040 gövdeleri yeni sayılmaz. Native033d70 orchestration ve N5 compatibility sınırlaması bu aşamada kapatılmış değildir.

Girdi SHA256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c`. REA `binary_session`,6.5.0 sunucu/katalog eşleşmesini ve Ghidra12.1.4 kullanılabilirliğini doğruladı. Aynı girdiye bağlı retained assembly/pseudocode kullanılır; hedef binary açılmadı/yüklenmedi/çalıştırılmadı/değiştirilmedi. Gerçek CFG/token/achievement içeriği okunmadı. CRT üretimi beklemede ve önceki kaynakları korunur.

02b940 pseudo Evidence `ev_f221fb3cc36fe09dd9caa0f3215baa048e64812b380d8145aa83469378390c26`; assembly `ev_30e5199683fa1f5ae0a663c2aaf7a0d7e9b025a364160464f517665e10ac75f2`. Tam gövde kesitleri `request-growth-evidence.asm.txt` ve `.pseudo.c.txt` içindedir.

| Adres | Korunan davranış |
|---|---|
|02b94b..02b981|Oldsize bir kez saklanır; unsigned INT64_MAX-oldsize<extra ise00ce80. Oldcap ve newsize saklanır; hint etkisizdir.|
|02b985..02b9ab|Capacity=max(newsize OR15,oldcap+oldcap/2); rounded/max ve geometric overflow için maximum capacity fallback.|
|02b9b5..02ba0d|Cap+1 byte;4096 threshold,39 ekbyte,32-byte alignment ve data-8 raw header. LargeNULL05c974, overflow00cd30; küçükNULL metadata sonrasında COPY sınırına kadar kontrolsüzdür.|
|02ba1a/02ba1e|Yeni capacity, ardından size **üç COPY öncesinde** yayımlanır; eski pointer henüz yerindedir.|
|02ba43..02ba6e|Cached oldcap heap dalını seçer; heap oldpointer allocation ve metadata yazmaları SONRASINDA canlı prefix'ten okunur. Prefix(offset), insert(insert_count), suffix(oldsize-offset-erase+1, NUL dahil) COPY sırası korunur.|
|02baa2..02bac8|Inline kaynak prefix byte'ları kullanılır; native heap pointer en sona kadar yazılmadığından orijinal inline kaynak hâlâ mevcuttur. Aynı üç COPY sırası vardır.|
|02ba73..02ba9b|Heap eski tahsis üç COPY'den sonra bırakılır. Cached oldcap+1 sized argument,4096 threshold için+39, unsigned(data-base-8)<=31 header kontrolü. **Assembly02ba9b CALL04b0f8 sized**; decompiler unsized-thunk adı esas alınmaz.|
|02bacd..02bad0|Yeni pointer yalnız ilk8 storage byte'ına en SON yazılır; size/capacity tekrar yazılmaz. Original value prefix referansı döner.|

**5.304 çekirdek +1.144 entegrasyon =6.448 yeni kontrol PASS.** Nihai ortak build sonucu local/common validation dosyalarına hash ile bağlanır. Standalone modül build script'i ayrıca koşturulmuş sayılmaz. İnceleme aynı ajan tarafından adresli karşılaştırma ve sentetik testlerle yapılmıştır; bağımsız reviewer PASS iddiası yoktur.

Çekirdek testler:binary payload/embedded NUL, oldsize0/1/7/15/16/31/63/100/4094/4095/8191, spare0/7/32, baş/orta/son offset ve değişen insert sayılarıyla bağımsız std::string oracle; literal prefix/insert/suffix+NUL count'ları; geometric/rounded capacity,4096 threshold/header ve32-byte alignment; eski inline/heap lifetime, source alias'ı eskiallocation serbest kalmadan kullanma; gerçek034730→final adapter→02b940; clamped erase; metadata-before-COPY, live oldpointer-after-allocation, copy/free exception prefix'leri; free callback pointer/size/capacity değişiminde yalnız final pointer'ın overwrite edilmesi; smallNULL bounded COPY failure/largeNULL invalid ayrımı; length terminal, sembolik maximum fallback allocation ve old-header invalid prefix.

02b940 gerçek valid-state growth arguments varsayar; offset/erase'yi bu gövde yeniden clamp etmez. Bu caller sözleşmesi034730'da sağlanır. INT64_MAX/capacity overflow örnekleri gerçek dev tahsis değildir; provider allocation-prefix'inde durdurulur.00cd30 dalı kaynakta korunur, valid arithmetic için somut erişim iddia edilmez. COPY/SIMD/allocator native instruction equivalence, bozuk pointer faults ve EH/SEH/concurrency ayrıca açık kalır. N6 raw alias arithmetic davranışı değiştirilmez.

Entegrasyonda request büyüme allocator'ı aynı request pool'u kullanır; eski allocation02b940 ile, yeni allocation gerçek028d30 outer cleanup ile bırakılır. Response COPY/deque pool'u ayrıdır. Allocation sonrası yeni byte bölgeleri ve hizalı header view'ları birlikte güncellenir. Bu coherent projection, native allocator/MSVC ABI yerine geçmez.

Borrowed/owned × external-plain/external-hex-prepared/self-source altı kombinasyonda13 request büyütülür. Yeni LSX attribute'u dış kaynaktan veya kendi eski request allocation'ından eklenir; external büyükörnekte150000-byte attribute hizalı allocation yolunu çalıştırır. Prefix çağrı bitmeden semantic handler callback'e görünür. Mevcut N5 message/XML semantic source ve gerçek N1/N2/N3/N4/N6/queue/send/finish zincirinde iki map büyümesi, tam-byte XML/NUL FIFO ve bağımsız65536-byte chunk oracle doğrulanır.150000-byte response field ayrıca sınanır. Deferred response, hem original hem grown request yok olduktan sonra da içeriğini korur. Sonraki handler exception'ında grown prefix kalır; normal-tail cleanup uydurulmaz.

Seed fixture tahsisleri doğrudan owned pool'da kurulur; bunlar API allocation sayacında değildir. Her request için bir API growth allocation ve iki sized free (seeded old + grown new) vardır; canlı request block sayısının0 olduğu ayrıca kontrol edilir. İlk lifetime assertion bu sayaç ayrımına göre düzeltildi; production gövdesi değiştirilmedi.

Bu caller preparation **native033d70/042480 orchestration değildir**. N5 normal-return adapter ve callback/exception gap korunur. Registered handler/cipher/response constructor,023dd0 send constructor, completion ve AsyncSendApi kontrollüdür; SDK WSASend/gerçek network/socket/thread veya yeni027a20/IOCP/027c10 worker çalışmaz. Native allocator/free/terminal/078d30,global/vtable/STL/ABI/EH/SEH/concurrency/full DLL açık.13 önceki normal-source closure'a bir growth closure eklenir; N5 selected compatibility provider ayrı kalır.

Sıradaki N8:03d190 native response hex-encode ve native033d70 typed-prefix akışına giden dönüşüm bağı. Genel CRT üretimi beklemede kalır. Toplam DLL tamamlanma yüzdesi test/adres sayılarından türetilmez.
