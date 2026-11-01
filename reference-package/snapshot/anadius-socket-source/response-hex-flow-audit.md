# N8 — 03d190 response hex-encode alan ve çağrı denetimi

24 Ekim 2026. Kaynak: `response_hex.hpp/cpp`. SHA-256 kimliği `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` olan anadius64.dll'nin önceden alınmış statik Ghidra dökümleri kullanıldı. REA6.5.0 sunucu/catalog hizası ve kullanılabilir Ghidra12.1.4 sağlayıcısı `response-hex-rea-session.json` ile doğrulandı. Hedef açılmadı/çalıştırılmadı. Orijinal DLL/EXE/CFG değişmedi; gerçek CFG/token/achievement içerikleri okunmadı.

Bu paket **bir typed-prefix normal gövde** ekler. `messages.cpp::hex_encode_03d190(std::string&)` daha önce aynı adresi semantic olarak eşlemişti: bir yeni unique adres veya yeni source-provider closure değildir. Önceki581 kaynak dosyası ve CRT üretimi değişmedi. Native033d70/N5 gap kapanmadı.

## Adresli dayanak

Assembly Evidence: `ev_7cc933e3f97d44f4cb2932484a7fc51fedeb38e403d3a0c8eedad13c1eaea6b2`.
Pseudocode Evidence: `ev_52ce061acb986f9fe9ccdcaeedc30b80c6a1bf7335b59d2f3c6e82743e888599`.
Hedef bölümler `response-hex-evidence.asm.txt` / `.pseudo.c.txt`; yeniden kurtarılmış sayılmayan on bağlam işlevi ayrı `response-hex-context.*` dosyalarındadır. Döküm ve kaynak hashleri `response-hex-evidence.json` içinde tutulur.

| Native adres | Gözlem | Yeni kaynak davranışı |
|---|---|---|
| 0x18003d19a–03d1a4 | Orijinal size+10 RSI'da tutulur; sıfırsa doğrudan dönüş | Boş input hiçbir alan/bayt/terminator yazmaz; pointer çözmez |
| 0x18003d1af–03d1c9 | Boyut unsigned iki katına çıkar; doubled≤old yolunda size ve NUL yazılır | Ham64bit wrap korunur, yeni overflow terminali icat edilmez |
| 0x18003d1cf–03d204 | cap-old≥old ise doubled size önce; tail'e old kadar sıfır; cached tail sonunda NUL | Mevcut010040 in-capacity resize sözleşmesi yeniden kullanılır; fill exception alan prefix'i korunur |
| 0x18003d220 | Doğrudan CALL008350(value,old,0,old,0) | Gerçek008350 source allocation/COPY/fill/free/pointer sırası |
| 0x18003d225–03d243 | Hedef offset doubled-2, orijinal kalan count; cap/pointer live okunur | Count cached, her iterasyonda veriye prefix üzerinden erişim |
| 0x18003d27a /03d29f | Önce düşük nibble dest+1, sonra yüksek nibble dest | Aynı yazım sırası; 0–9 için+0x30, a–f için+0x57 |
| 0x18003d2a2–03d2aa | Hedef-2, kaynak-1; sıfıra kadar geri döngü | Yerinde büyüyen çıktı okunmamış input'u ezmez |

Native03d190 içindeki doğrudan çağrılar **03d1ff→0793d0** (zero fill) ve **03d220→008350** (growth) şeklindedir. **010040'a native CALL yoktur**: yeni C++ gövde aynı inlined resize sözleşmesini mevcut kaynakla birleştirir.0793d0/078d30 genel CRT gövdeleri yeniden kurtarılmadı; bounded byte fill/COPY test/compatibility sağlayıcılarıdır.008350 altındaki allocator/free/terminal sağlayıcıları zorunlu kalır.

## Doğrulama

