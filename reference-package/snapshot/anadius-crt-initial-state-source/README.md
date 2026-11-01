# Sahip olunan başlangıç durumu

`ImageState`, çıkarılmış ilk locale grafiğini ve multibyte verilerini bağımsız C++ belleğinde oluşturur. `initial_state.cpp` özgün DLL'den kod çalıştırmaz. Bu modül yeni özgün fonksiyon gövdesi eklemez; mevcut veri modüllerini, native LONG sayaç kayıt defterini ve yeniden yazılmış locale ref fonksiyonlarını birleştirir.

Locale veri modülünün 96 bölgesi ayrı 8-byte hizalı allocation'lara kopyalanır; 116 relocation kaydı bu allocation'ların adreslerine uygulanır. C++ verilerindeki sıfır pointer slotları böylece canlı sahip olunan adreslere dönüşür. Özgün tercih edilen VA'lar canlı pointer yapılmaz. Locale +0x10, time +0x15c ve multibyte +0 sayaçlarında `std::construct_at<LONG>` ile gerçek LONG ömürleri başlatılır. Kontrol edilmiş bu üç adres native Interlocked işlemlerine kaydedilir.

Başlangıç locale refcount **1**, time refcount **0**, multibyte refcount **0**'dır. Category/time kimlikleri kopyalanmış grafiğin adresleridir. Current-locale/current-multibyte/type/case global pointer'ları ve üç başlangıç flag'i sıfır kalır; locale mask **0xfffffffe**'dir. Bunlar imajın başlangıç değerleridir. CRT başlangıç fonksiyonlarının sonradan kuracağı süreç durumu bu modül tarafından çalıştırılmaz.

Nesne kopyalanamaz ve taşınamaz. Dönen pointer ve span'lerin geçerli olması için sahibi hayatta kalmalıdır. Eşzamanlı ref işlemleri sırasında kayıt defteri değişmez; üç gerçek LONG native atomiklerle güncellenir. Yıkım ve byte görünümlerinin okunması için eşzamanlı sayaç işlemleri bitmiş olmalıdır. Keyfi adresleri native atomik işlemlere kabul eden bir arabirim sunulmaz; yalnızca kayıtlı sayaç adresleri geçerlidir.

`./build.ps1` kendi fixture EXE'sini derler ve çalıştırır. 2866 kontrol; 116 edge'in hedeflerini, diğer byte'ların korunmasını, multibyte veri eşitliğini, iki bağımsız sahibi, sınır reddetmelerini ve gerçek locale-ref kaynak kodunun dört worker ile dengeli ref işlemlerini kapsar. Orijinal DLL yürütülmez veya değiştirilmez. Bu doğrulama bütün DLL davranışı ya da özgün ABI eşitliği iddiası değildir.

Bağımsız inceleme sonucu **PASS**; actionable bulgu yok. `review-initial-state.json` tam kaynak hash'lerini kaydeder. İnceleme, orijinal hash'e bağlı dosyadaki 94 raw-backed bölgeyi ve 116 DIR64 entry/pointer slotunu ayrıca karşılaştırdı; iki loader zero-fill bölgesi ayrıldı. Bu incelemede kaynak dosyaları değiştirilmedi; yalnızca kendi fixture build'i çalıştırıldı.

`node verify-rea.cjs` üç yeni Ghidra12.1.4 okumasındaki locale(0x1800a17e0/352bayt), multibyte(0x1800a1c70/552bayt) ve time(0x18008ad20/704bayt) verilerini hash'e bağlı özgün rawPE ile karşılaştırır:1608bayt eşleşti. `rea-comparison.json` adres, dosya offset'i, SHA256 ve EvidenceID'leri içerir. Tam yanıtlar ve kapatılmış sağlayıcı snapshot'ı `../source-workspace/initial-data-rea-evidence.json` ve `initial-data.snapshot.json` içindedir. Bu statik kanıt, başlangıç kodunun çalıştırılması değildir.
