# ACRT çift callback bağları

Bu modül, `anadius64.dll` içindeki `[0x18008aa60,0x18008ab60)` aralığının **16 çiftini**, daha önce kurtarılmış gerçek kaynak gövdelerine bağlayan yeni C++ bileşimidir. **Yeni özgün işlev gövdesi sayısı 0**. Özgün DLL çalıştırılmaz, değiştirilmez ve tercihli VA'lar callback olarak çağrılmaz.

`anadius::crt_pair_bindings::Id`, kaynakta tanımlanan küçük opaque token değerlerini kullanır; bunlar fonksiyon pointer'ı veya özgün adres değildir. `Effects`, mevcut initializer Effects sınıfından türeyip bool initializer ve uninitializer dispatch'lerini tamamlar. Void/int initializer ve flush politikaları **pure mandatory** kalır. Bilinmeyen token, NULL token'ın doğrudan çağrılması ve yanlış signature domain'indeki bilinen token açık `invalid_argument` verir.

| Sıra | Initialize token | Uninitialize token |
|---:|---|---|
| 0 | locale_initialize | NULL |
| 1 | encoded_initialize | NULL |
| 2 | thunks_initialize | thunks_uninitialize |
| 3 | success | success |
| 4 | locks_initialize | locks_uninitialize |
| 5 | heap_initialize | heap_uninitialize |
| 6 | NULL | vcrt_uninitialize |
| 7 | ptd_initialize | ptd_uninitialize |
| 8 | lowio_initialize | lowio_uninitialize |
| 9 | command_lines_initialize | success |
| 10 | multibyte_initialize | NULL |
| 11 | NULL | success |
| 12 | NULL | argv_cleanup |
| 13 | NULL | multibyte_uninitialize |
| 14 | success | environment_uninitialize |
| 15 | onexit_initialize | success |

Bütün `006500` konumları aynı `success` token'ını kullanır. Thunk initializer gerçek `067538` gövdesinin **tam uint64** sonucunu iletir; yalnız bool1'e indirgemez. Thunk uninitializer argümanın düşük8 bitini kullanır. VCRT uninitializer, dış argümandan bağımsız gerçek `0644f0` gövdesinin zorladığı0 üzerinden çalışır. Diğer callback'ler gerçek önceki kaynaklara yönlendirilir; başarı stub'ı üretilmez.

`Effects` taşınamayan/kopyalanamayan bir owner içinde sabit boyutlu `std::array<Pair,16>` tutar. `pairs()` bu canlı tabloya mutable span verir. Mevcut generic walker'a bu span verilmelidir; walker her konuma ulaştığında okuduğundan callback'in sonraki girdiyi değiştirmesi görünür. Tablonun boyutu ve depolaması işlem boyunca sabit kalır. `rebuilt_pairs()` ayrı bir başlangıç tablo değeri sağlar.

## Paylaşılan durumlar ve sınırlar

`Dependencies` bütün mevcut state/provider nesnelerine referans tutar. Constructor, runtime/heap/onexit cookie değerlerinin eşitliğini; termination'ın `encoded_callback_a4f10` referansının gerçekten `EncodedGlobals.a4f10` hücresine; CommandLines'ın narrow state referansının gerçekten argv CleanupState içindeki aynı argv nesnesine gittiğini doğrular. Onexit initializer actual termination state içindeki normal ve quick tablolarını kullanır. Diğer provider'ların aynı heap, kilit, PTD, locale/multibyte domain'ini paylaşması çağıranın sorumluluğudur; opaque arabirimlerden bunların hepsi kanıtlanamaz.

Mevcut iki tip `EncodedGlobals.a5178` ile `heap.State.encoded_handler_0a5178` alanlarını bağımsız değer olarak saklar ve doğrudan alias olamaz. Gerçek `initialize_encoded_globals_0644b0` çağrısından hemen sonra **yalnız bu başlatma adımında** değer heap alanına bir kez aktarılır. Her initializer çağrısı aynı gövdeyi ve bu aktarımı yeniden uygular; gizli once guard yoktur. Sonraki new-handler/cookie değişikliklerinin tutarlılığını çağıran sağlar. Bu aktarım yeni bileşimdir; modül canonical native global graph eşitliği iddia etmez.