**1.876 çekirdek +834 entegrasyon =2.710 yeni kontrol PASS.** Son otorite ortak `source-workspace/build-record.json`; yerel mirror/common kayıtları aynı build hashine bağlıdır. Standalone socket scripti bu aşamada tekrar çalıştırılmış sayılmaz; CMake tanımı eklendi, CMake yürütmesi iddia edilmez.

Çekirdek oracle bağımsız digit lookup kullanır: tüm256 byte, embedded NUL, inline/heap ve spare/growth,0–69 uzunlukları ile128/256/2047/2048/2049/4094/4095/8192 sınırları, tam byte alphabet ve terminatorlar karşılaştırılır. Allocation rounding ve4096 hizalı header/sized-free davranışı gerçek008350 ile sınanır. Boş, eşlenmemiş ve bozuk-cap prefix dahi değişmeden kalır.

Fill sırasında size ve heap pointer değişimi testleri cached count/live pointer ayrımını gösterir. Allocation, COPY, fill-before/after ve old-free failure prefix'leri kontrol edilir; rollback eklenmez. Kısmi mapping düşük nibble yazısından sonra yüksek nibble store'u reddederek native yazım sırasını gözler. Symbolic doubled wrap size0/NUL yazdıktan sonra cached count okumasında bounded hata verir. Geçerli büyük size/cap sınırı008350 length terminaline gider. Bunlar native fault/SEH veya gerçek dev allocation eşdeğerliği değildir.

Entegrasyonda N5 **plain semantic response** üreticisinin kontrollü constructor çevresinde payload NUL çıkarılır; gerçek03d190 çalışır; payload NUL gerçek010040 ile yeniden eklenir. Cipher/registered handler/response constructor kontrollüdür; bu hazırlık **native033d70 response-local orkestrasyonu değildir**. Beklenen sonuç bağımsız XML+hex oracle ile karşılaştırılır; eski semantic encoder oracle olarak kullanılmaz.

Borrowed/owned ve encoder açık/kapalı yollarında13 yanıt, deque0→8→16,150.000bayt alan ve yaklaşık300KiB encoded yanıt tam baytlarıyla sınanır. Gerçek009eb0 COPY/026670 map growth/028270 enqueue/02a500/021fd0/0261d0/025a70 zinciri kullanılır.64KiB parçalar bağımsız sayılır. Response growth orijinal response allocation pool'unu kullanır; queue COPY ayrı sahipliktedir. Ertelenmiş owned task, producer prefix'i bittikten sonra taşınmış response heap'ini yaşatır; task invoke sırasında queue COPY yapılır, kaynak daha sonra bırakılır. İsteğin eski/büyümüş depoları önceden temizlenir. Fixture mapping kaydı catch sırasında da kaldırılır; native exception rollback iddiası yoktur.

Ortak derleme: **189 üretim nesnesi (185 C++/4 C),191 normal +19 terminal grup,64 socket grubu**. Envanter2381 canonical satırdır; anadius2076 adresinden448'inde kaynak tanımı,449'unda compiled symbol correlation bulunur. Aynı adresin yeni varyantı unique kaynak eşlemesi artışı olarak sayılmaz. Test/kaynak oranı DLL tamamlanma yüzdesi değildir.

## Açık sınırlar ve sonraki adım

14 önceki source closure ve1 N5 selected compatibility adapter korunur. Native MSVC ABI/allocator/global/vtable/EH/SEH/concurrency/fault eşdeğerliği kurulmadı.023dd0 completion tüketimi/send API kontrollü; yeni027a20/IOCP/027c10 worker, gerçek socket veya orijinal hedef yürütmesi yoktur. İnceleme aynı ajanla yapıldı; bağımsız final reviewer PASS yoktur.

**N9:**033d70 native typed-prefix normal orkestrasyonunda request mutation/growth ve response codec bağlantılarını kurmak.042480 ve native lower etkiler ayrıca açık takip edilir. CRT üretimi aşama5'e ertelidir. Tam DLL henüz üretilmedi; ölçülmüş toplam yüzde veya güvenilir bitiş tarihi yoktur.