Bütün borrowed state, provider, callback depolaması ve bu table owner walker/cleanup tamamlanana kadar canlı kalmalıdır. Kullanımlar lifecycle sırasına göre dışarıdan senkronize edilir. Lowio ensure/create/inheritance/standard-handle provider'ları ve kalan void/int/flush politikaları zorunludur. Tam CRT ilklenmesi, native ABI, SEH veya keyfi paralel shutdown eşitliği iddia edilmez.

## Kanıt ve derleme

`collect-evidence.cjs`, SHA-256 `9e420c3aae5ea26410f0bf578f86c47e51e36ff770b9e2a28b5eaee46aed786c` kimliğine bağlı PE dosyasını salt okunur açar. 256 tablo baytını, bütün non-NULL callback'lerin gerçek DIR64 entry sözcüklerini/dosya ofsetlerini ve 16 çiftin opaque token karşılıklarını doğrular. Sonuç, yeniden kullanılan gerçek kaynak hash'leri ve sahip olunan kaynak hash'leri `evidence.json` içindedir. Yeni REA session açılmaz.

```powershell
node outputs/anadius-crt-pair-bindings-source/collect-evidence.cjs
& outputs/anadius-crt-pair-bindings-source/build.ps1
```

`build.ps1`, mevcut LLVM-MinGW ve ortak kısmi kaynak arşiviyle `C++20 -O2 -Wall -Wextra -Werror` üretim nesnesini ve root tarafından sağlanan `bindings_tests.cpp` fixture'ını derler/çalıştırır. Sonuç ile kaynak/arşiv hash'leri `validation.json` içine yazılır. Testler sahip olunan rebuilt state/provider'ları kullanır; özgün DLL çağrılmaz.

Fixture'ın `--dump` çıktısı gerçek derlenmiş `rebuilt_pairs()` dizisinin 16 token çiftini verir. Build sonrası çıkarıcı bu çıktıyı PE'den türetilmiş bütün 16 çiftle karşılaştırır ve `evidence.json.compiled_table` içine PASS, token değerleri ve kendi test EXE'sinin hash'ini kaydeder. İlk çıkarım henüz EXE yokken `not-built` kaydeder; build bunun gerçek karşılaştırmasını tamamlar.

## Birleşik fixture sonucu

`bindings_tests.cpp` **1088 kontrolden** geçti. Gerçek 16-pair başlangıç ve normal ters kapanış walker'ları bu modülün gerçek kaynak callback'lerini çağırır. Dört kontrollü başarısızlık noktası (lock4,heap5,PTD7,lowio8), rollback'te init=NULL slotlarının atlanması, normal reverse'de çağrılması,terminating yolunda walker'ın bypass edilmesi,ileriki tablo girdilerinin mutation'ı,yanlış imza/ID reddi ve0x100/0x101 düşük byte aktarımı sınandı. Ortak argv state'ine yayımlanan kendi komut metni gerçek configure/parser tarafından tüketildi;argv/environment tahsisleri cleanup sırasında bırakıldı.

Bu birleşik testin FLS/lock/heap/locale ve düşük seviye dosya sağlayıcıları kontrollü fixture'lardır; önceki native komponent testleriyle birleşik bir native DLL çalıştırıldığı iddia edilmez. Outer VCRT başlangıcı testte açıkça hazırlanır. Lowio ensure/inherited/stdhandle alt sağlayıcıları boş fixture tablosunu kullanır. Multibyte initializer gerçek source akışında kontrollü malloc başarısızlığını görür ve yine initialized=1 yapar;ilk global/son PTD release nedeniyle fixture'ın statik MB sayacı FFFFFFFF'e döner. Bu gözlem,sağlıklı native MB startup son durumu olarak sunulmaz. Cookie/A5178 projection ve bütün runtime alias'larının tamamlanması,lower native sağlayıcılar,SEH/unwind/export/DllMain ve bütün DLL eşdeğerliği açık kalır.

Bağımsız inceleme `review-pair-bindings.json`:PASS;üç üretim/test kaynak hash'i,ortak fixture hash'leri ve yeniden kullanılan kaynaklar kayıtlıdır. `design-audit.md/.json` desteklenen identity kontrollerini ve projection sınırlarını saklar. `rea-session.json` bu aşamadaki sunucu kimliği doğrulamasıdır; yeni native analiz sorgusu yapılmadan hash'e bağlı PE tablo baytları ve önceki gerçek kanıtlar tekrar kullanılmıştır.
